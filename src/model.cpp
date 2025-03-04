#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "model.h"
#include "Render.h"

Texture::Texture(string filename)
{
    int w, h, channel;
    float* data = stbi_loadf(filename.c_str(), &w, &h, &channel, 0);
    image_w = w;
    image_h = h;
    Color3f c(0);
    for (int i = 0; i < w * h; ++i) {
        int t = i * 3;
        c.x = data[t];
        c.y = data[t + 1];
        c.z = data[t + 2];
        image_color.push_back(c);
    }
    stbi_image_free(data);
}

Color3f Texture::get_color(const dvec2& uv)
{
    double u = clamp01(glm::fract(uv.x));        //保证纹理坐标在0-1之间
    double v = clamp01(glm::fract(uv.y));
    int x = static_cast<int>(u * image_w);      //获取纹理坐标在图像对应的像素点坐标
    int y = static_cast<int>(v * image_h);
    return image_color.at(y * image_w + x);
}


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
                string xmlname = mtlname.replace(mtlname.size() - 3, 3, "xml");
                loadCameraFromXML(parentPath.string() + "/" + xmlname);                
                load_material(parentPath.string() + "/" + res[1].str());      //get .mtl path                
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

//void Model::load_Model(string filename) {
//    // Load OBJ file using tinyobjloader
//    tinyobj::attrib_t attrib;
//    vector<tinyobj::shape_t> shapes;
//    vector<tinyobj::material_t> materials;
//
//    string err;
//    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str());
//
//    if (!ret) {
//        cerr << "Error loading OBJ file: " << err << endl;
//        return;
//    }
//
//    // 材料映射
//    map<string, Material> material_map;
//    vector<Triangle> triangles;
//
//    // 处理材质
//    for (const auto& mtl : materials) {
//        // 将材质信息映射到 Material 类实例
//        Material mtl_instance;
//        // 填充 mtl_instance, 例如:
//        // mtl_instance.color = glm::dvec3(mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2]);
//        material_map[mtl.name] = mtl_instance;
//    }
//
//    // 遍历所有形状和面
//    for (const auto& shape : shapes) {
//        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
//            size_t num_face_vertices = size_t(shape.mesh.num_face_vertices[f]);
//
//            // 每个面是一个三角形
//            if (num_face_vertices != 3) continue;
//
//            Triangle tri;  // 假设第一个材质
//
//            // 获取三角形的顶点索引、法线索引、纹理坐标索引
//            for (size_t v = 0; v < 3; ++v) {
//                int idx = shape.mesh.indices[f * 3 + v];
//
//                // 顶点坐标
//                tri.v[v] = dvec3(attrib.vertices[3 * idx + 0],
//                    attrib.vertices[3 * idx + 1],
//                    attrib.vertices[3 * idx + 2]);
//
//                // 法线
//                if (!attrib.normals.empty()) {
//                    int vn_idx = shape.mesh.indices[f * 3 + v].normal_index;
//                    tri.vn[v] = dvec3(attrib.normals[3 * vn_idx + 0],
//                        attrib.normals[3 * vn_idx + 1],
//                        attrib.normals[3 * vn_idx + 2]);
//                }
//
//                // 纹理坐标
//                if (!attrib.texcoords.empty()) {
//                    int uv_idx = shape.mesh.indices[f * 3 + v].texcoord_index;
//                    tri.uv[v] = dvec2(attrib.texcoords[2 * uv_idx + 0],
//                        attrib.texcoords[2 * uv_idx + 1]);
//                }
//            }
//
//            // 计算包围盒
//            tri.A = tri.B = tri.v[0];
//            for (int i = 1; i < 3; ++i) {
//                tri.A = glm::min(tri.A, tri.v[i]);
//                tri.B = glm::max(tri.B, tri.v[i]);
//            }
//
//            // 将三角形加入到三角形列表
//            triangles.push_back(tri);
//        }
//    }
//    cout << "Loaded " << triangles.size() << " triangles from " << filename << endl;
//}

void Model::load_material(string filename)
{
    std::filesystem::path pathObj(filename);
    std::filesystem::path parentPath = pathObj.parent_path();
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
            if (camerainfo.lightinfo.count(std::string(result[1])))
                materials.back().radiance = camerainfo.lightinfo[std::string(result[1])];
            continue;
        }
        
        // update material details
        if (regex_search(line, result, std::regex("\\s*Kd\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)")))
            materials.back().Kd = { stod(result[1]), stod(result[2]), stod(result[3]) };
        else if (regex_search(line, result, std::regex("\\s*Ks\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)")))
            materials.back().Ks = { stod(result[1]), stod(result[2]), stod(result[3]) };
        else if (regex_search(line, result, std::regex("\\s*Tr\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)")))
            materials.back().Tr = { stod(result[1]), stod(result[2]), stod(result[3]) };
        else if (regex_search(line, result, std::regex("\\s*Ns\\s+(\\S+)")))
            materials.back().Ns = stod(result[1]);
        else if (regex_search(line, result, std::regex("\\s*Ni\\s+(\\S+)")))
            materials.back().Ni = stod(result[1]);
        else if (regex_search(line, result, std::regex("map_Kd\\s+(\\S+)")))
        {
            textures.push_back(Texture(parentPath.string() + "/" + std::string(result[1])));
            materials.back().Map_Kd = std::make_shared<Texture>(textures.back());
        }
        else
            continue;
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
    //camerainfo.width = 500;
    //camerainfo.height = 500;
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
    for (pugi::xml_node lightNode : doc.children("light")) {
        string light_mtl = lightNode.attribute("mtlname").as_string();

        std::string radianceStr = lightNode.attribute("radiance").as_string();
        size_t comma1 = radianceStr.find(',');
        size_t comma2 = radianceStr.find(',', comma1 + 1);

        if (comma1 == std::string::npos || comma2 == std::string::npos) {
            std::cerr << "Error: Invalid radiance format in <light> node." << std::endl;
            continue;
        }
        dvec3 radiance;
        radiance.x = std::stod(radianceStr.substr(0, comma1));
        radiance.y = std::stod(radianceStr.substr(comma1 + 1, comma2 - comma1 - 1));
        radiance.z = std::stod(radianceStr.substr(comma2 + 1));
        camerainfo.lightinfo[light_mtl] = radiance;
    }
    /*camerainfo.print();
    lightinfo.print();*/
}