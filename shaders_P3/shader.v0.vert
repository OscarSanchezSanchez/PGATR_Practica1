#version 430

in vec3 inPos;
in vec4 inColor;
//in vec3 inNormal;

//out vec3 vNormal;
out vec4 vColor;

uniform mat4 modelViewProj;

void main()
{
	//vNormal = inNormal;
	gl_Position = vec4(inPos,1.0f);
	vColor = inColor;
}
