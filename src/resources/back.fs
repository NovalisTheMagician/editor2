#version 460 core

flat in int idx;

out vec4 fragColor;

layout(binding=0, std430) readonly buffer data
{
    mat4 viewProj;
    vec4 tint;
    vec4 majorTint;
    float hOffset, vOffset, period;
    int majorIndex;
};

void main() {
   if(idx == majorIndex)
       fragColor = majorTint;
   else
       fragColor = tint;
}
