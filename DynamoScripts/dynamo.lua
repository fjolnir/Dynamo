-- iOS does not allow JIT (But the LuaJIT interpreter is still plenty fast)
jit.off()

local ffi = require("ffi")
local lib = ffi.C
local gl = require("OpenGLES")

dynamo = setmetatable({}, { __index = lib })

ffi.cdef[[
typedef float GLMFloat;
typedef void (*InsertionCallback_t)(void *aVal);
typedef void (*RemovalCallback_t)(void *aVal);
typedef void (*Obj_destructor_t)(void *aSelf);
typedef struct { char *name; long instanceSize; Obj_destructor_t destructor; } Class_t;
typedef struct { Class_t *class; long referenceCount; } _Obj_guts;
typedef void Obj_t;
Obj_t *obj_create_autoreleased(Class_t *aClass);
Obj_t *obj_create(Class_t *aClass);
Obj_t *obj_retain(Obj_t *aObj);
void obj_release(Obj_t *aObj);
Obj_t *obj_autorelease(Obj_t *aObj);
Class_t *obj_getClass(Obj_t *aObj);
typedef struct _LinkedList LinkedList_t;
typedef struct _LinkedListItem LinkedListItem_t;
struct _LinkedList { _Obj_guts _guts; LinkedListItem_t *head; LinkedListItem_t *tail; };
struct _LinkedListItem { LinkedListItem_t *previous, *next; void *value; };
extern LinkedList_t *llist_create(InsertionCallback_t aInsertionCallback, RemovalCallback_t aRemovalCallback);
extern void llist_pushValue(LinkedList_t *aList, void *aValue);
extern void *llist_popValue(LinkedList_t *aList);
extern bool llist_insertValue(LinkedList_t *aList, void *aValueToInsert, void *aValueToShift);
extern bool llist_deleteValue(LinkedList_t *aList, void *aValue);
void llist_empty(LinkedList_t *aList);
typedef void (*LinkedListApplier_t)(void *aValue);
extern void llist_apply(LinkedList_t *aList, LinkedListApplier_t aApplier);
typedef struct _Renderer Renderer_t;
typedef void (*RenderableDisplayCallback_t)(Renderer_t *aRenderer, void *aOwner, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation);
typedef struct _Renderable { _Obj_guts _guts; RenderableDisplayCallback_t displayCallback; int luaDisplayCallback; } Renderable_t;
extern Class_t Class_Renderable;
struct _Renderer { _Obj_guts _guts; GLuint frameBufferId; vec2_t viewportSize; vec3_t cameraOffset; matrix_stack_t
*worldMatrixStack; matrix_stack_t *projectionMatrixStack; LinkedList_t *renderables;  };
extern Renderer_t *renderer_create(vec2_t aViewPortSize, vec3_t aCameraOffset);
extern void renderer_display(Renderer_t *aRenderer, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation);
extern void renderer_pushRenderable(Renderer_t *aRenderer, void *aRenderable);
extern void renderer_popRenderable(Renderer_t *aRenderer);
extern bool renderer_insertRenderable(Renderer_t *aRenderer, void *aRenderableToInsert, void *aRenderableToShift);
extern bool renderer_deleteRenderable(Renderer_t *aRenderer, void *aRenderable);
typedef struct _Scene { _Obj_guts _guts; RenderableDisplayCallback_t displayCallback; int luaDisplayCallback; LinkedList_t *renderables; mat4_t transform; } Scene_t;
extern Scene_t *scene_create();
extern void scene_pushRenderable(Scene_t *aScene, void *aRenderable);
extern void scene_popRenderable(Scene_t *aScene);
extern bool scene_insertRenderable(Scene_t *aScene, void *aRenderableToInsert, void *aRenderableToShift);
extern bool scene_deleteRenderable(Scene_t *aScene, void *aRenderable);
typedef struct _GameTimer GameTimer_t;
typedef void (*GameTimer_updateCallback_t)(GameTimer_t *aTimer);
struct _GameTimer { _Obj_guts _guts; GLMFloat elapsed; GLMFloat timeSinceLastUpdate; GLMFloat desiredInterval; GLMFloat resetAt; short status; long ticks; GameTimer_updateCallback_t updateCallback; int luaUpdateCallback; LinkedList_t *scheduledCallbacks;};
extern GameTimer_t *gameTimer_create(GLMFloat aFps, GameTimer_updateCallback_t aUpdateCallback);
extern void gameTimer_step(GameTimer_t *aTimer, GLMFloat elapsed);
extern GLMFloat gameTimer_interpolationSinceLastUpdate(GameTimer_t *aTimer);
extern void gameTimer_pause(GameTimer_t *aTimer);
extern void gameTimer_resume(GameTimer_t *aTimer);
extern void gameTimer_reset(GameTimer_t *aTimer);
typedef struct _GameTimer_ScheduledCallback GameTimer_ScheduledCallback_t;
extern GameTimer_ScheduledCallback_t *gameTimer_afterDelay_luaCallback(GameTimer_t *aTimer, GLMFloat aDelay, int aCallback, bool aRepeats);
bool gameTimer_unscheduleCallback(GameTimer_t *aTimer, GameTimer_ScheduledCallback_t *aCallback);
extern GLMFloat dynamo_globalTime();
extern GLMFloat dynamo_time();
typedef struct _Texture { _Obj_guts _guts; RenderableDisplayCallback_t displayCallback; int luaDisplayCallback; vec3_t location;  GLuint id; vec2_t size; vec2_t pxAlignInset; void *subtextures; } Texture_t;
typedef union _TextureRect { vec4_t v; float *f; struct {     vec2_t origin;     vec2_t size; }; struct {     float u, v;     float w, h; }; } TextureRect_t;
extern const TextureRect_t kTextureRectEntire;
extern Texture_t *texture_loadFromPng(const char *aPath, bool aRepeatHorizontal, bool aRepeatVertical);
extern TextureRect_t textureRectangle_createWithPixelCoordinates(Texture_t *aTexture, vec2_t aOrigin, vec2_t aSize);
extern TextureRect_t textureRectangle_createWithSizeInPixels(Texture_t *aTexture, vec2_t aSize);
extern TextureRect_t textureRectangle_create(float aX, float aY, float aWidth, float aHeight);
extern bool texture_loadPackingInfo(Texture_t *aTexture, const char *aPath);
extern TextureRect_t texture_getSubTextureRect(Texture_t *aTexture, const char *aTexName);
extern vec2_t texture_getSubTextureOrigin(Texture_t *aTexture, const char *aTexName);
extern vec2_t texture_getSubTextureSize(Texture_t *aTexture, const char *aTexName);
typedef struct _TextureAtlas { _Obj_guts _guts; vec2_t origin;  vec2_t size;  vec2_t margin;  Texture_t *texture; } TextureAtlas_t;
extern TextureAtlas_t *texture_getSubTextureAtlas(Texture_t *aTexture, const char *aTexName, vec2_t aAtlasSize);
extern TextureAtlas_t *texAtlas_create(Texture_t *aTexture, vec2_t aOrigin, vec2_t aSize);
extern TextureRect_t texAtlas_getTextureRect(TextureAtlas_t *aAtlas, int x, int y);
union _rect_t { float f[4]; struct { vec2_t o; vec2_t s; }; struct { vec2_t origin; vec2_t size; }; };
typedef union _rect_t rect_t;
extern void draw_init(Renderer_t *aDefaultRenderer);
extern void draw_cleanup();
extern void draw_quad(vec3_t aCenter, vec2_t aSize, Texture_t *aTexture, TextureRect_t aTextureArea, vec4_t aColor, float aAngle, bool aFlipHorizontal, bool aFlipVertical);
extern void draw_texturePortion(vec3_t aCenter, Texture_t *aTexture, TextureRect_t aTextureArea, float aScale, float aAngle, float aAlpha, bool aFlipHorizontal, bool aFlipVertical);
extern void draw_texture(vec3_t aCenter, Texture_t *aTexture, float aScale, float aAngle, bool aFlipHorizontal, bool aFlipVertical);
extern void draw_textureAtlas(TextureAtlas_t *aAtlas, int aNumberOfTiles, vec2_t *aOffsets, vec2_t *aCenterPoints);
extern void draw_textureAtlas_getVertices(TextureAtlas_t *aAtlas, int aNumberOfTiles, vec2_t *aOffsets, vec2_t *aCenterPoints, vec2_t **aoVertices, vec2_t **aoTexCoords, int *aoNumberOfVertices, GLuint **aoIndices, int *aoNumberOfIndices);
extern void draw_rect(vec2_t aCenter, vec2_t aSize, float aAngle, vec4_t aColor, bool aShouldFill);
extern void draw_ellipse(vec2_t aCenter, vec2_t aRadii, int aSubdivisions, float aAngle, vec4_t aColor, bool aShouldFill);
extern void draw_circle(vec2_t aCenter, float radius, int aSubdivisions, vec4_t aColor, bool aShouldFill);
extern void draw_polygon(int aNumberOfVertices, vec2_t *aVertices, vec4_t aColor, bool aShouldFill);
extern void draw_lineSeg(vec2_t aPointA, vec2_t aPointB, vec4_t aColor);
typedef struct _SpriteAnimation { int numberOfFrames; int currentFrame; bool loops; } SpriteAnimation_t;
typedef struct _Sprite { _Obj_guts _guts; RenderableDisplayCallback_t displayCallback; int luaDisplayCallback; TextureAtlas_t *_atlas; vec3_t location; vec2_t size; float scale, angle, opacity; bool flippedHorizontally; bool flippedVertically; int activeAnimation;  SpriteAnimation_t *animations; } Sprite_t;
typedef struct _SpriteBatch { _Obj_guts _guts; RenderableDisplayCallback_t displayCallback; int luaDisplayCallback; int spriteCount; LinkedList_t *sprites; unsigned vao, vbo, vertCount, vertCapacity; } SpriteBatch_t;
extern Class_t Class_SpriteBatch;
extern Sprite_t *sprite_create(vec3_t aLocation, vec2_t aSize, TextureAtlas_t *aAtlas, int aAnimationCapacity);
extern SpriteAnimation_t sprite_createAnimation(int aNumberOfFrames);
extern void sprite_step(Sprite_t *aSprite);
extern SpriteBatch_t *spriteBatch_create();
extern void spriteBatch_addSprite(SpriteBatch_t *aBatch, Sprite_t *aSprite);
extern bool spriteBatch_insertSprite(SpriteBatch_t *aBatch, Sprite_t *aSprite, Sprite_t *aSpriteToShift);
extern bool spriteBatch_deleteSprite(SpriteBatch_t *aBatch, Sprite_t *aSprite);
typedef struct _BackgroundLayer { _Obj_guts _guts; Texture_t *texture; float depth, opacity;} BackgroundLayer_t;
typedef struct _Background { _Obj_guts _guts; RenderableDisplayCallback_t displayCallback; int luaDisplayCallback; BackgroundLayer_t *layers[4]; vec2_t offset;} Background_t;
extern Background_t *background_create();
extern void background_setLayer(Background_t *aBackground, unsigned int aIndex, BackgroundLayer_t *aLayer);
extern BackgroundLayer_t *background_createLayer(Texture_t *aTexture, float aDepth);
typedef enum { kInputKey_arrowLeft,kInputKey_arrowRight, kInputKey_arrowUp,kInputKey_arrowDown, kInputKey_ascii,kInputMouse_leftClick,kInputMouse_rightClick,kInputMouse_leftDrag,kInputMouse_rightDrag,kInputMouse_move,kInputTouch1,kInputTouch2,kInputTouch3,kInputTouch4,kInputTouch5, kInputGyro } Input_type_t;
typedef enum { kInputState_up, kInputState_down } Input_state_t;
typedef struct _InputManager InputManager_t;
typedef struct _InputObserver InputObserver_t;
typedef void (*Input_handler_t)(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec3_t *aLocation, int aState, void *aMetaData);
struct _InputObserver { _Obj_guts _guts; Input_handler_t handlerCallback; int luaHandlerCallback; Input_type_t type; unsigned char code;  void *metaData;  Input_state_t lastKnownState;};
struct _InputManager { _Obj_guts _guts; LinkedList_t *observers; LinkedList_t *activeEvents;};
extern InputManager_t *input_createManager();
extern InputObserver_t *input_createObserver(Input_type_t aObservedType, Input_handler_t aHandlerCallback, char *aCode, void *aMetaData);
extern void input_addObserver(InputManager_t *aManager, InputObserver_t *aObserver);
extern bool input_removeObserver(InputManager_t *aManager, InputObserver_t *aObserver);
extern void input_postActiveEvents(InputManager_t *aManager);
extern void input_postMomentaryEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode, vec3_t *aLocation, Input_state_t aState);
extern void input_beginEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode, vec3_t *aLocation);
extern void input_endEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode);
typedef enum _TMXMap_orientation { kTMXMap_orthogonal, kTMXMap_isometric } TMXMap_orientation;
typedef struct _TMXProperty { char *name; char *value;} TMXProperty_t;
typedef struct _TMXTileset { int firstTileGid; int imageWidth, imageHeight; int tileWidth, tileHeight; int spacing; int margin; char *imagePath;} TMXTileset_t;
typedef struct _TMXTile { TMXTileset_t *tileset; int id; bool flippedVertically; bool flippedHorizontally;} TMXTile_t;
typedef struct _TMXLayer { char *name; float opacity; bool isVisible; int numberOfTiles; TMXTile_t *tiles; int numberOfProperties; TMXProperty_t *properties; } TMXLayer_t;
typedef struct _TMXObject { char *name; char *type; int x, y;  int width, height;  TMXTile_t tile;  int numberOfProperties; TMXProperty_t *properties; } TMXObject_t;
typedef struct _TMXObjectGroup { char *name; int numberOfObjects; TMXObject_t *objects; int numberOfProperties; TMXProperty_t *properties; } TMXObjectGroup_t;
typedef struct _TMXMap { _Obj_guts _guts; TMXMap_orientation orientation; int width, height;  int tileWidth, tileHeight;  int numberOfLayers; TMXLayer_t *layers; int numberOfTilesets; TMXTileset_t *tilesets; int numberOfObjectGroups; TMXObjectGroup_t *objectGroups; int numberOfProperties; TMXProperty_t *properties;} TMXMap_t;
extern TMXMap_t *tmx_readMapFile(const char *aFilename);
typedef struct _TMXLayerRenderable { _Obj_guts _guts; RenderableDisplayCallback_t displayCallback; int luaDisplayCallback; TMXLayer_t *layer; TMXMap_t *map;  TextureAtlas_t *atlas; GLuint posVBO, texCoordVBO, indexVBO; int indexCount;} TMXLayerRenderable_t;
extern TMXLayerRenderable_t *tmx_createRenderableForLayer(TMXMap_t *aMap, unsigned int aLayerIdx);
extern const char *tmx_mapGetPropertyNamed(TMXMap_t *aMap, const char *aPropertyName);
extern TMXLayer_t *tmx_mapGetLayerNamed(TMXMap_t *aMap, const char *aLayerName);
extern TMXObjectGroup_t *tmx_mapGetObjectGroupNamed(TMXMap_t *aMap, const char *aGroupName);
extern TMXObject_t *tmx_objGroupGetObjectNamed(TMXObjectGroup_t *aGroup, const char *aObjName);
extern Class_t Class_SoundEffect;
typedef struct _SoundEffect SoundEffect_t;
extern Class_t Class_BackgroundMusic;
typedef struct _BackgroundMusic BackgroundMusic_t;
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
extern void bgm_setLooping(BackgroundMusic_t *aBGM, bool aLoops);
extern SoundManager_t *soundManager_create();
extern bool soundManager_makeCurrent(SoundManager_t *aManager);
typedef struct _World World_t;
typedef struct _WorldShape WorldShape_t;
typedef uintptr_t WorldShapeGroup_t;
typedef struct _WorldEntity WorldEntity_t;
typedef struct _World_ContactPointSet { int count; struct { vec2_t point; vec2_t normal; GLMFloat depth; } points[4];} World_ContactPointSet;
typedef struct _World_CollisionInfo { WorldEntity_t *a; WorldEntity_t *b; bool firstContact; World_ContactPointSet contactPoints; void *cpArbiter;} World_CollisionInfo;
typedef void (*WorldEntity_CollisionHandler)(WorldEntity_t *aEntity, World_CollisionInfo *aCollisionInfo);
typedef void (*WorldEntity_UpdateHandler)(WorldEntity_t *aEntity);
extern Class_t Class_WorldEntity;
struct _WorldEntity { _Obj_guts _guts; World_t *world;  Obj_t *owner;  void *cpBody; LinkedList_t *shapes; WorldEntity_UpdateHandler cUpdateHandler; WorldEntity_CollisionHandler cPreCollisionHandler; WorldEntity_CollisionHandler cCollisionHandler; WorldEntity_CollisionHandler cPostCollisionHandler; int luaUpdateHandler; int luaPreCollisionHandler; int luaCollisionHandler; int luaPostCollisionHandler; };
struct _WorldShape { _Obj_guts _guts; void *cpShape;};
struct _World { _Obj_guts _guts; void *cpSpace; LinkedList_t *entities; WorldEntity_t *staticEntity; bool isPaused; };
typedef enum { kWorldJointType_Pin,    kWorldJointType_Slide,    kWorldJointType_Pivot,    kWorldJointType_Groove,    kWorldJointType_DampedSpring,    kWorldJointType_DampedRotarySpring,    kWorldJointType_RotaryLimit,    kWorldJointType_Ratchet,    kWorldJointType_Gear,    kWorldJointType_SimpleMotor } WorldJointType_t;
extern Class_t Class_WorldConstraint;
typedef struct _WorldConstraint {    World_t *world;     WorldEntity_t *a, *b;    WorldJointType_t type;    void *cpConstraint; } WorldConstraint_t;
extern World_t *world_create(void);
extern void world_step(World_t *aWorld, GameTimer_t *aTimer);
extern void world_setGravity(World_t *aWorld, vec2_t aGravity);
extern vec2_t world_gravity(World_t *aWorld);
extern void world_addEntity(World_t *aWorld, WorldEntity_t *aEntity);
extern void world_removeEntity(World_t *aWorld, WorldEntity_t *aEntity);
extern void world_addStaticEntity(World_t *aWorld, WorldEntity_t *aEntity);
void *world_pointQuery(World_t *aWorld, vec2_t aPoint, bool aQueryForShape);
extern vec2_t worldEnt_location(WorldEntity_t *aEntity);
extern WorldEntity_t *worldEnt_create(World_t *aWorld, Obj_t *aOwner, GLMFloat aMass, GLMFloat aMomentum);
extern void worldEnt_setLocation(WorldEntity_t *aEntity, vec2_t aLocation);
extern GLMFloat worldEnt_angle(WorldEntity_t *aEntity);
extern void worldEnt_setAngle(WorldEntity_t *aEntity, GLMFloat aAngle);
extern WorldShape_t *worldEnt_addShape(WorldEntity_t *aEntity, WorldShape_t *aShape);
extern void worldEnt_applyForce(WorldEntity_t *aEntity, vec2_t aForce, vec2_t aOffset);
extern void worldEnt_applyImpulse(WorldEntity_t *aEntity, vec2_t aImpulse, vec2_t aOffset);
extern vec2_t worldEnt_velocity(WorldEntity_t *aEntity);
extern void worldEnt_setVelocity(WorldEntity_t *aEntity, vec2_t aVelocity);
extern WorldShape_t *worldShape_createCircle(vec2_t aCenter, GLMFloat aRadius);
extern WorldShape_t *worldShape_createSegment(vec2_t a, vec2_t b, GLMFloat aThickness);
extern WorldShape_t *worldShape_createBox(vec2_t aSize);
extern WorldShape_t *worldShape_createPoly(unsigned aVertCount, vec2_t *aVerts);
extern GLMFloat worldShape_friction(WorldShape_t *aEntity);
extern void worldShape_setFriction(WorldShape_t *aEntity, GLMFloat aVal);
extern GLMFloat worldShape_elasticity(WorldShape_t *aEntity);
extern void worldShape_setElasticity(WorldShape_t *aEntity, GLMFloat aVal);
extern WorldShapeGroup_t worldShape_group(WorldShape_t *aShape);
extern void worldShape_setGroup(WorldShape_t *aShape, WorldShapeGroup_t aGroup);
extern void worldShape_setCollides(WorldShape_t *aShape, bool aCollides);
extern bool worldShape_collides(WorldShape_t *aShape);
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
extern void worldConstr_invalidate(WorldConstraint_t *aConstraint);
extern void draw_world(World_t *aWorld, bool aDrawBB);
extern void draw_worldShape(WorldShape_t *aShape, WorldEntity_t *aEntity, bool aDrawBB);
extern void draw_worldEntity(WorldEntity_t *aEntity, bool aDrawBB);
extern bool util_pathForResource(const char *name, const char *ext, const char *dir, char *output, int maxLen);
typedef enum { kPlatformMac, kPlatformIOS, kPlatformAndroid, kPlatformWindows, kPlatformOther } Platform_t;
extern Platform_t util_platform(void);
extern void _dynamo_log(const char *str);
typedef LinkedList_t Obj_autoReleasePool_t;
Obj_autoReleasePool_t *autoReleasePool_getGlobal();
void autoReleasePool_drain(Obj_autoReleasePool_t *aPool);
]]

dynamo.platforms = {
    mac     = lib.kPlatformMac,
    ios     = lib.kPlatformIOS,
    android = lib.kPlatformAndroid,
    windows = lib.kPlatformWindows,
    other   = lib.kPlatformOther
}
dynamo.platform = lib.util_platform()

--
-- Utilities

-- Retains an object and tells the gc to release it when it's no longer referenced
local function _obj_addToGC(obj)
    lib.obj_retain(obj)
    ffi.gc(obj, lib.obj_release)
    --ffi.gc(obj, function(obj)
        --dynamo.log("Collecting ", tostring(obj))
        --lib.obj_release(obj)
    --end)
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
    local prefix = ""
    if debug ~= nil then
        local info = debug.getinfo(2, "Sln")
        if info.what == "C" then
            prefix = "<C function>: "
        else
            prefix = string.format("%s:%s: ", info.short_src, info.currentline)
        end
    end
    lib._dynamo_log(prefix..table.concat({...}, ", "))
end

-- Override rgba to use premultiplied alpha
rgba = function(r,g,b,a)
    return vec4(r,g,b,a):premultiply()
end

dynamo.registerCallback = function(lambda)
    if type(lambda) == "function" then
        return dynamo_registerCallback(lambda)
    elseif type(lambda) == "number" then
        return lambda
    else
        error("Invalid callback")
    end
end
dynamo.unregisterCallback = function(id) return dynamo_unregisterCallback(id) end
--
-- Textures

dynamo.texture = {}

ffi.metatype("Texture_t", {
    __index = {
        getSubTextureRect = lib.texture_getSubTextureRect,
        getSubTextureOrigin = lib.texture_getSubTextureOrigin,
        getSubTextureSize = lib.texture_getSubTextureSize,
        getSubTextureAtlas = function(self, name, size)
            size = size or self:getSubTextureSize(name)
            local atlas = lib.texture_getSubTextureAtlas(self, name, size)
            return _obj_addToGC(atlas)
        end
    }
})

function dynamo.texture.load(path, packingInfoPath, tile)
    tile = tile or false

    local tex = lib.texture_loadFromPng(path, tile, tile)
    if tex == nil then
        dynamo.log("Couldn't find texture at path", path)
        return nil
    end
    if packingInfoPath ~= nil then
        lib.texture_loadPackingInfo(tex, packingInfoPath)
    end
    return _obj_addToGC(tex)
end


--
-- Texture atlases

dynamo.atlas = {}

ffi.metatype("TextureAtlas_t", {
    __index = {
        getTextureRect = lib.texAtlas_getTextureRect,
    }
})

dynamo.atlas.create = function(...) return _obj_addToGC(lib.texAtlas_create(...)) end

function dynamo.atlas.load(path, origin, size)
    local tex = dynamo.texture.load(path)
    return dynamo.atlas.create(tex, origin, size)
end


--
-- Sprites

dynamo.sprite = {}

ffi.metatype("Sprite_t", {
    __index = {
        step = lib.sprite_step,
        currentFrame = function(self)
            return self.animations[self.activeAnimation].currentFrame
        end
    },
    __newindex = function(self, key, val)
        if key == "currentFrame" then
            self.animations[self.activeAnimation].currentFrame = val
        elseif key == "atlas" then
            lib.obj_retain(val);
            lib.obj_release(self._atlas)
            self._atlas = val
        end
    end
})

local _vec2Type = ffi.typeof("vec2_t")
function dynamo.sprite.create(location, atlas, size, animations)
    animations = animations or {{1,0,false}}
    size = size or atlas.size

    if(ffi.istype(location, vec2_t)) then
        location = vec3(location.x, location.y, 0)
    end

    local sprite = lib.sprite_create(location, size, atlas, #animations)

    -- Add the animations to the sprite
    for i, animation in pairs(animations) do
        sprite.animations[i-1] = animation
    end

    return _obj_addToGC(sprite)
end

ffi.metatype("SpriteBatch_t", {
    __index = {
        addSprite = lib.spriteBatch_addSprite,
        insertSprite = lib.spriteBatch_insertSprite,
        replaceSprite = function(self, spriteToInsert, spriteToDelete)
            self:insertSprite(spriteToInsert, spriteToDelete)
            return self:deleteSprite(spriteToDelete)
        end,
        deleteSprite = lib.spriteBatch_deleteSprite
    }
})

function dynamo.sprite.createBatch(sprites)
    sprites = sprites or {}
    local batch = lib.spriteBatch_create()
    for i, sprite in pairs(sprites) do
        batch:addSprite(sprite)
    end
    return _obj_addToGC(batch)
end


--
-- The Renderer

ffi.metatype("Renderer_t", {
    __index = {
        display = lib.renderer_display,
        pushRenderable = lib.renderer_pushRenderable,
        pushRenderableFunction = function(self, lambda)
            self:pushRenderable(dynamo.renderable(lambda))
        end,
        popRenderable = lib.renderer_popRenderable,
        insertRenderable = lib.renderer_insertRenderable,
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

dynamo.scene = {}

ffi.metatype("Scene_t", {
    __index = {
        pushRenderable = lib.scene_pushRenderable,
        pushRenderableFunction = function(self, lambda)
            self:pushRenderable(dynamo.renderable(lambda))
        end,
        popRenderable = lib.scene_popRenderable,
        insertRenderable = lib.scene_insertRenderable,
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

function dynamo.scene.create(renderables, initialTransform)
    renderables = renderables or {}
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

dynamo.input = {
    types = {
        key = {
            arrowLeft  = lib.kInputKey_arrowLeft,
            arrowRight = lib.kInputKey_arrowRight,
            arrowUp    = lib.kInputKey_arrowUp,
            arrowDown  = lib.kInputKey_arrowDown,
            ascii      = lib.kInputKey_ascii,
        },
        mouse = {
            leftClick  = lib.kInputMouse_leftClick,
            rightClick = lib.kInputMouse_rightClick,
            leftDrag   = lib.kInputMouse_leftDrag,
            rightDrag  = lib.kInputMouse_rightDrag,
            move       = lib.kInputMouse_move
        },
        touch = {
            lib.kInputTouch1,
            lib.kInputTouch2,
            lib.kInputTouch3,
            lib.kInputTouch4,
            lib.kInputTouch5
        },
        gyro = lib.kInputGyro
    },
    states = {
        down = lib.kInputState_down,
        up   = lib.kInputState_up
    }
}

ffi.metatype("InputManager_t", {
    __index = {
        addObserver = function(self, desc)
            local observer = lib.input_createObserver(desc.type, nil, desc.charCode, desc.metaData)
            observer.luaHandlerCallback = dynamo.registerCallback(desc.callback)
            lib.input_addObserver(self, observer)
            return _obj_addToGC(observer)
        end,
        removeObserver = lib.input_removeObserver,
        postActiveEvents = lib.input_postActiveEvents,
        postMomentaryEvent = lib.input_postMomentaryEvent,
        beginEvent = lib.input_beginEvent,
        endEvent = lib.input_endEvent,
        postTouchEvent = function(self, finger, isDown, x, y)
            finger = finger or 0
            local pos = vec3(x or 0, y or 0, 0)
            local state = lib.kInputState_up
            if isDown == true then
                state = lib.kInputState_down
            end
            self:postMomentaryEvent(dynamo.input.types.touch[1] + finger, nil, pos, state)
        end,
        postGyroEvent = function(self, x, y, z)
            local thetas = vec3(x, y, z)
            self:postMomentaryEvent(dynamo.input.types.gyro, nil, thetas, lib.kInputState_up)
        end
    }
})

local _createInputManager = function(...) return _obj_addToGC(lib.input_createManager(...)) end


--
-- Game timer

ffi.metatype("GameTimer_t", {
    __index = {
        step = lib.gameTimer_step,
        interpolation = lib.gameTimer_interpolationSinceLastUpdate,
        afterDelay = function(self, delay, callback, repeats)
            repeats = repeats or false
            callback = dynamo.registerCallback(callback)
            return lib.gameTimer_afterDelay_luaCallback(self, delay, callback, repeats)
        end,
        unschedule = function(self, callback)
            return lib.gameTimer_unscheduleCallback(self, callback)
        end,
        setUpdateHandler = function(self, lambda)
            local oldHandler = self.luaUpdateCallback
            self.luaUpdateCallback = dynamo.registerCallback(lambda)
            if oldHandler ~= -1 then
                dynamo_unregisterCallback(oldHandler)
            end
        end,
        pause  = lib.gameTimer_pause,
        resume = lib.gameTimer_resume,
        reset  = lib.gameTimer_reset
    }
})

local _createTimer = function(desiredFPS, updateCallback)
    local timer = lib.gameTimer_create(desiredFPS, nil)
    if updateCallback then
        timer:setUpdateHandler(updateCallback)
    end
    return _obj_addToGC(timer)
end

dynamo.globalTime = lib.dynamo_globalTime
dynamo.time = lib.dynamo_time

math.randomseed(dynamo.globalTime())


--
-- Custom renderables

function dynamo.renderable(lambda)
    local drawable = ffi.cast("Renderable_t*", lib.obj_create_autoreleased(ffi.cast("Class_t*", lib.Class_Renderable)))
    drawable.luaDisplayCallback = dynamo.registerCallback(lambda)
    return _obj_addToGC(drawable)
end


--
-- Maps

dynamo.map = {}

ffi.metatype("TMXMap_t", {
    __index = {
        getProperty           = lib.tmx_mapGetPropertyNamed,
        getLayer              = tmx_mapGetLayerNamed,
        getObjectGroup        = tmx_mapGetObjectGroupNamed,
        createLayerRenderable = function(...) return _obj_addToGC(lib.tmx_createRenderableForLayer(...)) end
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

dynamo.map.load = function(...) return _obj_addToGC(lib.tmx_readMapFile(...)) end

--
-- Backgrounds

dynamo.background = {}
dynamo.background.create = function(layers)
    if layers == nil or #layers == 0 then
        error("No layers provided for background")
    elseif #layers >= 4 then
        error("Backgrounds only support up to 4 layers")
    end
    
    local bg = lib.background_create()
    local i = 0;
    for _,layerInfo in pairs(layers) do
        local layer = lib.background_createLayer(layerInfo.texture, layerInfo.depth)
        if layerInfo.opacity ~= nil then
             layer.opacity = layerInfo.opacity
        end
        lib.background_setLayer(bg, i, layer)
        i = i+1
    end
    return _obj_addToGC(bg)
end

--
-- Sound

dynamo.sound = { sfx = {}, bgm = {} }
local _createSoundManager = function(...) return _obj_addToGC(lib.soundManager_create(...)) end

ffi.metatype("SoundEffect_t", {
    __index = {
        play      = lib.sfx_play,
        stop      = lib.sfx_stop,
        isPlaying = lib.sfx_isPlaying,
        unload    = lib.sfx_unload
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
        play      = lib.bgm_play,
        stop      = lib.bgm_stop,
        seek      = lib.bgm_seek,
        isPlaying = lib.bgm_isPlaying,
        unload    = lib.bgm_unload
    },
    __newindex = function(self, key, val)
        if key == "volume" then
            lib.bgm_setVolume(self, val)
        elseif key == "loops" then
            lib.bgm_setLooping(self, val)
        else
            error("Undefined key "..key)
        end
    end
})

dynamo.sound.sfx.load = function(...) return _obj_addToGC(lib.sfx_load(...)) end
dynamo.sound.bgm.load = function(...) return _obj_addToGC(lib.bgm_load(...)) end


--
-- Game world

ffi.metatype("World_t", {
    __index = {
        addEntity        = lib.world_addEntity,
        removeEntity        = lib.world_removeEntity,
        gravity          = lib.world_gravity,
        step             = lib.world_step,
        momentForCircle  = lib.world_momentForCircle,
        momentForSegment = lib.world_momentForSegment,
        momentForPoly    = lib.world_momentForPoly,
        momentForBox     = lib.world_momentForBox,
        draw             = lib.draw_world,
        drawShape        = lib.draw_worldShape,
        drawEntity       = lib.draw_worldEntity,
        pointQuery       = function(self, point, queryForShape)
            queryForShape = queryForShape or false
            local result = lib.world_pointQuery(self, point, queryForShape)
            if queryForShape then
                return ffi.cast("WorldShape_t*", result)
            else
                return ffi.cast("WorldEntity_t*", result)
            end
        end,
        createEntity     = function(world, owner, mass, momentum, shapes)
            shapes = shapes or {}
            local ret = lib.worldEnt_create(world, owner, mass, momentum)
            for i,shape in ipairs(shapes) do
                ret:addShape(shape)
            end
            return _obj_addToGC(ret)
        end,
        createCircleShape  = function(self, ...) return _obj_addToGC(lib.worldShape_createCircle(...)) end,
        createBoxShape     = function(self, ...) return _obj_addToGC(lib.worldShape_createBox(...)) end,
        createSegmentShape = function(self, a, b, thickness)
            return _obj_addToGC(lib.worldShape_createSegment(a, b, (thickness or 1)))
        end,
        createPolyShape = function(self, vertices)
            if #vertices < 3 then
                error("Too few vertices to create polygon shape")
            end
            local vertArr = ffi.new("vec2_t[?]", #vertices, unpack(vertices))
            return _obj_addToGC(lib.worldShape_createPoly(#vertices, vertArr))
        end
    },
    __newindex = function(self, key, val)
        if key == "gravity" then
            lib.world_setGravity(self, val)
        end
    end
})
local _createWorld = function(...)
    return _obj_addToGC(lib.world_create(...))
end

ffi.metatype("WorldEntity_t", {
    __index = {
        location     = lib.worldEnt_location,
        angle        = lib.worldEnt_angle,
        velocity     = lib.worldEnt_velocity,
        addShape     = lib.worldEnt_addShape,
        applyForce   = lib.worldEnt_applyForce,
        applyImpulse = lib.worldEnt_applyImpulse,

        -- Shapes
        addCircleShape = function(self, center, radius)
            return self:addShape(self.world:createCircleShape(center, radius))
        end,
        addBoxShape = function(self, size)
            return self:addShape(self.world:createBoxShape(size))
        end,
        addSegmentShape = function(self, a, b, thickness)
            return self:addShape(self.world:createSegmentShape(a, b, thickness))
        end,
        addPolyShape = function(self, vertices)
            return self:addShape(self.world:createPolyShape(vertices))
        end,

        -- Constraints
        createPinJoint           = function(...) return _obj_addToGC(lib.worldConstr_createPinJoint(...)) end,
        createSlideJoint         = function(...) return _obj_addToGC(lib.worldConstr_createSlideJoint(...)) end,
        createPivotJoint         = function(...) return _obj_addToGC(lib.worldConstr_createPivotJoint(...)) end,
        createGrooveJoint        = function(...) return _obj_addToGC(lib.worldConstr_createGrooveJoint(...)) end,
        createDampedSpring       = function(...) return _obj_addToGC(lib.worldConstr_createDampedSpringJoint(...)) end,
        createDampedRotarySpring = function(...) return _obj_addToGC(lib.worldConstr_createDampedRotarySpringJoint(...)) end,
        creatRotaryLimitJoint    = function(...) return _obj_addToGC(lib.worldConstr_createRotaryLimitJoint(...)) end,
        createRatchetJoint       = function(...) return _obj_addToGC(lib.worldConstr_createRatchetJoint(...)) end,
        createGearJoint          = function(...) return _obj_addToGC(lib.worldConstr_createGearJoint(...)) end,
        createSimpleMotorJoint   = function(...) return _obj_addToGC(lib.worldConstr_createSimpleMotorJoint(...)) end
    },
    __newindex = function(self, key, val)
        if key == "location" then
            lib.worldEnt_setLocation(self, val)
        elseif key == "angle" then
            lib.worldEnt_setAngle(self, val)
        elseif key == "velocity" then
            lib.worldEnt_setVelocity(self, val)
        elseif key == "updateHandler" then
            self.luaUpdateHandler = dynamo.registerCallback(val)
        elseif key == "preCollisionHandler" then
            self.luaPreCollisionHandler = dynamo.registerCallback(val)
        elseif key == "collisionHandler" then
            self.luaCollisionHandler = dynamo.registerCallback(val)
        elseif key == "postCollisionHandler" then
            self.luaPostCollisionHandler = dynamo.registerCallback(val)
        end
    end
})

ffi.metatype("WorldShape_t", {
    __index = {
        friction = lib.worldShape_friction,
        elasticity = lib.worldShape_elasticity,
        group = lib.worldShape_group,
        collides = lib.worldShape_collides
    },
    __newindex = function(self, key, val)
        if key == "friction" then
            lib.worldShape_setFriction(self, val)
        elseif key == "elasticity" then
            lib.worldShape_setElasticity(self, val)
        elseif key == "group" then
            lib.worldShape_setGroup(self, val)
        elseif key == "collides" then
            lib.worldShape_setCollides(self, val)
        end
    end
})

ffi.metatype("WorldConstraint_t", {
    __index = {
        invalidate = lib.worldConstr_invalidate
    }
})


--
-- Lifecycle/HighLevelInterface functions
dynamo.initialized = false

function dynamo.init(viewport, desiredFPS, ...)
    assert(dynamo.initialized == false)
    dynamo.initialized = true

    gl.glEnable(gl.GL_BLEND);
    gl.glBlendFunc(gl.GL_ONE, gl.GL_ONE_MINUS_SRC_ALPHA)
    gl.glEnable(gl.GL_CULL_FACE);
    gl.glFrontFace(gl.GL_CW);
    gl.glDisable(gl.GL_DEPTH_TEST);

    dynamo.renderer = _createRenderer(viewport)
    lib.draw_init(dynamo.renderer)
    dynamo.renderer:handleResize(viewport)

    dynamo.timer = _createTimer(desiredFPS, nil)

    dynamo.input.manager = _createInputManager()
    dynamo.soundManager  = _createSoundManager()
    lib.soundManager_makeCurrent(dynamo.soundManager)

    dynamo.world = _createWorld()
    dynamo.world.gravity = vec2(0, -100)

    local startCallback = rawget(_G, "dynamoStartCallback")
    if startCallback ~= nil then
        startCallback(...)
    end

    return true
end

dynamo.cleanupHandler = function()
    -- Do nothing by default
end

function dynamo.cleanup()
    dynamo.initialized = false
    if dynamo.cleanupHandler ~= nil then
        dynamo.cleanupHandler()
    end
    dynamo.log("Dynamo is cleaning up after itself")

    -- Release all resources
    dynamo.renderer = nil
    dynamo.timer = nil
    dynamo.input.manager = nil
    dynamo.world = nil
    dynamo.soundManager = nil
    lib.soundManager_makeCurrent(nil)
    lib.draw_cleanup()
end

local _messages = nil
-- Passes a message to the host application
function dynamo.passMessage(key, value)
    value = value or ""
    local t = type(value)
    if t ~= "string" and t ~= "number" and t ~= "boolean" then
        error("Invalid type for message")
    elseif type(key) ~= "string" then
        error("Invalid type for message key")
    end
    _messages = _messages or {}
    _messages[key] = value
end

function dynamo.pause()
    dynamo.timer:pause()
end

function dynamo.resume()
    dynamo.timer:resume()
end

function dynamo.cycle()
    if dynamo.initialized ~= true then
        return
    end
    dynamo.input.manager:postActiveEvents()
    dynamo.timer:step(dynamo.time())
    dynamo.world:step(dynamo.timer)
    dynamo.renderer:display(dynamo.timer.timeSinceLastUpdate, dynamo.timer:interpolation())
    lib.autoReleasePool_drain(lib.autoReleasePool_getGlobal())

    local ret = _messages
    _messages = nil
    return ret
end

return dynamo
