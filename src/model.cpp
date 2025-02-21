#include "model.h"

Model::Model(string filename)
{
    cout << "[Model] " << filename << endl;

    ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open OBJ file: " << filename << std::endl;
        return;
    }

    std::filesystem::path pathObj(filename);
    std::filesystem::path parentPath = pathObj.parent_path();
    
    string line;
    vector<string> v_;
    vector<string> vn_;
    vector<string> vt_;
    vector<string> f_;
    while (getline(file, line))
    {
        if (line[0] == 'm') // mtllib
        {
            std::smatch res;
            if (regex_search(line, res, std::regex("mtllib\\s+(\\S+)")))
            {
                string mtlname = res[1].str();
                load_material(parentPath.string() + "/" + mtlname);      //get .mtl path
                string xmlname = mtlname.replace(mtlname.size() - 3, 3, "xml");
                loadCameraFromXML(parentPath.string() + "/" + xmlname);
            }
        }
        else if (line[0] == 'v')
        {
            if (line[1] == ' ') // v
                v_.push_back(line);
            else if (line[1] == 'n') // vn
                vn_.push_back(line);
            else if (line[1] == 't') // vt
                vt_.push_back(line);
        }
        else if (line[0] == 'f') // f
        {
            f_.push_back(line);
        }
        else if (line[0] == 'u') // usemtl
        {
            f_.push_back(line);
        }
    }

#pragma omp sections
    {
#pragma omp section
        {
            for (auto line : v_)
            {
                stringstream ss(line.substr(2));
                glm::dvec3 v;
                ss >> v.x >> v.y >> v.z;
                vertex.push_back(v);
            }
        }
#pragma omp section
        {
            for (auto line : vn_)
            {
                stringstream ss(line.substr(3));
                glm::dvec3 n;
                ss >> n.x >> n.y >> n.z;
                normal.push_back(n);
            }
        }
#pragma omp section
        {
            for (auto line : vt_)
            {
                stringstream ss(line.substr(3));
                glm::dvec2 t;
                ss >> t.x >> t.y;
                texture.push_back(t);
            }
        }
#pragma omp section
        {
            int curr_material_id = 0;
            for (auto line : f_)
            {
                if (line[0] == 'u')
                {
                    std::smatch res;
                    if (regex_search(line, res, std::regex("usemtl\\s+(\\S+)")))
                        curr_material_id = material_map[std::string(res[1])];
                }
                else
                {
                    char trash;
                    glm::imat3x4 face_;
                    std::istringstream ss(line);
                    ss >> trash;
                    for (int i = 0; i < 3; ++i)
                    {
                        ss >> face_[i][0] >> trash >> face_[i][1] >> trash >> face_[i][2];
                        face_[i][0]--;
                        face_[i][1]--;
                        face_[i][2]--;
                        face_[i][3] = curr_material_id;
                    }
                    face.push_back(face_);
                }
            }
        }
    }
}

void Model::load_material(string filename)
{
    std::ifstream fs;
    fs.open(filename);
    if (!fs.is_open())
    {
        std::cerr << "Error: Cannot open mtl file: " << filename << std::endl;
        return;
    }
    std::string line;
    std::smatch result;
    // read .mtl file
    while (getline(fs, line))
    {
        if (regex_search(line, std::regex("\\s*#")) || regex_match(line, std::regex("\\s*")))
            continue;
        // 'newmtl' find, add a default material to array
        if (regex_search(line, result, std::regex("newmtl\\s+(\\S+)")) && result.size() == 2)
        {
            material_map[std::string(result[1])] = material_map.size();
            materials.push_back(Material());
            continue;
        }
        // update material details
        if (regex_search(line, result, std::regex("\\s*Kd\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)")))
            materials.back().Kd = { stof(result[1]), stof(result[2]), stof(result[3]) };
        else if (regex_search(line, result, std::regex("\\s*Ks\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)")))
            materials.back().Ks = { stof(result[1]), stof(result[2]), stof(result[3]) };
        else if (regex_search(line, result, std::regex("\\s*Kr\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)")))
            materials.back().Tr = { stof(result[1]), stof(result[2]), stof(result[3]) };
        else if (regex_search(line, result, std::regex("\\s*Ns\\s+(\\S+)")))
            materials.back().Ns = stof(result[1]);
        else if (regex_search(line, result, std::regex("\\s*Ni\\s+(\\S+)")))
            materials.back().Ni = stof(result[1]);
        else
            continue;
    }
    for (Material& i : materials)
    {
        double kd = glm::max(i.Kd.x, glm::max( i.Kd.y, i.Kd.z));
        double ks = glm::max(i.Ks.x, glm::max(i.Ks.y, i.Ks.z));
        double tr = glm::max(i.Tr.x, glm::max( i.Tr.y, i.Tr.z));
        double _sum = kd + ks + tr; // kd + ks + Tr <= 1;
        if (_sum > 1)
        {
            kd /= _sum;
            ks /= _sum;
            tr /= _sum;
            i.Kd /= _sum;
            i.Ks /= _sum;
            i.Tr /= _sum;
        }
        // 调整加权系数，使得 Kd * kd + Ks * ks + Tr * Tr = (1,1,1)
        if (kd > 0)
            i.Kd /= kd;
        if (ks > 0)
            i.Ks /= ks;
        if (tr > 0)
            i.Tr /= tr;
        i.kd = kd;
        i.ks = ks;
        i.tr = tr;
    }
}

void Model::loadCameraFromXML(const std::string& filename) {
    pugi::xml_document doc;
    if (!doc.load_file(filename.c_str())) {
        std::cerr << "Error: Failed to load XML file: " << filename << std::endl;
        return;
    }

    // 获取 camera 节点
    pugi::xml_node cameraNode = doc.child("camera");
    if (!cameraNode) {
        std::cerr << "Error: No <camera> node found in XML file." << std::endl;
    }

    // 提取相机参数
    camerainfo.width = cameraNode.attribute("width").as_int();
    camerainfo.height = cameraNode.attribute("height").as_int();
    camerainfo.fovy = cameraNode.attribute("fovy").as_double();

    // 提取 eye 节点
    pugi::xml_node eyeNode = cameraNode.child("eye");
    if (eyeNode) {
        camerainfo.eye.x = eyeNode.attribute("x").as_double();
        camerainfo.eye.y = eyeNode.attribute("y").as_double();
        camerainfo.eye.z = eyeNode.attribute("z").as_double();
    }
    else {
        std::cerr << "Error: No <eye> node found in <camera>." << std::endl;
    }

    // 提取 lookat 节点
    pugi::xml_node lookatNode = cameraNode.child("lookat");
    if (lookatNode) {
        camerainfo.lookat.x = lookatNode.attribute("x").as_double();
        camerainfo.lookat.y = lookatNode.attribute("y").as_double();
        camerainfo.lookat.z = lookatNode.attribute("z").as_double();
    }
    else {
        std::cerr << "Error: No <lookat> node found in <camera>." << std::endl;
    }

    // 提取 up 节点
    pugi::xml_node upNode = cameraNode.child("up");
    if (upNode) {
        camerainfo.up.x = upNode.attribute("x").as_double();
        camerainfo.up.y = upNode.attribute("y").as_double();
        camerainfo.up.z = upNode.attribute("z").as_double();
    }
    else {
        std::cerr << "Error: No <up> node found in <camera>." << std::endl;
    }
    //get light info
    pugi::xml_node lightNode = doc.child("light");
    if (!lightNode) {
        std::cerr << "Error: No <light> node found in XML file." << std::endl;
    }
    lightinfo.light_mtl = lightNode.attribute("mtlname").as_string();
    std::string radianceStr = lightNode.attribute("radiance").as_string();
    size_t comma1 = radianceStr.find(',');
    size_t comma2 = radianceStr.find(',', comma1 + 1);

    if (comma1 == std::string::npos || comma2 == std::string::npos) {
        std::cerr << "Error: Invalid radiance format in <light> node." << std::endl;
    }

    lightinfo.radiance.x = std::stod(radianceStr.substr(0, comma1));
    lightinfo.radiance.y = std::stod(radianceStr.substr(comma1 + 1, comma2 - comma1 - 1));
    lightinfo.radiance.z = std::stod(radianceStr.substr(comma2 + 1));
    /*camerainfo.print();
    lightinfo.print();*/
}