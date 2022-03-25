#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "Module.h"
#include "Globals.h"
#include "Resource.h"

class ResourceBase;

class ResourceManager : public Module
{
public:
	ResourceManager(KoFiEngine* engine);
	~ResourceManager();

	bool Start();
	bool PreUpdate(float dt);
	bool CleanUp();
	void OnNotify(const Event& event);

	UID ImportFile(const char* assetPath);
	void SaveResource(Resource* resource);
	bool UnloadResource(UID uid);
	bool UnloadResource(Resource* resource);
	Resource* GetResourceFromLibrary(const char* libraryPath);
	UID LoadFromLibrary(const char* libraryPath);
	void DeleteFromLibrary(const char* libraryPath);
	void DeleteFromAssets(const char* assetsPath);
	bool TrimLibrary();

	//const Resource* RequestResource(uint uid) const;			Can't do it because of the maps
	UID Find(const char* assetPath) const;
	Resource* RequestResource(UID uid);
	ResourceType GetTypeFromExtension(const char* extension);
	const char* GetAssetsDirectoryFromType(const ResourceType);
	const char* GetLibraryDirectoryFromType(const ResourceType type);
	const char* GetValidPath(const char* path) const;
	const char* GetFileName(const char* path) const;

	void RefreshDirectoryFiles(const char* directory);
	void FindFilesToImport(std::vector<std::string>& assetFiles, std::vector<std::string>& metaFiles, std::map<std::string, std::string>& filePairs, std::vector<std::string>& toImport);
	void FindFilesToUpdate(std::map<std::string, std::string>& filePairs, std::vector<std::string>& toUpdate);
	void FindFilesToDelete(std::vector<std::string>& metaFiles, std::map<std::string, std::string>& filePairs, std::vector<std::string>& toDelete);
	void LoadFilesIntoLibrary(std::map<std::string, std::string>& filePairs);

	bool HasMetaFile(const char* assestsPath);
	bool ValidateMetaFile(const char* assetsPath, bool library);
	bool LoadMetaFileIntoLibrary(const char* assetsPath);
	bool GetLibraryPairs(const char* assetsPath, std::map<UID, ResourceBase>& pairs);
	bool GetResourceUIDsFromMeta(const char* assetsPath, std::vector<UID>& uids);
	bool GetResourceBasesFromMeta(const char* assetsPath, std::vector<ResourceBase>& bases);
	bool GetLibraryFilePathsFromMeta(const char* assetsPath, std::vector<std::string>& paths);

private:
	Resource* CreateNewResource(const char* assetPath, ResourceType type);

private:
	KoFiEngine* engine = nullptr;

	std::map<UID, Resource*> resourcesMap;
	std::map<UID, std::string> library;

	float fileRefreshRate;
	float fileRefreshTime;
};

#endif // !__RESOURCE_MANAGER_H__