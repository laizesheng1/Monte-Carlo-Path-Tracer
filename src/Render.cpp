#include "Render.h"

Render::Render(Model& m_model):model(m_model)
{
    setCamera();
    tranform_triangle();
}

void Render::tranform_triangle()
{
    // 清空现有的三角形数据
    triangles.clear();

    // 遍历模型中的所有面
    for (const auto& face : model.face)
    {
        Triangle triangle(model.materials[0]);      //默认第一个材料
        for (int i = 0; i < 3; ++i)
        {
            int v_idx = face[i][0];
            int vt_idx = face[i][1];
            int vn_idx = face[i][2];

            // 获取顶点、法线和纹理坐标
            dvec3 vertex = model.vertex[v_idx];
            dvec3 normal = model.normal[vn_idx];
            dvec2 uv = model.texture[vt_idx];

            // 应用缩放
            vertex *= scale;
            normal *= scale;

            // 应用平移（pose 是一个 4x4 的变换矩阵）
            dvec4 vertex_homogeneous = dvec4(vertex, 1.0);
            dvec4 normal_homogeneous = dvec4(normal, 0.0);

            vertex_homogeneous = pose * vertex_homogeneous;
            normal_homogeneous = pose * normal_homogeneous;

            // 转换回 3D 向量
            triangle.v[i] = dvec3(vertex_homogeneous);
            triangle.vn[i] = dvec3(normal_homogeneous);
            triangle.uv[i] = uv;
        }

        // 获取材质
        int material_idx = face[0][3];      // 每个面的材质索引在第一个顶点的 material_idx 中
        triangle.tri_mtl = model.materials[material_idx];

        // 计算 A 和 B（最小和最大顶点）
        triangle.A = min(triangle.v[0], min(triangle.v[1], triangle.v[2]));
        triangle.B = max(triangle.v[0], max(triangle.v[1], triangle.v[2]));
        triangles.push_back(triangle);
    }
}

bool Render::isIntersect(Ray& ray,Triangle& tri) {

    const double EPSILON = 1e-6; 
    dvec3 v0 = tri.v[0];
    dvec3 v1 = tri.v[1];
    dvec3 v2 = tri.v[2];
    dvec3 edge1 = v1 - v0;
    dvec3 edge2 = v2 - v0;

    // 计算光线方向与边2的叉积
    dvec3 h = cross(ray.direction, edge2);
    double det = dot(edge1, h);
    if (fabs(det) < EPSILON)
        return false;       // 光线与三角形平行，无交点
    double invDet = 1.0 / det;

    // 计算从起点到三角形第一个顶点的向量
    dvec3 s = ray.start - v0;
    // 计算 u 参数（重心坐标之一）
    double u = invDet * dot(s, h);
    if (u < 0.0 || u > 1.0)
        return false; // u 超出范围，交点不在三角形内

    // 计算 q 向量
    dvec3 q = cross(s, edge1);

    // 计算 v 参数（重心坐标之一）
    double v = invDet * dot(ray.direction, q);
    if (v < 0.0 || u + v > 1.0)
        return false; // v 超出范围，交点不在三角形内

    // 计算 t 参数（光线参数）
    double t = invDet * dot(edge2, q);

    // 检查 t 是否在光线的有效范围内
    if (t < ray.t1 || t > ray.t2)
        return false; // 交点不在光线范围内

    return true; // 光线与三角形相交
}

void Render::setCamera()
{   
    camera.w = model.camerainfo.width;
    camera.h = model.camerainfo.height;
    camera.eye = model.camerainfo.eye;
    camera.lookat = model.camerainfo.lookat;
    camera.up = model.camerainfo.up;
    camera.fov = model.camerainfo.fovy;
}

void Render::render(Scene& scene)
{
    int w = model.camerainfo.width, h = model.camerainfo.height;
    int cnt = w * h;
#pragma omp parallel for
    for (int i = 0; i < cnt; i++)
    {
        int x = i % w, y = i / w;
        Ray ray = cast_Ray(x, y);
        Color3f color = ray_tracing(ray, 0);
        scene.set_Pixel({ x,y }, color);
    }
}

Ray& Render::cast_Ray(int x, int y)
{
    float h = std::tan(camera.fov * std::_Pi_val / 180.f * 0.5f) * 2.0f;
    float aspect_ratio = static_cast<float>(camera.w) / camera.h;
    auto front = glm::normalize(camera.lookat - camera.eye);
    vec3 right = glm::normalize(glm::cross(front, camera.up));
    vec3 ver = camera.up * h;
    vec3 hor = right * h * aspect_ratio;

    float u = (x + rand1f()) / model.camerainfo.width;
    float v = (y + rand1f()) / model.camerainfo.height;
    vec3 pos = camera.eye + camera.lookat + (u - 0.5f) * hor + (v - 0.5f) * ver;
    vec3 dir = glm::normalize(pos - camera.eye);
    return Ray(camera.eye, dir);
}


Color3f& Render::ray_tracing(Ray& ray, int depth)
{
    Color3f L = Color3f(0.f);
    Color3f beta(1.f);

    if (depth > MAX_DEPTH) {
        return L;
    }

    for (auto& triangle : triangles) {
        if (isIntersect(ray, triangle)) {
            Material material = triangle.tri_mtl;
            Color3f contribution = calculateColor(material, ray, triangle);
            L += beta * contribution;
        }
    }

    // 如果需要进一步的递归（例如，考虑次级光线）
    // 例如，对于次级光线，可以创建新的光线并递归调用 ray_tracing
    

    return L;
}

Color3f Render::calculateColor(const Material& material, const Ray& ray, const Triangle& triangle)
{
    Color3f color(0.f, 0.f, 0.f);

    // 漫反射计算
    dvec3 diffuse = material.Kd * material.kd;
    dvec3 lightDir = -ray.direction;
    dvec3 normal = (triangle.v[0] + triangle.v[1] + triangle.v[2]) / 3.0;
    double diff = glm::dot(normal, lightDir);
    if (diff > 0) {
        color += Color3f(diffuse * diff);
    }

    // 镜面反射计算
    dvec3 specular = material.Ks * material.ks;
    dvec3 viewDir = glm::normalize(ray.direction);
    dvec3 reflectDir = glm::reflect(-lightDir, normal);
    double spec = pow(glm::dot(viewDir, reflectDir), material.Ns);
    if (spec > 0) {
        color += Color3f(specular * spec);
    }

    // 折射计算（透明度）
    if (material.Tr != dvec3(0, 0, 0) && material.tr > 0) {
        // 折射光线计算，需要根据斯涅尔定律来计算折射方向
        // 以及根据材质的折射率（Ni）来决定光线是折射还是反射
        dvec3 refractDir;
        double refractiveIndexRatio = material.Ni / 1.0; // 假设外部介质折射率为1.0
        double cosi = glm::dot(-ray.direction, normal);
        double k = 1 - refractiveIndexRatio * refractiveIndexRatio * (1 - cosi * cosi);
        if (k >= 0) {
            refractDir = glm::refract(ray.direction, normal, refractiveIndexRatio);
            // 根据折射率计算颜色贡献，这里简化处理
            color += Color3f(material.Tr * material.tr);
        }
    }

    return color;
}
