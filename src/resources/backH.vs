#version 460 core

layout(location=0) in vec2 inPosition;

flat out int idx;

layout(binding=0, std430) readonly buffer data
{
    mat4 viewProj;
    vec4 tint;
    vec4 majorTint;
    float hOffset, vOffset, period;
    int majorIndex;
};

void main() {
   float off = period * gl_InstanceID + hOffset;
   vec2 pos = inPosition + vec2(0, off);
   gl_Position = viewProj * vec4(pos, 0, 1);
   idx = gl_InstanceID;
}
