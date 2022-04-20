------------------- Variables --------------------

lifeTime = 10.0	-- secs --iv required
lifeTimer = 0
effectRadius = 250.0
componentRigidBody = gameObject:GetRigidBody()

-------------------- Methods ---------------------

-- Called each loop iteration
function Update(dt)
	
	if (lifeTimer <= lifeTime) then
	
		lifeTimer = lifeTimer + dt
	else
		DeleteGameObject()
	end
end

-- Move to destination
function MoveToDestination(dt)

	local targetPos2D = { destination.x, destination.z }
	local pos2D = { componentTransform:GetPosition().x, componentTransform:GetPosition().z }
	local d = Distance(pos2D, targetPos2D)
	local vec2 = { targetPos2D[1] - pos2D[1], targetPos2D[2] - pos2D[2] }

	if (d > 5.0) then

		-- Movement
		vec2 = Normalize(vec2, d)
		if (componentRigidBody ~= nil) then
			componentRigidBody:Set2DVelocity(float2.new(vec2[1] * speed * dt, vec2[2] * speed * dt))
		end

		-- Rotation
		local rad = math.acos(vec2[2])
		if(vec2[1] < 0)	then
			rad = rad * (-1)
		end
		componentTransform:SetRotation(float3.new(componentTransform:GetRotation().x, componentTransform:GetRotation().y, rad))
	else
		
		destination = nil
		if (componentRigidBody ~= nil) then
			componentRigidBody:Set2DVelocity(float2.new(0,0))
		end
	end
end

function SetDestination()
	if (target ~= nil) then
		destination = target:GetTransform():GetPosition()

		local targetPos2D = { destination.x, destination.z }
		local pos2D = { componentTransform:GetPosition().x, componentTransform:GetPosition().z }
		local d = Distance(pos2D, targetPos2D)
		local vec2 = { targetPos2D[1] - pos2D[1], targetPos2D[2] - pos2D[2] }
		vec2 = Normalize(vec2, d)
		if (componentRigidBody ~= nil) then
			componentRigidBody:SetRigidBodyPos(float3.new(vec2[1] * 15, componentTransform:GetPosition().y, vec2[2] * 15))
		end
	end
end

--------------------------------------------------

print("Decoy.lua compiled succesfully")