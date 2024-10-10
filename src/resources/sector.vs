#version 460 core

layout(location=0) in vec2 inPosition;
layout(location=1) in vec4 inColor;
layout(location=2) in vec2 inTexCoords;
layout(location=3) in int texId;

out vec4 outColor;
out vec2 outTexCoords;
out flat int outTexId;

layout(binding=0, std430) readonly buffer data
{
    mat4 viewProj;
    vec4 tint;
    vec2 coordOffset;
};

void main() {
   gl_Position = viewProj * vec4(inPosition, 0, 1);
   outColor = inColor;
   outTexCoords = inTexCoords;
   outTexId = texId;
}
