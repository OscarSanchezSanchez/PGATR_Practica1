#version 430

in vec3 inPos;
//in vec3 inNormal;

//out vec3 vNormal;

uniform mat4 modelViewProj;

void main()
{
	//vNormal = inNormal;
	gl_Position = modelViewProj * vec4(inPos, 1.0);
}
