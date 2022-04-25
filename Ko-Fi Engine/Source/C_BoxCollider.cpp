#include "C_BoxCollider.h"
#include "Engine.h"
#include "Globals.h"
#include "GameObject.h"
#include "M_Physics.h"
#include "C_RigidBody.h"

C_BoxCollider::C_BoxCollider(GameObject *parent) : Component(parent)
{
	type = ComponentType::BOX_COLLIDER;
}

C_BoxCollider::~C_BoxCollider()
{
}

bool C_BoxCollider::Start()
{
	if (!collider)
	{
		float3 boundingBoxSize = owner->BoundingAABB().maxPoint - owner->BoundingAABB().minPoint;

		if (boundingBoxSize.x != 0 && boundingBoxSize.y != 0 && boundingBoxSize.z != 0)
		{
			boxShape = owner->GetEngine()->GetPhysics()->GetPhysicsCommon().createBoxShape(reactphysics3d::Vector3(boundingBoxSize.x / 2, boundingBoxSize.y / 2, boundingBoxSize.z / 2));
		}
		else
		{
			boxShape = owner->GetEngine()->GetPhysics()->GetPhysicsCommon().createBoxShape(reactphysics3d::Vector3(1, 1, 1));
		}
		reactphysics3d::Transform transform = reactphysics3d::Transform::identity();
		collider = owner->GetComponent<C_RigidBody>()->GetBody()->addCollider(boxShape, transform);
	}
	return true;
}

bool C_BoxCollider::Update(float dt)
{
	return true;
}

bool C_BoxCollider::CleanUp()
{
	return true;
}

bool C_BoxCollider::InspectorDraw(PanelChooser *chooser)
{
	if (ImGui::CollapsingHeader("Box Collider"))
	{
		std::string newFilter = GetFilter() == "" ? "Set Filter" : GetFilter();
		if (ImGui::BeginCombo("Set Filter##", newFilter.c_str()))
		{
			std::map<unsigned int, std::string> filterMap = owner->GetEngine()->GetPhysics()->GetFiltersMap();
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
		float3 newScaleFactor = GetScaleFactor();
		if (ImGui::DragFloat3("##scale", &(newScaleFactor[0]), 0.1f, 1.0f, 50000.0f))
		{
			SetScaleFactor(newScaleFactor);
			UpdateScaleFactor();
		}
	}
	return true;
}

void C_BoxCollider::Save(Json &json) const
{
	json["type"] = "boxCollider";

	json["filter"] = filter;
	json["is_trigger"] = isTrigger;
	json["scale_factor"] = {scaleFactor.x, scaleFactor.y, scaleFactor.z};
	json["center"] = {center.x, center.y, center.z};
}

void C_BoxCollider::Load(Json &json)
{
	filter = json.at("filter");
	UpdateFilter();

	isTrigger = json.at("is_trigger");
	UpdateIsTrigger();

	std::vector<float> values = json.at("scale_factor").get<std::vector<float>>();
	scaleFactor = float3(values[0], values[1], values[2]);
	values.clear();
	UpdateScaleFactor();

	values = json.at("center").get<std::vector<float>>();
	center = float3(values[0], values[1], values[2]);
	values.clear();
	UpdateCenter();

	hasUpdated = true;
}

void C_BoxCollider::UpdateFilter()
{
	std::map<unsigned int, std::string> filterMap = owner->GetEngine()->GetPhysics()->GetFiltersMap();
	bool **filterMatrix = owner->GetEngine()->GetPhysics()->filterMatrix;
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

void C_BoxCollider::UpdateScaleFactor()
{
	float3 boundingBoxSize = owner->BoundingAABB().maxPoint - owner->BoundingAABB().minPoint;
	reactphysics3d::Transform oldTransform = collider->getLocalToBodyTransform();
	owner->GetComponent<C_RigidBody>()->GetBody()->removeCollider(collider);
	owner->GetEngine()->GetPhysics()->GetPhysicsCommon().destroyBoxShape(boxShape);
	boxShape = owner->GetEngine()->GetPhysics()->GetPhysicsCommon().createBoxShape(reactphysics3d::Vector3((boundingBoxSize.x / 2) * scaleFactor.x, (boundingBoxSize.y / 2) * scaleFactor.y, (boundingBoxSize.z / 2) * scaleFactor.z));
	collider = owner->GetComponent<C_RigidBody>()->GetBody()->addCollider(boxShape, oldTransform);
}

void C_BoxCollider::UpdateIsTrigger()
{
	collider->setIsTrigger(isTrigger);
}

void C_BoxCollider::UpdateCenter()
{
	reactphysics3d::Transform newCenterTransform(reactphysics3d::Vector3(center.x, center.y, center.z), collider->getLocalToBodyTransform().getOrientation());
	collider->setLocalToBodyTransform(newCenterTransform);
}
