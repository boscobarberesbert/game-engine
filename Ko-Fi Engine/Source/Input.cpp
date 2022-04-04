#include "Globals.h"
#include "Input.h"
#include "Engine.h"
#include "SDL.h"
#include "Renderer3D.h"
#include "SceneManager.h"
#include "Editor.h"
#include "Log.h"
#include "ImGuiAppLog.h"
#include "Window.h"
#include "FileSystem.h"
#include "Importer.h"
#include "ComponentMaterial.h"
#include "Importer.h"
// FIXME: The list of meshes should be in scene intro.
#include "GameObject.h"
#include "Texture.h"

#include <imgui_impl_sdl.h>

#define MAX_KEYS 300

Input::Input(KoFiEngine* engine) : Module()
{
	name = "Input";

	keyboard = new KEY_STATE[MAX_KEYS];
	memset(keyboard, KEY_IDLE, sizeof(KEY_STATE) * MAX_KEYS);
	memset(mouse_buttons, KEY_IDLE, sizeof(KEY_STATE) * MAX_MOUSE_BUTTONS);
	this->engine = engine;
}

// Destructor
Input::~Input()
{
	CleanUp();
}

bool Input::Awake(Json configModule)
{
	bool ret = true;

	ret = LoadConfiguration(configModule);

	return ret;
}

// Called before render is available
bool Input::Init()
{
	CONSOLE_LOG("Init SDL input event system");
	appLog->AddLog("Init SDL input event system\n");
	bool ret = true;
	SDL_Init(0);

	if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		CONSOLE_LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		appLog->AddLog("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

	return ret;
}

// Called every draw update
bool Input::PreUpdate(float dt)
{
	SDL_PumpEvents();

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	for (int i = 0; i < MAX_KEYS; ++i)
	{
		if (keys[i] == 1)
		{
			if (keyboard[i] == KEY_IDLE)
				keyboard[i] = KEY_DOWN;
			else
				keyboard[i] = KEY_REPEAT;
		}
		else
		{
			if (keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN)
				keyboard[i] = KEY_UP;
			else
				keyboard[i] = KEY_IDLE;
		}
	}

	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

	mouse_x /= SCREEN_SIZE;
	mouse_y /= SCREEN_SIZE;
	mouse_z = 0;

	for (int i = 0; i < 5; ++i)
	{
		if (buttons & SDL_BUTTON(i))
		{
			if (mouse_buttons[i] == KEY_IDLE)
				mouse_buttons[i] = KEY_DOWN;
			else
				mouse_buttons[i] = KEY_REPEAT;
		}
		else
		{
			if (mouse_buttons[i] == KEY_REPEAT || mouse_buttons[i] == KEY_DOWN)
				mouse_buttons[i] = KEY_UP;
			else
				mouse_buttons[i] = KEY_IDLE;
		}
	}

	mouse_x_motion = mouse_y_motion = 0;

	bool quit = false;
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_MOUSEWHEEL:
			mouse_z = event.wheel.y;
			break;

		case SDL_MOUSEMOTION:
			mouse_x = event.motion.x / SCREEN_SIZE;
			mouse_y = event.motion.y / SCREEN_SIZE;

			mouse_x_motion = event.motion.xrel / SCREEN_SIZE;
			mouse_y_motion = event.motion.yrel / SCREEN_SIZE;
			break;

		case SDL_MOUSEBUTTONDOWN:
			for (auto m : engine->AllModules())
			{
				m->OnClick(event);
			}
			break;

		case SDL_QUIT:
			quit = true;
			break;

		case SDL_WINDOWEVENT:
		{
			switch (event.window.event)
			{
				//case SDL_WINDOWEVENT_LEAVE:
			case SDL_WINDOWEVENT_HIDDEN:
			case SDL_WINDOWEVENT_MINIMIZED:
				//SDL_MinimizeWindow(win->window);
				break;
				//case SDL_WINDOWEVENT_ENTER:
			case SDL_WINDOWEVENT_SHOWN:
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			case SDL_WINDOWEVENT_MAXIMIZED:
				//SDL_MaximizeWindow(win->window);
				break;
			case SDL_WINDOWEVENT_RESTORED:
				//windowEvents[WE_SHOW] = true;
				//SDL_RestoreWindow(win->window);
				break;
			}
			if (event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				engine->GetWindow()->SetWidth(event.window.data1);
				engine->GetWindow()->SetHeight(event.window.data2);
			}
			
			break;
		}
		//case (SDL_DROPFILE): {      // In case if dropped file
		//	dropped_filedir = event.drop.file;
		//	// Shows directory of dropped file
		//	SDL_ShowSimpleMessageBox(
		//		SDL_MESSAGEBOX_INFORMATION,
		//		"File dropped on window",
		//		dropped_filedir,
		//		window->window
		//	);
		//	SDL_free(dropped_filedir);    // Free dropped_filedir memory
		//	break;
		//}
		case SDL_DROPFILE:
		{
			std::string tmp;
			tmp.assign(event.drop.file);
			if (!tmp.empty())
			{
				if (tmp.find(".fbx") != std::string::npos)
				{
					Importer::GetInstance()->sceneImporter->Import(tmp.c_str());
				}
				else if ((tmp.find(".jpg") || tmp.find(".png")) != std::string::npos)
				{
					// Apply texture
					if (engine->GetEditor()->panelGameObjectInfo.selectedGameObjectID != -1)
					{
						GameObject* go = engine->GetSceneManager()->GetCurrentScene()->GetGameObject(engine->GetEditor()->panelGameObjectInfo.selectedGameObjectID);

						if (go->GetComponent<ComponentMaterial>())
						{
							Texture texture = Texture();
							Importer::GetInstance()->textureImporter->Import(tmp.c_str(), &texture);

							go->GetComponent<ComponentMaterial>()->texture = texture;
							//cMaterial->textures.push_back(texture);
						}
					}
				}
			}
			break;
		}
		}
	}
	ImGui_ImplSDL2_ProcessEvent(&event);

	if (quit == true || keyboard[SDL_SCANCODE_ESCAPE] == KEY_UP)
		return false;

	return true;
}

// Called before quitting
bool Input::CleanUp()
{
	CONSOLE_LOG("Quitting SDL input event subsystem");
	appLog->AddLog("Quitting SDL input event subsystem\n");

	delete[] keyboard;

	if(dropped_filedir)
		RELEASE(dropped_filedir);

	engine = nullptr;

	SDL_QuitSubSystem(SDL_INIT_EVENTS);

	return true;
}

// Method to receive and manage events
void Input::OnNotify(const Event& event)
{
	// Manage events
}

bool Input::SaveConfiguration(Json& configModule) const
{
	return true;
}

bool Input::LoadConfiguration(Json& configModule)
{
	return true;
}

bool Input::InspectorDraw()
{
	return true;
}
