#pragma once
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <map>
#include <functional>
#include <vector>

class key_handler
{
public:
	static key_handler *get_handler_instance(GLFWwindow *window)
	{
		static std::map<GLFWwindow *, key_handler *> instances;
		try
		{
			return instances.at(window);
		}
		catch (std::out_of_range)
		{
			return instances[window] = new key_handler(window);
		}
	}
	key_handler(const key_handler &other) = delete;
	key_handler(key_handler &&other) = delete;

	key_handler &operator=(const key_handler &other) = delete;
	key_handler &operator=(key_handler &&other) = delete;

	static void handle()
	{
		glfwPollEvents();
	}

	static void disable_handler(GLFWwindow *window)
	{
		glfwSetKeyCallback(window, nullptr);
	}

	static void enable_handler(GLFWwindow *window)
	{
		glfwSetKeyCallback(window, callback);
	}

	int key_state(int key) const
	{
		try
		{
			return key_states.at(key);
		}
		catch (std::out_of_range)
		{
			return GLFW_RELEASE;
		}
	}

private:
	std::map<int, int> key_states;

	key_handler(GLFWwindow *window)
	{
		glfwSetKeyCallback(window, callback);
	}

	static void callback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		get_handler_instance(window)->key_states[key] = action;
	}
};

class mouse_handler
{
public:
	static mouse_handler *get_handler_instance(GLFWwindow *window)
	{
		static std::map<GLFWwindow *, mouse_handler *> instances;
		try
		{
			return instances.at(window);
		}
		catch (std::out_of_range)
		{
			return instances[window] = new mouse_handler(window);
		}
	}

	void add_position_callback(std::function<void(double, double)> new_callback)
	{
		pos_callbacks.push_back(std::move(new_callback));
	}

	static void disable_position_callback(GLFWwindow *window)
	{
		glfwSetCursorPosCallback(window, nullptr);
	}

	static void enable_position_callback(GLFWwindow *window)
	{
		glfwSetCursorPosCallback(window, pos_callback);
	}

	void add_button_callback(std::function<void(int, int, int)> new_callback)
	{
		button_callbacks.push_back(std::move(new_callback));
	}

	static void disable_button_callback(GLFWwindow *window)
	{
		glfwSetMouseButtonCallback(window, nullptr);
	}

	static void enable_button_callback(GLFWwindow *window)
	{
		glfwSetMouseButtonCallback(window, button_callback);
	}

	void add_enter_exit_callback(std::function<void(int)> new_callback)
	{
		enter_exit_callbacks.push_back(std::move(new_callback));
	}

	static void disable_enter_exit_callback(GLFWwindow *window)
	{
		glfwSetCursorEnterCallback(window, nullptr);
	}

	static void enable_enter_exit_callback(GLFWwindow *window)
	{
		glfwSetCursorEnterCallback(window, enter_exit_callback);
	}

	int button_state(int button) const
	{
		try
		{
			return button_states.at(button);
		}
		catch (std::out_of_range)
		{
			return GLFW_RELEASE;
		}
	}

	bool is_in_window() const
	{
		return in_window;
	}

private:
	std::vector<std::function<void(double, double)>> pos_callbacks;
	std::vector<std::function<void(int, int, int)>> button_callbacks;
	std::vector<std::function<void(int)>> enter_exit_callbacks;

	std::map<int, int> button_states;

	bool in_window;

	mouse_handler(GLFWwindow *window)
	{
		glfwSetCursorPosCallback(window, pos_callback);
		glfwSetMouseButtonCallback(window, button_callback);
		glfwSetCursorEnterCallback(window, enter_exit_callback);
	}

	static void pos_callback(GLFWwindow *window, double xpos, double ypos)
	{

		for (const auto &f : get_handler_instance(window)->pos_callbacks)
		{
			f(xpos, ypos);
		}
	}
	static void button_callback(GLFWwindow *window, int button, int action, int mods)
	{
		mouse_handler *cur_handler = get_handler_instance(window);

		cur_handler->button_states[button] = action;

		for (const auto &f : cur_handler->button_callbacks)
		{
			f(button, action, mods);
		}
	}
	static void enter_exit_callback(GLFWwindow *window, int entered)
	{
		mouse_handler *cur_handler = get_handler_instance(window);

		cur_handler->in_window = entered;

		for (const auto &f : cur_handler->enter_exit_callbacks)
		{
			f(entered);
		}
	}
};

class window_size_handler
{
public:
	static window_size_handler *get_handler_instance(GLFWwindow *window)
	{
		static std::map<GLFWwindow *, window_size_handler *> instances;
		try
		{
			return instances.at(window);
		}
		catch (std::out_of_range)
		{
			return instances[window] = new window_size_handler(window);
		}
	}

	void add_callback(std::function<void(int, int)> new_callback)
	{
		callbacks.push_back(std::move(new_callback));
	}

	static void disable_callback(GLFWwindow *window)
	{
		glfwSetWindowSizeCallback(window, nullptr);
	}

	static void enable_callback(GLFWwindow *window)
	{
		glfwSetWindowSizeCallback(window, callback);
	}

	int width()
	{
		return w;
	}
	int height()
	{
		return h;
	}
	double aspect()
	{
		return (double)w / h;
	}

private:
	std::vector<std::function<void(int, int)>> callbacks;
	int w;
	int h;

	window_size_handler(GLFWwindow *window)
	{
		glfwSetWindowSizeCallback(window, callback);
		glfwGetWindowSize(window, &w, &h);
	}

	static void callback(GLFWwindow *window, int width, int height)
	{
		window_size_handler *cur_handler = get_handler_instance(window);
		cur_handler->w = width;
		cur_handler->h = height;
		for (const auto &f : cur_handler->callbacks)
		{
			f(width, height);
		}
	}
};