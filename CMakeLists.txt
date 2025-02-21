cmake_minimum_required(VERSION 3.15)

project(PathTracer VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(OpenGL REQUIRED)
# find_package(GLEW REQUIRED)

link_directories(${CMAKE_SOURCE_DIR}/packages/glfw/lib)
include_directories(${CMAKE_SOURCE_DIR}/packages)
include_directories(${CMAKE_SOURCE_DIR}/src/*.h)
add_library(pugixml STATIC
    ${CMAKE_SOURCE_DIR}/packages/xml/pugixml.cpp
    ${CMAKE_SOURCE_DIR}/packages/xml/pugixml.hpp
)

file(GLOB_RECURSE ALL_SOURCE   
${CMAKE_SOURCE_DIR}/src/*.cpp
${CMAKE_SOURCE_DIR}/src/*.h
${CMAKE_SOURCE_DIR}/src/*.hpp
)
add_executable(${PROJECT_NAME} ${ALL_SOURCE})

target_link_libraries(${PROJECT_NAME}  glfw3.lib opengl32.lib pugixml)
source_group(TREE "${CMAKE_SOURCE_DIR}" FILES ${ALL_SOURCE})