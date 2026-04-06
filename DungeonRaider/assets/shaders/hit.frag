#version 330 core

in vec2 vTexCoord;
in vec4 vColorTint;

out vec4 FragColor;
uniform sampler2D uTexture;

void main()
{
    vec4 texColor = texture(uTexture, vTexCoord);
    if (vColorTint.a * texColor.a < 0.1) discard;

    FragColor = vec4(1.0,1.0,1.0, texColor.a * vColorTint.a);
}