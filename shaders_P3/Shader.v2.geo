#version 400

layout(triangles) in;
layout(line_strip, max_vertices=8) out;

uniform mat4 modelViewProj;

in vec3 teNormal[3];

float normal_length = 1.0;

void main()
{
  //------ 3 lines for the 3 vertex normals
  //
  for(int i = 0; i < gl_in.length(); i++)
  {
    vec3 N = teNormal[i];
    
    gl_Position = modelViewProj * gl_in[i].gl_Position;
    EmitVertex();
    
    gl_Position = modelViewProj * (gl_in[i].gl_Position + vec4(N * normal_length, 0.0));
    EmitVertex();
    
    EndPrimitive();
  }
  

  //------ One line for the face normal
  //
  vec3 P0 = gl_in[0].gl_Position.xyz;
  vec3 P1 = gl_in[1].gl_Position.xyz;
  vec3 P2 = gl_in[2].gl_Position.xyz;
  
  vec3 V0 = P0 - P1;
  vec3 V1 = P2 - P1;
  
  vec3 N = cross(V1, V0);
  N = normalize(N);
  
  // Center of the triangle
  vec3 P = (P0+P1+P2) / 3.0;
  
  gl_Position = modelViewProj * vec4(P, 1.0);
  EmitVertex();
  
  gl_Position = modelViewProj * vec4(P + N * normal_length, 1.0);
  EmitVertex();
  EndPrimitive();
}