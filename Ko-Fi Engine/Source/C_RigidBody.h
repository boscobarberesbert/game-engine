#ifndef __C_RIGIDBODY2_H__
#define __C_RIGIDBODY2_H__
#include "Component.h"
#include "reactphysics3d/reactphysics3d.h"

class C_RigidBody : public Component
{
public:
	//constructors
	C_RigidBody(GameObject* parent);
	~C_RigidBody();

	//Game Loop
	bool Start() override;
	bool Update(float dt) override;
	bool CleanUp() override;
	bool InspectorDraw(PanelChooser* chooser) override;//OnGUi

	void UpdateBodyType();
	void UpdateEnableGravity();
	void UpdateMass();
	void UpdateConstrains();

	//Serialization
	void Save(Json& json) const override;
	void Load(Json& json) override;

	//Getter & Setters
	inline reactphysics3d::RigidBody* GetBody() { return this->body; }

	inline std::string GetBodyType() const { return bodyType; };
	inline void SetBodyType(const std::string newBodyType) { bodyType = newBodyType; };

	inline bool GetUseGravity() const { return useGravity; };
	inline void SetUseGravity(const bool newUseGravity) { useGravity = newUseGravity; };

	inline float GetMass() const { return mass; };
	inline void SetMass(const float newMass) { mass = newMass; };

	inline void FreezePositionX(const bool freeze) { freezePositionX = freeze; }
	inline void FreezePositionY(const bool freeze) { freezePositionY = freeze; }
	inline void FreezePositionZ(const bool freeze) { freezePositionZ = freeze; }
	inline void FreezeRotationX(const bool freeze) { freezeRotationX = freeze; }
	inline void FreezeRotationY(const bool freeze) { freezeRotationY = freeze; }
	inline void FreezeRotationZ(const bool freeze) { freezeRotationZ = freeze; }
	inline void FreezePositions(const bool freezeX, const bool freezeY, const bool freezeZ) { freezePositionX = freezeX; freezePositionY = freezeY; freezePositionZ = freezeZ; }
	inline void FreezeRotations(const bool freezeX, const bool freezeY, const bool freezeZ) { freezeRotationX = freezeX; freezeRotationY = freezeY; freezeRotationZ = freezeZ; }
	inline bool GetFreezePositionX() const { return freezePositionX; }
	inline bool GetFreezePositionY() const { return freezePositionY; }
	inline bool GetFreezePositionZ() const { return freezePositionZ; }
	inline bool GetFreezeRotationX() const { return freezeRotationX; }
	inline bool GetFreezeRotationY() const { return freezeRotationY; }
	inline bool GetFreezeRotationZ() const { return freezeRotationZ; }

private:
	reactphysics3d::RigidBody* body = nullptr;

	// Variables serialized
	std::string bodyType = "Dynamic";
	bool useGravity = true;
	float mass = 1.0f;
	bool freezePositionX = false;
	bool freezePositionY = false;
	bool freezePositionZ = false;
	bool freezeRotationX = false;
	bool freezeRotationY = false;
	bool freezeRotationZ = false;
};

#endif // !__C_RIGIDBODY_H__
