#ifndef _COMPONENT_IMAGE_H_
#define _COMPONENT_IMAGE_H_

#include "Component.h"
#include "ComponentTransform2D.h"

#include "Texture.h"

#include "glew.h"

#include "Globals.h"
#include <string.h>
#include <vector>

#include "MathGeoLib/Math/float4x4.h"
#include "MathGeoLib/Math/float4.h"

class SDL_Texture;
class SDL_Surface;

class ComponentImage : public Component {
public:
	ComponentImage(GameObject* parent);
	~ComponentImage();

	bool Update(float dt) override;
	bool PostUpdate(float dt) override;
	bool InspectorDraw(PanelChooser* chooser) override;

	//SDL_Texture* LoadTexture(const char* path);
	//SDL_Texture* const LoadSurface(SDL_Surface* surface);

	void SetTexture(const char* path);
	float2 GetMask() { return mask; };

	//void OnLoad(const JSONReader& reader) override;
	//void OnSave(JSONWriter& writer) const override;

	Texture openGLTexture;

private:
	void FreeTextures();

	float2 mask = { 1.0f, 1.0f };
	GLuint fboId = 0;
};

#endif
