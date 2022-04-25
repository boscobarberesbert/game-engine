#include "C_CapsuleCollider.h"
#include "Engine.h"
#include "GameObject.h"
#include "M_Physics.h"
#include "C_RigidBody.h"

C_CapsuleCollider::C_CapsuleCollider(GameObject* parent) : Component(parent)
{
	type = ComponentType::CAPSULE_COLLIDER;

}

C_CapsuleCollider::~C_CapsuleCollider()
{
}

bool C_CapsuleCollider::Start()
{
	if (!collider)
	{
		float3 boundingBoxSize = owner->BoundingAABB().maxPoint - owner->BoundingAABB().minPoint;
		capsuleShape = owner->GetEngine()->GetPhysics()->GetPhysicsCommon().createCapsuleShape(boundingBoxSize.x / 2, boundingBoxSize.y / 2);
		reactphysics3d::Transform transform = reactphysics3d::Transform::identity();

		collider = owner->GetComponent<C_RigidBody>()->GetBody()->addCollider(capsuleShape, transform);
	}
	
	return true;
}

bool C_CapsuleCollider::Update(float dt)
{
	return true;
}

bool C_CapsuleCollider::CleanUp()
{
	return true;
}

bool C_CapsuleCollider::InspectorDraw(PanelChooser* chooser)
{
	if (ImGui::CollapsingHeader("Capsule Collider"))
	{
		std::string newFilter = GetFilter() == "" ? "Set Filter" : GetFilter();
		if (ImGui::BeginCombo("Set Filter##", newFilter.c_str()))
		{
			std::map<unsigned int, std::string> filterMap = owner->GetEngine()->GetPhysics()->GetFiltersMap();
			bool** filterMatrix = owner->GetEngine()->GetPhysics()->filterMatrix;
			for (auto iter = filterMap.begin(); iter != filterMap.end(); ++iter)
			{
				if (ImGui::Selectable(iter->second.c_str()))
				{
					SetFilter(iter->second);
					UpdateFilter();
				}
			}

			ImGui::EndCombo();
		}

		bool newIsTrigger = GetIsTrigger();
		if (ImGui::Checkbox("Is Trigger##", &newIsTrigger))
		{
			SetIsTrigger(newIsTrigger);
			UpdateIsTrigger();
		}

		ImGui::Text("Center");
		ImGui::SameLine();
		float3 newCenter = GetCenter();
		if (ImGui::DragFloat3("##center", &(newCenter[0]), 0.1f))
		{
			SetCenter(newCenter);
			UpdateCenter();
		}
		ImGui::Text("Scale");
		ImGui::SameLine();
		float2 newScaleFactor = GetScaleFactor();
		if (ImGui::DragFloat2("##scale", &(newScaleFactor[0]), 0.1f, 1.0f, 50000.0f))
		{
			SetScaleFactor(newScaleFactor);
			UpdateScaleFactor();
		}
	}
	return true;
}

void C_CapsuleCollider::Save(Json& json) const
{
	json["type"] = "capsuleCollider";

	json["filter"] = filter;
	json["is_trigger"] = isTrigger;
	json["scale_factor"] = { scaleFactor.x, scaleFactor.y};
	json["center"] = { center.x, center.y, center.z };
}

void C_CapsuleCollider::Load(Json& json)
{
	filter = json.at("filter");
	UpdateFilter();

	isTrigger = json.at("is_trigger");
	UpdateIsTrigger();

	std::vector<float> values = json.at("scale_factor").get<std::vector<float>>();
	scaleFactor = float2(values[0], values[1]);
	values.clear();
	UpdateScaleFactor();

	values = json.at("center").get<std::vector<float>>();
	center = float3(values[0], values[1], values[2]);
	values.clear();
	UpdateCenter();
}

void C_CapsuleCollider::UpdateFilter()
{
	std::map<unsigned int, std::string> filterMap = owner->GetEngine()->GetPhysics()->GetFiltersMap();
	bool** filterMatrix = owner->GetEngine()->GetPhysics()->filterMatrix;
	for (auto iter : filterMap)
	{
		if (iter.second == filter)
		{
			collider->setCollisionCategoryBits(iter.first);
			unsigned int mask = 0;
			for (int i = 0; i < filterMap.size(); ++i)
			{
				if (filterMatrix[iter.first - 1][i])
				{
					mask |= i + 1;
				}
			}
			collider->setCollideWithMaskBits(mask);
		}
	}
}

void C_CapsuleCollider::UpdateScaleFactor()
{
	float3 boundingBoxSize = owner->BoundingAABB().maxPoint - owner->BoundingAABB().minPoint;
	reactphysics3d::Transform oldTransform = collider->getLocalToBodyTransform();
	owner->GetComponent<C_RigidBody>()->GetBody()->removeCollider(collider);
	owner->GetEngine()->GetPhysics()->GetPhysicsCommon().destroyCapsuleShape(capsuleShape);
	capsuleShape = owner->GetEngine()->GetPhysics()->GetPhysicsCommon().createCapsuleShape((boundingBoxSize.x / 2) * scaleFactor.x, (boundingBoxSize.y / 2) * scaleFactor.y);
	collider = owner->GetComponent<C_RigidBody>()->GetBody()->addCollider(capsuleShape, oldTransform);
}

void C_CapsuleCollider::UpdateIsTrigger()
{
	collider->setIsTrigger(isTrigger);
}

void C_CapsuleCollider::UpdateCenter()
{
	reactphysics3d::Transform newCenterTransform(reactphysics3d::Vector3(center.x, center.y, center.z), collider->getLocalToBodyTransform().getOrientation());
	collider->setLocalToBodyTransform(newCenterTransform);
}
