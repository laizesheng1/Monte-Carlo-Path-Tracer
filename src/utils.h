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

inline float rand1f() {
    static std::random_device rd;
    static std::mt19937 generator(rd());
    static std::uniform_real_distribution<> distrib(0, 1);
    return distrib(generator);
}