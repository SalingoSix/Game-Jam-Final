#ifndef _HG_cModel_
#define _HG_cModel_

#include <vector>
#include <string>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "cShaderProgram.h"
#include "cMesh.h"

class cModel
{
public:
	cModel(std::string path);
	cModel(std::string path, std::vector<glm::mat4> modelMatrices, GLuint texID);
	void Draw(cShaderProgram shader);

private:
	std::vector<sTexture> textures_loaded;
	std::vector<cMesh> meshes;
	std::string directory;

	bool instanced;

	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	cMesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<sTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	unsigned int TextureFromFile(const char* path, const std::string &directory, bool gamma = false);

	std::vector<glm::mat4> vecModels;
	GLuint texID;
};

#endif