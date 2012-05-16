-- There's a crasher bug in JIT that I haven't figured out a way to work around
--jit.off()

local ffi = require("ffi")
local lib = ffi.C
local gl = require("OpenGLES")

local dynamo = setmetatable({}, { __index = lib })

ffi.cdef[[
typedef float GLMFloat;

typedef void (*InsertionCallback_t)(void *aVal);
typedef void (*RemovalCallback_t)(void *aVal);

// ----- Object
typedef void (*Obj_destructor_t)(void *aSelf);

typedef struct {
	char *name;
	long instanceSize;
	Obj_destructor_t destructor;
} Class_t;

// Add _Obj_guts _guts; at the beginning of a struct type in order to make it a valid, retainable object
typedef struct {
	Class_t *class;
	long referenceCount;
} _Obj_guts;

typedef void Obj_t;

Obj_t *obj_create_autoreleased(Class_t *aClass);
Obj_t *obj_create(Class_t *aClass);
Obj_t *obj_retain(Obj_t *aObj);
void obj_release(Obj_t *aObj);
Obj_t *obj_autorelease(Obj_t *aObj);
Class_t *obj_getClass(Obj_t *aObj);

// ----- Linked list
typedef struct _LinkedList LinkedList_t;
typedef struct _LinkedListItem LinkedListItem_t;
struct _LinkedList {
	_Obj_guts _guts;
	LinkedListItem_t *head;
	LinkedListItem_t *tail;
};

struct _LinkedListItem {
	LinkedListItem_t *previous, *next;
	void *value;
};

extern LinkedList_t *llist_create(InsertionCallback_t aInsertionCallback, RemovalCallback_t aRemovalCallback);

extern void llist_pushValue(LinkedList_t *aList, void *aValue);
extern void *llist_popValue(LinkedList_t *aList);
extern bool llist_insertValue(LinkedList_t *aList, void *aValueToInsert, void *aValueToShift);
extern bool llist_deleteValue(LinkedList_t *aList, void *aValue);
void llist_empty(LinkedList_t *aList);

typedef void (*LinkedListApplier_t)(void *aValue);
extern void llist_apply(LinkedList_t *aList, LinkedListApplier_t aApplier); 


// ----- Renderer
typedef struct _Renderer Renderer_t;
typedef void (*RenderableDisplayCallback_t)(Renderer_t *aRenderer, void *aOwner, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation);

// For defining an object you wish to have rendered
typedef struct _Renderable {
	_Obj_guts _guts;
	RenderableDisplayCallback_t displayCallback;
} Renderable_t;
extern Class_t Class_Renderable;

// The renderer object
struct _Renderer {
	_Obj_guts _guts;
	GLuint frameBufferId; // The FBO the renderer should draw to
	vec2_t viewportSize;
	vec3_t cameraOffset;
	matrix_stack_t *worldMatrixStack;
	matrix_stack_t *projectionMatrixStack;
	LinkedList_t *renderables; // For internal use only
};

// Renderer creation
extern Renderer_t *renderer_create(vec2_t aViewPortSize, vec3_t aCameraOffset);

// Display
extern void renderer_display(Renderer_t *aRenderer, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation);

// Renderable list management
extern void renderer_pushRenderable(Renderer_t *aRenderer, void *aRenderable);
extern void renderer_popRenderable(Renderer_t *aRenderer);
extern bool renderer_insertRenderable(Renderer_t *aRenderer, void *aRenderableToInsert, void *aRenderableToShift);
extern bool renderer_deleteRenderable(Renderer_t *aRenderer, void *aRenderable);

// ----- Scene

typedef struct _Scene {
	_Obj_guts _guts;
	RenderableDisplayCallback_t displayCallback;
	LinkedList_t *renderables;
	mat4_t transform;
} Scene_t;

extern Scene_t *scene_create();

// Scene object list management (Objects need to conform to the layout of Renderable_t)
extern void scene_pushRenderable(Scene_t *aScene, void *aRenderable);
extern void scene_popRenderable(Scene_t *aScene);
extern bool scene_insertRenderable(Scene_t *aScene, void *aRenderableToInsert, void *aRenderableToShift);
extern bool scene_deleteRenderable(Scene_t *aScene, void *aRenderable);

// ----- Game timer
typedef struct _GameTimer GameTimer_t;
typedef void (*GameTimer_updateCallback_t)(GameTimer_t *aTimer);
typedef void (*GameTimer_scheduledCallback_t)(GameTimer_t *aTimer, void *aContext);

// All values are in seconds
struct _GameTimer {
	_Obj_guts _guts;
	GLMFloat elapsed;
	GLMFloat timeSinceLastUpdate;
	GLMFloat desiredInterval; // The minimum interval between updates
	long ticks;
	GameTimer_updateCallback_t updateCallback;
	LinkedList_t *scheduledCallbacks;
};
extern GameTimer_t *gameTimer_create(GLMFloat aFps, GameTimer_updateCallback_t aUpdateCallback);
extern void gameTimer_step(GameTimer_t *aTimer, GLMFloat elapsed);
extern GLMFloat gameTimer_interpolationSinceLastUpdate(GameTimer_t *aTimer);
extern void gameTimer_afterDelay(GameTimer_t *aTimer, GLMFloat aDelay, GameTimer_scheduledCallback_t aCallback, void *aContext);

// ----- Texture
typedef struct _Texture {
	_Obj_guts _guts;
	RenderableDisplayCallback_t displayCallback;
	vec3_t location; // The location to use if the texture is drawn directly as a renderable
	GLuint id;
	vec2_t size;
	// We need to inset the texture rectangle ever so slightly in order
	// to prevent bleeding when using texture atlases
	vec2_t pxAlignInset;
} Texture_t;

// A  structure to specify areas to sample from a texture (in UV coordinates)
typedef union _TextureRect {
	vec4_t v;
	float *f;
	struct {
		vec2_t origin;
		vec2_t size;
	};
	struct {
		float u, v;
		float w, h;
	};
} TextureRect_t;

extern const TextureRect_t kTextureRectEntire;

extern Texture_t *texture_loadFromPng(const char *aPath, bool aRepeatHorizontal, bool aRepeatVertical);
// Generates a UV texture rectangle from pixel coordinates
extern TextureRect_t textureRectangle_createWithPixelCoordinates(Texture_t *aTexture, vec2_t aOrigin, vec2_t aSize);
extern TextureRect_t textureRectangle_createWithSizeInPixels(Texture_t *aTexture, vec2_t aSize);
// Takes UV (0.0-1.0) coordinates
extern TextureRect_t textureRectangle_create(float aX, float aY, float aWidth, float aHeight);

// ----- Texture Atlas
typedef struct _TextureAtlas {
	_Obj_guts _guts;
	vec2_t origin; // The point to consider as (0,0)
	vec2_t size; // The size of each subtexture
	vec2_t margin; // The gap between frames in the texture
	Texture_t *texture;
} TextureAtlas_t;

extern TextureAtlas_t *texAtlas_create(Texture_t *aTexture, vec2_t aOrigin, vec2_t aSize);
// Gets the sampling rectangle for the subtexture at a given offset
extern TextureRect_t texAtlas_getTextureRect(TextureAtlas_t *aAtlas, int x, int y);

// ----- Draw utils
// Rectangles (Origin: bottom left)
union _rect_t {
	float f[4];
	struct { vec2_t o; vec2_t s; };
	struct { vec2_t origin; vec2_t size; };
};
typedef union _rect_t rect_t;

extern void draw_init(Renderer_t *aDefaultRenderer);
extern void draw_cleanup();
extern void draw_quad(vec3_t aCenter, vec2_t aSize, Texture_t *aTexture, TextureRect_t aTextureArea, vec4_t aColor, float aAngle, bool aFlipHorizontal, bool aFlipVertical);
extern void draw_texturePortion(vec3_t aCenter, Texture_t *aTexture, TextureRect_t aTextureArea, float aScale, float aAngle, bool aFlipHorizontal, bool aFlipVertical);
extern void draw_texture(vec3_t aCenter, Texture_t *aTexture, float aScale, float aAngle, bool aFlipHorizontal, bool aFlipVertical);
extern void draw_textureAtlas(TextureAtlas_t *aAtlas, int aNumberOfTiles, vec2_t *aOffsets, vec2_t *aCenterPoints);
extern void draw_textureAtlas_getVertices(TextureAtlas_t *aAtlas, int aNumberOfTiles, vec2_t *aOffsets, vec2_t *aCenterPoints,
	vec2_t **aoVertices, vec2_t **aoTexCoords, int *aoNumberOfVertices, GLuint **aoIndices, int *aoNumberOfIndices);
extern void draw_rect(rect_t aRect, float aAngle, vec4_t aColor, bool aShouldFill);
extern void draw_ellipse(vec2_t aCenter, vec2_t aRadii, int aSubdivisions, float aAngle, vec4_t aColor, bool aShouldFill);
extern void draw_circle(vec2_t aCenter, float radius, int aSubdivisions, vec4_t aColor, bool aShouldFill);
extern void draw_polygon(int aNumberOfVertices, vec2_t *aVertices, vec4_t aColor, bool aShouldFill);
extern void draw_lineSeg(vec2_t aPointA, vec2_t aPointB, vec4_t aColor);

// ----- Sprite
typedef struct _SpriteAnimation {
	int numberOfFrames;
	int currentFrame;
	bool loops;
} SpriteAnimation_t;

typedef struct _Sprite {
	_Obj_guts _guts;
	RenderableDisplayCallback_t displayCallback;
	TextureAtlas_t *atlas;
	vec3_t location;
	vec2_t size;
	float scale, angle;
	bool flippedHorizontally;
	bool flippedVertically;
	int activeAnimation; // The y offset of the active animation
	SpriteAnimation_t *animations;
} Sprite_t;

extern Sprite_t *sprite_create(vec3_t aLocation, vec2_t aSize, TextureAtlas_t *aAtlas, int aAnimationCapacity);
extern SpriteAnimation_t sprite_createAnimation(int aNumberOfFrames);

extern void sprite_step(Sprite_t *aSprite);

// ----- Background
typedef struct _BackgroundLayer {
	_Obj_guts _guts;
	Texture_t *texture;
	float depth;
} BackgroundLayer_t;

typedef struct _Background {
	_Obj_guts _guts;
	RenderableDisplayCallback_t displayCallback;
	BackgroundLayer_t *layers[4];
	vec2_t offset;
} Background_t;

extern Background_t *background_create();

extern void background_setLayer(Background_t *aBackground, unsigned int aIndex, BackgroundLayer_t *aLayer);
extern BackgroundLayer_t *background_createLayer(Texture_t *aTexture, float aDepth);

// ----- Input
typedef enum {
	kInputKey_arrowLeft,
	kInputKey_arrowRight,
	kInputKey_arrowUp,
	kInputKey_arrowDown,
	kInputKey_ascii,
	kInputMouse_leftClick,
	kInputMouse_rightClick,
	kInputMouse_leftDrag,
	kInputMouse_rightDrag,
	kInputMouse_move,

	// Touch events
	kInputTouch_tap1,
	kInputTouch_tap2,
	kInputTouch_tap3,
	kInputTouch_tap4,
	kInputTouch_tap5,
	kInputTouch_pan1,
	kInputTouch_pan2,
	kInputTouch_pan3,
	kInputTouch_pan4,
	kInputTouch_pan5

} Input_type_t;

typedef enum {
	kInputState_down,
	kInputState_up
} Input_state_t;

typedef struct _InputManager InputManager_t;
typedef struct _InputObserver InputObserver_t;

typedef void (*Input_handler_t)(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *aMetaData);

struct _InputObserver {
	_Obj_guts _guts;
	Input_handler_t handlerCallback;
	Input_type_t type;
	unsigned char code; // Used for ASCII keys
	void *metaData; // Arbitrary pointer for providing context

	Input_state_t lastKnownState;
};
struct _InputManager {
	_Obj_guts _guts;
	LinkedList_t *observers;
	LinkedList_t *activeEvents;
};

extern InputManager_t *input_createManager();
extern InputObserver_t *input_createObserver(Input_type_t aObservedType, Input_handler_t aHandlerCallback, char *aCode, void *aMetaData);

extern void input_addObserver(InputManager_t *aManager, InputObserver_t *aObserver);
extern bool input_removeObserver(InputManager_t *aManager, InputObserver_t *aObserver);


// Called from within the run loop to post an instance of each active event (held keys)
extern void input_postActiveEvents(InputManager_t *aManager);
// Calls any observers for a certain type of input
extern void input_postMomentaryEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode, vec2_t *aLocation, Input_state_t aState);
// Activates an event so it gets posted once per cycle
extern void input_beginEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode, vec2_t *aLocation);
extern void input_endEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode);

// ---- Maps
typedef enum _TMXMap_orientation {
	kTMXMap_orthogonal,
	kTMXMap_isometric
} TMXMap_orientation;

typedef struct _TMXProperty {
	char *name;
	char *value;
} TMXProperty_t;

typedef struct _TMXTileset {
	int firstTileGid;
	int imageWidth, imageHeight;
	int tileWidth, tileHeight;
	int spacing;
	int margin;
	char *imagePath;
} TMXTileset_t;

typedef struct _TMXTile {
	TMXTileset_t *tileset;
	int id;
	bool flippedVertically;
	bool flippedHorizontally;
} TMXTile_t;

typedef struct _TMXLayer {
	char *name;
	float opacity;
	bool isVisible;
	int numberOfTiles;
	TMXTile_t *tiles;
	int numberOfProperties;
	TMXProperty_t *properties; // Default NULL
} TMXLayer_t;

typedef struct _TMXObject {
	char *name;
	char *type;
	int x, y; // in pixels
	int width, height; // in pixels
	TMXTile_t tile; // Default -1
	int numberOfProperties;
	TMXProperty_t *properties; // Default NULL
} TMXObject_t;

typedef struct _TMXObjectGroup {
	char *name;
	int numberOfObjects;
	TMXObject_t *objects;
	int numberOfProperties;
	TMXProperty_t *properties; // Default NULL
} TMXObjectGroup_t;

typedef struct _TMXMap {
	_Obj_guts _guts;
	TMXMap_orientation orientation;
	int width, height; // Dimensions in tiles
	int tileWidth, tileHeight; // Dimensions of tiles in pixels
	
	int numberOfLayers;
	TMXLayer_t *layers;
	int numberOfTilesets;
	TMXTileset_t *tilesets;
	int numberOfObjectGroups;
	TMXObjectGroup_t *objectGroups;
	int numberOfProperties;
	TMXProperty_t *properties;
} TMXMap_t;

extern TMXMap_t *tmx_readMapFile(const char *aFilename);

// A renderable that manages a vbo for drawing a layer
typedef struct _TMXLayerRenderable {
	_Obj_guts _guts;
	RenderableDisplayCallback_t displayCallback;
	TMXLayer_t *layer;
	TMXMap_t *map; // Reference required so that we can retain it
	TextureAtlas_t *atlas;
	GLuint posVBO, texCoordVBO, indexVBO;
	int indexCount;
} TMXLayerRenderable_t;

extern TMXLayerRenderable_t *tmx_createRenderableForLayer(TMXMap_t *aMap, unsigned int aLayerIdx);

// Lookup helpers
extern const char *tmx_mapGetPropertyNamed(TMXMap_t *aMap, const char *aPropertyName);
extern TMXLayer_t *tmx_mapGetLayerNamed(TMXMap_t *aMap, const char *aLayerName);
extern TMXObjectGroup_t *tmx_mapGetObjectGroupNamed(TMXMap_t *aMap, const char *aGroupName);
extern TMXObject_t *tmx_objGroupGetObjectNamed(TMXObjectGroup_t *aGroup, const char *aObjName);

//
// Audio
extern Class_t Class_SoundEffect;
// Sound effect (for short sounds that need low latency)
typedef struct _SoundEffect SoundEffect_t;
// For longer non latency sensitive sounds, specifically BGM
extern Class_t Class_BackgroundMusic;
typedef struct _BackgroundMusic BackgroundMusic_t;
// Manages the audio state. Sounds can not be created or played if there is no current sound manager
// Generally you would just create one and have it persist while the app is running
extern Class_t Class_SoundManager;
typedef struct _SoundManager SoundManager_t;

extern SoundEffect_t *sfx_load(const char *aFilename);
extern void sfx_unload(SoundEffect_t *aSound);

extern void sfx_play(SoundEffect_t *aSound);
extern void sfx_stop(SoundEffect_t *aSound);
extern void sfx_toggle(SoundEffect_t *aSound);
extern bool sfx_isPlaying(SoundEffect_t *aSound);

extern void sfx_setVolume(SoundEffect_t *aSound, float aVolume);
extern void sfx_setLocation(SoundEffect_t *aSound, vec3_t aPos);
extern void sfx_setLooping(SoundEffect_t *aSound, bool aShouldLoop);
extern void sfx_setPitch(SoundEffect_t *aSound, float aPitch);

extern BackgroundMusic_t *bgm_load(const char *aFilename);
extern void bgm_unload(BackgroundMusic_t *aBGM);

extern void bgm_play(BackgroundMusic_t *aBGM);
extern void bgm_stop(BackgroundMusic_t *aBGM);
extern bool bgm_isPlaying(BackgroundMusic_t *aBGM);
extern void bgm_seek(BackgroundMusic_t *aBGM, float aSeconds);
extern void bgm_setVolume(BackgroundMusic_t *aBGM, float aVolume);

extern SoundManager_t *soundManager_create();
extern bool soundManager_makeCurrent(SoundManager_t *aManager);

//
// Game world

typedef struct _World World_t;
typedef struct _WorldShape WorldShape_t;
typedef struct _WorldEntity WorldEntity_t;

typedef struct _World_ContactPointSet {
	int count;
	struct {
		vec2_t point;
		vec2_t normal;
		GLMFloat depth;
	} points[4];
} World_ContactPointSet;

typedef struct _World_CollisionInfo {
	WorldEntity_t *a;
	WorldEntity_t *b;
	bool firstContact;
	World_ContactPointSet contactPoints;
	void *cpArbiter;
} World_CollisionInfo;

typedef void (*WorldEntity_CollisionHandler)(WorldEntity_t *aEntity, World_t *aWorld, World_CollisionInfo *aCollisionInfo);
typedef void (*WorldEntity_UpdateHandler)(WorldEntity_t *aEntity, World_t *aWorld);

// A Game entity is an object that can be rendered and/or included in the physics simulation
extern Class_t Class_WorldEntity;
struct _WorldEntity {
	_Obj_guts _guts;
    World_t *world; // Weak reference
	Obj_t *owner; // Weak reference (Owner should retain the entity)
	void *cpBody;
	LinkedList_t *shapes;
	WorldEntity_UpdateHandler updateHandler;
	WorldEntity_CollisionHandler preCollisionHandler;
	WorldEntity_CollisionHandler collisionHandler;
	WorldEntity_CollisionHandler postCollisionHandler;
};

// A collision shape, attached to one and only one(!) entity
struct _WorldShape {
	_Obj_guts _guts;
	void *cpShape;
};

struct _World {
	_Obj_guts _guts;
	void *cpSpace;
	LinkedList_t *entities;
	WorldEntity_t *staticEntity;
};

typedef enum {
    kWorldJointType_Pin,
    kWorldJointType_Slide,
    kWorldJointType_Pivot,
    kWorldJointType_Groove,
    kWorldJointType_DampedSpring,
    kWorldJointType_DampedRotarySpring,
    kWorldJointType_RotaryLimit,
    kWorldJointType_Ratchet,
    kWorldJointType_Gear,
    kWorldJointType_SimpleMotor
} WorldJointType_t;

extern Class_t Class_WorldConstraint;
typedef struct _WorldConstraint {
    World_t *world; // Weak
    WorldEntity_t *a, *b;
    WorldJointType_t type;
    void *cpConstraint;
} WorldConstraint_t;

extern World_t *world_create(void);
extern void world_step(World_t *aWorld, GameTimer_t *aTimer);
extern void world_setGravity(World_t *aWorld, vec2_t aGravity);
extern vec2_t world_gravity(World_t *aWorld);
extern void world_addEntity(World_t *aWorld, WorldEntity_t *aEntity);
extern void world_addStaticEntity(World_t *aWorld, WorldEntity_t *aEntity);
extern WorldEntity_t *world_pointQuery(World_t *aWorld, vec2_t aPoint);

extern vec2_t worldEnt_location(WorldEntity_t *aEntity);
extern WorldEntity_t *worldEnt_create(World_t *aWorld, Obj_t *aOwner, GLMFloat aMass, GLMFloat aMomentum);
extern void worldEnt_setLocation(WorldEntity_t *aEntity, vec2_t aLocation);
extern GLMFloat worldEnt_angle(WorldEntity_t *aEntity);
extern void worldEnt_setAngle(WorldEntity_t *aEntity, GLMFloat aAngle);
extern void worldEnt_addShape(WorldEntity_t *aEntity, WorldShape_t *aShape);
extern void worldEnt_applyForce(WorldEntity_t *aEntity, vec2_t aForce, vec2_t aOffset);
extern void worldEnt_applyImpulse(WorldEntity_t *aEntity, vec2_t aImpulse, vec2_t aOffset);
extern vec2_t worldEnt_velocity(WorldEntity_t *aEntity);
extern void worldEnt_setVelocity(WorldEntity_t *aEntity, vec2_t aVelocity);

extern WorldShape_t *worldShape_createCircle(vec2_t aCenter, GLMFloat aRadius);
extern WorldShape_t *worldShape_createSegment(vec2_t a, vec2_t b, GLMFloat aThickness);
extern WorldShape_t *worldShape_createBox(vec2_t aSize);
// Takes an array of counter clockwise winded vertices
extern WorldShape_t *worldShape_createPoly(unsigned aVertCount, vec2_t *aVerts);
extern GLMFloat worldShape_friction(WorldShape_t *aEntity);
extern void worldShape_setFriction(WorldShape_t *aEntity, GLMFloat aVal);
extern GLMFloat worldShape_elasticity(WorldShape_t *aEntity);
extern void worldShape_setElasticity(WorldShape_t *aEntity, GLMFloat aVal);

extern GLMFloat world_momentForCircle(GLMFloat aMass, GLMFloat aInnerRadius, GLMFloat aOuterRadius, vec2_t aOffset);
extern GLMFloat world_momentForSegment(GLMFloat aMass, vec2_t a, vec2_t b);
extern GLMFloat world_momentForPoly(GLMFloat aMass, unsigned aVertCount, vec2_t *aVerts, vec2_t aOffset);
extern GLMFloat world_momentForBox(GLMFloat aMass, vec2_t aSize);

extern WorldConstraint_t *worldConstr_createPinJoint(WorldEntity_t *a, WorldEntity_t *b, vec2_t aAnchorA, vec2_t aAnchorB);
extern WorldConstraint_t *worldConstr_createSlideJoint(WorldEntity_t *a, WorldEntity_t *b, vec2_t aAnchorA, vec2_t aAnchorB, GLMFloat minDist, GLMFloat maxDist);
extern WorldConstraint_t *worldConstr_createPivotJoint(WorldEntity_t *a, WorldEntity_t *b, vec2_t aPivot);
extern WorldConstraint_t *worldConstr_createGrooveJoint(WorldEntity_t *a, WorldEntity_t *b, vec2_t aGrooveStart, vec2_t aGrooveEnd, vec2_t aAnchorB);
extern WorldConstraint_t *worldConstr_createDampedSpringJoint(WorldEntity_t *a, WorldEntity_t *b, vec2_t aAnchorA, vec2_t aAnchorB, GLMFloat aRestLength, GLMFloat aStiffness, GLMFloat aDamping);
extern WorldConstraint_t *worldConstr_createDampedRotarySpringJoint(WorldEntity_t *a, WorldEntity_t *b, GLMFloat restAngle, GLMFloat aStiffness, GLMFloat aDamping);
extern WorldConstraint_t *worldConstr_createRotaryLimitJoint(WorldEntity_t *a, WorldEntity_t *b, GLMFloat aMinAngle, GLMFloat aMaxAngle);
extern WorldConstraint_t *worldConstr_createRatchetJoint(WorldEntity_t *a, WorldEntity_t *b, GLMFloat aPhase, GLMFloat aRatchet);
extern WorldConstraint_t *worldConstr_createGearJoint(WorldEntity_t *a, WorldEntity_t *b, GLMFloat aPhase, GLMFloat aRatio);
extern WorldConstraint_t *worldConstr_createSimpleMotorJoint(WorldEntity_t *a, WorldEntity_t *b, GLMFloat aRate);

extern void draw_world(World_t *aWorld, bool aDrawBB);
extern void draw_worldShape(WorldShape_t *aShape, WorldEntity_t *aEntity, bool aDrawBB);
extern void draw_worldEntity(WorldEntity_t *aEntity, bool aDrawBB);

//
// Utils
extern bool util_pathForResource(const char *name, const char *ext, const char *dir, char *output, int maxLen);

typedef enum {
	kPlatformMac,
	kPlatformIOS,
	kPlatformAndroid,
	kPlatformWindows,
	kPlatformOther
} Platform_t;
extern Platform_t util_platform(void);
extern void _debug_log(const char *str);
]]

dynamo.platforms = {
	mac = 0,
	ios = 1,
	android = 2,
	windows = 3,
	other = 4
}
dynamo.platform = tonumber(lib.util_platform())

dynamo.kBackground_maxLayers = 4

--
-- Utilities

-- Retains an object and tells the gc to release it when it's no longer referenced
local function _obj_addToGC(obj)
	lib.obj_retain(obj)
	ffi.gc(obj, lib.obj_release)
	return obj
end

function dynamo.pathForResource(name, type, directory)
	type = type or nil
	directory = directory or nil
	local maxLen = 1024
	local strPtr = ffi.new("char[?]", maxLen)
	local exists = lib.util_pathForResource(name, type, directory, strPtr, maxLen)
	if exists then
		return ffi.string(strPtr)
	else
		return nil
	end
end

function dynamo.log(...)
	prefix = ""
	if debug ~= nil then
		local info = debug.getinfo(1, "Sln")
		if info.what == "C" then
			prefix = "<C function>: "
		else
			print(info.short_src)
			print(info.currentline)
			print(info.name)
			
			prefix = string.format("%s:%s (%s): ", info.short_src, info.currentline, info.name)
		end
	end
	lib._debug_log(prefix..table.concat({...}))
end

--
-- Textures
function dynamo.loadTexture(path)
	local tex = lib.texture_loadFromPng(path, false, false)
	if tex == nil then
		print("Couldn't find texture at path", path)
		return nil
	end
	return _obj_addToGC(tex)
end


--
-- Texture atlases
ffi.metatype("TextureAtlas_t", {
	__index = {
		getTextureRect = lib.texAtlas_getTextureRect,
	}
})

function dynamo.createTextureAtlas(texture, origin, size)
	assert(texture)
	local tex = lib.texAtlas_create(texture, origin, size)
	return _obj_addToGC(tex)
end

function dynamo.loadTextureAtlas(path, origin, size)
	local tex = dynamo.loadTexture(path)
	local atlas = dynamo.createTextureAtlas(tex, origin, size)
	return _obj_addToGC(atlas)
end

--
-- Sprites
ffi.metatype("Sprite_t", {
	__index = {
		step = lib.sprite_step
	}
})

function dynamo.createSprite(location, size, atlas, animations)
	animations = animations or {}
	local sprite = lib.sprite_create(location, size, atlas, #animations)
	lib.obj_retain(sprite)

	-- Add the animations to the sprite
	for i, animation in pairs(animations) do
		sprite.animations[i-1] = animation
	end

	return _obj_addToGC(sprite)
end

--
-- The Renderer
ffi.metatype("Renderer_t", {
	__index = {
		display = lib.renderer_display,
		pushRenderable = lib.renderer_pushRenderable,
		popRenderable = lib.renderer_popRenderable,
		insertRenderable = lib.renderer_pushRenderable,
		deleteRenderable = lib.renderer_deleteRenderable,
		handleResize = function(self, viewport)
			self.viewportSize = viewport
			self.projectionMatrixStack:pushItem(mat4_ortho(0, viewport.w, 0, viewport.h, -1, 1));
		end
	}
})

local function _createRenderer(viewportSize)
	local renderer = lib.renderer_create(viewportSize, {{0,0,0}})
	return _obj_addToGC(renderer)
end


--
-- Scenes

ffi.metatype("Scene_t", {
	__index = {
		pushRenderable = lib.scene_pushRenderable,
		popRenderable = lib.scene_popRenderable,
		insertRenderable = lib.scene_pushRenderable,
		deleteRenderable = lib.scene_deleteRenderable,
		rotate = function(self, angle, axis)
			axis = axis or vec3(0, 0, 1)
			self.transform = mat4_rotate(self.transform, angle, axis.x, axis.y, axis.z)
		end,
		scale = function(self, x, y, z)
			z = z or 1
			self.transform = mat4_scale(self.transform, x, y, 1)
		end,
		translate = function(self, x, y, z)
			z = z or 0
			self.transform = mat4_translate(self.transform, x, y, 0)
		end
	}
})

function dynamo.createScene(renderables, initialTransform)
	objects = objects or {}
	initialTransform = initialTransform or mat4_identity

	local scene = lib.scene_create()
	for i,renderable in pairs(renderables) do
		scene:pushRenderable(renderable)
	end
	scene.transform = initialTransform

	return _obj_addToGC(scene)
end

--
-- Input manager

ffi.metatype("InputManager_t", {
	__index = {
		addObserver = function(self, desc)
			local observer = lib.input_createObserver(desc.type, desc.callback, desc.charCode, desc.metaData)
			lib.input_addObserver(self, observer)
			return _obj_addToGC(observer)
		end,
		removeObserver = lib.input_removeObserver,
		postActiveEvents = lib.input_postActiveEvents,
		postMomentaryEvent = lib.input_postMomentaryEvent,
		beginEvent = lib.input_beginEvent,
		endEvent = lib.input_endEvent
	}
})

local function _createInputManager()
	local inputManager = lib.input_createManager()
	return _obj_addToGC(inputManager)
end


--
-- Game timer

ffi.metatype("GameTimer_t", {
	__index = {
		step = lib.gameTimer_step,
		interpolation = lib.gameTimer_interpolationSinceLastUpdate,
		afterDelay = function(self, delay, lambda)
			lib.gameTimer_afterDelay(self, delay, lambda, nil)
		end
	}
})

local function _createTimer(desiredFPS, updateCallback)
	local timer = lib.gameTimer_create(desiredFPS, updateCallback)
	return _obj_addToGC(timer)
end

-- Time function (Apple/Linux only for now)
if dynamo.platform == dynamo.platforms.ios or dynamo.platform == dynamo.platforms.mac then
	ffi.cdef[[
	struct mach_timebase_info {
		uint32_t numer;
		uint32_t denom;
	};
	typedef struct mach_timebase_info *mach_timebase_info_t;
	typedef struct mach_timebase_info	mach_timebase_info_data_t;

	void mach_timebase_info(mach_timebase_info_t info);
	uint64_t mach_absolute_time(void);
	]]
	local C = ffi.C
	function dynamo.globalTime()
		local timebase = ffi.new("mach_timebase_info_data_t[1]")
		C.mach_timebase_info(timebase)
		local absolute = C.mach_absolute_time()
		local nanosec = (absolute * timebase[0].numer) / timebase[0].denom

		return tonumber(nanosec)/1000000000
	end

	dynamo.startTime = dynamo.globalTime()
	function dynamo.time()
		return dynamo.globalTime() - dynamo.startTime
	end
elseif dynamo.platform == dynamo.platforms.android then
	ffi.cdef[[
		typedef int32_t clockid_t;
		typedef long time_t;
		struct timespec {
			time_t   tv_sec;  /* seconds */
			long     tv_nsec; /* nanoseconds */
		};
		int clock_gettime(clockid_t clk_id, struct timespec *tp);
	]]
	local CLOCK_MONOTONIC = 1

	function dynamo.globalTime()
		local now = ffi.new("struct timespec[1]")
		C.clock_gettime(CLOCK_MONOTONIC, now);
		return tonumber(now[0].tv_sec) + tonumber(now[0].tv_nsec)/1000000000
	end

	dynamo.startTime = dynamo.globalTime()
	function dynamo.time()
		return dynamo.globalTime() - dynamo.startTime
	end
else
	error("Unsupported platform "..tostring(tonumber(dynamo.platform)))
end

function dynamo.renderable(lambda)
	local drawable = ffi.cast("Renderable_t*", lib.obj_create_autoreleased(ffi.cast("Class_t*", lib.Class_Renderable)))
	drawable.displayCallback = lambda
	return drawable
end


--
-- Maps
ffi.metatype("TMXMap_t", {
	__index = {
		getProperty = lib.tmx_mapGetPropertyNamed,
		getLayer = tmx_mapGetLayerNamed,
		getObjectGroup = tmx_mapGetObjectGroupNamed,
		createLayerRenderable = lib.tmx_createRenderableForLayer
	}
})

ffi.metatype("TMXLayer_t", {
	__index = {
		-- Generates the texture offsets and screen coordinates to draw each tile
		generateAtlasDrawInfo = function(self, map)
			local texOffsets = ffi.new("vec2_t[?]", self.numberOfTiles)
			local screenCoords = ffi.new("vec2_t[?]", self.numberOfTiles)
			
			for y=0, map.height-1 do
				for x=0, map.width-1 do
					local idx = y*map.width + x
					local tile = self.tiles[idx]
					texOffsets[idx] = tile.tileset:texCoordForID(tile.id)
					screenCoords[idx] = vec2(math.floor(map.tileWidth*x + map.tileWidth/2), math.floor(map.tileHeight*y + map.tileHeight/2))
				end
			end
			return screenCoords, texOffsets
		end

	}
})

ffi.metatype("TMXObjectGroup_t", {
	__index = lib.tmx_objGroupGetObjectNamed
})

function math.round(num)
	return math.floor(num+0.5)
end

ffi.metatype("TMXTileset_t", {
	__index = {
		loadAtlas = function(self)
			local fullPath = dynamo.pathForResource(self.imagePath)
			assert(fullPath ~= nil)
			local tex = dynamo.loadTexture(fullPath)
			assert(tex ~= nil)
			local atlas = dynamo.createTextureAtlas(tex, vec2(self.margin, self.margin), vec2(self.tileWidth, self.tileHeight))
			atlas.margin = vec2(self.spacing, self.spacing)
			return atlas
		end,
		texCoordForID = function(self, id)
			local tilesPerRow = math.floor((self.imageWidth - self.spacing) / (self.tileWidth + self.spacing))
			local u = id % tilesPerRow
			local rows = math.floor((self.imageHeight - self.spacing) / (self.tileHeight + self.spacing))
			local v = math.floor(id / tilesPerRow)
			return vec2(u,v)
		end
	}
})

function dynamo.loadMap(path)
	local ret = lib.tmx_readMapFile(path)
	return _obj_addToGC(ret)
end


--
-- Sound

local function _createSoundManager()
	local ret = lib.soundManager_create()
	return _obj_addToGC(ret)
end

ffi.metatype("SoundEffect_t", {
	__index = {
		play = lib.sfx_play,
		stop = lib.sfx_stop,
		isPlaying = lib.sfx_isPlaying,
		unload = lib.sfx_unload
	},
	__newindex = function(self, key, val)
		if key == "location" then
			lib.sfx_setLocation(self, val)
		elseif key == "loops" then
			lib.sfx_setLooping(self, val)
		elseif key == "pitch" then
			lib.sfx_setPitch(self, val)
		elseif key == "volume" then
			lib.sfx_setVolume(self, val)
		else
			error("Undefined key "..key)
		end
	end
})

ffi.metatype("BackgroundMusic_t", {
	__index = {
		play = lib.bgm_play,
		stop = lib.bgm_stop,
		seek = lib.bgm_seek,
		isPlaying = lib.bgm_isPlaying,
		unload = lib.bgm_unload
	},
	__newindex = function(self, key, val)
		if key == "volume" then
			lib.bgm_setVolume(self, val)
		else
			error("Undefined key "..key)
		end
	end
})

function dynamo.loadSFX(path)
	local ret = lib.sfx_load(path)
	return _obj_addToGC(ret)
end

function dynamo.loadBGM(path)
	local ret = lib.bgm_load(path)
	return _obj_addToGC(ret)
end


--
-- Game world

ffi.metatype("World_t", {
	__index = {
		addEntity = lib.world_addEntity,
		gravity = lib.world_gravity,
		step = lib.world_step,
		momentForCircle = lib.world_momentForCircle,
		momentForSegment = lib.world_momentForSegment,
		momentForPoly = lib.world_momentForPoly,
		momentForBox = lib.world_momentForBox,
		draw = lib.draw_world,
		drawShape = lib.draw_worldShape,
		drawEntity = lib.draw_worldEntity,
		pointQuery = lib.world_pointQuery
	},
	__newindex = function(self, key, val)
		if key == "gravity" then
			lib.world_setGravity(self, val)
		end
	end
})

ffi.metatype("WorldEntity_t", {
	__index = {
		location = lib.worldEnt_location,
		--location = function(self)
			--print("passed self: ", self)
			--return lib.worldEnt_location(self)
		--end,
		angle = lib.worldEnt_angle,
		velocity = lib.worldEnt_velocity,
		addShape = lib.worldEnt_addShape,
		applyForce = lib.worldEnt_applyForce,
		applyImpulse = lib.worldEnt_applyImpulse,
		createPinJoint = lib.worldConstr_createPinJoint,
		createSlideJoint = lib.worldConstr_createSlideJoint,
		createPivotJoint = lib.worldConstr_createPivotJoint,
		createGrooveJoint = lib.worldConstr_createGrooveJoint,
		createDampedSpring = lib.worldConstr_createDampedSpringJoint,
		createDampedRotarySpring = lib.worldConstr_createDampedRotarySpringJoint,
		creatRotaryLimitJoint = lib.worldConstr_createRotaryLimitJoint,
		createRatchetJoint = lib.worldConstr_createRatchetJoint,
		createGearJoint = lib.worldConstr_createGearJoint,
		createSimpleMotorJoint = lib.worldConstr_createSimpleMotorJoint
	},
	__newindex = function(self, key, val)
		if key == "location" then
			lib.worldEnt_setLocation(self, val)
		elseif key == "angle" then
			lib.worldEnt_setAngle(self, val)
		elseif key == "velocity" then
			lib.worldEnt_setVelocity(self, val)
		end
	end
})

ffi.metatype("WorldShape_t", {
	__index = {
		friction = lib.worldShape_friction,
		elasticity = lib.worldShape_elasticity
	},
	__newindex = function(self, key, val)
		if key == "friction" then
			lib.worldShape_setFriction(self, val)
		elseif key == "elasticity" then
			lib.worldShape_setElasticity(self, val)
		end
	end
})


local function _createWorld()
	return _obj_addToGC(lib.world_create())
end

function dynamo.createEntity(world, owner, mass, momentum, shapes)
	shapes = shapes or {}
	local ret = lib.worldEnt_create(world, owner, mass, momentum)
	for i,shape in ipairs(shapes) do
		ret:addShape(shape)
	end
	return _obj_addToGC(ret)
end

function dynamo.createCircleShape(center, radius)
	local ret = lib.worldShape_createCircle(center, radius)
	return _obj_addToGC(ret)
end

function dynamo.createSegmentShape(a, b, thickness)
	local ret = lib.worldShape_createSegment(a, b, (thickness or 1))
	return _obj_addToGC(ret)
end

function dynamo.createBoxShape(size)
	local ret = lib.worldShape_createBox(size)
	return _obj_addToGC(ret)
end

function dynamo.createPolyShape(vertices)
	if #vertices < 3 then
		error("Too few vertices to create polygon shape")
	end
	local ret = lib.worldShape_createPoly(#vertices, vertices)
	return _obj_addToGC(ret)
end


--
-- Lifecycle/HighLevelInterface functions

dynamo.initialized = false
function dynamo.init(viewport, desiredFPS, updateCallback)
	assert(dynamo.initialized == false)
	dynamo.initialized = true

	dynamo.renderer = _createRenderer(viewport)
	lib.draw_init(dynamo.renderer)
	dynamo.renderer:handleResize(viewport)
	
	dynamo.timer = _createTimer(desiredFPS, updateCallback)
	lib.obj_retain(dynamo.renderer)
	lib.obj_retain(dynamo.timer)

	dynamo.inputManager = _createInputManager()

	dynamo.soundManager = _createSoundManager()
	lib.soundManager_makeCurrent(dynamo.soundManager)

	dynamo.world = _createWorld()
	dynamo.world.gravity = vec2(0,-980)

	return dynamo.renderer, dynamo.timer
end

function dynamo.cleanup()
	-- Release all resources
	dynamo.renderer = nil
	dynamo.timer = nil
	dynamo.inputManager = nil
	lib.draw_cleanup()
end

function dynamo.postTapEvent(finger, isDown, x, y)
	finger = finger or 0
	local pos = vec2(x or 0, y or 0)
	local state = lib.kInputState_up
	if isDown == true then 
		state = lib.kInputState_down
	end
	dynamo.inputManager:postMomentaryEvent(lib.kInputTouch_tap1+finger, nil, pos, state)
end

function dynamo.postPanEvent(finger, isDown, x, y)
	finger = finger or 0
	local pos = vec2(x or 0, y or 0)
	local state = lib.kInputState_up
	if isDown == 1 or isDown == true then
		state = lib.kInputState_down
	end
	dynamo.inputManager:postMomentaryEvent(lib.kInputTouch_pan1+finger, nil, pos, state)
end
postPanEvent = dynamo.postPanEvent

function dynamo.cycle()
	dynamo.inputManager:postActiveEvents()
	dynamo.timer:step(dynamo.time())
	dynamo.world:step(dynamo.timer)
	dynamo.renderer:display(dynamo.timer.timeSinceLastUpdate, dynamo.timer:interpolation())
end

return dynamo

