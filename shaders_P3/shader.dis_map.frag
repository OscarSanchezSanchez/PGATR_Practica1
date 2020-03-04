#version 400 core

out vec4 outColor;

in vec3 teNormal;
in vec3 tePos;
in vec2 teTexCoord;


uniform sampler2D colorTex;
uniform sampler2D emiTex;
//uniform sampler2D normalTex;
//uniform sampler2D specularTex;
//uniform sampler2D auxiliarTex;

uniform mat4 modelView;

//Intensidad ambiental
vec3 Ia = vec3(0.1);

//propiedades de la fuente de luz (direccional)
vec3 IL = vec3(1);
vec4 DL = vec4(-1,0,0,0); 


//Propiedades del objeto
vec3 Ka = vec3(1,0,0);
vec3 Kd = vec3(1,0,0);
vec3 Ks = vec3(1);
float n = 100.0;
vec3 Ke = vec3(0);

vec3 N;

vec3 shade()
{	
	vec3 cf = vec3(0);

	////Ambiental////
	cf += Ia * Ka;

	////Difuso////
	vec3 L = normalize(DL.xyz);
	cf += clamp(IL*Kd*dot(teNormal,L),0,1);

	////Especular////
	
	//vec3 V = normalize(-tePos); 
	//vec3 R = reflect(-L,N);
	//float fs = pow(max(0,dot(R,V)),n);
	//cf += IL*Ks*fs;
	
	
	return cf;
}

void main()
{
	Kd = texture(colorTex,teTexCoord).rgb;
	Ka = Kd;
	Ks = vec3(0.0);/*texture(specularTex,teTexCoord).rgb;*/
	Ke = texture(emiTex,teTexCoord).rgb;
	N = normalize(teNormal);
	//outColor = vec4(shade(),1.0); 
	outColor = vec4(texture(colorTex,teTexCoord).rgb,1);
}
