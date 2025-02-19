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
                load_material(parentPath.string() + "/" + res[1].str());      //get .mtl path
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