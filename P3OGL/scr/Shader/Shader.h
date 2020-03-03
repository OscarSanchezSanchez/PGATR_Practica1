#pragma once
#include <glm\fwd.hpp>
#include <glm/glm.hpp>
#include <vector>
class Shader
{
public:

	unsigned int vshader;
	unsigned int gshader;
	unsigned int fshader;
	unsigned int ID_program;

	unsigned int VAO, posVBO, normalVBO, textCoordVBO, triangleIndexVBO;

	//Uniform matrix
	int uModelViewMat;
	int uModelViewProjMat;
	int uNormalMat;
	int uProjectionMatrix;


	//Texturas Uniform  
	int uColorTex;
	int uEmiTex;

	//identificadores
	unsigned int inPos;
	unsigned int inColor;
	unsigned int inNormal;
	unsigned int inTexCoord;

	//Luz
	int uLightPosition;
	int uLightIntensity;

	glm::vec3 lightPosition;
	glm::vec3 lightIntensity;
};

