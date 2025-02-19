cmake_minimum_required(VERSION 3.15)

project(PathTracer VERSION 1.0.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(OpenGL REQUIRED)
# find_package(GLEW REQUIRED)

set(GLFW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/glfw/include")
set(GLFW_LIBRARIES "${CMAKE_SOURCE_DIR}/glfw/lib/glfw3.lib")
set(GLM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/glm")

include_directories(${GLFW_INCLUDE_DIR})
include_directories(${GLM_INCLUDE_DIR})

file(GLOB_RECURSE ALL_SOURCE   
${PROJECT_SOURCE_DIR}/src/*.c
${PROJECT_SOURCE_DIR}/src/*.cpp
${PROJECT_SOURCE_DIR}/src/*.hpp
${PROJECT_SOURCE_DIR}/src/*.h
)

include_directories(${PROJECT_SOURCE_DIR}/src)

# add the executable
add_executable(${PROJECT_NAME} ${ALL_SOURCE})
target_link_libraries(PathTracer PRIVATE OpenGL::GL ${GLFW_LIBRARIES})