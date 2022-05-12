#include "C_Animator.h"

// Modules
#include "Globals.h"
#include "Engine.h"
#include "Importer.h"
#include "M_ResourceManager.h"

// GameObject
#include "GameObject.h"
#include "C_Mesh.h"

// Resources
#include "R_Animation.h"

#include "FSDefs.h"
#include "Color.h"

#include <vector>
#include <string>
#include <map>

C_Animator::C_Animator(GameObject* parent) : Component(parent)
{
	type = ComponentType::ANIMATOR;
	animation = nullptr;

	createClipErrorMessage = false;
	deleteDefaultClipMessage = false;
}

C_Animator::~C_Animator()
{
	CleanUp();
}

bool C_Animator::Start()
{
	bool ret = false;

	// Setting default animation values for a GameObject without animation.
	if (animation == nullptr)
	{
		animation = new R_Animation();
		animation->SetName("Default animation");
		animation->SetDuration(10);
		animation->SetTicksPerSecond(24);
		animation->SetStartFrame(0);
		animation->SetEndFrame(10);
	}
	if (!selectedClip)
	{
		// Creating a default clip with 10 keyframes.
		ret = CreateClip(AnimatorClip(animation, "Default clip", 0, 10, 1.0f, true));
		SetSelectedClip("Default clip");
	}
	
	return ret;
}

bool C_Animator::Update(float dt)
{
	return true;
}

bool C_Animator::CleanUp()
{
	if (animation != nullptr)
	{
		owner->GetEngine()->GetResourceManager()->FreeResource(animation->GetUID());
		animation = nullptr;
	}

	clips.clear();

	if (selectedClip)
		selectedClip = nullptr;

	if (clipToDelete)
		clipToDelete = nullptr;

	C_Mesh* cMesh = owner->GetComponent<C_Mesh>();
	if (cMesh != nullptr)
		cMesh->GetMesh()->SetIsAnimated(false);

	return true;
}

bool C_Animator::InspectorDraw(PanelChooser* chooser)
{
	bool ret = true;

	if (ImGui::CollapsingHeader("Animator", ImGuiTreeNodeFlags_AllowItemOverlap))
	{
		if (DrawDeleteButton(owner, this))
			return true;

		ImGui::Text("Current Time: ");
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1));
		//if (ImGui::Selectable(mesh->path.c_str())) {}
		ImGui::PopStyleColor();

		ImGui::Text("Number of ticks: ");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "%d", 22);

		/*ImGui::Text("Animation State: ");
		if (ImGui::Checkbox("Playing", &playing))
			if(playing == true){}
			else{}*/

		// -- CLIP CREATOR
		ImGui::Text("Select Animation");
		
		ImGui::Text(animation->GetName().c_str());

		static char clipName[128] = "Enter Clip Name";
		ImGuiInputTextFlags inputTxtFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
		ImGui::InputText("Clip Name", clipName, IM_ARRAYSIZE(clipName), inputTxtFlags);

		ImGui::Text("Reel selector: ");
		int newStartFrame = animation->startFrame;
		if (ImGui::DragInt("Edit Start", &newStartFrame, 0, animation->duration))
			animation->startFrame = newStartFrame;
		int newEndFrame = animation->endFrame;
		if (ImGui::DragInt("Edit End", &newEndFrame, 0, animation->duration))
			animation->endFrame = newEndFrame;

		if (ImGui::Button("Create Clip", ImVec2(80, 35)))
		{
			if (animation->endFrame > animation->startFrame)
			{
				CreateClip(AnimatorClip(animation, clipName, animation->startFrame, animation->endFrame, 1.0f, true));
				createClipErrorMessage = false;
			}
			else
				createClipErrorMessage = true;
		}

		if (createClipErrorMessage)
			ImGui::TextColored(Red.ToImVec4(), "Please, select a valid clip interval.");

		ImGui::Text("Select Clip");
		if (ImGui::BeginCombo("Select Clip", ((selectedClip != nullptr) ? selectedClip->GetName().c_str() : "[SELECT CLIP]"), ImGuiComboFlags_None))
		{
			for (auto clip = clips.begin(); clip != clips.end(); ++clip)
			{
				if (ImGui::Selectable(clip->second.GetName().c_str(), (&clip->second == selectedClip), ImGuiSelectableFlags_None))
				{
					selectedClip = &clip->second;

					/*strcpy(editedName, selectedClip->GetName());
					editedStart = (int)selectedClip->GetStart();
					editedEnd = (int)selectedClip->GetEnd();
					editedSpeed = selectedClip->GetSpeed();
					editedLoop = selectedClip->IsLooped();

					editedMax = (selectedAnimation != nullptr) ? selectedAnimation->GetDuration() : 0;*/
				}
			}

			ImGui::EndCombo();
		}

		ImGui::Text("Delete Clip");
		if (ImGui::BeginCombo("Delete Clip", ((clipToDelete != nullptr) ? clipToDelete->GetName().c_str() : "[DELETE CLIP]"), ImGuiComboFlags_None))
		{
			for (auto clip = clips.begin(); clip != clips.end(); ++clip)
			{
				if (ImGui::Selectable(clip->second.GetName().c_str(), (&clip->second == clipToDelete), ImGuiSelectableFlags_None))
				{
					if (clip->second.GetName() != "Default clip")
					{
						clipToDelete = &clip->second;
						deleteDefaultClipMessage = false;
					}
					else
						deleteDefaultClipMessage = true;
				}
			}

			ImGui::EndCombo();
		}

		if (deleteDefaultClipMessage)
		{
			ImGui::TextColored(Red.ToImVec4(), "The default clip can't be deleted!");
			ImGui::TextColored(Red.ToImVec4(), "A clip is needed for the animation...");
		}

		if (ImGui::Button("Delete"))
		{
			if (clipToDelete == selectedClip)
				SetSelectedClip(std::string("Default clip"));

			if (clipToDelete)
			{
				clips.erase(clipToDelete->GetName().c_str());
				clipToDelete = nullptr;
			}
		}

		ImGui::Text("Clip Options: ");
		if (ImGui::Checkbox("Loop", &selectedClip->GetLoopBool()))
		{
			for (const auto& it : owner->GetParent()->children)
			{
				C_Animator* cAnim = it->GetComponent<C_Animator>();
				if(cAnim != nullptr)
					cAnim->selectedClip->SetLoopBool(selectedClip->GetLoopBool());
			}
		}

		/*ImGui::SameLine();
		if (ImGui::Button("Restart", ImVec2(70, 18)))
			Reset();*/
	}
	else
		DrawDeleteButton(owner, this);

	return ret;
}

void C_Animator::Save(Json& json) const
{
	json["type"] = (int)type;

	if (animation != nullptr)
	{
		json["animation"]["uid"] = animation->GetUID();
		json["animation"]["asset_path"] = animation->GetAssetPath();

		Json jsonClips;
		for (auto clip : clips)
		{
			jsonClips["mapString"] = clip.first.c_str();
			jsonClips["clipName"] = clip.second.GetName().c_str();
			jsonClips["clipStartFrame"] = clip.second.GetStartFrame();
			jsonClips["clipEndFrame"] = clip.second.GetEndFrame();
			jsonClips["clipDuration"] = clip.second.GetDuration();
			jsonClips["clipDurationInSeconds"] = clip.second.GetDurationInSeconds();
			jsonClips["clipLoop"] = clip.second.GetLoopBool();
			jsonClips["clipFinished"] = clip.second.GetFinishedBool();

			json["clips"].push_back(jsonClips);
		}
		json["selectedClip"] = selectedClip->GetName();
	}
}

void C_Animator::Load(Json& json)
{
	if (!json.empty())
	{
		RELEASE(animation);

		Json jsonAnimation = json.at("animation");

		UID uid = jsonAnimation.at("uid");
		owner->GetEngine()->GetResourceManager()->LoadResource(uid, jsonAnimation.at("asset_path").get<std::string>().c_str());
		animation = (R_Animation*)owner->GetEngine()->GetResourceManager()->RequestResource(uid);

		if (animation == nullptr)
			KOFI_ERROR(" Component Animation: could not load resource from library.");
		else
		{
			// Setting the animation resource to all the animated meshes of the owner's children.
			//for (GameObject* go : owner->GetChildren())
			//{
			//	C_Mesh* cMesh = go->GetComponent<C_Mesh>();
			//	if (cMesh != nullptr)
			//	{
			//		R_Mesh* rMesh = cMesh->GetMesh();
			//		if (rMesh->IsAnimated())
			//			rMesh->SetAnimation(animation);
			//	}
			//}

			for (const auto& clip : json.at("clips").items())
			{
				std::string key = clip.value().at("mapString");
				AnimatorClip c;
				c.SetName(clip.value().at("clipName").get<std::string>().c_str());
				c.SetStartFrame(clip.value().at("clipStartFrame"));
				c.SetEndFrame(clip.value().at("clipEndFrame"));
				c.SetDuration(clip.value().at("clipDuration"));
				c.SetDurationInSeconds(clip.value().at("clipDurationInSeconds"));
				c.SetLoopBool(clip.value().at("clipLoop"));
				c.SetFinishedBool(clip.value().at("clipFinished"));
				c.SetAnimation(animation);
				clips[key] = c;
			}
			SetSelectedClip(json.at("selectedClip"));
		}
	}
}

void C_Animator::Reset()
{
	//set the animation to the initial time value
}

bool C_Animator::CreateClip(const AnimatorClip& clip)
{
	if (clip.GetAnimation() == nullptr)
	{
		KOFI_ERROR(" Animator Component: Could not Add Clip { %s }! Error: Clip's R_Animation* was nullptr.", clip.GetName());
		return false;
	}
	if (clips.find(clip.GetName()) != clips.end())
	{ 
		KOFI_ERROR(" Animator Component: Could not Add Clip { %s }! Error: A clip with the same name already exists.", clip.GetName().c_str());
		return false;
	}

	clips.emplace(clip.GetName(), clip);
}

void C_Animator::SetAnimation(R_Animation* anim)
{
	if (this->animation != nullptr)
	{
		owner->GetEngine()->GetResourceManager()->FreeResource(this->animation->GetUID());
		this->animation = nullptr;
	}

	this->animation = anim;
}

AnimatorClip* C_Animator::GetSelectedClip()
{
	return selectedClip;
}

bool C_Animator::IsCurrentClipPlaying()
{
	return !GetSelectedClip()->GetFinishedBool();
}

bool C_Animator::IsCurrentClipLooping()
{
	return GetSelectedClip()->GetLoopBool();
}

void C_Animator::SetSelectedClip(std::string name)
{
	for (std::map<std::string, AnimatorClip>::iterator clip = clips.begin(); clip != clips.end(); ++clip)
	{
		if ((*clip).first == name)
		{
			selectedClip = &clip->second;
			break;
		}
	}
}
