#version 400 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec3 inNormal;

uniform mat4 model;

out vec3 vPos;
out vec2 vTexCoord;
out vec3 vNormal;

void main()
{
    vPos = inPos;
    vTexCoord = inTexCoord;
    vNormal = inNormal;
}