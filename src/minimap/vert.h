const char *map_vert_src = R"(
#version 430

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 txt_coord;

uniform mat4 ortho;

uniform mat4 model;

out vec2 tex_coord;

void main(void)
{
    gl_Position = ortho * model * vec4(pos, 1.0);

    tex_coord = txt_coord;
}
)";