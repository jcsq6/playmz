const char *frag_src = R"(
#version 430

in vec4 col;

out vec4 color;

void main(void){
    color = col;
}
)";