#ifndef TRIANGLE_H
#define TRIANGLE_H
#include "utils.h"
#include "model.h"
#include "AABB.h"
struct hitInfo;

class Triangle
{
public:
	dvec3 v[3], vn[3];
	dvec2 uv[3];
	dvec3 A, B;		//Bounding box min_vertex,max_vertex
	std::shared_ptr<Material> mtl;

	//bool hit(const Ray& ray, hitInfo& info, double& t_max) const;		//´«µÝt_max£¬Âý
	bool hit(const Ray& ray, hitInfo& info) const;		//get hit info && is intersection
	AABB get_bbox() const;
	Point2f sample_Point2();
	dvec3 center() const;
	float area() const;
	dvec3 interplote_Vertex(double b1, double b2) const;		//¶¥µã²åÖµ
	dvec3 interplote_Normal(double b1, double b2) const;
	dvec2 interplote_Texture(double b1, double b2) const;
	bool isIntersect(Ray& ray);
};
#endif
