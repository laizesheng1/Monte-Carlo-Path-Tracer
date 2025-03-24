一、编程环境
  Windows10, Visual Studio2022, msvc编译器
  CPU: AMD Ryzen 7 5800X.
二、环境配置
  将example-scenes-cg24放到项目文件的目录下，使用ｃｍａｋｅ进行编译。
三、代码结构
  AABB实现三角形的层次包围盒，用于构造BVH树。
  BSDF实现漫反射、Blinn-Phong高光、镜面反射、高光+反射+透射几种BXDF类型，其中透射在给定模型并没有使用到，用于render的bsdf采样。
  BVH用于加速三角形与光线的相交测试。
  model用于加载模型、纹理、材料以及相机参数。
  Render用于渲染模型，cast_Ray模拟发射光线，ray_tracing实现蒙特卡洛光线追踪方法，sample_light实现光源采样，ray_tracing实现了递归、非递归两种方式用于对比渲染速度。
  Scene用于获取渲染窗口需要的颜色数组及Render需要累加的颜色数组。
  Triangle保存模型三角形的信息，及在构建BVH树所需要的信息。
  ｍａｉｎ用于选择模型，并使用ｇｌｆｗ显示渲染的窗口。
四、实验结果
  保存至ｒｅｓｕｌｔｓ文件夹中。
