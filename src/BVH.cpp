#include "BVH.h"
#include "stack"
#include "Render.h"
#include <thread>

BVH::BVH(std::vector<std::shared_ptr<Triangle>>& tris)
{
	for (auto& tri:tris)
	{
		triangles.push_back(tri);
	}
	root=build(0, tris.size());
}

BVH_node* BVH::build(int l,int r)
{
	BVH_node* node = new BVH_node();
	//if (l == r)
	//	return nullptr;
	AABB lrBox;
	for (int i = l; i < r; i++)
	{
		lrBox = lrBox.Union(triangles[i]->get_bbox());
	}
	if (r - l <= num)
	{
		for (int i = l; i < r; i++)
		{
			node->contain_tri.push_back(triangles[i]);
		}
		node->box = lrBox;
		return node;
	}
	AABB mid_box;
	for (int i = l; i < r; ++i) {
		mid_box = mid_box.Union(triangles[i]->center());
	}
	int axis = mid_box.max_axis();
	float mid_val = (mid_box.A[axis] + mid_box.B[axis]) / 2.0f;
	auto mid = std::partition(triangles.begin() + l, triangles.begin() + r,			//按值划分、不排序、要均匀
		[axis, mid_val](const std::shared_ptr<Triangle>& p)->bool
		{
			return p->center()[axis] < mid_val;
		});
	int mid_idx = static_cast<int>(mid - triangles.begin());

	if (mid_idx == l || mid_idx == r) {
		mid_idx = (l + r) / 2;
	}
	node->left = build(l, mid_idx);
	node->right = build(mid_idx, r);
	node->box = lrBox;
	return node;
}

//bool BVH::hit(Ray& ray, hitInfo& info)		//非递归慢0.5s
//{
//	double pre_t = std::numeric_limits<double>::max();
//	bool ishit = false;
//	std::stack<BVH_node*> st;
//	st.push(root);
//	while(!st.empty())
//	{
//		BVH_node* node = st.top();
//		st.pop();
//		if (!node->box.Intersection(ray))
//		{
//			continue;
//		}
//		if (!node->left && !node->right)
//		{
//			for (std::shared_ptr tri : node->contain_tri)
//			{
//				if(tri->hit(ray, info, pre_t))
//				{
//					ishit = true;
//				}
//			}
//		}
//		else {
//			if(node->left)
//				st.push(node->left);
//			if(node->right)
//				st.push(node->right);
//		}
//	}
//	return ishit;
//}

bool BVH::hit(Ray& ray, hitInfo& info)
{
	return root->hit(ray,info);
}

bool BVH_node::hit(Ray& ray, hitInfo& info)
{
	bool is_hit = false;
	if (!this->box.Intersection(ray))
		return false;
	if (left)
		is_hit |= left->hit(ray, info);
	if (right)
		is_hit |= right->hit(ray, info);
	for (const auto& tri : this->contain_tri)
	{
		if (tri->hit(ray, info))
		{
			ray.t2 = info.t;
			is_hit = true;
		}
	}
	return is_hit;
}

bool BVH::has_hit(Ray& ray)
{
	return root->has_hit(ray);
}

bool BVH_node::has_hit(Ray& ray)		//don't update info,
{
	if (!this->box.Intersection(ray))
		return false;
	if (left && left->has_hit(ray))
		return true;
	if (right&& right->has_hit(ray))
		return true;
	for (const auto& tri : this->contain_tri)
	{
		if (tri->isIntersect(ray))
		{
			return true;
		}
	}
	return false;
}