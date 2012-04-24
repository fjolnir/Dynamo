jit.off() -- iOS doesn't allow jit (and there are some bugs with it)
ffi = require("ffi")
C = ffi.C

glm = require("glmath")
dynamo = require("dynamo")
local gl = require("OpenGL")

-- Setup dynamo
--local atlasPath = dynamo.pathForResource("character", "png")
--local atlas = dynamo.loadTextureAtlas(atlasPath, vec2_zero, vec2(32, 32))
--local sprite = dynamo.createSprite(vec3(100, 100,0), vec2(32, 32), atlas, {
	--{ numberOfFrames = 3, currentFrame = 0, loops = true },
	--{ numberOfFrames = 8, currentFrame = 0, loops = true },
	--{ numberOfFrames = 2, currentFrame = 0, loops = true },
	--{ numberOfFrames = 4, currentFrame = 0, loops = true },
	--{ numberOfFrames = 3, currentFrame = 0, loops = true },
	--{ numberOfFrames = 1, currentFrame = 0, loops = true },
	--{ numberOfFrames = 1, currentFrame = 0, loops = true },
	--{ numberOfFrames = 3, currentFrame = 0, loops = true }
--})
--sprite.activeAnimation = 1
--sprite.scale = 4

function update(timer)
end

dynamo.init(vec2(640, 980), 60, update)
gl.glClearColor(0,0,0,0)

local map = dynamo.loadMap(dynamo.pathForResource("maptest", "tmx"))

local mapLayers = {
	dynamo.createScene({ map:createLayerRenderable(0) }),
	dynamo.createScene({ map:createLayerRenderable(1) }),
	dynamo.createScene({ map:createLayerRenderable(2) }),
}
angle = 0
scene = dynamo.createScene({
	--sprite,
	mapLayers[1],
	mapLayers[2],
	mapLayers[3],
	dynamo.renderable(function(renderer, renderable, timeSinceLastFrame, interpolation)
		--dynamo.draw_lineSeg(vec2(0,0), dynamo.renderer.viewportSize, rgb(1,1,1))
		--for i=0,math.pi-0.05, 0.2 do
			--dynamo.draw_ellipse(vec2(640/2,980/2), vec2(200,60*math.sin(angle)), 50, angle+i, rgb(0,0,1), false)
		--end
		--scene:translate(dynamo.renderer.viewportSize.w/2, dynamo.renderer.viewportSize.h/2)
		--scene:rotate(0.1)
		--scene:translate(-dynamo.renderer.viewportSize.w/2, -dynamo.renderer.viewportSize.h/2)
		--angle = angle +0.05
		--local screenCoords, texOffsets = map.layers[0]:generateAtlasDrawInfo(map)
		--dynamo.draw_textureAtlas(mapAtlas, map.layers[0].numberOfTiles, texOffsets, screenCoords);
	end)
})
dynamo.renderer:pushRenderable(scene)

a = 0
lastPos = nil
dynamo.inputManager:addObserver({
	type = dynamo.kInputTouch_pan1,
	callback = function(manager, observer, location, state, metadata)
		location = location[0] -- Dereference the pointer
		if lastPos == nil then
			lastPos = { x=location.x, y=location.y }
		end

		local trans = vec2(location.x - lastPos.x, location.y - lastPos.y)
		scene:translate(trans.x, trans.y)
		mapLayers[1]:translate(-0.3*trans.x, -0.3*trans.y)
		mapLayers[3]:translate(1.03*trans.x, 1.03*trans.y)

		--scene:translate(10, 10)
		--sprite.flippedHorizontally = (lastPos.x > location.x)
		--sprite.location = vec3(location.x, location.y-60, 0)
		if state == dynamo.kInputState_down then
			lastPos.x = location.x
			lastPos.y = location.y
		else
			lastPos = nil
		end

		a = a+1
		if a%4 == 0 then
			--sprite.step(sprite)
		end
	end
})


function redraw()
	dynamo.cycle()
end

function cleanup()
	dynamo.cleanup()
end
