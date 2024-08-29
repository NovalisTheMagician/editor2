#version 460 core

in vec4 outColor;
in vec2 outTexCoords;

out vec4 fragColor;

layout(location=0) uniform sampler2D tex;
layout(binding=0, std430) readonly buffer data
{
    mat4 viewProj;
    vec4 tint;
    vec2 coordOffset;
};

void main() {
   fragColor = outColor * texture(tex, outTexCoords) * tint;
}
