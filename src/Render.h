#ifndef RENDER_H
#define RENDER_H

#include "model.h"
#include <optional>
#include <memory>
#include "AABB.h"
#include "Scene.h"

#define MAX_DEPTH 10
class BVH;

struct hitInfo
{
	double t = 0;	//time
	dvec3 wi = dvec3(0);	//
	dvec3 point = dvec3(0);		//point of intersection 
	dvec3 normal = dvec3(0);
	dvec2 uv = dvec2(0);
	bool front = false;
	std::shared_ptr<Material> mtl = nullptr;
};

class Triangle
{
public:
	dvec3 v[3],vn[3];
	dvec2 uv[3];
	dvec3 A, B;		//Bounding box min_vertex,max_vertex
	std::shared_ptr<Material> mtl;
	bool hit(const Ray& ray, hitInfo& info,double& t_max) const;		//get hit info && is intersection
	AABB get_bbox() const;
	dvec3 center() const;
private:		
	dvec3 interplote_Vertex(double b1, double b2) const;		//顶点插值
	dvec3 interplote_Normal(double b1, double b2) const;
	dvec2 interplote_Texture(double b1, double b2) const;
	
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

class Render
{
public:
	Render(Model& m_model);
	void render(Scene& scene);	
	dmat4x4 pose = dmat4x4(1.0);
	double scale = 1.0;
private:
	Model model;
	Camera camera;
	BVH* bvh;
	vector<Triangle> triangles;
	void tranform_triangle();
	bool isIntersect(Ray& ray, Triangle& tri);
	void setCamera();
	Color3f ray_tracing(Ray& ray,int depth);		//ray tracing
	Ray cast_Ray(int x,int y);		//获取从摄像机到pixel的ray
	Color3f calculateColor(const Material& material, const Ray& ray, const Triangle& triangle);
};

#endif