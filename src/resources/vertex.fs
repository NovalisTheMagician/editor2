#version 460 core
in vec4 outColor;
out vec4 fragColor;
uniform vec4 tint;
void main() {
   vec2 coord = gl_PointCoord - vec2(0.5);
   if(length(coord) > 0.5)
       discard;
   fragColor = outColor * tint;
}
