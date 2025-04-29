#version 460 core

in vec4 outColor;
in vec2 outTexCoords;

out vec4 fragColor;

layout(binding=0, std430) readonly buffer data
{
    mat4 viewProj;
    vec4 tint;
    vec2 coordOffset;
};

layout(location=0) uniform sampler2D tex;

void main()
{
    vec2 size = textureSize(tex, 0);
    vec4 texCol = texture(tex, outTexCoords * (1.0 / size));
    fragColor = outColor * texCol * tint;
}
