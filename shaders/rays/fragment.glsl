#version 430
in vec2 tex_coords;
out vec4 outColor;
layout(binding = 1) uniform sampler2D image;


void main() {
    vec4 color = texture(image,tex_coords);
    outColor = color;
}