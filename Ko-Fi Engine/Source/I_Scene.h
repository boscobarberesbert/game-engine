#ifndef __I_SCENE_H__
#define __I_SCENE_H__

#include "Globals.h"
#include <vector>
#include <string>
#include <map>

class KoFiEngine;
class aiScene;
class aiNode;
class aiMesh;
class aiMaterial;
class Scene;
class GameObject;
class R_Model;
class ModelNode;
class Resource;

class I_Scene
{
public:
	I_Scene(KoFiEngine* engine);
	~I_Scene();

	bool Import(R_Model* model, bool isPrefab = false);
	bool SaveModel(const R_Model* model, const char* path);
	bool LoadModel(const char* path, R_Model* model);

	bool SaveScene(Scene* scene, const char* name = nullptr);
	bool LoadScene(Scene* scene, const char* name);

	bool Import(const char* path, bool isPrefab = false);
	bool Save(Scene* scene, const char* customName = nullptr);
	bool Load(Scene* scene, const char* name);

private:
	void ImportNode(const aiScene* assimpScene, const aiNode* assimpNode, R_Model* model, const ModelNode& parent, bool isPrefab = false);
	const aiNode* ImportTransform(const aiNode* assimpNode, ModelNode& node);
	void ImportMeshesAndMaterials(const aiScene* assimpScene, const aiNode* assimpNode, R_Model* model, ModelNode& node);
	void ImportMesh(const char* nodeName, const aiMesh* assimpMesh, R_Model* model, ModelNode& node, const aiScene* assimpScene = nullptr);
	void ImportMaterial(const char* nodeName, const aiMaterial* assimpMaterial, uint materialIndex, ModelNode& node);

	bool IsDummyNode(const aiNode& assimpNode);

	void ImportNode(const aiScene* assimpScene, const aiNode* assimpNode, GameObject* parent, bool isPrefab = false);
	const aiNode* ImportTransform(const aiNode* assimpNode, GameObject* gameObj);
	void ImportMeshesAndMaterials(const aiScene* assimpScene, const aiNode* assimpNode, GameObject* gameObj);
	void ImportMesh(const char* nodeName, const aiMesh* assimpMesh, GameObject* gameObj, const aiScene* assimpScene = nullptr);
	void ImportMaterial(const char* nodeName, const aiMaterial* assimpMaterial, uint materialIndex, GameObject* gameObj);
	
	void CheckAndApplyForcedUID(Resource* resource);

public:
	std::map<std::string, UID> loadedTextures;

private:
	KoFiEngine* engine = nullptr;

	std::string nodeName;

	std::map<uint, ModelNode> loadedNodes;
	std::map<std::string, UID> forcedUIDs;

	// Instance of some generic Assimp variables in case we need them from the outside of the import method
	aiScene* assimpScene = nullptr;

	bool alreadyImportedAnim = false;
};

#endif // !__I_SCENE_H__