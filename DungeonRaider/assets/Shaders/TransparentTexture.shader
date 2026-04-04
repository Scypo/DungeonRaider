#shader vertex
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in float aInstanceIndex;

struct InstanceData 
{
    mat4 transform;
    vec4 colorTint;
    float textureSlot;
    float padding[3];
};

layout(std140, binding = 0) uniform CameraBuffer 
{
    mat4 view;
    mat4 projection;
};

layout(std430, binding = 1)readonly buffer instanceData
{
    InstanceData instances[];
};

out vec2 vTexCoord;
out float vTexSlot;
out vec4 vColorTint;

void main()
{
    InstanceData data = instances[int(aInstanceIndex)];
    gl_Position = projection * view * data.transform * vec4(aPosition, 1.0);

    vTexCoord = aTexCoord;
    vTexSlot = data.textureSlot;
    vColorTint = data.colorTint;
}

#shader fragment
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

    FragColor = finalColor;
}
