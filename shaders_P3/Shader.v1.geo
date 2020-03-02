#version 330 core

layout(triangles) in;

// Three lines will be generated: 6 vertices
layout(line_strip, max_vertices=6) out;

//uniform float normal_length;
float normal_length = 1.0;

uniform mat4 modelViewProj;

in vec3 vNormal[3];

void main()
{
  for(int i=0; i < gl_in.length(); i++)
  {
    gl_Position = modelViewProj * gl_in[i].gl_Position;
    vec3 N = vNormal[i];
    EmitVertex();
    
    gl_Position = modelViewProj * (gl_in[i].gl_Position + vec4(N * normal_length, 0.0));
    EmitVertex();
    
    EndPrimitive();
  }
}