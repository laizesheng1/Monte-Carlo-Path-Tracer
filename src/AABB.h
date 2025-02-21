#include "utils.h"
#include <memory>
#include <vector>
#include <string>

class Triangle
{

public:
	dvec3 v0,v1,v2,nomal,texture;
	dvec3 A, B;		//min vertex

};

class Scene
{
public:
	Scene(int width,int heigh);
	Color3f& get_pixel_at(const Point2i& location);
	const Color3b* getPixelsColor() const;
	void save_image(int frame);
private:	
	int w, h;
	std::unique_ptr<Color3f[]> m_Pixels;
	std::unique_ptr<std::vector<Color3b>> m_ColorsUchar;

};