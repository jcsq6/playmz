#pragma once
#include "image.h"
#include <GL/glew.h>

class texture
{
public:
    texture(const rgba_image &i, GLint wrap = GL_REPEAT, const float *border_color = nullptr)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (border_color)
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA2, i.image_width(), i.image_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, i.data());

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void use() const
    {
        glBindTexture(GL_TEXTURE_2D, id);
    }

    ~texture()
    {
        glDeleteTextures(1, &id);
    }

private:
    GLuint id;
};