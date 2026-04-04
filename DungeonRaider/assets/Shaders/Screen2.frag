#version 450 core

in vec2 vTexCoord;
in float vTexSlot;
in vec4 vColorTint;

out vec4 FragColor;

uniform sampler2D uTextures[32];
uniform float uTime;

void main()
{
    int slot = int(vTexSlot);
    vec2 uv = vTexCoord;

    vec4 color = texture(uTextures[slot], uv);
    vec2 centeredUV = uv - vec2(0.5);
    float dist = length(centeredUV) * 0.4;
    float brightness = clamp(1.0 - dist, 0.0, 1.0);

    brightness = pow(brightness, 3);
    FragColor = vec4(color.rgb * brightness, color.a);
}