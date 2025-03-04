#pragma once
#include "glm/glm.hpp"
#include <random>

using dvec4 = glm::dvec4;
using dvec3 = glm::dvec3;
using dvec2 = glm::dvec2;
using imat3x4 = glm::imat3x4;

using Color3f = glm::vec3;
using Color3b = glm::u8vec3;
using Point3i = glm::ivec3;
using Point2i = glm::ivec2;
using Point3f = glm::vec3;
using Point2f = glm::vec2;

using vec3= glm::vec3;
using dmat4x4 = glm::dmat4x4;

#define PI 3.1415926f
#define INV_PI (1.0f/3.1415926f)

inline float rand1f() {
    static std::random_device rd;
    static std::mt19937 generator(rd());
    static std::uniform_real_distribution<> distrib(0, 1);
    return distrib(generator);
}

inline double clamp01(float d) {
    if (d > 0.999f) return 0.999;
    if (d < 0.0f) return 0.0;
    return d;
}

inline vec3 random_cosine_direction() {
    auto r1 = rand1f();
    auto r2 = rand1f();

    auto phi = 2 * PI * r1;
    auto x = cos(phi) * sqrt(r2);
    auto y = sin(phi) * sqrt(r2);
    auto z = sqrt(1 - r2);

    return vec3(x, y, z);
}

inline float calculateFresnelDielectric(float incidentIOR, float transmittedIOR, float cosIncidentAngle, float cosTransmittedAngle) {
    auto parallelReflectance = (transmittedIOR * cosIncidentAngle - incidentIOR * cosTransmittedAngle) /
        (transmittedIOR * cosIncidentAngle + incidentIOR * cosTransmittedAngle);
    auto perpendicularReflectance = (incidentIOR * cosIncidentAngle - transmittedIOR * cosTransmittedAngle) /
        (incidentIOR * cosIncidentAngle + transmittedIOR * cosTransmittedAngle);
    return 0.5f * (parallelReflectance * parallelReflectance + perpendicularReflectance * perpendicularReflectance);
}
