#pragma once
#include <GL/glew.h>
#include <iostream>

class vao
{
public:
	vao()
	{
		glGenVertexArrays(1, &id);
	}
	vao(const vao &) = delete;
	vao(vao &&other)
	{
		id = other.id;
		other.id = 0;
	}
	vao &operator=(const vao &) = delete;
	vao &operator=(vao &&other)
	{
		id = other.id;
		other.id = 0;
		return *this;
	}
	void use() const
	{
		glBindVertexArray(id);
	}
	static void quit()
	{
		glBindVertexArray(0);
	}
	operator GLuint() const
	{
		return id;
	}
	~vao()
	{
		id = 0;
	}

private:
	GLuint id;
};

template <GLenum t>
class buffer
{
public:
	inline static constexpr GLenum target = t;

	buffer() : id{0}
	{
	}

	buffer(const buffer &) = delete;
	buffer(buffer &&other)
	{
		id = other.id;
		other.id = 0;
	}
	buffer &operator=(const buffer &) = delete;
	buffer &operator=(buffer &&other)
	{
		id = other.id;
		other.id = 0;
	}
	void use() const
	{
		glBindBuffer(t, id);
	}
	static void quit()
	{
		glBindBuffer(t, 0);
	}
	template <typename C>
	void attach_sub_data(const C &data, GLintptr byte_offset = 0) const
	{
		use();
		glBufferSubData(t, byte_offset, data.size() * sizeof(typename C::value_type), &data[0]);
	}
	template <typename C>
	void attach_sub_data(GLintptr byte_offset, GLsizeiptr byte_size, const C *data) const
	{
		use();
		glBufferSubData(t, byte_offset, byte_size, data);
	}
	template <typename T, GLsizeiptr N>
	void attach_sub_data(T (&data)[N], GLintptr byte_offset = 0) const
	{
		use();
		glBufferSubData(t, byte_offset, sizeof(data), data);
	}

	template <typename C>
	void attach_data(const C &data, GLenum usage = GL_STATIC_DRAW) const
	{
		use();
		glBufferData(t, data.size() * sizeof(typename C::value_type), &data[0], usage);
	}
	template <typename C>
	void attach_data(GLsizeiptr byte_size, const C *data, GLenum usage = GL_STATIC_DRAW) const
	{
		use();
		glBufferData(t, byte_size, data, usage);
	}
	template <typename T, GLsizeiptr N>
	void attach_data(T (&data)[N], GLenum usage = GL_STATIC_DRAW) const
	{
		use();
		glBufferData(t, sizeof(data), data, usage);
	}
	void reserve_data(GLsizeiptr byte_size, GLenum usage = GL_STATIC_DRAW) const
	{
		use();
		glBufferData(t, byte_size, nullptr, usage);
	}

	GLuint index() const
	{
		return id;
	}

	GLuint &index()
	{
		return id;
	}

	~buffer()
	{
		id = 0;
	}

private:
	GLuint id;
};

template <GLenum t>
buffer<t> make_buffer()
{
	buffer<t> d;
	glGenBuffers(1, &d.index());
	return d;
}

#define vbo_target GL_ARRAY_BUFFER
#define ebo_target GL_ELEMENT_ARRAY_BUFFER
#define ubo_target GL_UNIFORM_BUFFER
#define ssbo_target GL_SHADER_STORAGE_BUFFER

using vbo = buffer<vbo_target>;
using ebo = buffer<ebo_target>;
using ubo = buffer<ubo_target>;
using ssbo = buffer<ssbo_target>;