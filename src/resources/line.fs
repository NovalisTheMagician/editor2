#version 460 core
in vec4 outColor;
out vec4 fragColor;
uniform vec4 tint;
void main() {
   fragColor = outColor * tint;
}
