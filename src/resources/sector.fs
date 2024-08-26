#version 460 core
in vec4 outColor;
in vec2 outTexCoords;
out vec4 fragColor;
uniform sampler2D tex;
uniform vec4 tint;
void main() {
   fragColor = outColor * texture(tex, outTexCoords) * tint;
}
