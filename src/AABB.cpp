#include "AABB.h"
#include "Render.h"

AABB AABB::Union(AABB& box) const {
	return AABB(glm::min(A, box.A), glm::max(B, box.B));
}

AABB AABB::Union(dvec3 point) const {
	return AABB(glm::min(A, point), glm::max(B, point));
}

int AABB::max_axis() {
	int axis = 0;
	auto len = B.x - A.x;
	for (int i = 0; i < 3; i++) {
		auto t = B[i] - A[i];
		if (len < t) {
			len = t;
			axis = i;
		}
	}
	return axis;
}

bool AABB::Intersection(const Ray& ray) const {
	dvec3 v0 = (A - ray.start) / ray.direction;
	dvec3 v1 = (B - ray.start) / ray.direction;
	auto tmin = ray.t1, tmax = ray.t2;
	for (int i = 0; i < 3; i++) {
		if (v0[i] > v1[i]) std::swap(v0[i], v1[i]);
		v1[i] *= 1.001;
		tmin = tmin < v0[i] ? v0[i] : tmin;
		tmax = tmax > v1[i] ? v1[i] : tmax;
	}
	return tmin < tmax;
}


