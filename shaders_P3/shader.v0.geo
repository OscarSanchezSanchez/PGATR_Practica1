#version 330 core

layout(triangles) in;
layout(line_strip, max_vertices=3) out;

uniform mat4 modelViewProj;

void main(){
	for (int i = 0; i < gl_in.length(); i++)
	{
		gl_Position = modelViewProj * gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}