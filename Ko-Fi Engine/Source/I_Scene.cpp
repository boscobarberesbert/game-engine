#include "I_Scene.h"
#include "Engine.h"
#include "Log.h"
#include "Assimp.h"
#include "MathGeoTransform.h"
#include "FSDefs.h"
#include "JsonHandler.h"

#include "Scene.h"
#include "SceneManager.h"
#include "FileSystem.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentInfo.h"
#include "ComponentCamera.h"

#include "Mesh.h"
#include "Texture.h"
#include "Material.h"

#include "Importer.h"
#include "I_Mesh.h"
#include "I_Texture.h"
#include "I_Material.h"

I_Scene::I_Scene(KoFiEngine* engine) : engine(engine)
{

}

I_Scene::~I_Scene()
{

}

bool I_Scene::Import(const char* path)
{
	CONSOLE_LOG("[STATUS] Importer: Importing Scene: %s", path);

	std::string errorString = "[ERROR] Importer: Could not Import Model { " + std::string(path) + " }";

	if (path == nullptr)
		CONSOLE_LOG("[ERROR] Importer: Path is nullptr.");

	const aiScene* assimpScene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);

	if (assimpScene == nullptr || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
	{
		CONSOLE_LOG("[ERROR] Importer: %s! Error: Assimp Error [%s]", errorString.c_str(), aiGetErrorString());
		return false;
	}

	fbxName = path;
	fbxName = fbxName.substr(fbxName.find_last_of("/\\") + 1);

	ImportNode(assimpScene, assimpScene->mRootNode, engine->GetSceneManager()->GetCurrentScene()->rootGo);

	//ImportAnimations(assimpScene, mesh);

	return true;
}

GameObject* I_Scene::ImportModel(const char* path)
{
	// TODO: WE SHOULD CHANGE THIS
	GameObject* tmp = nullptr;
	return tmp;
}

void I_Scene::ImportNode(const aiScene* assimpScene, const aiNode* assimpNode, GameObject* parent)
{
	fbxName = (assimpNode == assimpScene->mRootNode) ? fbxName : assimpNode->mName.C_Str();

	GameObject* gameObj = engine->GetSceneManager()->GetCurrentScene()->CreateEmptyGameObject(fbxName.c_str());

	assimpNode = ImportTransform(assimpNode, gameObj);
	ImportMeshesAndMaterials(assimpScene, assimpNode, gameObj);

	parent->AttachChild(gameObj);

	for (unsigned int i = 0; i < assimpNode->mNumChildren; ++i)
	{
		ImportNode(assimpScene, assimpNode->mChildren[i], gameObj);
	}
}

const aiNode* I_Scene::ImportTransform(const aiNode* assimpNode, GameObject* child)
{
	// Assimp generates dummy nodes to store multiple FBX transformations.
	// All those transformations will be collapsed to the first non-dummy node.

	aiTransform aiT;

	assimpNode->mTransformation.Decompose(aiT.scale, aiT.rotation, aiT.position);

	float3 position = { aiT.position.x, aiT.position.y, aiT.position.z };
	Quat rotation = { aiT.rotation.x, aiT.rotation.y, aiT.rotation.z, aiT.rotation.w };
	float3 scale = { aiT.scale.x, aiT.scale.y, aiT.scale.z };

	while (IsDummyNode(*assimpNode))
	{
		// As dummies will only have one child, selecting the next one to process is easy.
		assimpNode = assimpNode->mChildren[0];

		// Getting the Transform stored in the dummy node.
		assimpNode->mTransformation.Decompose(aiT.scale, aiT.rotation, aiT.position);

		float3 dummyPosition = { aiT.position.x, aiT.position.y, aiT.position.z };
		Quat dummyRotation = { aiT.rotation.x, aiT.rotation.y, aiT.rotation.z, aiT.rotation.w };
		float3 dummyScale = { aiT.scale.x, aiT.scale.y, aiT.scale.z };

		// Adding the dummy's Transform to the current one.
		position += dummyPosition;
		rotation = rotation * dummyRotation;
		scale = { scale.x * dummyScale.x, scale.y * dummyScale.y, scale.z * dummyScale.z };
	}

	child->GetComponent<ComponentTransform>()->SetPosition(position);
	child->GetComponent<ComponentTransform>()->SetRotation(rotation);
	child->GetComponent<ComponentTransform>()->SetScale(scale);

	CONSOLE_LOG("[STATUS] Importer: Imported transforms of node: %s", assimpNode->mName.C_Str());

	return assimpNode;
}

bool I_Scene::IsDummyNode(const aiNode & assimpNode)
{
	// All dummy nodes contain the "_$AssimpFbx$_" string and only one child node.
	return (strstr(assimpNode.mName.C_Str(), "_$AssimpFbx$_") != nullptr && assimpNode.mNumChildren == 1);
}

void I_Scene::ImportMeshesAndMaterials(const aiScene* assimpScene, const aiNode* assimpNode, GameObject* gameObj)
{
	if (assimpScene == nullptr || assimpNode == nullptr || gameObj == nullptr)
	{
		CONSOLE_LOG("[ERROR] Importer: Assimp Scene / Node or GameObject is nullptr.");
		return;
	}
	
	if (!assimpScene->HasMeshes())
	{
		CONSOLE_LOG("[ERROR] Importer: Assimp does not have any Mesh.");
		return;
	}

	const char* nodeName = assimpNode->mName.C_Str();

	for (unsigned int i = 0; i < assimpNode->mNumMeshes; ++i)
	{
		aiMesh* assimpMesh = assimpScene->mMeshes[assimpNode->mMeshes[i]];

		if (assimpMesh != nullptr && assimpMesh->HasFaces())
		{
			ImportMesh(nodeName, assimpMesh, gameObj);

			if (assimpScene->HasMaterials() && assimpMesh->mMaterialIndex >= 0)
			{
				aiMaterial* assimpMaterial = assimpScene->mMaterials[assimpMesh->mMaterialIndex];
				ImportMaterial(nodeName, assimpMaterial, assimpMesh->mMaterialIndex, gameObj);
			}
		}
		else
			CONSOLE_LOG("[ERROR] Importer: aiMesh is missing or does not have faces.");
	}
}

void I_Scene::ImportMesh(const char* nodeName, const aiMesh* assimpMesh, GameObject* gameObj)
{
	//std::string assetPath = ASSETS_MODELS_DIR + std::string(nodeName) + MESH_EXTENSION;

	if (assimpMesh == nullptr || gameObj == nullptr)
	{
		return;
	}

	// Import Mesh to GameObject
	Mesh* mesh = new Mesh(Shape::NONE);
	Importer::GetInstance()->meshImporter->Import(assimpMesh, mesh);

	if (mesh == nullptr)
	{
		CONSOLE_LOG("[ERROR] Importer: Mesh (name: %s) was not imported correctly.", nodeName);
		return;
	}

	ComponentMesh* cMesh = gameObj->CreateComponent<ComponentMesh>();
	if (cMesh != nullptr)
		cMesh->SetMesh(mesh);
	else
	{
		CONSOLE_LOG("[ERROR] Component Mesh is nullptr.");
		return;
	}
}

void I_Scene::ImportMaterial(const char* nodeName, const aiMaterial* assimpMaterial, uint materialIndex, GameObject* gameObj)
{
	if (assimpMaterial == nullptr)
	{
		CONSOLE_LOG("[ERROR] Importer: aiMaterial is nullptr.");
		return;
	}

	aiString aiTexturePath;
	std::string texturePath;
	Texture texture;
	if (aiGetMaterialTexture(assimpMaterial, aiTextureType_DIFFUSE, materialIndex, &aiTexturePath) == AI_SUCCESS)
	{
		std::string textureFilename = aiTexturePath.C_Str();

		textureFilename = textureFilename.substr(textureFilename.find_last_of("/\\") + 1);

		texturePath = ASSETS_TEXTURES_DIR + textureFilename;

		texture = Texture();
		Importer::GetInstance()->textureImporter->Import(texturePath.c_str(), &texture);
	}

	// Import Material to GameObject
	ComponentMaterial* cMaterial = gameObj->CreateComponent<ComponentMaterial>();

	if (cMaterial != nullptr)
	{
		//cMaterial->textures.push_back(texture);
		cMaterial->texture = texture;
	}
	else
	{
		CONSOLE_LOG("[ERROR] Component Material is nullptr.");
		return;
	}

	Material* material = new Material();

	if (!Importer::GetInstance()->materialImporter->Import(assimpMaterial, material))
	{
		CONSOLE_LOG("[ERROR] Importer: error while importing the material.");
		return;
	}
	else
		cMaterial->SetMaterial(material);

	//if (textureFilename.size() > 0)
	//{
	//	std::string baseFilename = textureFilename.substr(textureFilename.find_last_of("/\\") + 1);
	//	std::string::size_type const p(baseFilename.find_last_of('.'));
	//	std::string filenameWithoutExtension = baseFilename.substr(0, p);
	//	std::string materialPath = ASSETS_MATERIALS_DIR + filenameWithoutExtension + MATERIAL_EXTENSION;
	//	std::string texturePath = ASSETS_TEXTURES_DIR + textureFilename.substr(textureFilename.find_last_of('\\') + 1);
	//	
	//	if (textureFilename.c_str() != nullptr)
	//	{
	//		engine->GetFileSystem()->CreateMaterial(materialPath.c_str(), filenameWithoutExtension.c_str(), texturePath.c_str());
	//		cMaterial->LoadMaterial(materialPath.c_str());
	//	}
	//}
}

bool I_Scene::Save(Scene* scene)
{
	bool ret = false;

	JsonHandler jsonHandler;
	Json jsonFile;

	const char* name = scene->name.c_str();
	std::vector<GameObject*> gameObjectList = scene->gameObjectList;

	jsonFile[name];
	jsonFile[name]["name"] = name;
	jsonFile[name]["active"] = scene->active;
	jsonFile[name]["game_objects_amount"] = gameObjectList.size();

	jsonFile[name]["game_objects_list"] = Json::array();
	for (std::vector<GameObject*>::iterator goIt = gameObjectList.begin(); goIt != gameObjectList.end(); ++goIt)
	{
		Json jsonGameObject;

		GameObject* gameObject = (*goIt);

		jsonGameObject["name"] = gameObject->GetName();
		jsonGameObject["active"] = gameObject->active;
		jsonGameObject["UID"] = gameObject->GetUID();

		// We don't want to save also its children here.
		// We will arrive and create them when they get here with the loop.
		// So, in order to keep track of parents and childrens, we will record the UID of the parent.
		if (gameObject->GetParent() != nullptr)
			jsonGameObject["parent_UID"] = gameObject->GetParent()->GetUID();

		std::vector<Component*> componentsList = gameObject->GetComponents();
		jsonGameObject["components"] = Json::array();
		for (std::vector<Component*>::iterator cmpIt = componentsList.begin(); cmpIt != componentsList.end(); ++cmpIt)
		{
			Json jsonComponent;

			Component* component = (*cmpIt);

			jsonComponent["active"] = component->active;

			switch (component->GetType())
			{
			case ComponentType::NONE:
				jsonComponent["type"] = "NONE";
				break;
			case ComponentType::TRANSFORM:
			{
				ComponentTransform* transformCmp = (ComponentTransform*)component;
				transformCmp->Save(jsonComponent);
				break;
			}
			case ComponentType::MESH:
			{
				ComponentMesh* meshCmp = (ComponentMesh*)component;
				meshCmp->Save(jsonComponent);
				break;
			}
			case ComponentType::MATERIAL:
			{
				ComponentMaterial* materialCmp = (ComponentMaterial*)component;
				materialCmp->Save(jsonComponent);
				break;
			}
			case ComponentType::INFO:
			{
				ComponentInfo* infoCmp = (ComponentInfo*)component;
				infoCmp->Save(jsonComponent);
				break;
			}
			case ComponentType::CAMERA:
			{
				ComponentCamera* cameraCmp = (ComponentCamera*)component;
				cameraCmp->Save(jsonComponent);
				break;
			}
			default:
				break;
			}
			jsonGameObject["components"].push_back(jsonComponent);
		}
		jsonFile[name]["game_objects_list"].push_back(jsonGameObject);
	}
	std::string path = SCENES_DIR + std::string(name) + SCENE_EXTENSION;

	ret = jsonHandler.SaveJson(jsonFile, path.c_str());

	return ret;
}

bool I_Scene::Load(Scene* scene, const char* name)
{
	bool ret = false;

	JsonHandler jsonHandler;
	Json jsonFile;

	std::string path = SCENES_DIR + std::string(name) + SCENE_EXTENSION;
	ret = jsonHandler.LoadJson(jsonFile, path.c_str());

	if (!jsonFile.is_null())
	{
		ret = true;

		//for (std::vector<GameObject*>::iterator goIt = gameObjects.begin(); goIt != gameObjects.end(); ++goIt)
		//{
		//	(*goIt)->CleanUp();
		//	RELEASE((*goIt));
		//}



		Json jsonGameObjects = jsonFile["game_objects_list"];
		for (const auto& goIt : jsonGameObjects.items())
		{
			Json jsonGo = goIt.value();

			std::string name = jsonGo["name"];
			bool active = jsonGo["active"];
			uint UID = jsonGo["UID"];
			uint parentUid = jsonGo["parent_UID"];
			GameObject* go = new GameObject(UID, engine, name.c_str());
			go->SetParentUID(parentUid);
			go->active = active;

			Json jsonCmp = jsonGo["components"];
			for (const auto& cmpIt : jsonCmp.items())
			{
				Json jsonCmp = cmpIt.value();
				bool active = jsonCmp["active"];
				std::string type = jsonCmp["type"];

				if (type == "transform")
				{
					ComponentTransform* transformCmp = go->GetComponent<ComponentTransform>();
					transformCmp->active = true;
					transformCmp->Load(jsonCmp);
				}
				else if (type == "mesh")
				{
					ComponentMesh* meshCmp = go->GetComponent<ComponentMesh>();
					if (meshCmp == nullptr)
					{
						meshCmp = go->CreateComponent<ComponentMesh>();
					}
					meshCmp->active = true;
					meshCmp->Load(jsonCmp);
				}
				else if (type == "material")
				{
					ComponentMaterial* materialCmp = go->GetComponent<ComponentMaterial>();
					if (materialCmp == nullptr)
					{
						materialCmp = go->CreateComponent<ComponentMaterial>();
					}
					materialCmp->active = true;
					materialCmp->Load(jsonCmp);
				}
				else if (type == "info")
				{
					ComponentInfo* infoCmp = go->GetComponent<ComponentInfo>();
					infoCmp->active = true;
					infoCmp->Load(jsonCmp);
				}
				else if (type == "camera")
				{
					ComponentCamera* cameraCmp = go->GetComponent<ComponentCamera>();
					if (cameraCmp == nullptr)
					{
						cameraCmp = go->CreateComponent<ComponentCamera>();
					}
					cameraCmp->active = true;
					cameraCmp->Load(jsonCmp);
				}
			}
			scene->gameObjectList.push_back(go);
		}

		for (std::vector<GameObject*>::iterator goIt = scene->gameObjectList.begin(); goIt < scene->gameObjectList.end(); ++goIt)
		{
			for (std::vector<GameObject*>::iterator childrenIt = scene->gameObjectList.begin(); childrenIt < scene->gameObjectList.end(); ++childrenIt)
			{
				if ((*goIt)->GetParentUID() == (*childrenIt)->GetUID() && (*childrenIt)->GetUID() != -1)
				{
					(*goIt)->AttachChild((*childrenIt));
				}
			}
		}
		ret = true;
	}
	else
		ret = false;

	return ret;
}