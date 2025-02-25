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
	
	
	//void processOBJString(char* stringObj);
	
private:
	char* filename;
	void loadOBJFile();
};