#include "C_Collider.h"

#include "GameObject.h"

#include "ComponentRigidBody.h"
#include "ComponentMesh.h"

#include "Physics.h"
#include "PxPhysicsAPI.h"

ComponentCollider2::ComponentCollider2(GameObject* parent, ColliderShape collType) : Component(parent)
{
	type = ComponentType::COLLIDER2;
	colliderShape = collType;

	CreateCollider(colliderShape);
}

ComponentCollider2::~ComponentCollider2()
{
	if (shape)
	{
		/*if (owner->GetComponent<ComponentRigidBody>())
			owner->GetComponent<ComponentRigidBody>()->GetRigidBody()->detachShape(*shape);*/
		shape->release();
		shape = nullptr;
	}
}

bool ComponentCollider2::Update(float dt)
{
	bool ret = true;

	if (hasUpdated)
	{
		ret = UpdateCollider();
		hasUpdated = false;
	}

	return ret;
}

bool ComponentCollider2::PostUpdate(float dt)
{
	bool ret = true;


	return ret;
}

// TODO: Separate updating between shape and state
bool ComponentCollider2::UpdateCollider()
{
	bool ret = true;

	CreateCollider(colliderShape);
	//// SHAPE UPDATE
	//if(shape)
	//	shape->release();

	//if (owner->GetComponent<ComponentMesh>())
	//	boxCollSize = owner->GetComponent<ComponentMesh>()->GetGlobalAABB().Size();

	//physx::PxVec3 localPos;
	//physx::PxBoxGeometry boxGeometry(boxCollSize.x / 2, boxCollSize.y / 2, boxCollSize.z / 2);

	//switch (GetColliderShape())
	//{
	//case ColliderShape::BOX:
	//	shape = owner->GetEngine()->GetPhysics()->GetPxPhysics()->createShape(boxGeometry, *owner->GetEngine()->GetPhysics()->GetPxMaterial());

	//	localPos = physx::PxVec3(centerPosition.x, centerPosition.y + boxCollSize.y / 2, centerPosition.z);
	//	shape->setLocalPose(physx::PxTransform(localPos));
	//	break;
	//case ColliderShape::SPHERE:

	//	break;
	//case ColliderShape::CAPSULE:

	//	break;
	//default:
	//	break;
	//}

	//// STATE UPDATE
	//if (shape)
	//{
	//	shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
	//	shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);

	//	physx::PxFilterData filterData;
	//	filterData.word0 = (int)GetCollisionLayer();

	//	shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
	//	shape->setSimulationFilterData(filterData);
	//	shape->setQueryFilterData(filterData);

	//	owner->GetComponent<ComponentRigidBody>()->GetRigidBody()->attachShape(*shape);
	//}

	return ret;
}

void ComponentCollider2::CreateCollider(ColliderShape collType)
{
	// SHAPE CREATION
	if (shape)
		shape->release();
	if (shape)
		owner->GetComponent<ComponentRigidBody>()->GetRigidBody()->detachShape(*shape);

	owner->GetEngine()->GetPhysics()->DeleteActor(owner->GetComponent<ComponentRigidBody>()->GetRigidBody());

	if (owner->GetComponent<ComponentMesh>())
		boxCollSize = owner->GetComponent<ComponentMesh>()->GetGlobalAABB().Size();
	
	/*if (owner->GetComponent<ComponentMesh>()->GetGlobalAABB().Size().x != boxCollSize.x || owner->GetComponent<ComponentMesh>()->GetGlobalAABB().Size().y != boxCollSize.y || owner->GetComponent<ComponentMesh>()->GetGlobalAABB().Size().z != boxCollSize.z)
		boxCollSize = owner->GetComponent<ComponentMesh>()->GetGlobalAABB().Size();*/

	physx::PxVec3 localPos;
	physx::PxTransform a;
	physx::PxBoxGeometry boxGeometry(boxCollSize.x / 2, boxCollSize.y / 2, boxCollSize.z / 2);

	switch (collType)
	{
	case ColliderShape::BOX:
		shape = owner->GetEngine()->GetPhysics()->GetPxPhysics()->createShape(boxGeometry, *owner->GetEngine()->GetPhysics()->GetPxMaterial());

		localPos = physx::PxVec3(centerPosition.x, centerPosition.y + boxCollSize.y / 2, centerPosition.z);
		shape->setLocalPose(physx::PxTransform(localPos));
		a = shape->getLocalPose();

		break;
	case ColliderShape::SPHERE:

		break;
	case ColliderShape::CAPSULE:

		break;
	default:
		break;
	}

	// STATE CREATION
	if (shape)
	{
		shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
		shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);

		physx::PxFilterData filterData;
		filterData.word0 = (int)GetCollisionLayer();

		shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		//shape->setSimulationFilterData(filterData);
		//shape->setQueryFilterData(filterData);

		owner->GetComponent<ComponentRigidBody>()->GetRigidBody()->attachShape(*shape);
	}


	owner->GetEngine()->GetPhysics()->AddActor(owner->GetComponent<ComponentRigidBody>()->GetRigidBody(), owner);
}

void ComponentCollider2::DrawCollider()
{
	if (drawCollider)
	{
		if (colliderShape == ColliderShape::BOX)
		{
			DrawBoxCollider();
		}
	}
}

void ComponentCollider2::DrawBoxCollider()
{

	float3 min = centerPosition - (boxCollSize / 2);
	//min = { centerPosition.x - boxCollSize.x / 2, centerPosition.y - boxCollSize.y / 2, centerPosition.z - boxCollSize.z  };
	float3 max = centerPosition + (boxCollSize / 2);
	//max = { centerPosition.x + boxCollSize.x / 2, centerPosition.y + boxCollSize.y / 2, centerPosition.z + boxCollSize.z / 2 };
	glLineWidth(2.0f);
	glColor3f(1.0f,0.0f,0.0f);
	//glPushMatrix();
	//glMultMatrixf(this->owner->GetTransform()->transformMatrix.Transposed().ptr());
	glBegin(GL_LINES);

	// Bottom 1
	glVertex3f(min.x, min.y, min.z);
	glVertex3f(max.x, min.y, min.z);

	glVertex3f(min.x, min.y, min.z);
	glVertex3f(min.x, max.y, min.z);

	glVertex3f(min.x, min.y, min.z);
	glVertex3f(min.x, min.y, max.z);

	// Bottom 2
	glVertex3f(max.x, min.y, max.z);
	glVertex3f(min.x, min.y, max.z);

	glVertex3f(max.x, min.y, max.z);
	glVertex3f(max.x, max.y, max.z);

	glVertex3f(max.x, min.y, max.z);
	glVertex3f(max.x, min.y, min.z);

	// Top 1
	glVertex3f(max.x, max.y, min.z);
	glVertex3f(min.x, max.y, min.z);

	glVertex3f(max.x, max.y, min.z);
	glVertex3f(max.x, min.y, min.z);

	glVertex3f(max.x, max.y, min.z);
	glVertex3f(max.x, max.y, max.z);

	// Top 2
	glVertex3f(min.x, max.y, max.z);
	glVertex3f(max.x, max.y, max.z);

	glVertex3f(min.x, max.y, max.z);
	glVertex3f(min.x, min.y, max.z);

	glVertex3f(min.x, max.y, max.z);
	glVertex3f(min.x, max.y, min.z);

	glEnd();
	glColor3f(1.f, 1.f, 1.f);
	glLineWidth(1.0f);
	/*glPopMatrix();*/

}

// Serialization 
void ComponentCollider2::Save(Json& json) const
{
	json["type"] = "collider2";

	json["collider_type"] = (int)colliderShape;
	json["collision_layer"] = (int)collisionLayer;

	json["enabled"] = enabled;
	json["is_trigger"] = isTrigger;
	json["draw_collider"] = drawCollider;

	json["box_collider_size"] = { boxCollSize.x, boxCollSize.y, boxCollSize.z };
	json["center_position"] = { centerPosition.x, centerPosition.y, centerPosition.z };
}
void ComponentCollider2::Load(Json& json)
{
	colliderShape = (ColliderShape)json.at("collider_type");
	collisionLayer = (CollisionLayer)json.at("collision_layer");

	enabled = json.at("enabled");
	isTrigger = json.at("is_trigger");
	drawCollider = json.at("draw_collider");

	std::vector<float> values = json.at("box_collider_size").get<std::vector<float>>();
	boxCollSize = float3(values[0], values[1], values[2]);
	values.clear();
	values = json.at("center_position").get<std::vector<float>>();
	centerPosition = float3(values[0], values[1], values[2]);
	values.clear();

	hasUpdated = true;
}

// On inspector draw
bool ComponentCollider2::InspectorDraw(PanelChooser* chooser)
{
	bool ret = true;

	if (ImGui::CollapsingHeader("Component Collider (physx)", ImGuiTreeNodeFlags_AllowItemOverlap))
	{
		DrawDeleteButton(owner, this);

		// COLLIDER SHAPE -----------------------------------------------------------------------------------------------
		ImGui::Text("Collider Shape:");
		// Take care with the order in the combo, it has to follow the ColliderShape enum class order
		ImGui::Combo("##combocollidershape", &colliderShapeInt, "Add Collider\0Box\0Sphere\0Capsule");
		ImGui::SameLine();
		if ((ImGui::Button("Assign##collidershape")))
		{
			switch (colliderShapeInt)
			{
			case (int)ColliderShape::NONE: break;
			case (int)ColliderShape::BOX: SetColliderShape((ColliderShape)colliderShapeInt); break;
			case (int)ColliderShape::SPHERE: /*SetColliderShape((ColliderShape)colliderShapeInt);*/ break;
			case (int)ColliderShape::CAPSULE: /*SetColliderShape((ColliderShape)colliderShapeInt);*/ break;
			}
			colliderShapeInt = 0; // This will reset the button to default when clicked
			hasUpdated = true;
		}
		ImGui::Text("Current collider shape: ");
		ImGui::SameLine();
		ImGui::Text("%s", GetColliderShapeString());

		ImGui::Separator();

		// COLLISION LAYER -----------------------------------------------------------------------------------------------
		ImGui::Text("Collison Layer:");
		// Take care with the order in the combo, it has to follow the CollisionLayer enum class order
		ImGui::Combo("##combocollisionlayer", &collisionLayerInt, "Default\0Player\0Enemy\0Bullet\0Terrain");
		ImGui::SameLine();
		if ((ImGui::Button("Assign##collisionlayer")))
		{
			switch (collisionLayerInt)
			{
			case (int)CollisionLayer::DEFAULT: SetCollisionLayer((CollisionLayer)collisionLayerInt); break;
			case (int)CollisionLayer::PLAYER: SetCollisionLayer((CollisionLayer)collisionLayerInt); break;
			case (int)CollisionLayer::ENEMY: SetCollisionLayer((CollisionLayer)collisionLayerInt); break;
			case (int)CollisionLayer::BULLET: SetCollisionLayer((CollisionLayer)collisionLayerInt); break;
			case (int)CollisionLayer::TERRAIN: SetCollisionLayer((CollisionLayer)collisionLayerInt); break;
			}
			collisionLayerInt = 0; // This will reset the button to default when clicked
			hasUpdated = true;
		}
		ImGui::Text("Current collision layer: ");
		ImGui::SameLine();
		ImGui::Text("%s", GetCollisionLayerString());
		ImGui::Separator();

		// ATTRIBUTES -----------------------------------------------------------------------------------------------
		if (ImGui::Checkbox("Enable##", &enabled)) hasUpdated = true;
		if (ImGui::Checkbox("IsTrigger##", &isTrigger)) hasUpdated = true;
		if (ImGui::Checkbox("Draw Collider##", &drawCollider)) hasUpdated = true;

		// COLLIDER CENTER POS & SIZE ----------------------------------------------------------------------------------------
		if (ImGui::TreeNodeEx("Size & Center position"))
		{
			float3 newSize2 = GetBoxCollSize();
			float newSize[3] = { newSize2.x, newSize2.y, newSize2.z };
			if (ImGui::DragFloat3("##boxcollsize", newSize))
			{
				boxCollSize = { newSize[0], newSize[1], newSize[2] };
			}
			ImGui::SameLine();
			ImGui::Text("Box collider size");

			float3 newCenterPos2 = GetCenterPosition();
			float newCenterPos[3] = { newCenterPos2.x, newCenterPos2.y, newCenterPos2.z };
			ImGui::InputFloat3("##centerpos", newCenterPos, "%.3f");
			ImGui::SameLine();
			ImGui::Text("Center position");

			ImGui::TreePop();
		}
		
	}
	else
		DrawDeleteButton(owner, this);

	return ret;
}

// Private methods
const char* ComponentCollider2::ColliderShapeToString(const ColliderShape collShape)
{
	switch (collShape)
	{
	case ColliderShape::NONE:
		return "NONE";
	case ColliderShape::BOX:
		return "BOX";
	case ColliderShape::CAPSULE:
		return "CAPSULE";
	case ColliderShape::SPHERE:
		return "SPHERE";
	default:
		return "ERROR, goto C_Collider.cpp refer 'ColliderShapeToString'";
	}
	return "ERROR, NO COLLIDER SHAPE";
}
const char* ComponentCollider2::CollisionLayerToString(const CollisionLayer collLayer)
{
	switch (collLayer)
	{
	case CollisionLayer::DEFAULT:
		return "DEFAULT";
	case CollisionLayer::PLAYER:
		return "PLAYER";
	case CollisionLayer::ENEMY:
		return "ENEMY";
	case CollisionLayer::BULLET:
		return "BULLET";
	case CollisionLayer::TERRAIN:
		return "TERRAIN";
	default:
		return "ERROR, goto C_Collider.cpp refer 'CollisionLayerToString'";
	}
	return "ERROR, NO COLLISION LAYER";
}

