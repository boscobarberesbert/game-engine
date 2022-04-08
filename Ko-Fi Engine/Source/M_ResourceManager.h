#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "Module.h"
#include "Globals.h"
#include "Resource.h"

class R_ResourceBase;

class M_ResourceManager : public Module
{
public:
	M_ResourceManager(KoFiEngine* engine);
	~M_ResourceManager();

	bool Awake(Json configModule);
	bool Start();
	bool PreUpdate(float dt);
	bool CleanUp();
	void OnNotify(const Event& event);

	UID ImportFile(const char* assetPath);
	UID LoadFromLibrary(const char* libraryPath);
	UID ImportFromAssets(const char* assetsPath);
	void DeleteFromLibrary(const char* libraryPath);
	void DeleteFromAssets(const char* assetsPath);

	bool TrimLibrary();
	bool HasImportIgnoredExtension(const char* assetsPath) const;

	//const Resource* RequestResource(uint uid) const;			Can't do it because of the maps
	void SaveResource(Resource* resource);
	bool UnloadResource(UID uid);
	bool UnloadResource(Resource* resource);

	UID Find(const char* assetPath) const;
	Resource* GetResourceFromLibrary(const char* libraryPath);
	Resource* RequestResource(UID uid);

	ResourceType GetTypeFromExtension(const char* extension);

	const char* GetAssetsDirectoryFromType(const ResourceType);
	const char* GetLibraryDirectoryFromType(const ResourceType type);

	std::string GetValidPath(const char* path) const;

	void RefreshDirectoryFiles(const char* directory);
	void FindFilesToImport(std::vector<std::string>& assetFiles, std::vector<std::string>& metaFiles, std::map<std::string, std::string>& filePairs, std::vector<std::string>& toImport);
	void FindFilesToUpdate(std::map<std::string, std::string>& filePairs, std::vector<std::string>& toUpdate);
	void FindFilesToDelete(std::vector<std::string>& metaFiles, std::map<std::string, std::string>& filePairs, std::vector<std::string>& toDelete);
	void LoadFilesIntoLibrary(std::map<std::string, std::string>& filePairs);

	bool HasMetaFile(const char* assestsPath);
	bool ValidateMetaFile(const char* assetsPath, bool libraryCheck = true);
	bool ValidateMetaFile(Json& json, bool libraryCheck = true);
	bool ResourceHasMetaType(Resource* resource) const;

	bool LoadMetaFileIntoLibrary(const char* assetsPath);
	bool GetLibraryPairs(const char* assetsPath, std::map<UID, R_ResourceBase>& pairs);
	bool GetResourceUIDsFromMeta(const char* assetsPath, std::vector<UID>& uids);
	bool GetResourceBasesFromMeta(const char* assetsPath, std::vector<R_ResourceBase>& bases);
	bool GetLibraryFilePathsFromMeta(const char* assetsPath, std::vector<std::string>& paths);
	// Engine config serialization --------------------------------------
	bool SaveConfiguration(Json& configModule) const override;
	bool LoadConfiguration(Json& configModule) override;
	// ------------------------------------------------------------------

	// Engine config inspector draw -------------------------------------
	bool InspectorDraw() override;
	// ------------------------------------------------------------------

	//void ReleaseResource(uint uid);

private:
	Resource* CreateNewResource(const char* assetPath, ResourceType type);

	bool SaveMetaFile(Resource* resource) const;
	bool LoadMetaFile(Json& json, const char* assetPath);

private:
	KoFiEngine* engine = nullptr;

	std::map<UID, Resource*> resourcesMap;
	std::map<UID, R_ResourceBase> library;

	float fileRefreshRate;
	float fileRefreshTime;
};

#endif // !__RESOURCE_MANAGER_H__