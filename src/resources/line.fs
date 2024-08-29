#version 460 core

in vec4 outColor;

out vec4 fragColor;

layout(binding=0, std430) readonly buffer data
{
    mat4 viewProj;
    vec4 tint;
};

void main() {
   fragColor = outColor * tint;
}
