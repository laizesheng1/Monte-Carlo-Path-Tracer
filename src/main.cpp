#include "AABB.h"
#include "Render.h"

int main(int argc, char** argv)
{
	string filename;
	//filename = "../example-scenes-cg24/bathroom2/bathroom2.obj";
	//filename = "../example-scenes-cg24/veach-mis/veach-mis.obj";
	//filename = "../example-scenes-cg24/cornell-box/cornell-box.obj";
	filename = "D:/Users/laizesheng/Desktop/example-scenes-cg24/veach-mis/veach-mis.obj";
	//filename = "D:/Users/laizesheng/Desktop/example-scenes-cg24/cornell-box/cornell-box.obj";
	//filename = "D:/Users/laizesheng/Desktop/example-scenes-cg24/bathroom2/bathroom2.obj";
	Model model(filename);
	cout << model.face.size() << " " << model.normal.size() << " " << model.vertex.size() << endl;
	int w = model.camerainfo.width, h = model.camerainfo.height;
	Scene scene(w, h);
	Render render(model);

	size_t lastDotPos = filename.rfind('/');
	std::string file_name = filename.substr(lastDotPos + 1);
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(w, h, file_name.c_str(), NULL, NULL);
	glfwMakeContextCurrent(window);
	int frame = 0;
	clock_t start_time, end_time;
	while (!glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		start_time = clock();
		render.render(scene);
		end_time = clock();
		glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, scene.getPixelsColor());
		std::cout << "frame: " << frame++ << "    frame cost: " << static_cast<double>(end_time - start_time) / CLOCKS_PER_SEC << "s\n";
		glfwSwapBuffers(window);
		glfwPollEvents();
		/*if ((frame & (frame - 1)) == 0)
			scene.save_image(frame, filename);*/
	}
	scene.save_image(frame, file_name);
	glfwTerminate();
}