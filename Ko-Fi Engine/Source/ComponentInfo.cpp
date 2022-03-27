#include "ComponentInfo.h"
#include "GameObject.h"
#include "Engine.h"
#include "SceneManager.h"

#include "imgui_stdlib.h"
#include "PanelChooser.h"

ComponentInfo::ComponentInfo(GameObject* parent) : Component(parent)
{
	type = ComponentType::INFO;
}

ComponentInfo::~ComponentInfo()
{}

bool ComponentInfo::CleanUp()
{
	return true;
}

bool ComponentInfo::Update(float dt)
{
	return true;
}

bool ComponentInfo::InspectorDraw(PanelChooser* chooser)
{
	bool ret = true;

	if (ImGui::CollapsingHeader("Info"))
	{
		ImGui::Text("Name:");
		ImGui::SameLine();
		ImGui::InputText("##Name", &(owner->name));
		ImGui::Checkbox("Active", &owner->active);
		ImGui::SameLine();

		// Take care with the order in the combo, it has to follow the Tag enum class order
		if (ImGui::Combo("##tagcombo", &tag, "Untagged\0Player\0Enemy\0Wall"))
		{
			if (owner->tag != (Tag)tag)
				owner->tag = (Tag)tag;
		}
	}

	return ret;
}

void ComponentInfo::Save(Json& json) const
{
	json["type"] = "info";
}

void ComponentInfo::Load(Json& json)
{
}