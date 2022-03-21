#ifndef __COMPONENT_ANIMATOR_H__
#define __COMPONENT_ANIMATOR_H__

#include <vector>
#include "Component.h"
#include "MathGeoLib/Math/float4x4.h"
#include "AnimatorClip.h"
#include <map>

class GameObject;
class Animation;

class ComponentAnimator : public Component
{
public:
	ComponentAnimator(GameObject* parent);
	~ComponentAnimator();

	bool Start();
	bool Update(float dt);
	bool CleanUp();
	bool InspectorDraw(PanelChooser* chooser);
	

	void Reset();

	bool CreateClip(const AnimatorClip& clip);

	void SetAnim(Animation* anim);

	bool playing;

	Animation* rAnim;
	
	std::map<std::string, AnimatorClip> clips;
	AnimatorClip* selectedClip;

};

#endif // __COMPONENT_ANIMATOR_H__
