#pragma once
#ifndef IM_GUI_HANDLER_H
#define IM_GUI_HANDLER_H
#include "EditorStyleHandler.h"
#include "Module.h"
#include "Globals.h"
#include "Panel.h"

// Modules
class Window;
class Renderer3D;
class Input;
class FileSystem;

// Panels
class MainBar;
class PanelTest;
class PanelConfiguration;
class PanelLog;
class PanelAbout;
class PanelChooser;

struct EngineConfig;

struct Mesh;
class Editor : public Module
{
public:

	Editor(Window* window, Renderer3D* renderer,Input* input,EngineConfig* engineConfig,FileSystem* filesystem);
	~Editor();

	bool Awake(Json configModule);
	bool Start();
	bool PreUpdate(float dt);
	bool Update(float dt);
	bool PostUpdate(float dt);
	bool CleanUp();
	void AddPanel(Panel* panel);
	PanelChooser* GetPanelChooser();
	void Markdown(const std::string& markdown_);
	void MarkdownExample();
public:
	bool buttonPressed = false;

private:
	//Style
	EditorStyleHandler styleHandler;
	// Panels
	MainBar* mainMenuBar = nullptr;
	PanelTest* panelTest = nullptr;
	PanelConfiguration* panelConfig = nullptr;
	PanelLog* panelLog = nullptr;
	PanelAbout* panelAbout = nullptr;
	PanelChooser* panelChooser = nullptr;

	// Needed modules
	Window* window = nullptr;
	Renderer3D* renderer = nullptr;
	FileSystem* fileSystem = nullptr;
	KoFiEngine* engine = nullptr;

	std::list<Panel*> panels;
public:
	//Open/Close panel bools
	bool toggleAboutPanel = false;
	bool toggleChooserPanel = false;
	std::vector<Mesh> meshes;
};

#endif IM_GUI_HANDLER_H