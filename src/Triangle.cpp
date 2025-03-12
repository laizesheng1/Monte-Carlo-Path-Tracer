#include "Triangle.h"
#include "Render.h"

AABB Triangle::get_bbox() const
{
    /*AABB box;
    for (int i = 0; i < 3; i++)
    {
        box=box.Union(v[i]);
    }
    return box;*/
    return AABB(A, B);
}

Point2f Triangle::sample_Point2() {
    float u = rand1f(), v = rand1f();
    if (u + v > 1) {
        u = 1 - u;
        v = 1 - v;
    }
    return Point2f(u, v);
}

float Triangle::area() const {
    vec3 a = v[1] - v[0];
    vec3 b = v[2] - v[0];
    return 0.5f * glm::length(glm::cross(a, b));
}

dvec3 Triangle::center() const
{
    return (v[0] + v[1] + v[2]) / 3.0;
}

dvec3 Triangle::interplote_Vertex(double b1, double b2) const
{
    return (1 - b1 - b2) * v[0] + b1 * v[1] + b2 * v[2];
}
dvec3 Triangle::interplote_Normal(double b1, double b2) const
{
    return glm::normalize((1 - b1 - b2) * vn[0] + b1 * vn[1] + b2 * vn[2]);
}
dvec2 Triangle::interplote_Texture(double b1, double b2) const
{
    return (1 - b1 - b2) * uv[0] + b1 * uv[1] + b2 * uv[2];
}

bool Triangle::hit(const Ray& ray, hitInfo& info) const
{
    dvec3 edge1 = v[1] - v[0];
    dvec3 edge2 = v[2] - v[0];
    dvec3 h = cross(ray.direction, edge2);
    double a = dot(edge1, h);
    if (abs(a) < 0.00001f)
        return false;
    dvec3 s = ray.start - v[0];
    double u = dot(s, h);
    dvec3 q = cross(s, edge1);
    double v = dot(ray.direction, q);
    double t = dot(edge2, q);
    double inv_a = 1.0 / a;
    u *= inv_a;
    v *= inv_a;
    t *= inv_a;

    if (t >= ray.t1 && t < ray.t2 && u >= 0 && v >= 0 && (1 - u - v) >= 0)
    {
        info.t = t;
        info.point = interplote_Vertex(u, v);
        info.normal = interplote_Normal(u, v);
        info.front = dot(info.normal, ray.direction) < 0.0;     // 判断射线是否与正面相交
        info.wi = -ray.direction;
        info.uv = interplote_Texture(u, v);
        info.mtl = mtl;
        if(glm::length(mtl->radiance))
            info.lightarea = this->area();
        return true;
    }
    return false;
}


bool Triangle::isIntersect(Ray& ray) {

    const double EPSILON = 1e-6;
    dvec3 edge1 = v[1] - v[0];
    dvec3 edge2 = v[2] - v[0];
    dvec3 h = cross(ray.direction, edge2);
    double det = dot(edge1, h);
    if (fabs(det) < EPSILON)
        return false;
    double invDet = 1.0 / det;
    dvec3 s = ray.start - v[0];
    double u = invDet * dot(s, h);
    if (u < 0.0 || u > 1.0)
        return false;
    dvec3 q = cross(s, edge1);
    double v = invDet * dot(ray.direction, q);
    if (v < 0.0 || u + v > 1.0)
        return false;
    double t = invDet * dot(edge2, q);
    if (t < ray.t1 || t > ray.t2)
        return false;

    return true;
}