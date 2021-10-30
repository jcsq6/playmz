const char *vert_src = R"(
#version 430

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 vertex_cols;

uniform mat4 mv_mat;
uniform mat4 proj_mat;

out vec4 col;

void main(void){
    gl_Position = proj_mat * mv_mat * vec4(pos, 1.0);
    col = vec4(vertex_cols, 1.0);
}
)";