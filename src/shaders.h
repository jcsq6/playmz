#pragma once
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <type_traits>
#include <memory>

class shader
{
public:
    shader() noexcept
    {
        s = 0;
    }

    shader(const shader &) = delete;
    shader(shader &&other) noexcept
    {
        s = other.s;
        other.s = 0;
    }

    shader &operator=(const shader &) = delete;
    shader &operator=(shader &&other) noexcept
    {
        s = other.s;
        other.s = 0;
        return *this;
    }

    void create(int shader_type)
    {
        if (!s)
        {
            s = glCreateShader(shader_type);
        }
    }

    void attach_source(const char *source) const
    {
        glShaderSource(s, 1, &source, nullptr);
    }

    void compile() const
    {
        glCompileShader(s);

        GLint completed = 0;

        glGetShaderiv(s, GL_COMPILE_STATUS, &completed);
        if (completed == GL_FALSE)
        {
            std::cout << "shader compilation failed!\n";
            int length = 0;
            int d = 0;
            char *log;
            glGetShaderiv(s, GL_INFO_LOG_LENGTH, &length);
            if (length)
            {
                log = new char[length];
                glGetShaderInfoLog(s, length, &d, log);
                std::cout << "Shader Log: " << log << "\n";
                delete[] log;
            }
        }
    }

    void attach_to_program(GLuint prog)
    {
        glAttachShader(prog, s);
        clean();
    }

    void clean()
    {
        glDeleteShader(s);
        s = 0;
    }

    bool is_ctreated() const
    {
        return s;
    }

    ~shader()
    {
        glDeleteShader(s);
        s = 0;
    }

private:
    GLuint s;
};

shader make_shader(const char *source, int shader_type)
{
    shader res;
    res.create(shader_type);
    res.attach_source(source);
    res.compile();
    return res;
}

class uniform
{
public:
    uniform() = default;
    uniform(const uniform &) = default;
    uniform(uniform &&) = default;

    uniform &operator=(const uniform &) = default;
    uniform &operator=(uniform &&) = default;

    uniform(const char *name, GLuint program)
    {
        get(name, program);
    }

    void get(const char *name, GLuint program)
    {
        loc = glGetUniformLocation(program, name);
    }

    GLint location() const
    {
        return loc;
    }

    template <typename T>
    void send(T v0)
    {
        static_assert(std::is_same<T, GLfloat>::value || std::is_same<T, GLint>::value || std::is_same<T, GLuint>::value, "glUniform only accepts certain types");

        if constexpr (std::is_same<T, GLfloat>::value)
        {
            glUniform1f(loc, v0);
        }
        else if constexpr (std::is_same<T, GLint>::value)
        {
            glUniform1i(loc, v0);
        }
        else if constexpr (std::is_same<T, GLuint>::value)
        {
            glUniform1ui(loc, v0);
        }
    }

    template <typename T>
    void send(T v0, T v1)
    {
        static_assert(std::is_same<T, GLfloat>::value || std::is_same<T, GLint>::value || std::is_same<T, GLuint>::value, "glUniform only accepts certain types");

        if constexpr (std::is_same<T, GLfloat>::value)
        {
            glUniform2f(loc, v0, v1);
        }
        else if constexpr (std::is_same<T, GLint>::value)
        {
            glUniform2i(loc, v0, v1);
        }
        else if constexpr (std::is_same<T, GLuint>::value)
        {
            glUniform2ui(loc, v0, v1);
        }
    }

    template <typename T>
    void send(T v0, T v1, T v2)
    {
        static_assert(std::is_same<T, GLfloat>::value || std::is_same<T, GLint>::value || std::is_same<T, GLuint>::value, "glUniform only accepts certain types");

        if constexpr (std::is_same<T, GLfloat>::value)
        {
            glUniform3f(loc, v0, v1, v2);
        }
        else if constexpr (std::is_same<T, GLint>::value)
        {
            glUniform3i(loc, v0, v1, v2);
        }
        else if constexpr (std::is_same<T, GLuint>::value)
        {
            glUniform3ui(loc, v0, v1, v2);
        }
    }

    template <typename T>
    void send(T v0, T v1, T v2, T v3)
    {
        static_assert(std::is_same<T, GLfloat>::value || std::is_same<T, GLint>::value || std::is_same<T, GLuint>::value, "glUniform only accepts certain types");

        if constexpr (std::is_same<T, GLfloat>::value)
        {
            glUniform4f(loc, v0, v1, v2, v3);
        }
        else if constexpr (std::is_same<T, GLint>::value)
        {
            glUniform4i(loc, v0, v1, v2, v3);
        }
        else if constexpr (std::is_same<T, GLuint>::value)
        {
            glUniform4ui(loc, v0, v1, v2, v3);
        }
    }

    template <glm::length_t components, typename T>
    void send(GLsizei count, const T *value)
    {
        static_assert(std::is_same<T, GLfloat>::value || std::is_same<T, GLint>::value || std::is_same<T, GLuint>::value, "glUniform only supports certain types");
        static_assert(components <= 4 && components >= 1, "glUniform doesn't handle data over 4 long");

        if constexpr (std::is_same<T, GLfloat>::value)
        {
            if (components == 1)
                glUniform1fv(loc, count, value);
            if (components == 2)
                glUniform2fv(loc, count, value);
            if (components == 3)
                glUniform3fv(loc, count, value);
            if (components == 4)
                glUniform4fv(loc, count, value);
        }
        else if constexpr (std::is_same<T, GLint>::value)
        {
            if (components == 1)
                glUniform1iv(loc, count, value);
            if (components == 2)
                glUniform2iv(loc, count, value);
            if (components == 3)
                glUniform3iv(loc, count, value);
            if (components == 4)
                glUniform4iv(loc, count, value);
        }
        else if constexpr (std::is_same<T, GLuint>::value)
        {
            if (components == 1)
                glUniform1uiv(loc, count, value);
            if (components == 2)
                glUniform2uiv(loc, count, value);
            if (components == 3)
                glUniform3uiv(loc, count, value);
            if (components == 4)
                glUniform4uiv(loc, count, value);
        }
    }

    template <glm::length_t width, glm::length_t height>
    void send(GLsizei count, GLboolean transpose, const GLfloat *value)
    {
        fun_whfv<width, height>()(loc, count, transpose, value);
    }

private:
    GLint loc;

    template <glm::length_t width, glm::length_t height>
    constexpr static auto fun_whfv()
    {
        static_assert(width <= 4 && width > 1 && height <= 4 && height > 1, "glUniform doesn't handle data over 4 long");

        if (width == 2)
        {
            if (height == 2)
                return glUniformMatrix2fv;
            if (height == 3)
                return glUniformMatrix2x3fv;
            if (height == 4)
                return glUniformMatrix2x4fv;
        }
        if (width == 3)
        {
            if (height == 2)
                return glUniformMatrix3x2fv;
            if (height == 3)
                return glUniformMatrix3fv;
            if (height == 4)
                return glUniformMatrix3x4fv;
        }
        if (width == 4)
        {
            if (height == 2)
                return glUniformMatrix4x2fv;
            if (height == 3)
                return glUniformMatrix4x3fv;
            if (height == 4)
                return glUniformMatrix4fv;
        }
    }
};

class program
{
public:
    program() noexcept
    {
        p = 0;
    }
    program(const program &) = delete;
    program(program &&other) noexcept
    {
        p = other.p;
        other.p = 0;
    }

    program &operator=(const program &) = delete;
    program &operator=(program &&other) noexcept
    {
        p = other.p;
        other.p = 0;
        return *this;
    }

    void create()
    {
        if (!p)
        {
            p = glCreateProgram();
        }
    }

    void attach_shader(shader &&s) const
    {
        s.attach_to_program(p);
    }

    template <typename... Shs>
    void attach_shaders(Shs &&...shaders) const
    {
        (shaders.attach_to_program(p), ...);
    }

    void link() const
    {
        glLinkProgram(p);

        GLint completed = 0;

        glGetProgramiv(p, GL_LINK_STATUS, &completed);
        if (completed == GL_FALSE)
        {
            int length = 0;
            int d = 0;
            char *log;
            glGetProgramiv(p, GL_INFO_LOG_LENGTH, &length);
            if (length)
            {
                log = new char[length];
                glGetProgramInfoLog(p, length, &d, log);
                std::cout << "Program Log: " << log << "\n";
                delete[] log;
            }
        }
    }

    void clean()
    {
        glDeleteProgram(p);
        p = 0;
    }

    void use() const
    {
        glUseProgram(p);
    }

    bool is_created() const
    {
        return p;
    }

    static void define_attribute(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)
    {
        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    }

    GLint get_uniform_loc(const char *name) const
    {
        return glGetUniformLocation(p, name);
    }

    uniform get_uniform(const char *name) const
    {
        return uniform{name, p};
    }

    operator GLuint() const
    {
        return p;
    }

    ~program()
    {
        glDeleteProgram(p);
        p = 0;
    }

private:
    GLuint p;
};

template <typename... Shs>
program make_program(Shs &&...shaders)
{
    program res;
    res.create();
    res.attach_shaders(shaders...);
    res.link();
    return res;
}