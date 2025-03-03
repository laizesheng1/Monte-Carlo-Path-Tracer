#include "BVH.h"
#include "stack"
#include "Render.h"

BVH::BVH(std::vector<Triangle> tris)
{
	for (auto& tri:tris)
	{
		triangles.push_back(make_shared<Triangle>(tri));
	}
	root=build(0, tris.size());
}

BVH_node* BVH::build(int l,int r)
{
	BVH_node* node = new BVH_node();
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

	//split triangles
	int split_axis = lrBox.max_axis();
	std::sort(triangles.begin() + l, triangles.begin() + r, [split_axis](const std::shared_ptr<Triangle>& a, const std::shared_ptr<Triangle>& b) {
		AABB box_a = a->get_bbox();
		AABB box_b = b->get_bbox();
		double center_a = (box_a.A[split_axis] + box_a.B[split_axis]) / 2.0;
		double center_b = (box_b.A[split_axis] + box_b.B[split_axis]) / 2.0;
		return center_a < center_b;
		});
	int mid = (l + r) / 2;
	node->left = build(l, mid);
	node->right = build(mid, r);
	node->box = lrBox;
	return node;
}

bool BVH::hit(Ray& ray, hitInfo& info)
{
	double pre_t = std::numeric_limits<double>::max();
	bool ishit = false;
	std::stack<BVH_node*> st;
	st.push(root);
	while(!st.empty())
	{
		BVH_node* node = st.top();
		st.pop();
		if (!node->box.Intersection(ray))
		{
			continue;
		}
		if (!node->left && !node->right)
		{
			for (std::shared_ptr tri : node->contain_tri)
			{
				if(tri->hit(ray, info, pre_t))
				{
					ishit = true;
				}
			}
		}
		else {
			if(node->left)
				st.push(node->left);
			if(node->right)
				st.push(node->right);
		}
	}
	return ishit;
}