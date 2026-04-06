#version 330 core

layout(location = 0) in vec3 aPosition;

layout(location = 1) in vec4 iUV;
layout(location = 2) in vec4 iColor;
layout(location = 3) in mat4 iModel;		

layout(std140) uniform CameraBuffer 
{
    mat4 view;
    mat4 projection;
};

out vec2 vTexCoord;	
out vec4 vColorTint;

void main()
{
    gl_Position = projection * view * iModel * vec4(aPosition, 1.0);
	int corner = gl_VertexID & 3;
	vec2 uv;
   
	uv.x = mix(iUV.x, iUV.y, aPosition.x);
	uv.y = mix(iUV.w, iUV.z, aPosition.y);
	vTexCoord = uv;
    
    vColorTint = iColor;
}