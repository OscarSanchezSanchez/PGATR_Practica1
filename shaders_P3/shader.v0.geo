#version 430

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

in vec4 vColor[];
uniform mat4 modelView;
uniform mat4 modelViewProj;

vec3 camRight = vec3(modelView[0][0], modelView[1][0], modelView[2][0]);
vec3 camUp = vec3(modelView[0][1], modelView[1][1], modelView[2][1]);

out vec2 gTexCoord;
out vec4 gColor;

void main(){
	gTexCoord = vec2(0.0, 0.0);
	gl_Position = modelViewProj * vec4(camRight * 0.5 * 0.01 + camUp * -0.5 * 0.01 + gl_in[0].gl_Position.xyz, 1.0);
	gColor = vColor[0];
	EmitVertex();

	gTexCoord = vec2(1.0, 0.0);
	gl_Position = modelViewProj * vec4(camRight * 0.5 * 0.01 + camUp * 0.5 * 0.01 + gl_in[0].gl_Position.xyz, 1.0);
	gColor = vColor[0];
	EmitVertex();

	gTexCoord = vec2(0.0, 1.0);
	gl_Position = modelViewProj * vec4(camRight * -0.5 * 0.01 + camUp * -0.5 * 0.01 + gl_in[0].gl_Position.xyz, 1.0);
	gColor = vColor[0];
	EmitVertex();

	gTexCoord = vec2(1.0, 1.0);
	gl_Position = modelViewProj * vec4(camRight * -0.5 * 0.01 + camUp * 0.5 * 0.01 + gl_in[0].gl_Position.xyz, 1.0);
	gColor = vColor[0];
	EmitVertex();
	
	EndPrimitive();
}