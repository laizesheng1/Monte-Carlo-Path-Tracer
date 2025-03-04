#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "omp.h"
#include <regex>
#include <glfw/include/GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "xml/pugixml.hpp"
#include "utils.h"

using namespace std;

class Triangle;

class Texture
{
public:
    Texture(string filename);
    Texture(Color3f c);
    vector<Color3f> image_color;
    Color3f get_color(const dvec2& uv);
private:
    int image_w, image_h;
};

struct Material
{
    //dvec3 Kd = dvec3(1, 1, 1);    // diffuse, 反射光线系数，0 表示吸收所有光线
    dvec3 Ks = dvec3(0, 0, 0);    // specular, 高光反射系数
    dvec3 Tr = dvec3(0, 0, 0);    // 折射透明度, 0 表示不透明
    double Ns = 1;                // 高光反射参数
    double Ni = 1;                // 物质折射率
    std::shared_ptr<Texture> Map_Kd;                      // 纹理贴图
    dvec3 radiance=dvec3(0);        //light radiance
};

class CameraInfo
{
public:
    dvec3 eye, lookat, up;
    double fovy;
    int height, width;
    map<string, dvec3> lightinfo;
    void print() const {
        std::cout << "Camera Parameters:\n";
        std::cout << "  Eye: (" << eye.x << ", " << eye.y << ", " << eye.z << ")\n";
        std::cout << "  Lookat: (" << lookat.x << ", " << lookat.y << ", " << lookat.z << ")\n";
        std::cout << "  Up: (" << up.x << ", " << up.y << ", " << up.z << ")\n";
        std::cout << "  Fovy: " << fovy << "\n";
        std::cout << "  Width: " << width << "\n";
        std::cout << "  Height: " << height << "\n";
    }
};

class Model
{
public:
	vector<dvec3> vertex;
	vector<dvec3> normal;
	vector<dvec2> texture;
    vector<imat3x4> face;   //{v_idx,vn_idx,vt_idx,material_idx}
    vector<Material> materials;
    CameraInfo camerainfo;
    vector<Texture> textures;
    Model(string filename);
private:
    //<material ,idx>
    map<string, int> material_map;
    void load_material(string filename);
    void loadCameraFromXML(const std::string& filename);
    void load_Model(string filename);       //use tinyobjloader
};
#endif