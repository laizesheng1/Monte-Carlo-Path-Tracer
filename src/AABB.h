#ifndef AABB_H
#define AABB_H

#include "utils.h"
#include <memory>
#include <vector>
#include <string>


class Scene
{
public:
	Scene(int width,int heigh);
	void set_Pixel(const Point2i& location,Color3f color);
	const Color3b* getPixelsColor() const;
	void save_image(int frame);
private:	
	int w, h;
	std::unique_ptr<Color3f[]> m_Pixels;
	std::unique_ptr<std::vector<Color3b>> m_ColorsUchar;

};
#endif