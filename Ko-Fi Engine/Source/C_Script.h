#pragma once

#include "Component.h"
#include <lua.hpp>
#include <sol.hpp>
#include <thread>
#include "M_SceneManager.h"
#include <queue>

#include "MathGeoLib/Math/float2.h"
#include "MathGeoLib/Math/float3.h"

class Scripting;
class GameObject;
class C_Transform;
class vector;
class InspectorVariable;
class C_Script;

using Json = nlohmann::json;

struct ScriptHandler
{
	ScriptHandler(GameObject* owner, C_Script* script);

	sol::protected_function_result script; // Check if it can be private
	Scripting* handler = nullptr;
	std::string path = "";
	std::vector<InspectorVariable*> inspectorVariables;
	sol::protected_function lua_start;
	bool lua_start_is_valid = false;
	sol::protected_function lua_event_handler;
	bool lua_event_handler_is_valid = false;
	sol::protected_function lua_update;
	bool lua_update_is_valid = false;
	sol::protected_function lua_update_UI;
	bool lua_update_UI_is_valid = false;
	sol::protected_function lua_post_update;
	bool lua_post_update_is_valid = false;
	bool isScriptLoaded = false;
};

struct ScriptingEvent
{
	ScriptingEvent(std::string _key, std::vector<std::variant<int, float, float2, float3, bool, std::string, std::vector<float3>, GameObject*>> _fields) {
		this->key = _key;
		this->fields = _fields;
	}

	std::string key;
	std::vector<std::variant<int, float, float2, float3, bool, std::string, std::vector<float3>, GameObject*>> fields;
};

class C_Script : public Component 
{
public:
	C_Script(GameObject* gameObject);
	~C_Script();

	bool Start() override;
	bool Update(float dt) override;
	static void InnerUpdate(float dt, ScriptHandler* sh, std::queue<ScriptingEvent>* queue, GameObject* go);
	bool PostUpdate(float dt) override;
	bool OnPlay() override;
	bool OnSceneSwitch() override;
	bool CleanUp() override;
	bool InspectorDraw(PanelChooser* chooser); // (OnGui)
	void Save(Json& json) const override;
	void Load(Json& json) override;
	void LoadInspectorVariables(Json& json);
	void RemoveOldVariables();
	void ReloadScript(ScriptHandler* script);

	void SetId(int id);



	static void UpdateInspectorVariables(float dt, ScriptHandler* sh, GameObject* go);
	static void UpdateEventHandler(float dt, ScriptHandler* sh, std::queue<ScriptingEvent>* queue);
	static void UpdateScript(float dt, ScriptHandler* sh);
	static void UpdateUIPlay(float dt, ScriptHandler* sh, GameObject* go);
	static void UpdateUIPause(float dt, ScriptHandler* sh, GameObject* go);
	static void PostUpdateScript(float dt, ScriptHandler* sh);

	ScriptHandler* s = nullptr;
	int id = -1;

	std::queue<ScriptingEvent> eventQueue;
	std::vector<std::shared_ptr<std::thread>> workers;
};