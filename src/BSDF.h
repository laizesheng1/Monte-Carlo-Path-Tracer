#pragma once
#ifndef BSDF_H
#define BSDF_H
#include "utils.h"

struct hitInfo;
class Triangle;

struct newRayinfo
{
	dvec3 dir;		//����֮��ķ���
	dvec3 ratio;	//ÿ�η����˥��ϵ��
	double pdf;		//
	newRayinfo(dvec3 d, dvec3 r, double p) :dir(d), ratio(r), pdf(p) {}
};

class BSDF
{
public:

};
#endif