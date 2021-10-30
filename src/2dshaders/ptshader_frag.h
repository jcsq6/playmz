const char *pt_shader_frag_src = R"(
#version 430

out vec4 color;

uniform vec4 col;

void main(void){

    color = col;
}
)";