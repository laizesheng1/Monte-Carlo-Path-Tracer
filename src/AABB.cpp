#include "AABB.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION	
#include "stb_image/stb_image_write.h"

Scene::Scene(int width, int heigh): w(width),h(heigh)
{
	m_Pixels = std::unique_ptr<Color3f[]>(new Color3f[w * h]);
	m_ColorsUchar = std::unique_ptr<std::vector<Color3b>>(new std::vector<Color3b>(w * h));
}

Color3f& Scene::get_pixel_at(const Point2i& location)
{
	int idx = location.y * w + location.x;
	return m_Pixels[idx];
}

const Color3b* Scene::getPixelsColor() const
{
    float gamma = 1.f / 2.f;
    for (int i = 0; i < w * h; i++)
    {
        Color3f rgb = m_Pixels[i];
        rgb = glm::pow(glm::clamp(rgb, vec3(0), vec3(1)), vec3(gamma));
        (*m_ColorsUchar)[i] = rgb * 255.99f;
    }
    return m_ColorsUchar->data();
}

void Scene::save_image(int frame)
{
    std::string file = "../results/" + std::to_string(frame) + ".png";
    stbi_write_png(file.c_str(), w, h, 3, getPixelsColor(), 0);
}