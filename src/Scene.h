#pragma once
#include "utils.h"
#include <memory>
#include <vector>
#include <string>

struct Pixels
{
	Color3f color;
	float spp;		//the number of sample per pixel
	Pixels() :color(0.0f), spp(0.0f) {}
};


class Scene
{
public:
	Scene(int width, int heigh);
	void set_Pixel(const Point2i& location, Color3f& color);	//累加颜色值
	const Color3b* getPixelsColor();		//获取多次反射后的颜色值
	void save_image(int frame, std::string filename);		//保存图片
private:
	int w, h;
	std::unique_ptr<Pixels[]> m_Pixels;
	std::unique_ptr<std::vector<Color3b>> m_ColorsUchar;

};