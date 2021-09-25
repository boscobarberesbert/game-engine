#include "ImGUIHandler.h"
#include "MathGeoLib/Algorithm/Random/LCG.h"
#include "SceneIntro.h"
#include "Log.h"
#include "Camera3D.h"
#include "Renderer3D.h"
#include "Window.h"
#include "Primitive.h"

SceneIntro::SceneIntro(Camera3D* camera, Window* window, Renderer3D* renderer, ImGuiHandler* imGUIHandler) : Module()
{
	// Needed modules
	this->camera = camera;
	this->window = window;
	this->renderer = renderer;
	this->imGUIHandler = imGUIHandler;

	check = true;
	random = 0;
}

SceneIntro::~SceneIntro()
{}

// Load assets
bool SceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;
	camera->Move(vec3(1.0f, 1.0f, 0.0f));
	camera->LookAt(vec3(0, 0, 0));

	return ret;
}

// Load assets
bool SceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

// Update
bool SceneIntro::Update(float dt)
{
	Plane p(0, 1, 0, 0);
	p.axis = true;
	p.Render();

	/*LCG randomGenerator;
	if (check) {
		random = randomGenerator.Int(5, 10);
		check = false;
	}*/

	imGUIHandler->CreateWin("Test", "Alex feka");

	return true;
}

void SceneIntro::OnCollision(PhysBody3D* body1, PhysBody3D* body2)
{
	
}
