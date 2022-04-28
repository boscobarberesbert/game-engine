-- player = Find("Character")
name = "Level_1"

local nameIVT = INSPECTOR_VARIABLE_TYPE.INSPECTOR_STRING
nameIV = InspectorVariable.new("name", nameIVT, name)
NewVariable(nameIV)

------------------- Physics setter ----------------------
componentRigidBody = gameObject:GetRigidBody()
componentBoxCollider = gameObject:GetBoxCollider()
---------------------------------------------------------

-- Called each loop iteration
function Update(dt)
	--if (gameObject:GetButton():IsPressed() == true) then
		-- gameObject:LoadScene("HUD_Scene")
	--	gameObject:ChangeScene(true, name)
	--end
end

------------------ Collisions --------------------
function OnTriggerEnter(go)
	if (go.tag == Tag.PLAYER) then
		SetChangeSceneAndName(true, name)
	end
end
--------------------------------------------------

print("UI_ChangeScenes.lua compiled succesfully")