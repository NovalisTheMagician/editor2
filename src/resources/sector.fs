#version 460 core

#extension GL_ARB_bindless_texture : require

in vec4 outColor;
in vec2 outTexCoords;
in flat int outTexId;

out vec4 fragColor;

layout(binding=0, std430) readonly buffer data
{
    mat4 viewProj;
    vec4 tint;
    vec2 coordOffset;
};
layout(binding=1, std430) readonly buffer textures_
{
    uvec2 textures[];
};

void main()
{
    sampler2D tex = sampler2D(textures[outTexId]);
    vec2 size = textureSize(tex, 0);
    vec4 texCol = texture(tex, outTexCoords * (1.0 / size));
    fragColor = outColor * texCol * tint;
}
