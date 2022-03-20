#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "Module.h"
#include "Globals.h"
#include "Resource.h"

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
	void UnloadResource(UID uid);
	void UnloadResource(Resource* resource);
	Resource* GetResourceFromLibrary(const char* libraryPath);
	UID LoadFromLibrary(const char* libraryPath);

	//const Resource* RequestResource(uint uid) const;			Can't do it because of the maps
	UID Find(const char* assetPath) const;
	Resource* RequestResource(UID uid);
	Resource::Type GetTypeFromExtension(const char* extension);
	const char* GetValidPath(const char* path) const;
	const char* GetFileName(const char* path) const;

	int StringCompare(const char* a, const char* b);

private:
	Resource* CreateNewResource(const char* assetPath, Resource::Type type);

private:
	KoFiEngine* engine = nullptr;

	std::map<UID, Resource*> resourcesMap;
	std::map<UID, std::string> library;

	float fileRefreshRate;
	float fileRefreshTime;
};

#endif // !__RESOURCE_MANAGER_H__