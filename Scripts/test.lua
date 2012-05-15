jit.off() -- iOS doesn't allow jit (and there are some bugs with it)
ffi = require("ffi")
C = ffi.C
require("glmath")
dynamo = require("dynamo")
local gl = require("OpenGLES")


dynamo.init(vec2(640, 980), 60)
gl.glClearColor(0,0,0,0)

local box = dynamo.createEntity(dynamo.world, nil, 1, dynamo.world.momentForBox(1, vec2(200,100)), {
	 dynamo.createBoxShape(vec2(200,100))
})
box.location = vec2(110, 700)

local circle = dynamo.createEntity(dynamo.world, nil, 1, dynamo.world.momentForCircle(1, 0, 70, vec2_zero), {
	dynamo.createCircleShape(vec2(0, 0), 70)
})
circle.elasticity = 1
circle.location = vec2(200, 500)

circle.collisionHandler = function(entity, world, collisionInfo)
	if collisionInfo.b == box then
		local pt = collisionInfo.contactPoints.points[0].point
		print("box and circle collided", pt.x, pt.y)
	else
		print("circle collided")
	end
end

circle:createPinJoint(box, vec2(0, 60), vec2(-20,-20))

dynamo.world:addEntity(box)
dynamo.world:addEntity(circle)

dynamo.world.staticEntity:addShape(dynamo.createSegmentShape(vec2(0,200), vec2(400, 0)))
dynamo.world.staticEntity:addShape(dynamo.createSegmentShape(vec2(400,0), vec2(800, 200)))
dynamo.world.staticEntity:addShape(dynamo.createSegmentShape(vec2(0,0), vec2(0, 980)))
dynamo.world.staticEntity:addShape(dynamo.createSegmentShape(vec2(0,0), vec2(640, 0)))
dynamo.world.staticEntity:addShape(dynamo.createSegmentShape(vec2(0,980), vec2(640, 980)))
dynamo.world.staticEntity:addShape(dynamo.createSegmentShape(vec2(640,0), vec2(640, 980)))

dynamo.renderer:pushRenderable(
	dynamo.renderable(function(renderer, renderable, timeSinceLastFrame, interpolation)
		dynamo.world:draw(false)
	end)
)


dynamo.timer.updateCallback = function(timer)
end

lastPos = nil
selectedEntity = nil
dynamo.inputManager:addObserver({
	type = dynamo.kInputTouch_pan1,
	callback = function(manager, observer, location, state, metadata)
		location = location[0] -- Dereference the pointer
		if lastPos == nil then
			lastPos = { x=location.x, y=location.y }
		end

		if selectedEntity == nil then
			selectedEntity = dynamo.world:pointQuery(location)
			print(selectedEntity)
		else
			local delta = vec2((location.x - lastPos.x)*4, (location.y - lastPos.y)*4)
			--selectedEntity.location = location
			selectedEntity:applyImpulse(delta, vec2_zero)
		end

		if state == dynamo.kInputState_down then
			lastPos.x = location.x
			lastPos.y = location.y
		else
			print("Touch ended")
			lastPos = nil
			selectedEntity = nil
		end
	end
})


function redraw()
	dynamo.cycle()
end

function cleanup()
	dynamo.cleanup()
end
