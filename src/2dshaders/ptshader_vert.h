const char *pt_shader_vert_src = R"(
#version 430

layout (location = 0) in vec3 pos;

uniform mat4 ortho;
uniform mat4 model;

uniform vec4 col;

void main(void)
{
    gl_Position = ortho * model * vec4(pos, 1.0);
}
)";