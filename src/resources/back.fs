#version 460 core
uniform vec4 tint;
uniform vec4 majorTint;
uniform int majorIndex;
flat in int idx;
out vec4 fragColor;
void main() {
   if(idx == majorIndex)
       fragColor = majorTint;
   else
       fragColor = tint;
}
