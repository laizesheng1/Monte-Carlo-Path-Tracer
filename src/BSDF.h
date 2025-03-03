#pragma once
#ifndef BSDF_H
#define BSDF_H
#include "utils.h"

struct hitInfo;
class Triangle;

struct newRayinfo
{
	dvec3 dir;		//反射之后的方向
	dvec3 ratio;	//每次反射的衰减系数
	double pdf;		//
	newRayinfo(dvec3 d, dvec3 r, double p) :dir(d), ratio(r), pdf(p) {}
};

class BSDF
{
public:

};
#endif