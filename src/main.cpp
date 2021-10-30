#define GLEW_STATIC
#undef GLFW_DLL

#include "image.h"

#include "app.h"

#include "object.h"
#include "shaders.h"

#include "quad.h"

#include "camera.h"

#include "maze.h"

#include "bounds.h"

#include "texture.h"

#include "shaders/frag.h"
#include "shaders/vert.h"

#include "minimap/vert.h"
#include "minimap/frag.h"

#include "2dshaders/ptshader_frag.h"
#include "2dshaders/ptshader_vert.h"

#include <iostream>
#include <fstream>

template <typename It>
std::vector<It> collides_with(It begin, It end, const bounding_box &box)
{
	std::vector<It> collisions;
	for (auto i = begin; i != end; ++i)
	{
		if (bounding_box::collides(*i, box))
			collisions.push_back(i);
	}
	return collisions;
}

int main(int argc, char *argv[])
{
	const char *file = argv[1];

	rgba_image maze(file);
	std::cout << maze[0][0].col << std::endl;

	constexpr float mpp = .5;

	std::array<float, 8 * 3> wall_cols;
	wall_cols.fill(0);
	for (int i = 0; i < wall_cols.size(); i += 3)
		wall_cols[i] = 1;

	std::array<float, 8 * 3> floor_cols;
	floor_cols.fill(1.0f);

	constexpr float clip_near = .1;
	constexpr float clip_far = 1000;

	application app(4, 3, 960, 540, "playmz");
	glfwSwapInterval(0);

	//minimap setup
	glm::mat4 ortho_mat = glm::ortho(0.f, (float)app.size_input->width(), (float)app.size_input->height(), 0.f, -1.f, 1.f);
	model map_model;
	map_model.scale(glm::vec3(2, 2, 2));
	glm::ivec2 map_dims{40, 40};

	mesh map_mesh({
					  2.f, 2.f, 0.f,						  //top left corner (0)
					  2.f + map_dims.x, 2.f, 0.f,			  //top right corner (1)
					  2.f, 2.f + map_dims.y, 0.f,			  //bottom left corner (2)
					  2.f + map_dims.x, 2.f + map_dims.y, 0.f //bottom right corner (3)
				  },
				  {0, 2, 1, 1, 2, 3});

	float map_txt_coords[8];

	program mp = make_program(make_shader(map_vert_src, GL_VERTEX_SHADER), make_shader(map_frag_src, GL_FRAGMENT_SHADER));

	float border_color[4] = {220 / 255.f, 220 / 255.f, 220 / 225.f, 1};

	texture maze_txtre(maze, GL_CLAMP_TO_BORDER, border_color);

	uniform ortho = mp.get_uniform("ortho");
	uniform map_model_uniform = mp.get_uniform("model");

	vao m_vao;
	m_vao.use();

	obj map(
		buffer_data<vbo_target>(map_mesh.vertices().data(), map_mesh.vertices().size() / 3, 3, 0, GL_STATIC_DRAW),
		buffer_data<vbo_target>(map_txt_coords, 4, 2, 1, GL_STATIC_DRAW),
		buffer_data<ebo_target>(map_mesh.indices().data(), map_mesh.indices().size(), GL_STATIC_DRAW));

	auto update_txt_coords = [&](int xcenter, int ycenter)
	{
		//top left
		map_txt_coords[0] = (xcenter - map_dims.x / 2.f) / maze.image_width();
		map_txt_coords[1] = (ycenter - map_dims.y / 2.f) / maze.image_height();

		//top right
		map_txt_coords[2] = (xcenter + map_dims.x / 2.f) / maze.image_width();
		map_txt_coords[3] = (ycenter - map_dims.y / 2.f) / maze.image_height();

		//bottom left
		map_txt_coords[4] = (xcenter - map_dims.x / 2.f) / maze.image_width();
		map_txt_coords[5] = (ycenter + map_dims.y / 2.f) / maze.image_height();

		//bottom right
		map_txt_coords[6] = (xcenter + map_dims.x / 2.f) / maze.image_width();
		map_txt_coords[7] = (ycenter + map_dims.y / 2.f) / maze.image_height();

		std::get<1>(map).b.attach_sub_data(0, 8 * sizeof(float), map_txt_coords);
	};

	update_txt_coords(0, 0);

	//pt setup
	glm::vec3 pt_data{map_mesh.vertices()[0] + map_dims.x / 2.f, map_mesh.vertices()[1] + map_dims.y / 2.f, map_mesh.vertices()[2]};
	model pt_model;

	program pt_p = make_program(make_shader(pt_shader_vert_src, GL_VERTEX_SHADER), make_shader(pt_shader_frag_src, GL_FRAGMENT_SHADER));

	uniform pt_p_ortho = pt_p.get_uniform("ortho");
	uniform pt_p_col = pt_p.get_uniform("col");
	uniform pt_p_model = pt_p.get_uniform("model");

	vao pt_vao;
	pt_vao.use();
	obj pt(buffer_data<vbo_target>(glm::value_ptr(pt_data), 1, 3, 0, GL_STATIC_DRAW));

	//3d graphics setup
	program sp = make_program(make_shader(vert_src, GL_VERTEX_SHADER), make_shader(frag_src, GL_FRAGMENT_SHADER));

	uniform mv = sp.get_uniform("mv_mat");
	uniform proj = sp.get_uniform("proj_mat");

	vao p_vao;
	p_vao.use();

	maze_loader mz_loader(maze.image_width(), maze.image_height(), maze);
	mz_loader.load({0, 0});

	glm::vec3 floor_dims{maze.image_width() * mpp, -1, maze.image_height() * mpp};

	quad floor_mesh(glm::vec3(0, 0, 0), floor_dims.x, floor_dims.y, floor_dims.z);

	glm::vec3 wall_size(mpp, 2, mpp);
	model wall_model;
	wall_model.scale(wall_size);

	model floor_model;

	std::vector<obj<buffer_data<vbo_target>, buffer_data<vbo_target>, buffer_data<ebo_target>>> walls;
	std::vector<bounding_box> wall_bounds;
	for (const auto &m : mz_loader.meshes())
	{
		std::vector<glm::vec3> verts;
		for (int i = 0; i < m.vertices().size(); i += 3)
		{
			verts.emplace_back(glm::vec3(wall_model * glm::vec4{m.vertices()[i], m.vertices()[i + 1], m.vertices()[i + 2], 1}));
		}
		wall_bounds.emplace_back(verts);
		walls.emplace_back(
			buffer_data<vbo_target>(m.vertices().data(), m.vertices().size() / 3, 3, 0, GL_STATIC_DRAW),
			buffer_data<vbo_target>(wall_cols.data(), wall_cols.size() / 3, 3, 1, GL_STATIC_DRAW),
			buffer_data<ebo_target>(m.indices().data(), m.indices().size(), GL_STATIC_DRAW));
	}

	wall_bounds.emplace_back(glm::vec3(0, 0, 0), floor_dims);

	obj floor(
		buffer_data<vbo_target>(floor_mesh.vertices().data(), floor_mesh.vertices().size() / 3, 3, 0, GL_STATIC_DRAW),
		buffer_data<vbo_target>(floor_cols.data(), floor_cols.size() / 3, 3, 1, GL_STATIC_DRAW),
		buffer_data<ebo_target>(floor_mesh.indices().data(), floor_mesh.indices().size(), GL_STATIC_DRAW));

	camera cam(-2, 1, -2);
	cam.look_at(0, 0, 0);

	glm::vec3 dcam;
	constexpr float speed = 3;
	constexpr float sprint_mult = 2;

	glm::vec3 player_dims(mpp, 1.75, mpp);
	glm::vec3 cam_player_off(player_dims.x / 2, player_dims.y - mpp, player_dims.z / 2);

	bounding_box player(cam - cam_player_off, player_dims);

	glm::mat4 mv_mat;

	bool matrix_update_switch = true;
	glm::vec2 mouse_pos = {app.size_input->width() / 2, app.size_input->height() / 2};
	glm::vec2 delta_pos;

	//this puts the cam_angs where they are supposed to be according to the direction of the camera. Without it will jump
	//cos(asin(x)) = sqrt(1 - x * x)
	//x is negative to put in right domain
	glm::vec2 cam_angs{
		-180 * acosf(cam.dir().x / sqrtf(1 - cam.dir().y * cam.dir().y)) / glm::pi<float>(),
		180 * asinf(cam.dir().y) / glm::pi<float>()};

	float sensitivity;

	glfwSetCursorPos(app.main_window, mouse_pos.x, mouse_pos.y);
	glfwSetInputMode(app.main_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	float fov = 180;

	auto update_cam_dir = [&](double xpos, double ypos)
	{
		matrix_update_switch = true;

		delta_pos.x = (float)xpos - mouse_pos.x;
		delta_pos.y = (float)ypos - mouse_pos.y;

		//std::cout << delta_pos.x << ", " << delta_pos.y << "\n";

		cam_angs += delta_pos * sensitivity;

		//std::cout << cam_angs.x << ", " << cam_angs.y << "\n";

		if (cam_angs.y > 89.0f)
			cam_angs.y = 89.0f;
		else if (cam_angs.y < -89.0f)
			cam_angs.y = -89.0f;

		cam.set_dir(
			cosf(glm::radians(cam_angs.x)) * cosf(glm::radians(cam_angs.y)),
			sinf(glm::radians(cam_angs.y)),
			sinf(glm::radians(cam_angs.x)) * cosf(glm::radians(cam_angs.y)));

		mouse_pos.x = xpos;
		mouse_pos.y = ypos;
	};

	app.mouse_input->add_position_callback(update_cam_dir);

	auto update_viewport = [](int width, int height)
	{
		glViewport(0, 0, width, height);
	};

	auto update_proj_mat = [&cam](int width, int height)
	{
		cam.update_proj_mat(width, height);
	};

	auto update_sensitivity = [&](int width, int height)
	{
		sensitivity = fov / width;
	};

	auto center_mouse = [&](int entered)
	{
		if (!entered)
		{
			mouse_pos = {app.size_input->width() / 2, app.size_input->height() / 2};
			glfwSetCursorPos(app.main_window, mouse_pos.x, mouse_pos.y);
		}
	};

	app.mouse_input->add_enter_exit_callback(center_mouse);

	app.size_input->add_callback(update_viewport);
	app.size_input->add_callback(update_proj_mat);
	app.size_input->add_callback(update_sensitivity);

	update_proj_mat(app.size_input->width(), app.size_input->height());
	update_sensitivity(app.size_input->width(), app.size_input->height());
	update_viewport(app.size_input->width(), app.size_input->height());

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	glFrontFace(GL_CCW);

	float last = 0;
	float now;
	float dt;

	while (!glfwWindowShouldClose(app.main_window))
	{
		now = glfwGetTime();
		dt = now - last;
		last = now;

		//std::cout << "\r" << std::fixed << 1 / dt << "fps";

		app.key_input->handle();

		if (app.key_input->key_state(GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			app.mouse_input->disable_position_callback(app.main_window);
			app.mouse_input->disable_enter_exit_callback(app.main_window);

			glfwSetInputMode(app.main_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			bool released = false;
			while (!glfwWindowShouldClose(app.main_window))
			{
				app.key_input->handle();

				if (released && app.key_input->key_state(GLFW_KEY_ESCAPE) == GLFW_PRESS)
					glfwSetWindowShouldClose(app.main_window, GL_TRUE);
				else if (!released && app.key_input->key_state(GLFW_KEY_ESCAPE) == GLFW_RELEASE)
					released = true;

				if (app.key_input->key_state(GLFW_KEY_ENTER) || app.key_input->key_state(GLFW_KEY_KP_ENTER))
				{
					glfwSetCursorPos(app.main_window, mouse_pos.x, mouse_pos.y);
					glfwSetInputMode(app.main_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
					break;
				}
			}

			app.mouse_input->enable_position_callback(app.main_window);
			app.mouse_input->enable_enter_exit_callback(app.main_window);
		}

		dcam = {0, 0, 0};

		if (app.key_input->key_state(GLFW_KEY_W))
			dcam -= glm::vec3(cam.dir().x, 0, cam.dir().z);
		if (app.key_input->key_state(GLFW_KEY_S))
			dcam += glm::vec3(cam.dir().x, 0, cam.dir().z);

		if (app.key_input->key_state(GLFW_KEY_A))
			dcam -= glm::vec3(cam.right().x, 0, cam.right().z);
		if (app.key_input->key_state(GLFW_KEY_D))
			dcam += glm::vec3(cam.right().x, 0, cam.right().z);

		if (app.key_input->key_state(GLFW_KEY_LEFT_SHIFT))
			dcam -= cam.up();
		if (app.key_input->key_state(GLFW_KEY_SPACE))
			dcam += cam.up();

		if (app.key_input->key_state(GLFW_KEY_W) || app.key_input->key_state(GLFW_KEY_A) ||
			app.key_input->key_state(GLFW_KEY_S) || app.key_input->key_state(GLFW_KEY_D) ||
			app.key_input->key_state(GLFW_KEY_LEFT_SHIFT) || app.key_input->key_state(GLFW_KEY_SPACE) ||
			matrix_update_switch)
		{
			if (dcam.x || dcam.y || dcam.z)
			{
				bounding_box pn = player + dt * speed * (app.key_input->key_state(GLFW_KEY_R) ? sprint_mult : 1) * glm::normalize(dcam);
				if (auto cs = collides_with(wall_bounds.begin(), wall_bounds.end(), pn); !cs.empty())
				{
					glm::vec3 cross{0, 0, 0};
					glm::vec3 inc;
					for (auto it : cs)
					{
						inc = bounding_box::intersection(pn, *it);

						cross += inc;
					}

					pn -= cross;
				}
				cam = pn.min + cam_player_off;
				player = pn;

				update_txt_coords(cam.x / mpp, cam.z / mpp);
			}
			matrix_update_switch = false;
			cam.update_view_mat();
		}

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//draw walls and floor
		sp.use();

		//send uniform variable matrices to shader
		proj.send<4, 4>(1, GL_FALSE, glm::value_ptr(cam.proj_matrix()));

		p_vao.use();

		mv_mat = cam.view_matrix() * wall_model;
		mv.send<4, 4>(1, GL_FALSE, glm::value_ptr(mv_mat));
		for (const auto &wall : walls)
		{
			wall.draw(GL_TRIANGLES);
		}

		mv_mat = cam.view_matrix() * floor_model;
		mv.send<4, 4>(1, GL_FALSE, glm::value_ptr(mv_mat));
		floor.draw(GL_TRIANGLES);

		//draw mini map

		//needed so walls don't clip over map
		glDisable(GL_DEPTH_TEST);

		mp.use();

		m_vao.use();

		ortho.send<4, 4>(1, GL_FALSE, glm::value_ptr(ortho_mat));
		map_model_uniform.send<4, 4>(1, GL_FALSE, glm::value_ptr((glm::mat4)map_model));

		glActiveTexture(GL_TEXTURE0);
		maze_txtre.use();

		map.draw(GL_TRIANGLES);

		glBindTexture(GL_TEXTURE_2D, 0);

		//draw point
		glPointSize(3);

		pt_p.use();

		pt_vao.use();

		pt_p_ortho.send<4, 4>(1, GL_FALSE, glm::value_ptr(ortho_mat));
		pt_p_model.send<4, 4>(1, GL_FALSE, glm::value_ptr((glm::mat4)map_model));
		pt_p_col.send<float>(0.f, 0.f, 204 / 255.f, 1.0);

		pt.draw(GL_POINTS);

		glPointSize(1);
		glEnable(GL_DEPTH_TEST);

		glfwSwapBuffers(app.main_window);
	}
	std::cout << "\n";
}