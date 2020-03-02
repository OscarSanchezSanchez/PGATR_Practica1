#pragma once
#include <glm\fwd.hpp>
#include <glm/glm.hpp>
#include <vector>
class Model
{
public:
	Model(char* filename);
	std::vector < glm::vec3 > vertices;
	std::vector < glm::vec2 > uvs;
	std::vector < glm::vec3 > normals;
	std::vector <int> indices;
	
	//cosas que van en el shader
	struct program
	{
		unsigned int vshader;
		unsigned int gshader;
		unsigned int fshader;
		unsigned int program;
	};

	unsigned int vao;
	unsigned int posVBO;
	unsigned int colorVBO;
	unsigned int normalVBO;
	unsigned int texCoordVBO;
	unsigned int triangleIndexVBO;

	//uniform 
	unsigned int uModelViewMat;
	unsigned int uModelViewProjMat;
	unsigned int uNormalMat;
	unsigned int uProjectionMatrix;
	unsigned int uLightPosition;
	unsigned int uLightIntensity;
	
	//Texturas Uniform  
	int uColorTex;
	int uEmiTex;

	//identificadores
	unsigned int inPos;
	unsigned int inColor;
	unsigned int inNormal;
	unsigned int inTexCoord;
	
private:
	char* filename;
	void loadOBJFile();
};