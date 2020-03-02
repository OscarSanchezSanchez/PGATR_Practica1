#include "Model.h"

#include "OBJ_loader.h"

Model::Model(char*  filename)
{
	this->filename = filename;
	this->loadOBJFile();
}

void Model::loadOBJFile()
{
	// Initialize Loader
	objl::Loader Loader;

	// Load .obj File
	bool loadout = Loader.LoadFile(this->filename);
	if (loadout)
	{
		for (unsigned int i = 0; i < Loader.LoadedMeshes.size(); i++)
		{
			// Copy one of the loaded meshes to be our current mesh
			objl::Mesh curMesh = Loader.LoadedMeshes[i];
			for (int j = 0; j < curMesh.Vertices.size(); j++)
			{
				glm::vec3 vector;

				//position
				vector.x = curMesh.Vertices[j].Position.X;
				vector.y = curMesh.Vertices[j].Position.Y;
				vector.z = curMesh.Vertices[j].Position.Z;
				this->vertices.push_back(vector);
				//normals
				vector.x = curMesh.Vertices[j].Normal.X;
				vector.y = curMesh.Vertices[j].Normal.Y;
				vector.z = curMesh.Vertices[j].Normal.Z;
				this->normals.push_back(vector);	
				//uv
				glm::vec2 vector2;
				vector2.x = curMesh.Vertices[j].TextureCoordinate.X;
				vector2.y = curMesh.Vertices[j].TextureCoordinate.Y;
				this->uvs.push_back(vector2);
			}
			for (int j = 0; j < curMesh.Indices.size(); j++)
			{
				indices.push_back((int)curMesh.Indices[j]);
			}
		}
	}
	else
	{
		// Create/Open e1Out.txt
		std::ofstream file("e1Out.txt");

		// Output Error
		file << "Failed to Load File. May have failed to find it or it was not an .obj file.\n";

		// Close File
		file.close();
	}
}
