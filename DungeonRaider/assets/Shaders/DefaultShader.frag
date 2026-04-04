#version 450 core

in vec2 vTexCoord;
in float vTexSlot;
in vec4 vColorTint;

out vec4 FragColor;
uniform sampler2D uTextures[32];

void main()
{
    int slot = int(vTexSlot);
    vec4 texColor = texture(uTextures[slot], vTexCoord);
    vec4 finalColor = texColor * vColorTint;

    if (finalColor.a < 0.1) discard;

    FragColor = finalColor;
}