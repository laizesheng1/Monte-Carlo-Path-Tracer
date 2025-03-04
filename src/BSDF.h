#pragma once
#ifndef BSDF_H
#define BSDF_H
#include "utils.h"

struct hitInfo;
class Triangle;

class coordiantetransform
{
public:
	vec3 u, v, w;

	coordiantetransform(const vec3& n):w(n) {       //以n为z轴建立的局部坐标系
		vec3 a = (fabs(w.x) > 0.9f) ? vec3(0, 1, 0) : vec3(1, 0, 0);
		v = glm::normalize(cross(w, a));
		u = glm::cross(w, v);
	}

	vec3 localToworld(const vec3& a) const {
		return a.x * u + a.y * v + a.z * w;
	}

	vec3 worldTolocal(const vec3& t) const {
		return { glm::dot(t,u),glm::dot(t,v),glm::dot(t,w) };
	}
};

struct Scatterinfo
{
	dvec3 wo;		//反射之后的方向
	dvec3 f;	    //颜色
	double pdf;		//
    Scatterinfo(dvec3 d, dvec3 c, double p) :wo(d), f(c), pdf(p) {}
    Scatterinfo() {}
};

class Scatter
{
public:
    Scatter(const Color3f& r) : reflect(r) {}
    virtual Scatterinfo Sample() const = 0;
    virtual float Pdf(const vec3& wi) const = 0;
public:
    Color3f reflect;
    float weight;
};

class Diffuse : public Scatter
{
public:
    Diffuse(const Color3f& r, const vec3& wo) : Scatter(r), m_wo(wo) {}
    virtual Scatterinfo Sample() const override;
    virtual float Pdf(const vec3& wi) const override;
private:
    vec3 m_wo;      //local wi
};

class Specular : public Scatter
{
public:
    Specular(const Color3f& r, float co, const vec3& wo)
        : Scatter(r), coefficient(co), m_wo(wo)
    {
    }
    virtual Scatterinfo Sample() const override;
    virtual float Pdf(const vec3& wi) const override;
private:
    float coefficient;
    vec3 m_wo;
};

class BSDF
{
public:

};
#endif