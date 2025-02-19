#include <iostream>
#include <vector>
#include "GLFW/glfw3.h"
#include "glm.hpp"

using namespace std;
using namespace glm;

struct Material
{
    dvec3 Kd = dvec3(1, 1, 1);    // diffuse, 反射光线系数，0 表示吸收所有光线
    dvec3 Ks = dvec3(0, 0, 0);    // specular, 高光反射系数
    dvec3 Kr = dvec3(0, 0, 0);    // 折射透明度, 0 表示不透明
    dvec3 Le = dvec3(0, 0, 0);    // 自发光
    double kd = 1;                // 漫反射颜色
    double ks = 0;                // 镜面反射颜色
    double Tr = 0;                // reflect fraction, Monte Carlo
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



    
};
