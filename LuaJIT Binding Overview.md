# Dynamo LuaJIT Binding – *Draft*

## Components

* [Initialization & Lifetime](#init)
* [Texture](#texture)
* [Texture Atlas](#atlas)
* [Audio](#audio)
* [Sprite](#sprite)
* [Renderer](#renderer)
* [Scene](#scene)
* [Input](#input)
* [Time](#time)
* [Map](#map)
* [World (Physics)](#world)
* [Utilities](#utils)

## Commonly used terms
* **Renderable** An object that can be passed to the renderer to be automatically rendered on each frame.

<a name="init"></a>
# Initialization & lifetime

To initialize Dynamo you simply call

`dynamo.init(viewport, desiredFPS, updateCallback)`

* viewport(vec2): The viewport size
* desiredFPS(number): The preferred game updates per second
* updateCallback(function): a function to call on each iteration

	dynamo {
		renderer,
		input,
		audio,
		timer,
		world
	}

When you're done and want to exit you should call

`dynamo.cleanup()`

To draw a frame (and update game state if necessary) you should call

`dynamo.cycle()`

From your application's draw handler


<a name="texture"></a>
# Texture

To load a texture you use

`dynamo.texture.load(path, packingInfoPath)`

* path(string): The path to the image file to load. (PNGs preferred, but different platforms may support additional formats)
* packingInfoPath(string, optional): Path to a JSON file containing information about subtextures packed into the file to be loaded (Can be generated using the JSON export setting in TexturePacker)

Returns a texture object which you can either push directly as a renderable, or pass to a different object such as a sprite or a texture atlas.

A Texture has the following methods/properties

* `texture.location = vec3` 
* `texture:getSubTextureRect(texName)`
* `texture:getSubTextureOrigin(texName)`
* `texture:getSubTextureSize(texName)`
* `texture:getSubTextureAtlas(texName)` Returns an Atlas originating at the location of `texName` within `texture`


<a name="atlas"></a>
# Texture Atlas

To create a texture you'd use

`dynamo.atlas.create(texture, origin, size)` Which takes uses *texture*

or

`dynamo.atlas.load(path, origin, size)` which loads a texture from *path*

* origin(vec2): The pixel coordinates of the atlas origin within the texture
* size(vec2): The size of an individual cell in the atlas

Returns a texture atlas object which currently is only used in conjuction with a sprite

* `atlas.origin = vec2`
* `atlas.size = vec2`
* `atlas.margin = vec2` Gap between cells in the atlas
* `atlas.texture` The sampled texture


<a name="audio"></a>
# Audio

There are two types of audio objects, SFX & BGM. SFX being for shorter latency sensitive samples, and BGM for long running background tracks.

## SFX
To load a sfx you'd use

`dynamo.sound.sfx.load(path)`

Which returns a SoundEffect object with the following methods/properties

* `sfx:isPlaying()`
* `sfx:play()`
* `sfx.location = number` Sets the location of the sound source in 3D space
* `sfx.looping = boolean` Sets whether or not the sample loops
* `sfx.pitch = number`
* `sfx.volume = number`
* `sfx:stop()`
* `sfx:unload()` Unloads the sample from memory. You should do this yourself when you're done with the sample and not wait for the garbage collector to do it since there's only a limited amount of sample slots available (16 on Android for example)

## BGM
To load a BGM you'd use

`dynamo.sound.bgm.load(path)`

Which returns a BGM object with the following methods/properties

* `bgm:isPlaying()`
* `bgm:play()`
* `bgm:seek(seconds)` Seeks the track to the given location
* `bgm.volume = number`
* `bgm:stop()`
* `bgm:unload()` Unloads the track from memory. You should do this yourself when you're done with the sample and not wait for the garbage collector to do it since BGM tracks usually take up a large amount of memory.


<a name="sprite"></a>
# Sprite
To create a sprite you'd use

`dynamo.sprite.create(location, atlas, size, animations)`

* location(vec3/2): The location of the sprite in game space
* atlas: The texture atlas to sample 
* size(vec2): The size of the sprite in game space (Texture is stretched to fit) – Optional, if omitted the atlas size is used
* animations: An array of animation descriptions each one as follows: `{ numberOfFrames=number, currentFrame=number, loops=boolean }` – Optional, if omitted a single, single frame animation is created

Returns a sprite object with the following methods/properties

* `sprite:step()` Steps one frame forward in the sprite's current animation
* `sprite.location = vec3`
* `sprite.size = vec2` Sets the size of the sprite (Texture is stretched to fit)
* `sprite.scale = number`
* `sprite.angle = number` (In radians)
* `sprite.flippedVertically = boolean`
* `sprite.flippedHorizontally = boolean`
* `sprite.activeAnimation = number` The active animation (0 being the bottom one)
* `sprite.opacity = number` The opacity the sprite is drawn at (0.0-1.0)


## Batch sprite
A batch sprite allows multiple sprites that share the same texture to be rendered in a single draw call.

To create a batch sprite you'd use

`dynamo.sprite.createBatch(sprites)`

* `sprites` is an optional array of sprites to initialize the batch with

Returns a BatchSprite object with the following methods

* `batch:addSprite(sprite)`
* `batch:insertSprite(sprite, spriteToShift)`
* `batch:deleteSprite(sprite)`


<a name="renderer"></a>
# Renderer
The renderer is automatically created when you run `dynamo.init` and is accessible using `dynamo.renderer`. It has the following methods

* `dynamo.renderer:pushRenderable(renderable)`
* `dynamo.renderer:popRenderable(renderable)`
* `dynamo.renderer:insertRenderable(renderableToInsert, renderableToShiftUp)`
* `dynamo.renderer:deleteRenderable(renderable)`


<a name="scene"></a>
# Scene
A Scene is a group of renderables with a common transformation. It's useful to organize renderables into groups (and hierarchies by adding scenes inside scenes)

To create a scene you'd use

`dynamo.scene.create(renderables, initialTransform)`

* renderables: Array of renderables to initialize the scene with
* initialTransform(mat4): Optional and defaults to identity matrix

Returns a Scene object with the following methods/properties

* `scene:pushRenderable(renderable)`
* `scene:popRenderable(renderable)`
* `scene:insertRenderable(renderableToInsert, renderableToShiftUp)`
* `scene:deleteRenderable(renderable)`
* `scene:rotate(angle)`
* `scene:scale(xScale, yScale)`
* `scene:translate(xTrans, yTrans)`


<a name="input"></a>
# Input
The input handling system consists of two components. The input manager (Automatically created), and input observers.

The input manager (accessible using `dynamo.input.manager`) has the following methods

* `dynamo.input.manager:addObserver(observer)`
* `dynamo.input.manager:removeObserver(observer)`

To create an input observer you would call

`dynamo.input.manager:addObserver(observer)` where the observer is a dictionary describing an input observer. Such as:

	{
		type = type,
		handlerCallback = function(manager, observer, location, hasEnded)
	}

It returns a reference to the observer that can be passed to `manager:removeObserver`

Available input types are

	types = {
		key = {
			arrow<Up,Down,Left,Right>
			ascii
		}
		mouse = {
			<left,right>Click
			<left,right>Drag
			move
		}
		touch = { 1-5 } -- Array
	}


<a name="time"></a>
# Time
The time component consists of the `GameTimer` object & the `time` function. GameTimer manages the game loop and calls the update callback when necessary. `time()` returns the duration since launch. (Time is always expressed in seconds)

`GameTimer` is automatically created for you and accessible through `dynamo.timer`. It has the following methods/properties:

* `dynamo.timer.elapsed`
* `dynamo.timer.timeSinceLastUpdate`
* `dynamo.timer.ticks` The number of game loop iterations since update.
* `dynamo.timer.updateCallback` a callback function that is called once per game iteration
* `dynamo.timer:interpolation()` Calculates the current interpolation between game updates. (Useful for renderables to calculate a smooth interpolation)
* `dynamo.timer:afterDelay(delay, function)` Calls a `function` after a set `delay`
* `dynamo.timer:reset()` Resets the timer to 0

<a name="map"></a>
# Map
Dynamo has built in support for a subset of the TMX map format (Generated using [Tiled](http://www.mapeditor.org/))

To load a map you'd use

`dynamo.map.load(path)`

Which returns a Map object with the following methods/properties

* `map:getProperty(key)` Gets a map property (Set using your map editor)
* `map:getLayer(layerName)` Finds a map layer by name
* `map:getObjectGroup(groupName)` Finds an object group by name
* `map:createLayerRenderable(layerIndex)` Creates a renderable object that draws the map layer at `layerIndex`
* `map.width` in tiles
* `map.height` in tiles
* `map.tileWidth` in pixels
* `map.tileHeight`in pixels
* `map.orientation`
* `map.numberOfLayers`
* `map.layers`
* `map.numberOfTilesets`
* `map.tilesets`


<a name="world"></a>
# World (Physics)
Dynamo wraps [Chipmunk](http://chipmunk-physics.net) to implement it's physics simulation. The model consists of the following parts

* **World** The space in which the simulation occurs, entities not in the same world will not affect each other
* **Entities** An entity with a mass.
* **Shapes** Shapes are attached to entities in order to ..give them shape. A shape effectively defines the collision area of an entity and when a shape collides with a shape of a different entity, both entities have forces applied to them.
* **Joints/Constraints** Connect entities together to form more complicated objects.

The World is automatically created and can be accessed using `dynamo.world`. It has the following methods/properties

* `dynamo.world:gravity() / dynamo.world.gravity = vec2`
* `dynamo.world:pointQuery(point)` Returns the entity at `point` or `nil` if there is none
* `dynamo.world:addEntity(entity)`
* `dynamo.world:createEntity(owner, mass, momentOfInertia, shapes)`
	* `owner` An arbitrary pointer to provide context to callbacks
	* `mass`
	* `momentOfInertia`
	* `shapes` An array of shapes to initialize the entity with
* `dynamo.world:createCircleShape(center, radius)`
* `dynamo.world:createBoxShape(size)`
* `dynamo.world:createSegmentShape(a, b, thickness)`
* `dynamo.world:createPolyShape(vertices)`
* `dynamo.world:momentForCircle(mass, innerRadius, outerRadius, offsetFromEntityCenter)` Calculates moment of inertia for a circle
* `dynamo.world:momentForSegment(mass, a, b)` Calculates moment of inertia for a line segment
* `dynamo.world:momentForPoly(mass, vertices, offset)` Calculates moment of inertia for a polygon
* `dynamo.world:momentForBox(mass, size)` Calculates moment of inertia for a box

An Entity (created using `world:createEntity`) has the following methods/properties

* `entity:location() / entity.location = vec2`
* `entity:angle() / entity.angle = number`
* `entity:velocity() / entity.velocity = vec2`
* `entity:addShape(shape)`
* `entity:applyForce(forceVec)`
* `entity:applyImpulse(impulseVec)`
* `entity:addCircleShape(center, radius)`
* `entity:addBoxShape(size)`
* `entity:addSegmentShape(a, b, thickness)`
* `entity:addPolyShape(vertices)`
* `entity:createPinJoint(counterpart, anchorA, anchorB)`
* `entity:createSlideJoint(counterpart, anchorA, anchorB)`
* `entity:createPivotJoint(counterpart, pivot)`
* `entity:createGrooveJoint(counterpart, grooveStart, grooveEnd, anchorB)`
* `entity:createDampedSpring(counterpart, anchorA, anchorB, restLength, stiffness, damping)`
* `entity:createDampedRotarySpring(counterpart, restAngle, stiffness, damping)`
* `entity:creatRotaryLimitJoint(counterpart, minAngle, maxAngle)`
* `entity:createRatchetJoint(counterpart, phase, ratchet)`
* `entity:createGearJoint(counterpart, phase, ratio)`
* `entity:createSimpleMotorJoint(counterpart, rate)`

A Shape has the following methods/properties

* `shape:friction() / shape.friction = number`
* `shape:elasticity() / shape.elasticity = number`
* `shape:group() / shape.group = number` Shapes in the same group do not collide (0 indicates "no group")

For a detailed description of the different joint/constraint types I recommend looking at the [Chipmunk Documentation](http://chipmunk-physics.net/release/ChipmunkLatest-Docs/#ConstraintTypes). The only dynamo specific method is `constraint:invalidate()` which removes the constraint from the world and disconnects it from it's shapes for good.



<a name="utils"></a>
# Utilities

* `dynamo.log(arguments)` Prints a description of the passed arguments when building the host app with `-DDYNAMO_DEBUG` (Outputs nothing in release builds)
* `dynamo.pathForResource(name, ext, folder)` Returns the path for a resource matching the passed criteria (ext & folder are optional)
* `dynamo.platform()` Returns the platform you are currently running on
	* Currently available are: dynamo.platforms.<mac,ios,android,windows,other>
	