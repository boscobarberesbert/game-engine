//#pragma once
#include "Component.h"
#include "Mesh.h"
#include "par_shapes.h"
#include "MathGeoLib/Geometry/OBB.h"
#include "MathGeoLib/Geometry/AABB.h"

class ComponentTransform;
class ComponentMaterial;

class ComponentMesh : public Component
{
public:
	enum class Shape
	{
		CUBE,
		SPHERE,
		CYLINDER
	};
//
	ComponentMesh(GameObject* parent);
	//ComponentMesh(GameObject* parent,Shape shape);
	~ComponentMesh();
	//void CopyParMesh(par_shapes_mesh* parMesh);
//
	bool Start(const char* path);
	bool PreUpdate();
	bool Update();
	bool PostUpdate();
	bool CleanUp();
//
//	void LoadMesh(const char* path);
	bool InspectorDraw(PanelChooser* chooser);
//
	uint GetVertices();
	void SetMesh(Mesh* mesh);
	Mesh* GetMesh();
//
//public:
//	ComponentMaterial* materialComponent;
//

	void SetPath(std::string path);
	void SetVertexNormals(bool vertexNormals);
	void SetFacesNormals(bool facesNormals);

	GameObject* GetParent();
	std::string GetPath();
	bool GetVertexNormals();
	bool GetFacesNormals();
	void GenerateBounds();
	// Functions for the bounding boxes
	AABB GetAABB();
	void GetGlobalBoundingBox();
	void DrawBoundingBox(const AABB& aabb, const float3& rgb);

private:
	//Bounding sphere
	float3 centerPoint = float3::zero;
	float radius;

	std::string path = "";
	Mesh* mesh = nullptr;
//	//COMPONENT_SUBTYPE subtype = COMPONENT_SUBTYPE::COMPONENT_MESH_MESH;
//	// Checkboxes vertex and faces bools to toggle
	bool vertexNormals = false;
	bool facesNormals = false;

	// Bounding boxes
	OBB obb;
	AABB aabb;
};