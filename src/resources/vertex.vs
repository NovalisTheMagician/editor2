#version 460 core

layout(location=0) in vec2 inPosition;
layout(location=1) in vec4 inColor;

out vec4 outColor;

layout(binding=0, std430) readonly buffer data
{
    mat4 viewProj;
    vec4 tint;
};

void main() {
   gl_Position = viewProj * vec4(inPosition, 0, 1);
   outColor = inColor;
}
