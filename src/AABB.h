#ifndef AABB_H
#define AABB_H

#include "utils.h"
#include <memory>
#include <vector>
#include <string>

class Ray;

class AABB
{
public:
	AABB() :A(dvec3(std::numeric_limits<double>::max())), B(dvec3(std::numeric_limits<double>::lowest())) {}
	AABB(dvec3 _min, dvec3 _max) :A(_min), B(_max) {}

    AABB Union(AABB& box) const;
    AABB Union(dvec3 point) const;
	int max_axis();
    bool Intersection(const Ray& ray) const;
public:
    dvec3 A,B;
};


#endif