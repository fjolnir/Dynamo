jit.off() -- iOS doesn't allow jit (and there are some bugs with it)
ffi = require("ffi")
C = ffi.C
require("glmath")
dynamo = require("dynamo")
local gl = require("OpenGLES")


dynamo.init(vec2(640, 980), 60)
gl.glClearColor(0,0,0,0)

local boxShape = dynamo.createBoxShape(vec2(200,100))
boxShape.elasticity = 0.3
local box = dynamo.createEntity(dynamo.world, nil, 1, dynamo.world.momentForBox(1, vec2(200,100)), {boxShape})
box.location = vec2(110, 700)
dynamo.world:addEntity(box)
dynamo.log("Created box", tostring(box))

local circle = dynamo.createEntity(dynamo.world, nil, 1, dynamo.world.momentForCircle(1, 0, 70, vec2_zero), {
	dynamo.createCircleShape(vec2(0, 0), 70)
})
circle.location = vec2(200, 500)
dynamo.world:addEntity(circle)

local seg1 = dynamo.createSegmentShape(vec2(0,200), vec2(400, 0))
seg1.elasticity = 0.4
dynamo.world.staticEntity:addShape(seg1)
dynamo.world.staticEntity:addShape(dynamo.createSegmentShape(vec2(400,0), vec2(800, 200)))
dynamo.world.staticEntity:addShape(dynamo.createSegmentShape(vec2(0,0), vec2(0, 980)))
dynamo.world.staticEntity:addShape(dynamo.createSegmentShape(vec2(0,0), vec2(640, 0)))
dynamo.world.staticEntity:addShape(dynamo.createSegmentShape(vec2(0,980), vec2(640, 980)))
dynamo.world.staticEntity:addShape(dynamo.createSegmentShape(vec2(640,0), vec2(640, 980)))

dynamo.renderer:pushRenderable(
	dynamo.renderable(function(renderer, renderable, timeSinceLastFrame, interpolation)
		local loc = box:location()
		dynamo.world.drawEntity(box, true)
		dynamo.world.drawEntity(dynamo.world.staticEntity, true)
	end)
)


dynamo.timer.updateCallback = function(timer)
end

lastPos = nil
dynamo.inputManager:addObserver({
	type = dynamo.kInputTouch_pan1,
	callback = function(manager, observer, location, state, metadata)
		location = location[0] -- Dereference the pointer
		if lastPos == nil then
			lastPos = { x=location.x, y=location.y }
		end

		local entity = dynamo.world:pointQuery(location)
		print(entity)
		if entity ~= nil then
			local delta = vec2((location.x - lastPos.x)*4, (location.y - lastPos.y)*4)
			entity.location = location
		end

		if state == dynamo.kInputState_down then
			lastPos.x = location.x
			lastPos.y = location.y
		else
			lastPos = nil
		end
	end
})


function redraw()
	dynamo.cycle()
end

function cleanup()
	dynamo.cleanup()
end
