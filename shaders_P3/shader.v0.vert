#version 400

in vec3 inPos;
in vec3 inNormal;

out vec3 vNormal;
out vec2 texCoord;

uniform mat4 modelViewProj;

void main()
{
	vNormal = inNormal;
	gl_Position = vec4(inPos, 1.0);
}
