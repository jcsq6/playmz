#pragma once
#include <vector>
#include <algorithm>
#include <variant>
#include <tuple>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "buffers.h"

class mesh
{
public:
	mesh() {}
	mesh(std::vector<float> &&verts, std::vector<unsigned int> &&indices) : vs{std::move(verts)}, is{std::move(indices)}, c{0, 0, 0}
	{
		for (int i = 0; i < vs.size(); i += 3)
		{
			c.x += vs[i + 0];
			c.y += vs[i + 1];
			c.z += vs[i + 2];
		}
		c /= vs.size() / 3;
	}

	const std::vector<float> &vertices() const
	{
		return vs;
	}

	const std::vector<unsigned int> &indices() const
	{
		return is;
	}

	std::vector<float> &vertices()
	{
		return vs;
	}

	std::vector<unsigned int> &indices()
	{
		return is;
	}

	glm::vec3 center() const
	{
		return c;
	}

protected:
	std::vector<float> vs;
	std::vector<unsigned int> is;

	glm::vec3 c;
};

class model : public glm::mat4
{
public:
	model() : glm::mat4(1) {}

	using glm::mat4::mat;
	using glm::mat4::operator=;

	void translate(const glm::vec3 &offset)
	{
		*this *= glm::translate(glm::mat4(1), offset);
	}

	void scale(const glm::vec3 &scaler)
	{
		*this *= glm::scale(glm::mat4(1), scaler);
	}

	void rotate(float radians, const glm::vec3 &axis, const glm::vec3 &center = {0, 0, 0})
	{
		translate(center);
		*this *= glm::rotate(glm::mat4(1), radians, axis);
		translate(-center);
	}

	friend glm::mat4 operator*(const glm::mat4 &o, const model &m)
	{
		return o * static_cast<glm::mat4>(m);
	}

	friend glm::mat4 operator*(const model &m, const glm::mat4 &o)
	{
		return static_cast<glm::mat4>(m) * o;
	}
};

template <typename T>
struct GL_t
{
	constexpr GLenum type()
	{
		if (std::is_same_v<T, GLfloat>)
			return GL_FLOAT;
		else if (std::is_same_v<T, GLbyte>)
			return GL_BYTE;
		else if (std::is_same_v<T, GLubyte>)
			return GL_UNSIGNED_BYTE;
		else if (std::is_same_v<T, GLshort>)
			return GL_SHORT;
		else if (std::is_same_v<T, GLushort>)
			return GL_UNSIGNED_SHORT;
		else if (std::is_same_v<T, GLint>)
			return GL_INT;
		else if (std::is_same_v<T, GLuint>)
			return GL_UNSIGNED_INT;
	}
};

using GL_TYPE_t = std::variant<GL_t<GLfloat>, GL_t<GLbyte>, GL_t<GLubyte>, GL_t<GLshort>, GL_t<GLushort>, GL_t<GLint>, GL_t<GLuint>>;

constexpr GLenum type(GL_TYPE_t t)
{
	if (t.index() == 0)
		return std::get<0>(t).type();
	else if (t.index() == 1)
		return std::get<1>(t).type();
	else if (t.index() == 2)
		return std::get<2>(t).type();
	else if (t.index() == 3)
		return std::get<3>(t).type();
	else if (t.index() == 4)
		return std::get<4>(t).type();
	else if (t.index() == 5)
		return std::get<5>(t).type();
	else if (t.index() == 6)
		return std::get<6>(t).type();
	return 0;
}

template <GLenum t>
class buffer_data
{
public:
	static constexpr GLenum target = t;

	template <typename T>
	buffer_data(const T *data, int num_elements, int element_sz, int location, GLenum usage) : b{make_buffer<target>()}, element_size{element_sz}, element_count{num_elements}, loc{location}
	{
		type = GL_t<T>{};
		b.use();
		b.attach_data(element_count * element_size * sizeof(T), data, usage);
	}

	buffer<target> b;
	GL_TYPE_t type;
	int element_count;
	int element_size;
	int loc;
};

template <>
class buffer_data<GL_ELEMENT_ARRAY_BUFFER>
{
public:
	static constexpr GLenum target = GL_ELEMENT_ARRAY_BUFFER;

	template <typename T>
	buffer_data(const T *data, int buffer_sz, GLenum usage) : b{make_buffer<GL_ELEMENT_ARRAY_BUFFER>()}, element_count{buffer_sz}
	{
		type = GL_t<T>{};
		b.use();
		b.attach_data(element_count * sizeof(T), data, usage);
	}

	buffer<GL_ELEMENT_ARRAY_BUFFER> b;
	GL_TYPE_t type;
	int element_count;
};

template <typename... Ts>
class obj
{
public:
	obj(Ts &&...buffers) : buffs{std::forward<Ts>(buffers)...}
	{
	}

	void draw(GLenum primitive_type) const
	{
		loop_draw(primitive_type, buffs);
	}

	const auto &buffers() const
	{
		return buffs;
	}

	auto &buffers()
	{
		return buffs;
	}

private:
	std::tuple<Ts...> buffs;

	template <int i = 0>
	static void loop_draw(GLenum primitive_type, const std::tuple<Ts...> &bs)
	{
		bool has_ebo = false;
		const buffer_data<ebo_target> *e = nullptr;
		int sz;

		//doesn't handle anything besides vbos and ebos so far
		if constexpr (std::get<i>(bs).target == vbo_target)
		{
			std::get<i>(bs).b.use();
			glVertexAttribPointer(std::get<i>(bs).loc, std::get<i>(bs).element_size, type(std::get<i>(bs).type), GL_FALSE, 0, 0);
			glEnableVertexAttribArray(std::get<i>(bs).loc);
			sz = std::get<i>(bs).element_count;
		}
		else if constexpr (std::get<i>(bs).target == ebo_target)
		{
			has_ebo = true;
			e = &std::get<i>(bs);
		}

		if constexpr (i + 1 < sizeof...(Ts))
		{
			loop_draw<i + 1>(primitive_type, bs);
		}
		else
		{
			if (has_ebo)
			{
				std::get<i>(bs).b.use();
				glDrawElements(primitive_type, e->element_count, type(e->type), 0);
			}
			else
				glDrawArrays(primitive_type, 0, sz);
		}
	}
};

namespace std
{
	template <std::size_t i, typename... Ts>
	const auto &get(const obj<Ts...> &obj)
	{
		return std::get<i>(obj.buffers());
	}

	template <std::size_t i, typename... Ts>
	auto &get(obj<Ts...> &obj)
	{
		return std::get<i>(obj.buffers());
	}
}