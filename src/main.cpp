#include "model.h"
#include "AABB.h"

int main(int argc, char** argv)
{
	Model model("../example-scenes-cg24/bathroom2/bathroom2.obj");
	cout << model.face.size() << " " << model.normal.size() << " " << model.vertex.size();
	int w = model.camerainfo.width, h = model.camerainfo.height;
	Scene scene(w, h);

	glfwInit();
	GLFWwindow* window = glfwCreateWindow(w, h, "window", NULL, NULL);
	glfwMakeContextCurrent(window);
	int frame = 0;
	clock_t start_time, end_time;
	while (!glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		start_time = clock();
		//scene.render();
		end_time = clock();
		//scene.camera.spp++;
		glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, scene.getPixelsColor());
		std::cout << "frame: " << frame++ << "    frame cost: " << static_cast<double>(end_time - start_time) / CLOCKS_PER_SEC << "s\n";
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
}