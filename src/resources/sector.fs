#version 460 core

in vec4 outColor;
in vec2 outTexCoords;
in vec2 outScreenPos;

out vec4 fragColor;

layout(binding=0, std430) readonly buffer data
{
    mat4 viewProj;
    vec4 tint;
    vec2 coordOffset;
    float zoom;
};

layout(location=0) uniform sampler2D tex;

float hatchLineAA(vec2 pos, float angle, float spacing, float thickness)
{
    float c = cos(angle);
    float s = sin(angle);

    float d = pos.x * c + pos.y * s;

    float coord = mod(d, spacing);
    float aa = fwidth(d);

    float distToLine = min(coord, spacing - coord);
    float halfWidth = max(thickness, aa * 0.5);
    return 1.0 - smoothstep(halfWidth - aa, halfWidth + aa, distToLine);
}

float hatchLine(vec2 pos, float angle, float spacing, float thickness)
{
    float c = cos(angle);
    float s = sin(angle);

    float d = pos.x * c + pos.y * s;

    float coord = mod(d, spacing);

    float distToLine = min(coord, spacing - coord);

    float px = fwidth(d);
    float halfWidth = max(thickness, px * 0.5);

    return 1.0 - step(halfWidth, distToLine);
}

const float hatchAngle = 0.785398;
const float hatchSpacing = 16;
const float hatchThickness = 0.2;

void main()
{
    vec2 size = textureSize(tex, 0);
    vec4 texCol = texture(tex, outTexCoords * (1.0 / size));

    float mask1 = hatchLine(outScreenPos, hatchAngle, hatchSpacing, hatchThickness);
    float mask2 = hatchLine(outScreenPos, hatchAngle + 1.570796, hatchSpacing, hatchThickness);
    float mask = max(mask1, mask2);

    vec4 color = texCol;
    color.rgb = mix(color.rgb, outColor.rgb, mask * outColor.a);
    fragColor = color * tint;
}
