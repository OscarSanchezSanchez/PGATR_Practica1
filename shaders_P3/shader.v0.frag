#version 430
in vec2 gTexCoord;
out vec4 outColor;
uniform sampler2D alphaTex;

void main()
{
	vec2 c = vec2(1,0);	
	outColor = texture(alphaTex, gTexCoord);
	float r = pow((gTexCoord.x - 0.5),2);
	float s = pow((gTexCoord.y - 0.5),2);
	if(r+s > 0.2)
	{
		discard;
	}

}
