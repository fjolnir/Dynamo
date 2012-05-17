jit.off() -- iOS doesn't allow jit (and there are some bugs with it)
ffi = require("ffi")
C = ffi.C
require("glmath")
dynamo = require("dynamo")
local gl = require("OpenGLES")

dynamo.init(vec2(640, 980), 24)
gl.glClearColor(0,0,0,0)

local d = dynamo
local dw = d.world

local carBodyMass = 3
local carBodySize = vec2(300, 90)
local carWheelMass = 1
local carWheelRadius = 25
local car = {
	body = dw:createEntity(nil, carBodyMass, dw.momentForBox(carBodyMass, carBodySize)),
	leftWheel = dw:createEntity(nil, carWheelMass, dw.momentForCircle(carWheelMass, 0, carWheelRadius, vec2_zero)),
	rightWheel = dw:createEntity(nil, carWheelMass, dw.momentForCircle(carWheelMass, 0, carWheelRadius, vec2_zero))
}
(car.body:addBoxShape(carBodySize)).group = 1;
local lws = car.leftWheel:addCircleShape(vec2_zero, carWheelRadius)
lws.group = 1
lws.friction = 0.7
local rws = car.rightWheel:addCircleShape(vec2_zero, carWheelRadius)
rws.group = 1
rws.friction = 0.7

car.body.location = vec2(250, 95+200)
car.leftWheel.location = vec2(120, 50+200)
car.rightWheel.location = vec2(380, 50+200)

--car.body:createPinJoint(car.leftWheel, vec2(-130, -45), vec2_zero)
--car.body:createPinJoint(car.rightWheel, vec2(130, -45), vec2_zero)
local loc = vec2(-130, -45)
car.body:createGrooveJoint(car.leftWheel, loc - vec2(0,30), loc + vec2(0, 20), vec2_zero)
car.body:createDampedSpring(car.leftWheel, loc, vec2_zero, 10, 60, 10)
loc = vec2(130, -45)
car.body:createGrooveJoint(car.rightWheel, loc - vec2(0,30), loc + vec2(0, 20), vec2_zero)
car.body:createDampedSpring(car.rightWheel, loc, vec2_zero, 20, 40, 10)


dw:addEntity(car.body)
dw:addEntity(car.leftWheel)
dw:addEntity(car.rightWheel)

--dynamo.world.staticEntity:addSegmentShape(vec2(0,200), vec2(400, 0))
--dynamo.world.staticEntity:addSegmentShape(vec2(400,0), vec2(800, 200))
dynamo.world.staticEntity:addSegmentShape(vec2(0,0), vec2(0, 980))
local floor = dynamo.world.staticEntity:addSegmentShape(vec2(0,0), vec2(640, 0))
floor.friction = 3
dynamo.world.staticEntity:addSegmentShape(vec2(0,980), vec2(640, 980))
dynamo.world.staticEntity:addSegmentShape(vec2(640,0), vec2(640, 980))
collectgarbage("collect")
dynamo.renderer:pushRenderable(
	dynamo.renderable(function(renderer, renderable, timeSinceLastFrame, interpolation)
		dynamo.world:draw(false)
	end)
)


dynamo.timer.updateCallback = function(timer)
	if timer.ticks % 2 == 0 then
	end
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
			selectedEntity:applyImpulse(delta, location - selectedEntity:location())
		end

		if state == dynamo.kInputState_down then
			lastPos.x = location.x
			lastPos.y = location.y
		else
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
