#ifndef RENDER_H
#define RENDER_H

#include "model.h"
#include <optional>
#include <memory>
#include "AABB.h"
#include "Scene.h"
#include "Triangle.h"

#define MAX_DEPTH 10
class BVH;

struct hitInfo
{
	double t = 0;	//time
	dvec3 wi = dvec3(0);		//入射方向
	dvec3 point = dvec3(0);		//point of intersection 
	dvec3 normal = dvec3(0);
	dvec2 uv = dvec2(0);
	bool front = false;		//用于计算反射时是否打到光源
	std::shared_ptr<Material> mtl = nullptr;
	float lightarea=0.f;		//计算pdf
};

struct Ray
{
	dvec3 start;
	dvec3 direction;
	double t1=0.0001, t2= std::numeric_limits<double>::max();		//t1~~0,t2=transmission time
	Ray(dvec3 s,dvec3 d):start(s),direction(d){}
};

struct Camera
{
	double fov;
	dvec3 eye;
	dvec3 lookat;
	dvec3 up;
	int w, h;
};

struct lightinfo
{
	vec3 wo;
	vec3 f;
	float pdf;
	Ray ray;
};

class Render
{
public:
	Render(Model& m_model);
	void render(Scene& scene);	
private:
	Model model;
	Camera camera;
	BVH* bvh;
	std::vector<std::shared_ptr<Triangle>> triangles;
	std::vector<std::shared_ptr<Triangle>> lights;
	void tranform_triangle();
	void setCamera();
	Color3f ray_tracing(Ray& ray,int depth);		//ray tracing
	Ray cast_Ray(int x,int y);		//获取从摄像机到pixel的ray
	Color3f ray_tracing(Ray& ray);
	Color3f sample_light(hitInfo& info);
	lightinfo sample(hitInfo& info);
};

#endif