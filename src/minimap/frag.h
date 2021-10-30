const char *map_frag_src = R"(
#version 430

layout (binding = 0) uniform sampler2D image;

in vec2 tex_coord;

out vec4 color;

void main(void){
    color = texture(image, tex_coord);
}
)";