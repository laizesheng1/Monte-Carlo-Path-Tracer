#ifndef BVH_H
#define BVH_H

#include "utils.h"
#include "AABB.h"
#include "vector"

class Triangle;
struct hitInfo;

struct BVH_node
{
	BVH_node* left;
	BVH_node* right;
	std::vector<std::shared_ptr<Triangle>> contain_tri;
	AABB box;
	BVH_node() :left(nullptr), right(nullptr) {}
	bool hit(Ray& ray, hitInfo& info);
	bool has_hit(Ray& ray);
};

class BVH
{
public:
	BVH(std::vector<std::shared_ptr<Triangle>>& tris);
	bool hit(Ray& ray, hitInfo& info);
	bool has_hit(Ray& ray);
private:
	BVH_node* build(int l, int r);
	std::vector<std::shared_ptr<Triangle>> triangles;
	BVH_node* root;
	int num=5;
};
#endif