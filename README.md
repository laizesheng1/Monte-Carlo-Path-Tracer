# 一、编程环境

​	 Windows10, Visual Studio2022, msvc编译器
​	 CPU: AMD Ryzen 7 5800X.

# 二、环境配置

​	将example-scenes-cg24放到项目文件的目录下，使用cmake进行编译。

# 三、代码结构

​	AABB实现三角形的层次包围盒，用于构造更加平衡高效的BVH树。
​	BSDF实现漫反射、Blinn-Phong高光、镜面反射几种BXDF类型，用于Render的bsdf采样。
​	BVH用于加速三角形与光线的相交测试。
​	model用于加载模型、纹理、材料以及相机参数。
​	Render用于渲染模型，cast_Ray模拟发射光线，ray_tracing实现蒙特卡洛光线追踪方法，sample_light实现	光源采样，ray_tracing实现了递归、非递归两种方式用于对比渲染速度。
​	Scene用于获取渲染窗口需要的颜色数组及Render需要累加的颜色数组。
​	Triangle保存模型三角形的信息，及在构建BVH树所需要的信息。
​	main用于选择模型，并使用glfw显示渲染的窗口。

# 四、实验结果

​	   保存在results文件夹中。
**	结果说明**：bathroom.obj的柜子法向量似乎反了，导致bsdf实现漫反射时本应判断反射/入射方向是否在背面的逻辑删除了，同时会导致本应更黑的、有阴影的地方变得不明显。

