#include "AABB.h"
#include "Render.h"

int main(int argc, char** argv)
{
	Model model("../example-scenes-cg24/bathroom2/bathroom2.obj");
	//Model model("../example-scenes-cg24/cornell-box/cornell-box.obj");
	//Model model("../example-scenes-cg24/veach-mis/veach-mis.obj");
	//Model model("C:/Users/wuyifan/Desktop/example-scenes-cg23/cornell-box/cornell-box.obj");
	//Model model("D:/Users/laizesheng/Desktop/example-scenes-cg24/bathroom2/bathroom2.obj");
	cout << model.face.size() << " " << model.normal.size() << " " << model.vertex.size() << endl;
	int w = model.camerainfo.width, h = model.camerainfo.height;
	Scene scene(w, h);
	Render render(model);


	glfwInit();
	GLFWwindow* window = glfwCreateWindow(w, h, "window", NULL, NULL);
	glfwMakeContextCurrent(window);
	int frame = 0;
	clock_t start_time, end_time;
	while (!glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		start_time = clock();
		render.render(scene);
		end_time = clock();
		//scene.camera.spp++;
		glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, scene.getPixelsColor());
		std::cout << "frame: " << frame++ << "    frame cost: " << static_cast<double>(end_time - start_time) / CLOCKS_PER_SEC << "s\n";
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
}