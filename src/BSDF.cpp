#include "BSDF.h"
#include "Render.h"

Scatterinfo Diffuse::Sample() const
{    
    if (m_wo.z < 0.f)
        return Scatterinfo(dvec3(0), dvec3(0), 0);
    auto info = Scatterinfo();
    // cosine hemisphere sampling
    float phi = rand1f() * 2 * PI;
    float theta = 0.5f * acos(1 - 2 * rand1f());
    vec3 v(sin(theta) * cos(phi),
        sin(theta) * sin(phi),
        cos(theta));
    auto pdf = std::abs(v[2]) / PI;
    if (v.z < 0 || m_wo.z < 0)
        info.f = Color3f(0);
    else info.f = reflect / PI;
    info.pdf = pdf;
    info.wo = cos(theta) * m_wo;
    return info;
}