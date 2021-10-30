#pragma once
#include <GL/glew.h>
#include "input_handler.h"

struct application
{
	application(int major, int minor, int window_width, int window_height, const char *window_title)
	{
		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);

		main_window = glfwCreateWindow(window_width, window_height, window_title, nullptr, nullptr);

		key_input = key_handler::get_handler_instance(main_window);
		size_input = window_size_handler::get_handler_instance(main_window);
		mouse_input = mouse_handler::get_handler_instance(main_window);

		glViewport(0, 0, size_input->width(), size_input->height());

		glfwMakeContextCurrent(main_window);

		glewExperimental = GL_TRUE;

		glewInit();
	}
	void close()
	{
		glfwDestroyWindow(main_window);
		glfwTerminate();
	}
	~application()
	{
		glfwDestroyWindow(main_window);
		glfwTerminate();
	}
	GLFWwindow *main_window;
	key_handler *key_input;
	mouse_handler *mouse_input;
	window_size_handler *size_input;
};