jit.off() -- iOS doesn't allow jit (and there are some bugs with it)
ffi = require("ffi")
C = ffi.C
require("glmath")
require("dynamo")
local gl = require("OpenGLES")

dynamo.init(vec2(640, 980), 24)
gl.glClearColor(0,0,0,0)

local d = dynamo
local dw = d.world

local map = dynamo.map.load(dynamo.pathForResource("maptest.tmx"))
dynamo.renderer:pushRenderable(map:createLayerRenderable(0))
dynamo.renderer:pushRenderable(map:createLayerRenderable(1))
dynamo.renderer:pushRenderable(map:createLayerRenderable(2))

dynamo.input.manager:addObserver({
    type = dynamo.input.types.touch[1],
    callback = function()
        dynamo.log("1")
    end
})

dynamo.input.manager:addObserver({
    type = dynamo.input.types.touch[2],
    callback = function()
        dynamo.log("2")
    end
})


dynamo.timer:afterDelay(4.0, function()
    print("delay over", dynamo.time())
end)

dynamo.timer:afterDelay(8.0, function()
    print("2nd delay over", dynamo.time())

    dynamo.timer:afterDelay(4.0, function()
    print("delay over AGAIN", dynamo.time())
end)

end)
