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

            // 获取顶点、法线和纹理坐标
            dvec3 vertex = model.vertex[v_idx];
            dvec3 normal = model.normal[vn_idx];
            dvec2 uv = model.texture[vt_idx];
            triangle->v[i] = vertex;
            triangle->vn[i] = normal;
            triangle->uv[i] = uv;
        }

        // 获取材质
        int material_idx = face[0][3];      // 每个面的材质索引在第一个顶点的 material_idx 中
        triangle->mtl = std::make_shared<Material>(model.materials[material_idx]);

        // 计算 A 和 B（最小和最大顶点）
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
        //Color3f color = ray_tracing(ray, 0);      //递归
        Color3f color = ray_tracing(ray);       //非递归
        scene.set_Pixel({ x,y }, color);
    }
}

Ray Render::cast_Ray(int x, int y)
{
    double h = std::tan(camera.fov * std::_Pi_val / 180.0 * 0.5) * 2.0;
    dvec3 front = glm::normalize(camera.lookat - camera.eye);
    dvec3 right = glm::normalize(glm::cross(front, camera.up));
    double u = ((x + rand1f()) / camera.w - 0.5) * h * camera.w / camera.h;
    double v = ((y + rand1f()) / camera.h - 0.5) * h;
    dvec3 dir = glm::normalize(front + u * right + v * camera.up);
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
    if (glm::length(info.mtl->radiance) > 0.01)     //材质为自发光
    {
        return info.mtl->radiance;
    }
    Scatterinfo scat_info;
    auto& mat = info.mtl;
    auto L = sample_light(info);

    BSDF bsdf(info);
    scat_info = bsdf.Sample();      

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
        if (bounces == 0 && glm::length(mat->radiance) > 0.0001)   //直接光照
            L += vec3(mat->radiance);
        BSDF bsdf(info);
        auto lightsample = sample(info);
        if (lightsample.pdf != 0 && !bvh->has_hit(lightsample.ray))     //光源采样
        {            
            float cos_theta =fabs(glm::dot(vec3(info.normal), lightsample.wo));
            float weight = power_heuristic(lightsample.pdf / float(lights.size()), bsdf.Pdf(lightsample.wo));    // 多重重要性采样
            L += weight * beta * vec3(lightsample.f) * bsdf.Fx(lightsample.wo) * cos_theta / lightsample.pdf * float(lights.size());
        }

        //BSDF采样
        Scatterinfo scat_info;
        scat_info = bsdf.Sample();
        if (scat_info.pdf == 0.f)
            break;       
        //new ray
        Ray new_ray(info.point, scat_info.wo);
        float cos_theta = std::fabs(glm::dot(vec3(info.normal), scat_info.wo));
        beta *= scat_info.f * cos_theta / scat_info.pdf;
        
        hitInfo nextInfo;
        Ray tmp = new_ray;
        if (!bvh->hit(tmp, nextInfo))       //注意会更新ray hitinfo
            break;
        if (glm::length(nextInfo.mtl->radiance)&&nextInfo.front)      //如果newray从光源出发
        {
            if(scat_info.isMirrorReflect)
            {
                L += beta * vec3(nextInfo.mtl->radiance);
            }
            else{
                auto d = info.point - nextInfo.point;
                auto dist2 = glm::length(d) * glm::length(d);
                auto cosine = glm::dot(glm::normalize(d), nextInfo.normal);
                float lightPdf = 0.f;
                if (cosine != 0)
                    lightPdf = dist2 / cosine / float(lights.size()) / nextInfo.lightarea;      //除nextinfo.point所在light的面积
                auto weight = power_heuristic(scat_info.pdf, lightPdf);
                L += beta * vec3(nextInfo.mtl->radiance) * weight;
            }
        }
        
        if (bounces > 3)
        {
            auto q = std::min(std::max(std::max(beta.x, beta.y), beta.z), 0.95f);
            if (rand1f() > q)
                break;
            beta /= q;            
        }
        ray = new_ray;
        info = nextInfo;
    }
    return L;
}

Color3f Render::sample_light(hitInfo& info)     //采样光源
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
    float pdf = 0.f;
    //if (cos != 0)
        pdf = d2 / cos / light->area();

    float t = glm::length(d);
    Ray r(info.point, dir);
    r.t2 = t;
    if (!bvh->has_hit(r)) {
        return vec3(light->mtl->radiance) * info.mtl->Map_Kd->get_color(info.uv) * std::fabs(glm::dot(vec3(info.normal), dir)) / pdf / 2.0f;
    }
    return vec3(0);
}

lightinfo Render::sample(hitInfo& info)     //采样光源，用于计算非递归光源采样
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
    float pdf = 0.f;
    if(cos!=0)
        pdf = d2 / cos / light->area();     //

    vec3 f = vec3(light->mtl->radiance);
    float t = glm::length(d);
    Ray r(info.point, dir);
    r.t2 = t;
    return { dir,f,pdf ,r};
}