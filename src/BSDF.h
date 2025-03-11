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
    coordiantetransform() {}
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
	vec3 wo;		//反射之后的方向
	vec3 f;	    //颜色
	float pdf;		//
    Scatterinfo(vec3 d, vec3 c, double p) :wo(d), f(c), pdf(p) {}
    Scatterinfo() {}
};

class Scatter
{
public:
    Scatter(const Color3f& r) : reflect(r) {}
    virtual vec3 Fx(const vec3& wi) const = 0;
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
    virtual vec3 Fx(const vec3& wi) const override;
    virtual Scatterinfo Sample() const override;
    virtual float Pdf(const vec3& wi) const override;
private:
    vec3 m_wo;      //local wi
};

class Specular : public Scatter
{
public:
    Specular(const Color3f& r, float co, const vec3& wo)
        : Scatter(r), coefficient(co), m_wo(wo){ }
    virtual vec3 Fx(const vec3& wi) const override;
    virtual Scatterinfo Sample() const override;
    virtual float Pdf(const vec3& wi) const override;
private:
    float coefficient;
    vec3 m_wo;
};

class specular_reflection :public Scatter
{
public:
    specular_reflection(const vec3& wo) : Scatter(Color3f(1)), m_wo(wo) {}
    virtual Color3f Fx(const vec3& wi) const override { return Color3f(0.f); }
    virtual Scatterinfo Sample() const override;
    virtual float Pdf(const vec3& wi) const override { return 0.f; }
private:
    vec3 m_wo;
};

class specular_reflection_transmission : public Scatter
{
public:
    specular_reflection_transmission(const vec3& wo,const float& ni)
        : Scatter(Color3f(1)), m_wo(wo), Ni(ni) {}
    virtual Color3f Fx(const vec3& wi) const override { return Color3f(0); }
    virtual Scatterinfo Sample() const override;
    virtual float Pdf(const vec3& wi) const override { return 0.f; }
private:
    vec3 m_wo;
    float Ni;
};

class BSDF
{
public:
    BSDF(hitInfo& info);
    Color3f Fx(const vec3& wi) const;
    Scatterinfo Sample() const;
    float Pdf(const vec3& wi) const;
    void get_sample_weight();
    void energy_conservation();
    std::vector<std::shared_ptr<Scatter>> bxdfs;
    bool isallType = false;
private:
    coordiantetransform onb;
};

class BRDF
{
public:
    BRDF() {}
    Scatterinfo lambertian_diffuse(hitInfo& info);
    Scatterinfo blinn_phong_specular(hitInfo& info);
    Scatterinfo specular_reflection_and_transmission(hitInfo& info);

};
#endif