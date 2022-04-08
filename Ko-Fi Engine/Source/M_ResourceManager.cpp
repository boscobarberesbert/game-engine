#include "M_ResourceManager.h"
#include "Engine.h"
#include "Log.h"
#include "RNG.h"
#include "FSDefs.h"
#include "Importer.h"
#include "M_FileSystem.h"
#include "R_ResourceBase.h"

#include "json.hpp"
#include "JsonHandler.h"
using Json = nlohmann::json;

#include <filesystem>

M_ResourceManager::M_ResourceManager(KoFiEngine* engine) : Module(),
fileRefreshRate(0.0f),
fileRefreshTime(0.0f)
{
	name = "ResourceManager";
	this->engine = engine;
}

M_ResourceManager::~M_ResourceManager()
{

}

bool M_ResourceManager::Awake(Json configModule)
{
	bool ret = true;

	ret = LoadConfiguration(configModule);

	return ret;
}

bool M_ResourceManager::Start()
{
	CONSOLE_LOG("Starting M_ResourceManager...");
	appLog->AddLog("Starting M_ResourceManager...\n");

	fileRefreshRate = 5.0f;

	//RefreshDirectoryFiles(ASSETS_DIR);
	//RefreshDirectoryFiles(LIBRARY_DIR);

	//TrimLibrary();

	//Find prefabs

	return true;
}

bool M_ResourceManager::PreUpdate(float dt)
{
	//fileRefreshTime += dt;
	//if (fileRefreshTime > fileRefreshRate)
	//{
	//	std::map<UID, Resource*>::iterator it = resourcesMap.begin();
	//	while (it != resourcesMap.end())
	//	{
	//		if (it->second->GetReferenceCount() == 0)
	//		{
	//			UID resourceUID = it->second->GetUID();
	//			++it;
	//			DeallocateResource(resourceUID);
	//			continue;
	//		}
	//		++it;
	//	}
	//	fileRefreshTime = 0.0f;
	//}

	return true;
}

bool M_ResourceManager::CleanUp()
{
	CONSOLE_LOG("Cleaning M_ResourceManager up...");
	//appLog->AddLog("Cleaning M_ResourceManager up...\n");

	bool ret = true;

	//std::map<UID, Resource*>::iterator it;
	//for (it = resourcesMap.begin(); it != resourcesMap.end(); ++it)
	//{
	//	it->second->CleanUp();
	//	RELEASE(it->second);
	//}

	//resourcesMap.clear();
	//library.clear();

	return ret;
}

// Method to receive and manage events
void M_ResourceManager::OnNotify(const Event& event)
{
	// Manage events
}

bool M_ResourceManager::TrimLibrary()
{
	std::vector<std::string> files;
	engine->GetFileSystem()->DiscoverAllFiles(LIBRARY_DIR, files);

	std::map<std::string, UID> fileUIDs;
	for (auto file = files.cbegin(); file != files.cend(); ++file)
	{
		UID uid = 0;
		sscanf(engine->GetFileSystem()->GetFileName((*file).c_str()), "%u", &uid);

		if (uid != 0)
			fileUIDs.emplace((*file), uid);
	}

	for ( std::map<std::string,UID>::const_iterator UID = fileUIDs.cbegin(); UID != fileUIDs.cend(); ++UID)
	{
		if (library.find(UID->second) == library.end())
		{
			std::filesystem::remove(UID->first.c_str());
		}
	}

	fileUIDs.clear();

	files.clear();
	files.shrink_to_fit();

	return true;
}

UID M_ResourceManager::ImportFile(const char* assetPath)
{
	UID uid = 0;
	if (assetPath == nullptr)
	{
		LOG_BOTH("[ERROR] Resource Manager: loading file, file path was nullptr.");
		return uid;
	}

	if (HasImportIgnoredExtension(assetPath))
	{
		LOG_BOTH("[ERROR] Resource Manager: loading file, the file extension has an import ignored extension.");
		return uid;
	}

	std::filesystem::path path = assetPath;
	ResourceType type = GetTypeFromExtension(path.extension().string().c_str());

	if (type == ResourceType::UNKNOWN)
	{
		LOG_BOTH("[ERROR] Resource Manager: loading file, unkown file type.");
		return uid;
	}

	std::string cleanPath = GetValidPath(assetPath);
	if (cleanPath.c_str() == nullptr)
	{
		LOG_BOTH("[ERROR] Resource Manager: loading file, couldn't validate path.");
		return uid;
	}

	bool metaIsValid = ValidateMetaFile(assetPath);
	if (metaIsValid)
	{
		LOG_BOTH("File to import was already in the library.");

		std::map<UID, R_ResourceBase> libraryItems;
		GetLibraryPairs(assetPath, libraryItems);

		for (auto& item : libraryItems)
		{
			if (library.find(item.first) == library.end())
				library.emplace(std::make_pair(item.first, item.second));
		}
		uid = libraryItems.begin()->first;
		libraryItems.clear();
	}
	else
	{
		if (HasMetaFile(assetPath))
			DeleteFromLibrary(assetPath);

		uid = ImportFromAssets(assetPath);

		if (uid == 0)
			LOG_BOTH("[ERROR] Resource Manager: loading file, error loading file from assets.");
	}

	return uid;
}

void M_ResourceManager::RefreshDirectoryFiles(const char* directory)
{
	if (directory == nullptr)
	{
		CONSOLE_LOG("[ERROR] Resource Manager: trying to refresh directory files, string was nullptr.");
		return;
	}

	std::vector<std::string> toImport;
	std::vector<std::string> toUpdate;
	std::vector<std::string> toDelete;

	std::vector<std::string> assetFiles;
	std::vector<std::string> metaFiles;
	std::map<std::string, std::string> filePairs;

	engine->GetFileSystem()->DiscoverAllFilesFiltered(directory, assetFiles, metaFiles, META_EXTENSION);

	FindFilesToImport(assetFiles, metaFiles, filePairs, toImport);
	FindFilesToUpdate(filePairs, toUpdate);
	FindFilesToDelete(metaFiles, filePairs, toDelete);

	LoadFilesIntoLibrary(filePairs);

	filePairs.clear();
	metaFiles.clear();
	metaFiles.shrink_to_fit();
	assetFiles.clear();
	assetFiles.shrink_to_fit();

	for (uint i = 0; i < toDelete.size(); ++i)
		DeleteFromLibrary(toDelete[i].c_str());

	for (uint i = 0; i < toUpdate.size(); ++i)
	{
		std::string extension = toUpdate[i].c_str();
		extension = extension.substr(extension.find_last_of("."), extension.size());

		if (extension == "h" || extension == "particles" || extension == "navmesh" || extension == "shader" || extension == "png")
		{
			DeleteFromLibrary(toUpdate[i].c_str());
			ImportFile(toUpdate[i].c_str());
		}
	}

	for (uint i = 0; i < toImport.size(); ++i)
		ImportFile(toImport[i].c_str());

	toDelete.clear();
	toDelete.shrink_to_fit();
	toUpdate.clear();
	toUpdate.shrink_to_fit();
	toImport.clear();
	toImport.shrink_to_fit();
}

void M_ResourceManager::FindFilesToImport(std::vector<std::string>& assetFiles, std::vector<std::string>& metaFiles, std::map<std::string, std::string>& filePairs, std::vector<std::string>& toImport)
{
	if (assetFiles.empty())
		return;

	std::string metaFile = "";
	for (uint i = 0; i < assetFiles.size(); ++i)
	{
		if (HasImportIgnoredExtension(assetFiles[i].c_str()))
			continue;

		metaFile = assetFiles[i] + META_EXTENSION;
		bool find = false;
		for (uint j = 0; j < metaFiles.size(); ++j)
		{
			if (metaFile == metaFiles[j])
				find = true;
		}

		if (find)
		{
			filePairs.emplace(assetFiles[i], metaFile);

			if (!ValidateMetaFile(assetFiles[i].c_str(), false))
				toImport.push_back(assetFiles[i]);
		}
		else
			toImport.push_back(assetFiles[i]);
	}

	metaFile.clear();
	metaFile.shrink_to_fit();
}

void M_ResourceManager::FindFilesToUpdate(std::map<std::string, std::string>& filePairs, std::vector<std::string>& toUpdate)
{
	if (filePairs.empty())
		return;

	for (auto item : filePairs)
	{
		auto assetModification = std::filesystem::last_write_time(item.first);
		auto metaModification = std::filesystem::last_write_time(item.second);
		if (assetModification != metaModification)
		{
			CONSOLE_LOG("Modification time discrepancy with file %s\n", item.first);
			toUpdate.push_back(item.first);
		}
	}
}

void M_ResourceManager::FindFilesToDelete(std::vector<std::string>& metaFiles, std::map<std::string, std::string>& filePairs, std::vector<std::string>& toDelete)
{
	if (metaFiles.empty())
		return;

	for (uint i = 0; i < metaFiles.size(); ++i)
	{
		std::string assetPath = metaFiles[i];
		assetPath = assetPath.substr(0,assetPath.find_last_of(META_EXTENSION) - 5);
		if (filePairs.find(assetPath) == filePairs.end())
		{
			toDelete.push_back(assetPath);
		}
	}
}

void M_ResourceManager::LoadFilesIntoLibrary(std::map<std::string, std::string>& filePairs)
{
	if (filePairs.empty())
		return;

	for (auto item : filePairs)
	{
		LoadMetaFileIntoLibrary(item.first.c_str());
	}
}

bool M_ResourceManager::LoadMetaFileIntoLibrary(const char* assetsPath)
{
	if (assetsPath == nullptr)
	{
		CONSOLE_LOG("[ERROR] Resource Manager: loading meta file into library, assets path was nullptr.");
		return false;
	}

	std::map<UID, R_ResourceBase> libraryPairs;
	GetLibraryPairs(assetsPath, libraryPairs);

	if (libraryPairs.empty())
		return true;

	for (auto item : libraryPairs)
		library.emplace(item.first, item.second);

	libraryPairs.clear();

	return true;
}

bool M_ResourceManager::GetLibraryPairs(const char* assetsPath, std::map<UID, R_ResourceBase>& pairs)
{
	if (assetsPath == nullptr)
	{
		CONSOLE_LOG("[ERROR] Resource Manager: getting library pairs, assets path was nullptr.");
		return false;
	}

	std::vector<UID> resourceUIDs;
	std::vector<R_ResourceBase> bases;
	GetResourceUIDsFromMeta(assetsPath, resourceUIDs);
	GetResourceBasesFromMeta(assetsPath, bases);

	if (resourceUIDs.size() != bases.size())
		CONSOLE_LOG("[ERROR] Resource Manager: missmatching resource bases and UIDs");

	for (uint i = 0; i < bases.size(); ++i)
		pairs.emplace(resourceUIDs[i], bases[i]);

	resourceUIDs.clear();
	resourceUIDs.shrink_to_fit();
	bases.clear();
	bases.shrink_to_fit();

	return true;
}

bool M_ResourceManager::GetResourceUIDsFromMeta(const char* assetsPath, std::vector<UID>& uids)
{
	if (assetsPath == nullptr)
	{
		LOG_BOTH("[ERROR] Resource Manager: getting UIDs from meta, assets path was nullptr.");
		return false;
	}

	//Load Meta File & get contained resources
	{
		//char* buffer = nullptr;
		//ParsonNode metaRoot = LoadMetaFile(assetsPath, &buffer);
		//ParsonArray containedArray = metaRoot.GetArray("ContainedResources");
		//RELEASE_ARRAY(buffer);

		//if (!metaRoot.NodeIsValid())
		//{
		//	LOG("%s! Error: Given Assets Path had no correspondent Meta File.", errorString.c_str());
		//	return false;
		//}
		//if (!containedArray.ArrayIsValid())
		//{
		//	LOG("%s! Error: Contained Array in Meta File was not valid.", errorString.c_str());
		//	return false;
		//}
	}

	// UID of main resource
	{
		//uint32 resourceUid = (uint32)metaRoot.GetNumber("UID");
		//if (resourceUid == 0)
		//{
		//	LOG("%s! Error: Main Resource UID was 0.", errorString.c_str());
		//	return false;
		//}

		//resourceUids.push_back(resourceUid);
	}

	// UIDs of contained resources
	{
		//uint32 containedUid = 0;
		//ParsonNode containedNode = ParsonNode();
		//for (uint i = 0; i < containedArray.size; ++i)
		//{
		//	containedNode = containedArray.GetNode(i);
		//	if (!containedNode.NodeIsValid())
		//	{
		//		continue;
		//	}

		//	containedUid = 0;																									// Resetting the contained uid
		//	containedUid = (uint32)containedNode.GetNumber("UID");
		//	if (containedUid == 0)
		//	{
		//		LOG("[WARNING] Resource Manager: Contained UID was not valid!");
		//		continue;
		//	}

		//	resourceUids.push_back(containedUid);
		//}
	}

	return true;
}

bool M_ResourceManager::GetResourceBasesFromMeta(const char* assetsPath, std::vector<R_ResourceBase>& bases)
{
	if (assetsPath == nullptr)
	{
		CONSOLE_LOG("[ERROR] Resource Manager: getting resource bases from meta, assets path was nullptr.");
		return false;
	}
	//Load Meta File & get contained resources
	{
		//char* buffer = nullptr;
		//ParsonNode metaRoot = LoadMetaFile(assetsPath, &buffer);
		//ParsonArray containedArray = metaRoot.GetArray("ContainedResources");
		//RELEASE_ARRAY(buffer);

		//if (!metaRoot.NodeIsValid())
		//{
		//	LOG("%s! Error: Given Assets Path had no associated .meta file.", errorString.c_str());
		//	return false;
		//}
		//if (!containedArray.ArrayIsValid())
		//{
		//	LOG("%s! Error: ContainedResources array in Meta File was not valid.", errorString.c_str());
		//	return false;
		//}
	}

	// Gets Resource Base from main resource
	{
		//uint32 UID = (uint32)metaRoot.GetNumber("UID");
		//ResourceType type = (ResourceType)((int)metaRoot.GetNumber("Type"));
		//std::string rAssetsPath = metaRoot.GetString("AssetsPath");
		//std::string rAssetsFile = metaRoot.GetString("Name");
		//std::string rLibraryPath = metaRoot.GetString("LibraryPath");
		//std::string rLibraryFile = metaRoot.GetString("LibraryFile");
		//if (UID == 0)
		//{
		//	LOG("%s! Error: Main Resource UID was 0.", errorString.c_str());
		//	return false;
		//}

		//resourceBases.push_back(R_ResourceBase(UID, rAssetsPath, rAssetsFile, rLibraryPath, rLibraryFile, type));
	}

	// Gets Resource Base from contained resources
	{
		//for (uint i = 0; i < containedArray.size; ++i)
		//{
		//	ParsonNode containedNode = containedArray.GetNode(i);
		//	if (!containedNode.NodeIsValid())
		//	{
		//		continue;
		//	}

		//	std::string directory = "[NONE]";
		//	std::string extension = "[NONE]";

		//	uint32 containedUID = (uint32)containedNode.GetNumber("UID");
		//	ResourceType containedType = (ResourceType)((int)containedNode.GetNumber("Type"));
		//	std::string containedAssetsPath = "[NONE]";
		//	std::string containedAssetsFile = containedNode.GetString("Name");
		//	std::string containedLibraryPath = containedNode.GetString("LibraryPath");
		//	std::string containedLibraryFile = "[NONE]";
		//	bool success = GetAssetsDirectoryAndExtensionFromType(containedType, directory, extension);
		//	if (!success)
		//	{
		//		continue;
		//	}
		//	if (containedUID == 0)
		//	{
		//		continue;
		//	}

		//	containedAssetsPath = directory + containedAssetsFile;
		//	containedLibraryFile = App->fileSystem->GetFileAndExtension(containedLibraryPath.c_str());
		//	resourceBases.push_back(R_ResourceBase(containedUID, containedAssetsPath, containedAssetsFile, containedLibraryPath, containedLibraryFile, containedType));	// WIP until revision.
		//}
	}

	return true;
}

bool M_ResourceManager::GetLibraryFilePathsFromMeta(const char* assetsPath, std::vector<std::string>& paths)
{
	if (assetsPath == nullptr)
	{
		CONSOLE_LOG("[ERROR] Resource Manager: getting file paths from meta, assets path was nullptr.");
		return false;
	}

	//Load Meta File & get contained resources
	{
		//char* buffer = nullptr;
		//ParsonNode metaRoot = LoadMetaFile(assetsPath, &buffer);
		//ParsonArray containedArray = metaRoot.GetArray("ContainedResources");
		//RELEASE_ARRAY(buffer);

		//if (!metaRoot.NodeIsValid())
		//{
		//	LOG("%s! Error: Given Assets Path had no associated .meta file.", errorString.c_str());
		//	return false;
		//}
		//if (!containedArray.ArrayIsValid())
		//{
		//	LOG("%s! Error: ContainedResources array in Meta File was not valid.", errorString.c_str());
		//	return false;
		//}
	}

	std::string directory = "";
	std::string extension = "";

	// Gets path from main resource
	{
		//uint32 resourceUid = (uint32)metaRoot.GetNumber("UID");
		//ResourceType type = (ResourceType)((int)metaRoot.GetNumber("Type"));
		//bool success = GetLibraryDirectoryAndExtensionFromType(type, directory, extension);

		//if (!success)
		//{
		//	LOG("%s! Error: Could not get the Library Directory and Extension from Resource Type.", errorString.c_str());
		//	return false;
		//}
		//if (resourceUid == 0)
		//{
		//	LOG("%s! Error: Main Resource UID was 0.", errorString.c_str());
		//	return false;
		//}
		//if (directory == "[NONE]" || extension == "[NONE]")
		//{
		//	LOG("%s! Error: Main Resource Library Directory or Extension strings were not valid.", errorString.c_str());
		//	return false;
		//}

		//std::string libraryPath = directory + std::to_string(resourceUid) + extension;
		//filePaths.push_back(libraryPath);
	}

	// Gets paths from contained resources
	{
		//ParsonNode containedNode = ParsonNode();
		//uint32 containedUid = 0;
		//ResourceType containedType = ResourceType::NONE;
		//std::string containedPath = "[NONE]";
		//for (uint i = 0; i < containedArray.size; ++i)
		//{
		//	containedNode = containedArray.GetNode(i);
		//	if (!containedNode.NodeIsValid())
		//	{
		//		continue;
		//	}

		//	directory = "[NONE]";
		//	extension = "[NONE]";

		//	containedUid = (uint32)containedNode.GetNumber("UID");
		//	containedType = (ResourceType)((int)containedNode.GetNumber("Type"));

		//	success = GetLibraryDirectoryAndExtensionFromType(containedType, directory, extension);
		//	if (!success)
		//	{
		//		continue;
		//	}
		//	if (containedUid == 0)
		//	{
		//		continue;
		//	}
		//	if (directory == "[NONE]" || extension == "[NONE]")
		//	{
		//		continue;
		//	}

		//	containedPath = directory + std::to_string(containedUid) + extension;
		//	filePaths.push_back(containedPath);
		//}
	}

	return true;
}

void M_ResourceManager::DeleteFromLibrary(const char* libraryPath)
{
	if (libraryPath == nullptr)
	{
		CONSOLE_LOG("[ERROR] Resource Manager: deleting file from library, assets path was nullptr.");
		return;
	}

	if (!HasMetaFile(libraryPath))
	{
		CONSOLE_LOG("[ERROR] Resource Manager: deleting file from library, %s file couldn't be found or doesn't exist.", libraryPath);
		return;
	}

	std::vector<UID> resourceUids;
	std::vector<std::string> toDelete;

	GetResourceUIDsFromMeta(libraryPath, resourceUids);
	GetLibraryFilePathsFromMeta(libraryPath, toDelete);

	for (uint i = 0; i < resourceUids.size(); ++i)
	{
		UnloadResource(resourceUids[i]);
	}

	for (uint i = 0; i < toDelete.size(); ++i)
	{
		std::filesystem::remove(toDelete[i].c_str());
	}

	toDelete.clear();
	toDelete.shrink_to_fit();
	resourceUids.clear();
	resourceUids.shrink_to_fit();

}

bool M_ResourceManager::HasMetaFile(const char* assetsPath)
{
	if (assetsPath == nullptr)
	{
		CONSOLE_LOG("[ERROR] Resource Manager: checking for meta file, assets path was nullptr.");
		return false;
	}
	std::string path = assetsPath + std::string(META_EXTENSION);
	return std::filesystem::exists(path);
}

bool M_ResourceManager::ValidateMetaFile(const char* assetsPath, bool libraryCheck)
{
	bool ret = true;

	if (assetsPath == nullptr)
	{
		CONSOLE_LOG("[ERROR] Resource Manager: validating meta file, assetsPath was nullptr.");
		return false;
	}

	std::string metaPath = assetsPath + std::string(META_EXTENSION);

	if (!std::filesystem::exists(metaPath))
	{
		CONSOLE_LOG("[ERROR] Resource Manager: validating meta file, couldn't find meta file: %s", metaPath.c_str());
		return false;
	}

	JsonHandler jsonHandler;
	Json jsonMeta;

	ret = jsonHandler.LoadJson(jsonMeta, assetsPath);

	if (!jsonMeta.is_null())
	{
		std::string libraryPath = jsonMeta.at("library_path");

		if (!std::filesystem::exists(libraryPath))
		{
			CONSOLE_LOG("[ERROR] Resource Manager: validating library path, file doesn't exist.");
			return false;
		}

		UID uid = (UID)jsonMeta.at("uid");

		if (libraryCheck && (library.find(uid) == library.end()))
		{
			CONSOLE_LOG("[ERROR] Resource Manager: resource UID not found in library");
			return false;
		}

		if (!jsonMeta.at("contained_resources").empty())
		{
			for (const auto& resource : jsonMeta.at("contained_resources").items())
			{
				UID containedUid = resource.value().at("uid");

				std::string containedLibraryPath = jsonMeta.at("library_path");

				if (!std::filesystem::exists(containedLibraryPath))
				{
					CONSOLE_LOG("[ERROR] Resource Manager: validating library path, file doesn't exist.");
					return false;
				}
				if (libraryCheck && (library.find(containedUid) == library.end()))
				{
					CONSOLE_LOG("[ERROR] Resource Manager: contained resource UID not found in library");
					return false;
				}
			}
		}
	}
	return ret;
}

bool M_ResourceManager::ValidateMetaFile(Json& json, bool libraryCheck)
{
	bool ret = true;

	if (json.empty() || json.is_null())
	{
		CONSOLE_LOG("[ERROR] Resource Manager: validating meta file, json is null or empty.");
		return false;
	}

	std::string libraryPath = json.at("library_path");

	if (!std::filesystem::exists(libraryPath))
	{
		CONSOLE_LOG("[ERROR] Resource Manager: validating library path, file doesn't exist.");
		return false;
	}

	UID uid = (UID)json.at("uid");

	if (libraryCheck && (library.find(uid) == library.end()))
	{
		CONSOLE_LOG("[ERROR] Resource Manager: resource UID not found in library");
		return false;
	}

	if (!json.at("contained_resources").empty())
	{
		for (const auto& resource : json.at("contained_resources").items())
		{
			UID containedUid = resource.value().at("uid");

			std::string containedLibraryPath = json.at("library_path");

			if (!std::filesystem::exists(containedLibraryPath))
			{
				CONSOLE_LOG("[ERROR] Resource Manager: validating library path, file doesn't exist.");
				return false;
			}
			if (libraryCheck && (library.find(containedUid) == library.end()))
			{
				CONSOLE_LOG("[ERROR] Resource Manager: contained resource UID not found in library");
				return false;
			}
		}
	}
	return ret;
}

bool M_ResourceManager::ResourceHasMetaType(Resource* resource) const
{
	if (resource == nullptr)
	{
		LOG_BOTH("[ERROR] Resource Manager: checking meta type, resource was nullptr.");
		return false;
	}

	switch (resource->GetType())
	{
	case ResourceType::TEXTURE: { return true; } break;
	case ResourceType::SHADER: { return true; } break;
	default: break;
	}

	return false;
}

Resource* M_ResourceManager::CreateNewResource(const char* assetPath, ResourceType type)
{
	Resource* ret = new Resource(type);

	resourcesMap[ret->GetUID()] = ret;
	ret->SetAssetsPathAndFile(assetPath, engine->GetFileSystem()->GetFileName(assetPath));
	ret->SetLibraryPathAndFile();
	return ret;
}

bool M_ResourceManager::SaveMetaFile(Resource* resource) const
{
	if (resource == nullptr)
	{
		CONSOLE_LOG("[ERROR] Resource Manager: saving meta file, resource was nullptr.");
		return false;
	}

	Json metaFile;

	metaFile["uid"] = resource->GetUID();
	metaFile["type"] = (uint)resource->GetType();
	metaFile["name"] = resource->GetAssetFile();
	metaFile["assets_path"] = resource->GetAssetPath();
	metaFile["library_file"] = resource->GetLibraryFile();
	metaFile["library_path"] = resource->GetLibraryPath();
	//TODO: ADD MODIFICATION TIME

	resource->SaveMeta(metaFile); // This json doesn't go anywhere :/

	//TODO: Serialize to file & save file i think?
	//JsonHandler jsonHandler;

	return true;
}

bool M_ResourceManager::LoadMetaFile(Json& json, const char* assetPath)
{
	if (assetPath == nullptr)
	{
		LOG_BOTH("[ERROR] Resource Manager: loading meta file, assetPath was nullptr.");
		return false;
	}

	//TODO: I don't know what to do here, I think it's this:
	// engine->GetFileSystem()->OpenFile(assetPath);
	UID uid = (UID)json.at("uid");
	Resource* r = RequestResource(uid);

	if (r == nullptr)
	{
		LOG_BOTH("Error loading meta file, resource was not found.");
		return false;
	}

	ResourceType type = (ResourceType)json.at("type");
	if (r->GetType() != type)
	{
		LOG_BOTH("Error loading meta file, resource type missmatch.");
		return false;
	}


	r->SetUID(uid);
	r->SetAssetFile(json.at("name").get<std::string>().c_str());
	r->SetAssetPath(json.at("assets_path").get<std::string>().c_str());
	r->SetLibraryFile(json.at("library_file").get<std::string>().c_str());
	r->SetLibraryPath(json.at("library_path").get<std::string>().c_str());

	//r->LoadMeta(json); // I'm not sure about this since the function is empty

	return true;
}

Resource* M_ResourceManager::RequestResource(UID uid)
{
	// Find if the resource is already loaded
	std::map<UID, Resource*>::iterator it = resourcesMap.find(uid);

	if (it != resourcesMap.end())
	{
		it->second->ModifyReferenceCount(1);
		return it->second;
	}

	auto libIt = library.find(uid);
	if (libIt != library.end())
	{
		return GetResourceFromLibrary(libIt->second.assetsPath.c_str());
	}
	return nullptr;
}

UID M_ResourceManager::Find(const char* assetPath) const
{
	for (auto r : resourcesMap)
	{
		if (r.second->GetAssetPath() == assetPath)
			return r.first;
	}
	return -1;
}

void M_ResourceManager::SaveResource(Resource* resource)
{
	if (resource == nullptr)
	{
		LOG_BOTH("[ERROR] Resource Manager: saving resource, resource was nullptr.");
		return;
	}

	switch (resource->GetType())
	{
	case ResourceType::MESH:
		Importer::GetInstance()->meshImporter->Save((R_Mesh*)resource, resource->GetLibraryPath());
		break;
	case ResourceType::SCENE:
		Importer::GetInstance()->sceneImporter->Save((Scene*)resource);
		break;
	case ResourceType::FONT:
		//TODO: Save Font
		break;
	default:
		break;
	}

	if (ResourceHasMetaType(resource))
		SaveMetaFile(resource);

	library.emplace(resource->GetUID(), R_ResourceBase(resource));
}

bool M_ResourceManager::UnloadResource(Resource* resource)
{
	if (resource == nullptr)
	{
		CONSOLE_LOG("[ERROR] Resource Manager: trying to unload resource, resource was nullptr.");
		return false;
	}
	UID uid = resource->GetUID();
	resource->CleanUp();
	RELEASE(resource);
	if (resourcesMap.find(uid) != resourcesMap.end())
		resourcesMap.erase(uid);
	else
	{
		CONSOLE_LOG("[ERROR] Resource Manager: trying to unload resource, unloaded resource was not inside map!");
		return false;
	}
	return true;
}

bool M_ResourceManager::UnloadResource(UID uid)
{
	if (library.find(uid) != library.end())
		library.erase(uid);

	if (resourcesMap.find(uid) != resourcesMap.end())
	{
		Resource* r = resourcesMap[uid];
		r->CleanUp();
		RELEASE(r);
		resourcesMap.erase(uid);
	}
	else
	{
		CONSOLE_LOG("[ERROR] Resource Manager: trying to unload resource, resource was not inside map!");
		return false;
	}
	return true;
}

Resource* M_ResourceManager::GetResourceFromLibrary(const char* libraryPath)
{
	if (libraryPath == nullptr)
	{
		LOG_BOTH("[ERROR] Resource Manager: getting resource, library path was nullptr.");
		return nullptr;
	}

	UID uid = LoadFromLibrary(libraryPath);
	if (uid == 0)
	{
		LOG_BOTH("[ERROR] Resource Manager: getting resource from library, could not get resource uid from assests path.");
		return nullptr;
	}

	Resource* resource = RequestResource(uid);
	if (resource == nullptr)
	{
		LOG_BOTH("[ERROR] Resource Manager: getting resource from library, could not request resource.");
	}

	return resource;
}

UID M_ResourceManager::LoadFromLibrary(const char* libraryPath)
{
	std::string cleanPath = GetValidPath(libraryPath);
	if (cleanPath.c_str() == nullptr)
	{
		LOG_BOTH("[ERROR] Resource Manager: loading from library, couldn't validate path.");
		return -1;
	}

	Json jsonRoot;
	LoadMetaFile(jsonRoot, libraryPath);
	bool metaIsValid = ValidateMetaFile(jsonRoot);
	if (jsonRoot.empty())
	{
		LOG_BOTH("[ERROR] Resource Manager: loading from library, could not get the meta root node.");
		return 0;
	}
	if (!metaIsValid)
	{
		LOG_BOTH("[ERROR] Resource Manager: loading from library, could not validate meta root node.");
		return 0;
	}

	UID uid = (UID)jsonRoot.at("uid");

	if (resourcesMap.find(uid) != resourcesMap.end())
		return uid;

	//TODO: Allocate resource function

	for (const auto& resource : jsonRoot.at("resources").items())
	{
		std::string containedPath;
		std::string containedName = resource.value().at("name").get<std::string>();
		UID containedUid = (UID)resource.value().at("uid");

		//TODO: I don't know how to do this
		//
		//App->fileSystem->SplitFilePath(assetsPath, &containedPath, nullptr, nullptr);
		//containedName = containedNode.GetString("Name");
		//containedPath += containedName;

		if (resourcesMap.find(containedUid) != resourcesMap.end())
		{
			continue;
		}

		//TODO: Allocate resource function
		containedPath.clear();
		containedPath.shrink_to_fit();
		containedName.clear();
		containedName.shrink_to_fit();
	}

	return uid;
}

UID M_ResourceManager::ImportFromAssets(const char* assetsPath)
{
	UID uid;

	if (assetsPath == nullptr)
	{
		LOG_BOTH("[ERROR] Resource Manager: loading from assets, path was nullptr.");
		return -1;
	}

	//char* buffer = nullptr;
	//uint read = App->fileSystem->Load(assetsPath, &buffer);
	//if (read == 0)
	//{
	//	LOG("[ERROR] Resource Manager: Could not Import File %s! Error: File System could not Read the File.", assetsPath);
	//	return 0;
	//}

	ResourceType type = GetTypeFromExtension(assetsPath);
	Resource* resource = CreateNewResource(assetsPath, type);

	switch (type)
	{
	case ResourceType::MESH:
		//Importer::GetInstance()->meshImporter->Import();
		break;
	case ResourceType::TEXTURE:
		//Importer::GetInstance()->textureImporter->Import();
		break;
	case ResourceType::SCENE:
		//Importer::GetInstance()->sceneImporter->Import();
		break;
	case ResourceType::SHADER:
		break;
	case ResourceType::FONT:
		break;
	case ResourceType::UNKNOWN:
		break;
	default:
		break;
	}

	//RELEASE_ARRAY(buffer);

	uid = resource->GetUID();
	SaveResource(resource);
	UnloadResource(resource);

	return uid;
}

ResourceType M_ResourceManager::GetTypeFromExtension(const char* extension)
{
	ResourceType ret = ResourceType::UNKNOWN;

	// TODO
	//LUA?
	if (engine->GetFileSystem()->StringCompare(extension, TEXTURE_EXTENSION) == 0)
		ret = ResourceType::TEXTURE;
	else if (engine->GetFileSystem()->StringCompare(extension, ".fbx") == 0)
		ret = ResourceType::MESH;
	else if (engine->GetFileSystem()->StringCompare(extension, SCENE_EXTENSION) == 0)
		ret = ResourceType::SCENE;
	else if (engine->GetFileSystem()->StringCompare(extension, SHADER_EXTENSION) == 0)
		ret = ResourceType::SHADER;
	else if (engine->GetFileSystem()->StringCompare(extension, FONT_EXTENSION) == 0)
		ret = ResourceType::FONT;

	return ret;
}

const char* M_ResourceManager::GetAssetsDirectoryFromType(const ResourceType type)
{
	const char* ret = nullptr;

	switch (type)
	{
	case ResourceType::MESH:
		//TODO: Add dir for mesh? (shouldn't it be model & material)
		break;
	case ResourceType::TEXTURE:
		ret = ASSETS_TEXTURES_DIR;
		break;
	case ResourceType::SCENE:
		ret = ASSETS_SCENES_DIR;
		break;
	case ResourceType::SHADER:
		ret = ASSETS_SHADERS_DIR;
		break;
	case ResourceType::FONT:
		ret = ASSETS_FONTS_DIR;
		break;
	default:
		break;
	}

	return ret;
}

const char* M_ResourceManager::GetLibraryDirectoryFromType(const ResourceType type)
{
	const char* ret = nullptr;

	switch (type)
	{
	case ResourceType::MESH:
		ret = MESHES_DIR;
		break;
	case ResourceType::TEXTURE:
		ret = TEXTURES_DIR;
		break;
	case ResourceType::FONT:
		ret = FONT_DIR;
		break;
	default:
		break;
	}

	return ret;
}

void M_ResourceManager::DeleteFromAssets(const char* assetsPath)
{
	if (assetsPath == nullptr)
	{
		LOG_BOTH("[ERROR] Resource Manager: deleting from assets, assets path was nullptr.");
		return;
	}

	std::vector<UID> resourceUIDs;
	std::vector<std::string> toDelete;

	GetResourceUIDsFromMeta(assetsPath, resourceUIDs);
	GetLibraryFilePathsFromMeta(assetsPath, toDelete);

	std::string metaPath = assetsPath + std::string(META_EXTENSION);
	toDelete.push_back(assetsPath);
	toDelete.push_back(metaPath);

	for (uint i = 0; i < resourceUIDs.size(); ++i)
	{
		UnloadResource(resourceUIDs[i]);
	}

	for (uint i = 0; i < toDelete.size(); ++i)
	{
		std::filesystem::remove(toDelete[i].c_str());
	}

	toDelete.clear();
	toDelete.shrink_to_fit();
	resourceUIDs.clear();
	resourceUIDs.shrink_to_fit();
}

std::string M_ResourceManager::GetValidPath(const char* path) const
{
	std::string normalizedPath = path;

	for (uint i = 0; i < normalizedPath.size(); ++i)
	{
		if (normalizedPath[i] == '\\')
		{
			normalizedPath[i] = '/';
		}
	}

	size_t assetStart = normalizedPath.find("Assets");
	size_t libraryStart = normalizedPath.find("Library");
	std::string resultPath;
	if (assetStart != std::string::npos)
		resultPath = normalizedPath.substr(assetStart, normalizedPath.size()).c_str();
	else if (libraryStart != std::string::npos)
		resultPath = normalizedPath.substr(libraryStart, normalizedPath.size()).c_str();
	else
		LOG_BOTH("[ERROR] Resource Manager: Couldn't validate path.");

	return resultPath;
}

bool M_ResourceManager::HasImportIgnoredExtension(const char* assetsPath) const
{
	std::filesystem::path filePath = assetsPath;
	return (engine->GetFileSystem()->StringCompare(filePath.extension().string().c_str(), ".ini") == 0
		|| engine->GetFileSystem()->StringCompare(filePath.extension().string().c_str(), ".json") == 0
		|| engine->GetFileSystem()->StringCompare(filePath.extension().string().c_str(), ".ttf") == 0);
}
bool M_ResourceManager::SaveConfiguration(Json& configModule) const
{
	return true;
}

bool M_ResourceManager::LoadConfiguration(Json& configModule)
{
	return true;
}

bool M_ResourceManager::InspectorDraw()
{
	return true;
}