#include "Render.h"
#include "BVH.h"
#include "BSDF.h"

Render::Render(Model& m_model):model(m_model)
{
    setCamera();
    tranform_triangle();
    bvh=new BVH(triangles);
}

void Render::tranform_triangle()
{
    for (const auto& face : model.face)
    {
        std::shared_ptr<Triangle> triangle= std::make_shared<Triangle> (Triangle());
        for (int i = 0; i < 3; ++i)
        {
            int v_idx = face[i][0];
            int vn_idx = face[i][1];
            int vt_idx = face[i][2];

            // ��ȡ���㡢���ߺ���������
            dvec3 vertex = model.vertex[v_idx];
            dvec3 normal = model.normal[vn_idx];
            dvec2 uv = model.texture[vt_idx];
            triangle->v[i] = vertex;
            triangle->vn[i] = normal;
            triangle->uv[i] = uv;
        }

        // ��ȡ����
        int material_idx = face[0][3];      // ÿ����Ĳ��������ڵ�һ������� material_idx ��
        triangle->mtl = std::make_shared<Material>(model.materials[material_idx]);

        // ���� A �� B����С����󶥵㣩
        triangle->A = min(triangle->v[0], min(triangle->v[1], triangle->v[2]));
        triangle->B = max(triangle->v[0], max(triangle->v[1], triangle->v[2]));
        triangles.push_back(triangle);

        if (glm::length(triangle->mtl->radiance) > 0.01)
            lights.push_back(triangle);        
    }
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
    int w = camera.w, h = camera.h;
    int cnt = w * h;
#pragma omp parallel for
    for (int i = 0; i < cnt; i++)
    {
        int x = i % w, y = i / w;
        Ray ray = cast_Ray(x, y);
        //Color3f color = ray_tracing(ray, 0);
        Color3f color = ray_tracing(ray);
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
    if (glm::length(info.mtl->radiance) > 0.01)     //����Ϊ�Է���
    {
        return info.mtl->radiance;
    }
    Scatterinfo scat_info;
    auto& mat = info.mtl;
    auto L = sample_light(info);

    BSDF bsdf(info);
    scat_info = bsdf.Sample();
       
    //BRDF brdf;
    //if (mat->Ni > 1) {
    //    scat_info = brdf.specular_reflection_and_transmission(info);
    //}
    //else if (glm::length(mat->Ks)) {
    //    scat_info = brdf.blinn_phong_specular(info);
    //}
    //else
    //    scat_info = brdf.lambertian_diffuse(info);

    if (glm::length(scat_info.wo) < 0.00001f)
        return vec3(0);
    Ray new_ray(info.point, scat_info.wo);
    return L+scat_info.f * std::fabs(glm::dot(vec3(info.normal), scat_info.wo)) * ray_tracing(new_ray, depth + 1) / scat_info.pdf;
}

Color3f Render::ray_tracing(Ray& ray)
{
    Color3f L = Color3f(0.f);
    Color3f beta(1.f);
    hitInfo info;
    for (int bounces = 0; ; bounces++)
    {
        if (!bvh->hit(ray, info))
            break;
        auto& mat = info.mtl;
        if (bounces == 0 && glm::length(mat->radiance) > 0.0001)   //ֱ�ӹ���
            L += vec3(mat->radiance);
        BSDF bsdf(info);
        auto lightsample = sample(info);
        if (lightsample.pdf != 0 && !bvh->has_hit(lightsample.ray))
        {
            float cos_theta = std::fabs(glm::dot(vec3(info.normal), lightsample.wo));
            float weight = power_heuristic(lightsample.pdf/ float(lights.size()), bsdf.Pdf(lightsample.wo));  // ƽ������ʽ��Ҫ�Բ���
            L += beta * vec3(lightsample.light->mtl->radiance) * bsdf.Fx(lightsample.wo) * cos_theta / lightsample.pdf * weight * float(lights.size());
            //L += vec3(lightsample.light->mtl->radiance) * info.mtl->Map_Kd->get_color(info.uv) *beta * std::fabs(glm::dot(vec3(info.normal), lightsample.wo)) / lightsample.pdf;
        }

        //BSDF����
        Scatterinfo scat_info;
        scat_info = bsdf.Sample();
        if (scat_info.pdf == 0.f)
            break;       
        //new ray
        Ray new_ray(info.point, scat_info.wo);
        float cos_theta = std::fabs(glm::dot(vec3(info.normal), scat_info.wo));
        beta *= scat_info.f * cos_theta / scat_info.pdf;
        
        //if (nextInfo.front)      //���newray�ӹ�Դ����
        //{
        //    auto d = info.point - nextInfo.point;
        //    auto dist2 = glm::length(d)* glm::length(d);
        //    auto cosine = glm::dot(glm::normalize(d), nextInfo.normal);
        //    auto lightPdf = dist2 / cosine / lightsample.light->area() / float(lights.size());     //��info.point����light�����

        //    auto weight = power_heuristic(scat_info.pdf, lightPdf);
        //    L += beta * vec3(lightsample.f)* weight;
        //}
        // Russian roulette
        if (bounces > 3)
        {
            auto q = std::min(std::max(std::max(beta.x, beta.y), beta.z), 0.95f);
            if (rand1f() > q)
                break;
            beta /= q;
        }
        ray = new_ray;
    }
    return L;
}

Color3f Render::sample_light(hitInfo& info)
{
    int cnt = lights.size();
    int idx = std::min(static_cast<int>(rand1f() * cnt), cnt - 1);
    auto light = lights[idx];
    Point2f sample_uv = light->sample_Point2();
    vec3 point = light->interplote_Vertex(sample_uv.x, sample_uv.y);
    vec3 normal = light->interplote_Normal(sample_uv.x, sample_uv.y);
    vec3 d = point - vec3(info.point);
    vec3 dir = glm::normalize(d);
    float d2 = glm::dot(d, d);
    float cos = glm::dot(-dir, normal);
    float pdf = d2 / cos / light->area() ;

    float t = glm::length(d);
    Ray r(info.point, dir);
    r.t2 = t;
    if (!bvh->has_hit(r)) {
        return vec3(light->mtl->radiance) * info.mtl->Map_Kd->get_color(info.uv) * std::fabs(glm::dot(vec3(info.normal), dir)) / pdf / 2.0f;
    }
    return vec3(0);
}

lightinfo Render::sample(hitInfo& info)
{
    int cnt = lights.size();
    int idx = std::min(static_cast<int>(rand1f() * cnt), cnt - 1);
    auto light = lights[idx];
    Point2f sample_uv = light->sample_Point2();
    vec3 point = light->interplote_Vertex(sample_uv.x, sample_uv.y);
    vec3 normal = light->interplote_Normal(sample_uv.x, sample_uv.y);
    vec3 d = point - vec3(info.point);
    vec3 dir = glm::normalize(d);
    float d2 = glm::dot(d, d);
    float cos = glm::dot(-dir, normal);
    float pdf = d2 / cos / light->area();
    vec3 f = vec3(light->mtl->radiance);
    float t = glm::length(d);
    Ray r(info.point, dir);
    r.t2 = t;
    return { light,dir,f,pdf ,r};
}