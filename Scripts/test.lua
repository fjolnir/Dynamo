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

local map = dynamo.loadMap(dynamo.pathForResource("maptest.tmx"))
dynamo.renderer:pushRenderable(map:createLayerRenderable(0))
--dynamo.renderer:pushRenderable(map:createLayerRenderable(1))
--dynamo.renderer:pushRenderable(map:createLayerRenderable(2))


function redraw()
	dynamo.cycle()
end

function cleanup()
	dynamo.cleanup()
end
