#version 430
in vec2 gTexCoord;
out vec4 outColor;
uniform sampler2D alphaTex;

void main()
{
	vec2 c = vec2(1,0);	
	outColor = texture(alphaTex, gTexCoord);  

}
