#include "Render.h"
#include "BVH.h"

AABB Triangle::get_bbox() const 
{
    AABB box;
    for (int i = 0; i < 3; i++)
    {
        box=box.Union(v[i]);
    }
    return box;
    //return AABB(A, B);
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

bool Triangle::hit(const Ray& ray, hitInfo& info, double& t_max) const
{
    dvec3 edge1 = v[1] - v[0];
    dvec3 edge2 = v[2] - v[0];
    dvec3 h = cross(ray.direction, edge2);
    double a = dot(edge1, h);
    dvec3 s = ray.start - v[0];
    double u = dot(s, h);
    dvec3 q = cross(s, edge1);
    double v = dot(ray.direction, q);
    double t = dot(edge2, q);
    double inv_a = 1.0 / a;
    u *= inv_a;
    v *= inv_a;
    t *= inv_a;

   if(t >= ray.t1 && t < ray.t2 && u >= 0 && v >= 0 && (1 - u - v) >= 0)
   {
       if(t<t_max)
       {
           info.t = t;
           //info.point = ray.start + ray.direction * t;
           info.point = interplote_Vertex(u, v);
           //info.normal = normalize(cross(edge1, edge2));
           info.normal = interplote_Normal(u, v);
           info.front = dot(info.normal, ray.direction) < 0.0;     // �ж������Ƿ��������ཻ
           //info.wi = ray.start - info.point;  // ���㵽������������
           info.wi = -ray.direction;
           info.uv = interplote_Texture(u, v);
           info.mtl = mtl;
           t_max = t;
           return true;
       }
   }
   return false;
}

Render::Render(Model& m_model):model(m_model)
{
    setCamera();
    tranform_triangle();
    bvh=new BVH(triangles);
}

void Render::tranform_triangle()
{
    // ������е�����������
    triangles.clear();

    // ����ģ���е�������
    for (const auto& face : model.face)
    {
        Triangle triangle= Triangle();
        for (int i = 0; i < 3; ++i)
        {
            int v_idx = face[i][0];
            int vn_idx = face[i][1];
            int vt_idx = face[i][2];

            // ��ȡ���㡢���ߺ���������
            dvec3 vertex = model.vertex[v_idx];
            dvec3 normal = model.normal[vn_idx];
            dvec2 uv = model.texture[vt_idx];

            //// Ӧ������
            //vertex *= scale;
            //normal *= scale;
            //// Ӧ��ƽ�ƣ�pose ��һ�� 4x4 �ı任����
            //dvec4 vertex_homogeneous = dvec4(vertex, 1.0);
            //dvec4 normal_homogeneous = dvec4(normal, 0.0);
            //vertex_homogeneous = pose * vertex_homogeneous;
            //normal_homogeneous = pose * normal_homogeneous;

            // ת���� 3D ����
            /*triangle.v[i] = dvec3(vertex_homogeneous);
            triangle.vn[i] = dvec3(normal_homogeneous);*/
            triangle.v[i] = vertex;
            triangle.vn[i] = normal;
            triangle.uv[i] = uv;
        }

        // ��ȡ����
        int material_idx = face[0][3];      // ÿ����Ĳ��������ڵ�һ������� material_idx ��
        triangle.mtl = std::make_shared<Material>(model.materials[material_idx]);

        // ���� A �� B����С����󶥵㣩
        triangle.A = min(triangle.v[0], min(triangle.v[1], triangle.v[2]));
        triangle.B = max(triangle.v[0], max(triangle.v[1], triangle.v[2]));
        triangles.push_back(triangle);

        if (glm::length(triangle.mtl->radiance) > 0.01)
            lights.push_back(triangle);        
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
        //Color3f color = ray_tracing(ray);
        scene.set_Pixel({ x,y }, color);
    }
}

Ray Render::cast_Ray(int x, int y)
{
    double h = std::tan(camera.fov * std::_Pi_val / 180.0 * 0.5) * 2.0;
    double aspect_ratio = static_cast<double>(camera.w) / camera.h;
    dvec3 front = glm::normalize(camera.lookat - camera.eye);
    dvec3 right = glm::normalize(glm::cross(front, camera.up));
    dvec3 ver = camera.up * h;
    dvec3 hor = right * h * aspect_ratio;

    double u = (x + rand1f()) / model.camerainfo.width;
    double v = (y + rand1f()) / model.camerainfo.height;
    dvec3 pos = camera.eye + front + (u - 0.5) * hor + (v - 0.5) * ver;
    dvec3 dir = glm::normalize(pos - camera.eye);
    return Ray(camera.eye, dir);
}


Color3f Render::ray_tracing(Ray& ray, int depth)
{
    if (depth > MAX_DEPTH) {
        return Color3f(0.f);
    }

    hitInfo info;
    if (!bvh->hit(ray, info))
    {
        return Color3f(0.f);
    }    
    else{
        dvec3 color_d = info.mtl->Kd * glm::dot(-ray.direction, info.normal);
        Color3f color = glm::vec3(color_d);
        if (info.mtl->Kd.x == 0 && info.mtl->Map_Kd != nullptr)
        {
            color = info.mtl->Map_Kd->get_color(info.uv);
        }
        return color;
    }

    vec3 reflect_dir = glm::reflect(ray.direction, info.normal);
    Ray new_ray(info.point, reflect_dir);
    return ray_tracing(new_ray, depth + 1);       
}

Color3f Render::ray_tracing(Ray& ray)
{
    Color3f L = vec3(0.0f);
    hitInfo info;
    for (int bounce = 0; bounce < 10; bounce++)
    {        
        if (!bvh->hit(ray, info))
        {
            break;
        }
        if (info.mtl->Kd.x == 0 && info.mtl->Map_Kd != nullptr)
        {
            L = info.mtl->Map_Kd->get_color(info.uv);
        }
        dvec3 color_d = info.mtl->Kd * glm::dot(-ray.direction, info.normal);
        Color3f color = glm::vec3(color_d);
        L += color;   
        vec3 reflect_dir = glm::reflect(ray.direction, info.normal);
        Ray new_ray(info.point, reflect_dir);
        ray = new_ray;
    }
    return L;
}

Color3f Render::sample_light(const hitInfo& info)
{

}