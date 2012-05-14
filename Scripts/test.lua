jit.off() -- iOS doesn't allow jit (and there are some bugs with it)
ffi = require("ffi")
C = ffi.C
require("glmath")
dynamo = require("dynamo")
local gl = require("OpenGLES")


dynamo.init(vec2(640, 980), 60)
gl.glClearColor(0,0,0,0)

local box = dynamo.createEntity(dynamo.world, nil, 1, dynamo.world.momentForBox(1, vec2(200,200)), {
	dynamo.createBoxShape(vec2(200,200))
})
box.location = vec2(110, 400)
dynamo.world:addEntity(box)
dynamo.log("Created box")

dynamo.world.staticEntity:addShape(dynamo.createSegmentShape(vec2(0,200), vec2(400, 0)))
dynamo.world.staticEntity:addShape(dynamo.createSegmentShape(vec2(400,0), vec2(800, 200)))


dynamo.log("Created static box")

dynamo.renderer:pushRenderable(
	dynamo.renderable(function(renderer, renderable, timeSinceLastFrame, interpolation)
		local loc = box:location()
		print(loc.x, loc.y)
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
