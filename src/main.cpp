#include "model.h"

int main(int argc, char** argv)
{
	Model model("../example-scenes-cg24/cornell-box/cornell-box.obj");
	cout << model.face.size() << " " << model.normal.size() << " " << model.vertex.size();
}