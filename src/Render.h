#pragma once
#include "model.h"
#include <optional>
#include "AABB.h"

#define MAX_DEPTH 10
class Scene;
class Model;

struct Triangle
{
	dvec3 v[3],vn[3];
	dvec2 uv[3];
	dvec3 A, B;		//Bounding box min_vertex,max_vertex
	Material& tri_mtl;
	Triangle(Material& material)
		: tri_mtl(material){}
};

struct Ray
{
	dvec3 start;
	dvec3 direction;
	double t1=0.0001, t2= std::numeric_limits<double>::max();		//
	Ray(dvec3 s,dvec3 d):start(s),direction(d){}
};

struct Camera
{
	float fov;
	vec3 eye;
	vec3 lookat;
	vec3 up;
	int w, h;
};

class Render
{
public:
	Render(Model& m_model);
	void render(Scene& scene);
	vector<Triangle> triangles;
	dmat4x4 pose = dmat4x4(1.0);
	double scale = 1.0;
private:
	Model model;
	Camera camera;
	void tranform_triangle();
	bool isIntersect(Ray& ray, Triangle& tri);
	void setCamera();
	Color3f& ray_tracing(Ray& ray,int depth);		//ray tracing
	Ray& cast_Ray(int x,int y);		//ªÒ»°ray
	Color3f calculateColor(const Material& material, const Ray& ray, const Triangle& triangle);
};