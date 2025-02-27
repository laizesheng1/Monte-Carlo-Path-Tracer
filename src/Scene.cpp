#define STB_IMAGE_WRITE_IMPLEMENTATION	
#include "stb_image/stb_image_write.h"
#include "Scene.h"
#include <iostream>

Scene::Scene(int width, int heigh) : w(width), h(heigh)
{
    m_Pixels = std::unique_ptr<Pixels[]>(new Pixels[w * h]);
    m_ColorsUchar = std::unique_ptr<std::vector<Color3b>>(new std::vector<Color3b>(w * h));
}

void Scene::set_Pixel(const Point2i& location, Color3f& color)
{
    int idx = location.y * w + location.x;
    // Replace NaN components with zero.
    if (color.x != color.x) color.x = 0.0;
    if (color.y != color.y) color.y = 0.0;
    if (color.z != color.z) color.z = 0.0;
    m_Pixels[idx].color += color;
    m_Pixels[idx].spp += 1.0f;
    //std::cout << "After update: idx = " << idx << ", color = " << m_Pixels[idx].color.x<<" "<< m_Pixels[idx].color.y << " "<< m_Pixels[idx].color.z << std::endl;
}

const Color3b* Scene::getPixelsColor()
{
    float gamma = 1.f / 2.f;
    for (int i = 0; i < w * h; i++)
    {
        Color3f rgb = m_Pixels[i].color / m_Pixels[i].spp;
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