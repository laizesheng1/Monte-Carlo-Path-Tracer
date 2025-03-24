#include "BSDF.h"
#include "Render.h"

vec3 Diffuse::Fx(const vec3& wi) const
{
    return reflect / PI;
}

Scatterinfo Diffuse::Sample() const
{    
    vec3 f = reflect / PI;
    float phi = rand1f() * 2 * PI;
    float theta = 0.5f * acos(1 - 2 * rand1f());
    vec3 dir(
        sin(theta) * cos(phi),
        sin(theta) * sin(phi),
        cos(theta)
    );
    float pdf = std::abs(dir.z) / PI;
    return Scatterinfo(dir, f, pdf);
}

float Diffuse::Pdf(const vec3& wi) const
{
    return (wi.z < 0.f || m_wo.z < 0.f) ? 0.f : (wi.z / PI);
}

vec3 Specular::Fx(const vec3& wi) const
{
    if (wi.z < 0.f || m_wo.z < 0.f)
        return vec3(0.f);
    auto H = glm::normalize(wi + m_wo);     //半程向量
    float factor = (coefficient + 2) / (2.f * PI);
    return reflect * factor * std::pow(H.z, coefficient);
}

Scatterinfo Specular::Sample() const
{
    if (m_wo.z < 0.f)
        return Scatterinfo(vec3(0), vec3(0), 0.f);

    // random halfway vector
    auto u = rand1f(), v = rand1f();
    auto phi = 2 * PI * u;
    auto cosTheta = std::pow(v, 1.f / (coefficient + 1));
    auto sinTheta = std::sqrt(1.f - cosTheta * cosTheta);
    vec3 H(
        sinTheta * cos(phi),
        sinTheta * sin(phi),
        cosTheta
    );
    auto wi = -m_wo + H * 2.f * glm::dot(H, m_wo);
    if (wi.z < 0.f)
    {
        return Scatterinfo(vec3(0), vec3(0), 0.f);
    }
    //pdf normalization
    auto pdf = (coefficient + 1) / (2.f * PI) * std::pow(cosTheta, coefficient);
    return {wi, Fx(wi), pdf};
}

float Specular::Pdf(const vec3& wi) const
{
    if (m_wo.z < 0.f || wi.z < 0.f)
    {
        return 0.f;
    }
    auto H = glm::normalize(wi + m_wo);
    //pdf normalization
    return (coefficient + 1) / (2.f * PI) * std::pow(H.z, coefficient);
}

Scatterinfo specular_reflection::Sample() const
{
    if(m_wo.z<0.f)
        return Scatterinfo(vec3(0), vec3(0), 0.f);
    auto wo = vec3(-m_wo.x, -m_wo.y, m_wo.z);
    auto f = Color3f(1.f) / m_wo.z;
    return { wo,f, 1.f ,true};
}

Scatterinfo specular_reflection_transmission::Sample() const
{
    auto backface = m_wo.z < 0.f;
    float eta_o = 1.f, eta_i = Ni;
    if (backface)
        std::swap(eta_o, eta_i);
    vec3 n = backface ? vec3(0, 0, -1) : vec3(0, 0, 1);
    float cos_o = backface ? -m_wo.z : m_wo.z;
    float sin_o = std::sqrt(1.f - cos_o * cos_o);
    float sin_i = eta_o / eta_i * sin_o;
    float cos_i = 0.f;

    // calculate fresnel
    float fresnel = 0.f;
    if (sin_i > 1.f)
        fresnel = 1.f;
    else
    {
        cos_i = std::sqrt(1.f - sin_i * sin_i);
        fresnel = calculateFresnelDielectric(eta_o, eta_i, cos_o, cos_i);
    }

    Scatterinfo info;
    if (rand1f() < fresnel)
    {
        auto wo = vec3(-m_wo.x, -m_wo.y, m_wo.z);
        info.f = fresnel * Color3f(1.f) / std::abs(wo.z);
        info.wo = wo;
        info.pdf = fresnel;
        //LOG_INFO("reflect, fresnel: {}", fresnel);
    }
    else
    {
        auto eta = eta_o / eta_i;
        auto wo = -eta * m_wo + (eta * cos_o - cos_i) * n;
        info.f = (1 - fresnel) * Color3f(1) * eta * eta / cos_i;
        info.wo = wo;
        info.pdf = 1.f - fresnel;
        //LOG_INFO("transmit, cos: {}->{}", cos_o, cos_i);
    }
    return info;
}

BSDF::BSDF(hitInfo& info)
{   
    onb = coordiantetransform(info.normal);
    auto localwi = onb.worldTolocal(info.wi);
    auto mat = info.mtl;
    auto kd = mat->Map_Kd->get_color(info.uv);
    auto ks = mat->Ks;
    auto ns = mat->Ns;

    if (mat->Ni > 1)       //透射transmission
    {
        bxdfs.push_back(std::make_shared<specular_reflection_transmission>(localwi, mat->Ni));
        isallType = true;
    }
    else if (glm::length(mat->Ks) )      //反射/specular
    {
        if(ns>=10000)
        {
            bxdfs.push_back(std::make_shared<specular_reflection>(localwi));        //完全镜面反射
        }
        else 
        {
            bxdfs.push_back(std::make_shared<Specular>(ks, ns, localwi));
        }
    }
    bxdfs.push_back(std::make_shared<Diffuse>(kd, localwi));
    get_sample_weight();
    energy_conservation();
}

Color3f BSDF::Fx(const vec3& wi) const
{
    auto wo = onb.worldTolocal(wi);
    Color3f ret(0.f);
    for (auto& bxdf : bxdfs)
    {
        ret += bxdf->Fx(wo);
    }
    return ret;
}

Scatterinfo BSDF::Sample() const
{
    std::vector<float> weightPrefixSum(bxdfs.size());
    for (int i = 0; i < bxdfs.size(); i++)
    {
        weightPrefixSum[i] = bxdfs[i]->weight;
        if (i)
            weightPrefixSum[i] += weightPrefixSum[i - 1];
    }
    auto it = std::lower_bound(weightPrefixSum.begin(), weightPrefixSum.end(), rand1f() * weightPrefixSum.back());
    int index = std::min(int(it - weightPrefixSum.begin()), (int)weightPrefixSum.size() - 1);

    // sample
    auto sample = bxdfs[index]->Sample();

    // accumulate value and pdf
    sample.pdf *= bxdfs[index]->weight;
    for (int i = 0; i < bxdfs.size(); i++)
    {
        if (index == i)
            continue;
        sample.f += bxdfs[i]->Fx(sample.wo);
        sample.pdf += bxdfs[i]->Pdf(sample.wo) * bxdfs[i]->weight;
        //sample.f += bxdfs[i]->Sample().f;
        //sample.pdf += bxdfs[i]->Sample().pdf * bxdfs[i]->weight;
    }
    sample.wo = onb.localToworld(sample.wo);
    return sample;
}

float BSDF::Pdf(const vec3& wi) const
{
    auto wo = onb.worldTolocal(wi);
    float ret = 0.f;
    for (auto bxdf : bxdfs)
    {
        ret += bxdf->Pdf(wo) * bxdf->weight;
        //ret += bxdf->Sample().pdf * bxdf->weight;
    }
    return ret;
}

void BSDF::get_sample_weight()
{
    auto luminance = [](const Color3f& r)
        {
            return r.x * 0.212671f + r.y * 0.715160f + r.z * 0.072169f;
        };
    std::vector<float> lums(bxdfs.size());
    float sum = 0.f;
    for (int i = 0; i < bxdfs.size(); i++)
    {
        lums[i] = luminance(bxdfs[i]->reflect);
        sum += lums[i];
    }

    if (sum == 0)
        return;
    float inverseLumSum = 1.f / sum;
    for (int i = 0; i < bxdfs.size(); i++)
    {
        bxdfs[i]->weight = lums[i] * inverseLumSum;
    }
}

void BSDF::energy_conservation()
{
    Color3f sum(0.f);
    for (auto bxdf : bxdfs)
    {
        sum += bxdf->reflect;
    }
    float maxComponent = std::max(sum.x, std::max(sum.y, sum.z));
    if (maxComponent < 1.f)
        return;
    for (auto bxdf : bxdfs)
    {
        bxdf->reflect /= maxComponent;
    }
}