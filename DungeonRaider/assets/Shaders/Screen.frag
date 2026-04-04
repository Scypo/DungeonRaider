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

    float scanline = sin(uv.y * 800.0 + uTime * 10.0) * 0.04;
    float offset = sin(uTime * 3.0 + uv.y * 10.0) * 0.003; 
    float r = texture(uTextures[slot], uv + vec2(offset, 0.0)).r;
    float g = texture(uTextures[slot], uv).g;
    float b = texture(uTextures[slot], uv - vec2(offset, 0.0)).b;

    vec3 glitchColor = vec3(r, g, b);
    glitchColor *= 0.95 + scanline;
    FragColor = vec4(glitchColor, 1.0) * vColorTint;
}
