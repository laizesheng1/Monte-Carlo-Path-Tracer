#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "omp.h"
#include <regex>
#include "GLFW/glfw3.h"
#include "glm.hpp"

using namespace std;
using namespace glm;

struct Material
{
    dvec3 Kd = dvec3(1, 1, 1);    // diffuse, 反射光线系数，0 表示吸收所有光线
    dvec3 Ks = dvec3(0, 0, 0);    // specular, 高光反射系数
    dvec3 Tr = dvec3(0, 0, 0);    // 折射透明度, 0 表示不透明
    double kd = 1;                // 漫反射颜色
    double ks = 0;                // 镜面反射颜色
    double tr = 0;                // reflect fraction, Monte Carlo
    double Ns = 1;                // 高光反射参数
    double Ni = 1;                // 物质折射率
    //Map_Kd;                      // 纹理贴图
};

class Model
{
public:
	vector<dvec3> vertex;
	vector<dvec3> normal;
	vector<dvec2> texture;
    vector<imat3x4> face;   //{v_idx,vt_idx,vn_idx,material_idx}
    vector<Material> materials;
    Model(string filename);
private:
    //<material ,idx>
    map<string, int> material_map;
    void load_material(string filename);
};
