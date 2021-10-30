cmake_minimum_required(VERSION 3.4)

add_executable(playmz "main.cpp" "shaders.h" "buffers.h" "input_handler.h" "app.h" "image.h" "quad.h")

if(MSVC)
	target_compile_options(mkmz PRIVATE "/MT")
endif()

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(PNG REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(playmz PRIVATE OpenGL::GL GLEW::glew glfw PNG::PNG Threads::Threads)
