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
	coordiantetransform(const vec3& n):w(n) {       //��nΪz�Ὠ���ľֲ�����ϵ
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
	vec3 wo;		//����֮��ķ���
	vec3 f;	    //��ɫ
	float pdf;		//
    bool isMirrorReflect = false;       //�Ƿ�Ϊ���淴��
    Scatterinfo(vec3 d, vec3 c, double p) :wo(d), f(c), pdf(p) {}
    Scatterinfo(vec3 d, vec3 c, double p,bool is) :wo(d), f(c), pdf(p) , isMirrorReflect(is){}
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

class Diffuse : public Scatter      //lambert
{
public:
    Diffuse(const Color3f& r, const vec3& wo) : Scatter(r), m_wo(wo) {}
    virtual vec3 Fx(const vec3& wi) const override;
    virtual Scatterinfo Sample() const override;
    virtual float Pdf(const vec3& wi) const override;
private:
    vec3 m_wo;      //local wi
};

class Specular : public Scatter         //Blinn-Phong
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

class specular_reflection :public Scatter       //mirror reflection
{
public:
    specular_reflection(const vec3& wo) : Scatter(Color3f(1)), m_wo(wo) {}
    virtual Color3f Fx(const vec3& wi) const override { return Color3f(0.f); }
    virtual Scatterinfo Sample() const override;
    virtual float Pdf(const vec3& wi) const override { return 0.f; }
private:
    vec3 m_wo;
};

class specular_reflection_transmission : public Scatter     //all /don't use
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
#endif