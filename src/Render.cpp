#include "Render.h"

Render::Render(Model& m_model):model(m_model)
{
    setCamera();
    tranform_triangle();
}

void Render::tranform_triangle()
{
    // ������е�����������
    triangles.clear();

    // ����ģ���е�������
    for (const auto& face : model.face)
    {
        Triangle triangle(model.materials[0]);      //Ĭ�ϵ�һ������
        for (int i = 0; i < 3; ++i)
        {
            int v_idx = face[i][0];
            int vt_idx = face[i][1];
            int vn_idx = face[i][2];

            // ��ȡ���㡢���ߺ���������
            dvec3 vertex = model.vertex[v_idx];
            dvec3 normal = model.normal[vn_idx];
            dvec2 uv = model.texture[vt_idx];

            // Ӧ������
            vertex *= scale;
            normal *= scale;

            // Ӧ��ƽ�ƣ�pose ��һ�� 4x4 �ı任����
            dvec4 vertex_homogeneous = dvec4(vertex, 1.0);
            dvec4 normal_homogeneous = dvec4(normal, 0.0);

            vertex_homogeneous = pose * vertex_homogeneous;
            normal_homogeneous = pose * normal_homogeneous;

            // ת���� 3D ����
            triangle.v[i] = dvec3(vertex_homogeneous);
            triangle.vn[i] = dvec3(normal_homogeneous);
            triangle.uv[i] = uv;
        }

        // ��ȡ����
        int material_idx = face[0][3];      // ÿ����Ĳ��������ڵ�һ������� material_idx ��
        triangle.tri_mtl = model.materials[material_idx];

        // ���� A �� B����С����󶥵㣩
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

    // ������߷������2�Ĳ��
    dvec3 h = cross(ray.direction, edge2);
    double det = dot(edge1, h);
    if (fabs(det) < EPSILON)
        return false;       // ������������ƽ�У��޽���
    double invDet = 1.0 / det;

    // �������㵽�����ε�һ�����������
    dvec3 s = ray.start - v0;
    // ���� u ��������������֮һ��
    double u = invDet * dot(s, h);
    if (u < 0.0 || u > 1.0)
        return false; // u ������Χ�����㲻����������

    // ���� q ����
    dvec3 q = cross(s, edge1);

    // ���� v ��������������֮һ��
    double v = invDet * dot(ray.direction, q);
    if (v < 0.0 || u + v > 1.0)
        return false; // v ������Χ�����㲻����������

    // ���� t ���������߲�����
    double t = invDet * dot(edge2, q);

    // ��� t �Ƿ��ڹ��ߵ���Ч��Χ��
    if (t < ray.t1 || t > ray.t2)
        return false; // ���㲻�ڹ��߷�Χ��

    return true; // �������������ཻ
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

    // �����Ҫ��һ���ĵݹ飨���磬���Ǵμ����ߣ�
    // ���磬���ڴμ����ߣ����Դ����µĹ��߲��ݹ���� ray_tracing
    

    return L;
}

Color3f Render::calculateColor(const Material& material, const Ray& ray, const Triangle& triangle)
{
    Color3f color(0.f, 0.f, 0.f);

    // ���������
    dvec3 diffuse = material.Kd * material.kd;
    dvec3 lightDir = -ray.direction;
    dvec3 normal = (triangle.v[0] + triangle.v[1] + triangle.v[2]) / 3.0;
    double diff = glm::dot(normal, lightDir);
    if (diff > 0) {
        color += Color3f(diffuse * diff);
    }

    // ���淴�����
    dvec3 specular = material.Ks * material.ks;
    dvec3 viewDir = glm::normalize(ray.direction);
    dvec3 reflectDir = glm::reflect(-lightDir, normal);
    double spec = pow(glm::dot(viewDir, reflectDir), material.Ns);
    if (spec > 0) {
        color += Color3f(specular * spec);
    }

    // ������㣨͸���ȣ�
    if (material.Tr != dvec3(0, 0, 0) && material.tr > 0) {
        // ������߼��㣬��Ҫ����˹�����������������䷽��
        // �Լ����ݲ��ʵ������ʣ�Ni�����������������仹�Ƿ���
        dvec3 refractDir;
        double refractiveIndexRatio = material.Ni / 1.0; // �����ⲿ����������Ϊ1.0
        double cosi = glm::dot(-ray.direction, normal);
        double k = 1 - refractiveIndexRatio * refractiveIndexRatio * (1 - cosi * cosi);
        if (k >= 0) {
            refractDir = glm::refract(ray.direction, normal, refractiveIndexRatio);
            // ���������ʼ�����ɫ���ף�����򻯴���
            color += Color3f(material.Tr * material.tr);
        }
    }

    return color;
}
