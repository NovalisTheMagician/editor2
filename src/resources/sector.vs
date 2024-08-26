#version 460 core
layout(location=0) in vec2 inPosition;
layout(location=1) in vec4 inColor;
layout(location=2) in vec2 inTexCoords;
out vec4 outColor;
out vec2 outTexCoords;
uniform mat4 viewProj;
uniform vec2 coordOffset;
void main() {
   gl_Position = viewProj * vec4(inPosition, 0, 1);
   outColor = inColor;
   outTexCoords = inTexCoords;
}
