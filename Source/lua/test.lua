local ffi = require("ffi")
local C = ffi.C
-- We need to actually load the dynamo dylib first because, dynamo.lua depends on glmath.lua, but glmath is compiled
-- into libdynamo^^;
ffi.load("../libdynamo.dylib", true)

local glfw = require("glfw")
local glm = require("glmath")
local dynamo = require("dynamo")

assert(glfw.glfwInit())
local viewport = vec2(800,600)

local window = glfw.glfwOpenWindow(viewport.w, viewport.h, glfw.GLFW_WINDOWED, "Dynamo Test", nil)
glfw.glfwSetInputMode(window, glfw.GLFW_STICKY_KEYS, 1)
glfw.glfwSwapInterval(1)

-- Setup dynamo
local atlas = dynamo.loadTextureAtlas("levels/character.png", vec2_zero, vec2(32, 32))
local sprite = dynamo.createSprite(vec3(100, 100,0), vec2(32, 32), atlas, {
	{ numberOfFrames = 3, currentFrame = 0, loops = true },
	{ numberOfFrames = 1, currentFrame = 0, loops = true },
	{ numberOfFrames = 1, currentFrame = 0, loops = true },
	{ numberOfFrames = 3, currentFrame = 0, loops = true },
	{ numberOfFrames = 4, currentFrame = 0, loops = true },
	{ numberOfFrames = 2, currentFrame = 0, loops = true },
	{ numberOfFrames = 8, currentFrame = 0, loops = true },
	{ numberOfFrames = 3, currentFrame = 0, loops = true }
})
sprite.activeAnimation = 6

function update(timer)
end

dynamo.init(viewport, 20, update)
dynamo.renderer:pushRenderable(sprite)

angle = 0
dynamo.renderer:pushRenderable(dynamo.renderable(function(renderer, owner, timeSinceLastFrame, interpolation)
	dynamo.draw_lineSeg(vec2(0,0), vec2(800,600), rgb(1,0,0))
	for i=0,math.pi, 0.2 do
		dynamo.draw_ellipse(vec2(400,300), vec2(100,30*math.sin(angle)), 500, angle+i, rgb(0,0,1), false)
	end
	angle = angle +0.1
end))

a = 0
lastPos = vec2(0,0)
dynamo.inputManager:addObserver({
	type = dynamo.kInputTouch_pan1,
	callback = function(manager, observer, location, state, metadata)
		sprite.flippedHorizontally = (lastPos.x > location.x)
		sprite.location = vec3(location.x, location.y, 0)
		lastPos = location
		a = a+1
		if a%4 == 0 then
			sprite.step(sprite)
		end
	end
})

glfw.glfwSetMousePosCallback(function(window, x, y)
	dynamo.postPanEvent(0, true, x, (viewport.h -y)+16)
end)

while glfw.glfwIsWindow(window) and glfw.glfwGetKey(window, glfw.GLFW_KEY_ESCAPE) ~= glfw.GLFW_PRESS do
	dynamo.cycle()
	glfw.glfwSwapBuffers()
	glfw.glfwPollEvents()
end
dynamo.cleanup()
glfw.glfwTerminate()
