#version 460 core
layout(location=0) in vec2 inPosition;
uniform mat4 viewProj;
uniform float offset;
uniform float period;
flat out int idx;
void main() {
   float off = period * gl_InstanceID + offset;
   vec2 pos = inPosition + vec2(off, 0);
   gl_Position = viewProj * vec4(pos, 0, 1);
   idx = gl_InstanceID;
}
