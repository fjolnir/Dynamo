// Note: Assumes cpFloat is the same type as GLMFloat
#include "world.h"
#include "input.h"
#include "luacontext.h"

#define VEC2_TO_CPV(v) ((cpVect){ v.x, v.y })
#define CPV_TO_VEC2(v) ((vec2_t){ v.x, v.y })

// Linked list appliers
// Used to add/remove shapes from the space
static void _addShapeToSpace(WorldShape_t *aShape, World_t *aWorld);
static void _removeShapeFromSpace(WorldShape_t *aShape, World_t *aWorld);
static void _removeEntityFromWorld(WorldEntity_t *aEntity, World_t *aWorld);

static void _callEntityUpdateCallback(WorldEntity_t *aEntity, World_t *aWorld);

static int collisionWillBegin(cpArbiter *aArbiter, struct cpSpace *aSpace, void *aData);
static void collisionDidBegin(cpArbiter *aArbiter, struct cpSpace *aSpace, void *aData);
static void collisionDidEnd(cpArbiter *aArbiter, struct cpSpace *aSpace, void *aData);

static void world_destroy(World_t *aWorld);
Class_t Class_World = {
    "World",
    sizeof(World_t),
    (Obj_destructor_t)&world_destroy
};

static void worldEnt_destroy(WorldEntity_t *aEntity);
Class_t Class_WorldEntity = {
    "WorldEntity",
    sizeof(WorldEntity_t),
    (Obj_destructor_t)&worldEnt_destroy
};

static void worldShape_destroy(WorldShape_t *aEntity);
Class_t Class_WorldShape = {
    "WorldShape",
    sizeof(WorldShape_t),
    (Obj_destructor_t)&worldShape_destroy
};

#pragma mark - World

World_t *world_create(void)
{
    World_t *out = obj_create_autoreleased(&Class_World);
    out->cpSpace = cpSpaceNew();
    out->cpSpace->data = out;
    cpSpaceAddCollisionHandler(out->cpSpace, 0, 0,
                               collisionWillBegin,
                               NULL,
                               collisionDidBegin,
                               collisionDidEnd, NULL);
    out->entities = obj_retain(llist_create((InsertionCallback_t)&obj_retain, &obj_release));
    
    // Create the static entity
    out->staticEntity = obj_create(&Class_WorldEntity);
    out->staticEntity->world = out;
    out->staticEntity->owner = out;
    out->staticEntity->cpBody = out->cpSpace->staticBody;
	out->staticEntity->luaUpdateHandler = -1;
    out->staticEntity->luaPreCollisionHandler = -1;
    out->staticEntity->luaCollisionHandler = -1;
    out->staticEntity->luaPostCollisionHandler = -1;
    out->cpSpace->staticBody->data = out->staticEntity;
    out->staticEntity->shapes = obj_retain(llist_create((InsertionCallback_t)&obj_retain, &obj_release));
    
    return out;
}

void world_step(World_t *aWorld, GameTimer_t *aTimer)
{
    float dt = 1.0/60.0;
    for(float t = 0.0f; t < aTimer->desiredInterval; t += dt)
        cpSpaceStep(aWorld->cpSpace, dt);
    llist_apply(aWorld->entities, (LinkedListApplier_t)&_callEntityUpdateCallback, aWorld);
}

void world_destroy(World_t *aWorld)
{
    llist_apply(aWorld->entities, (LinkedListApplier_t)&_removeEntityFromWorld, aWorld);
    world_removeEntity(aWorld, aWorld->staticEntity);
    cpSpaceFree(aWorld->cpSpace), aWorld->cpSpace = NULL;
    obj_release(aWorld->entities);
    obj_release(aWorld->staticEntity);
}

void world_addEntity(World_t *aWorld, WorldEntity_t *aEntity)
{
    dynamo_assert(aEntity != aWorld->staticEntity, "You cannot re-add static entity to world");
    cpSpaceAddBody(aWorld->cpSpace, aEntity->cpBody);
    llist_apply(aEntity->shapes, (LinkedListApplier_t)&_addShapeToSpace, aWorld);
    llist_pushValue(aWorld->entities, aEntity);
}

void world_removeEntity(World_t *aWorld, WorldEntity_t *aEntity)
{
    llist_apply(aEntity->shapes, (LinkedListApplier_t)&_removeShapeFromSpace, aWorld);
    llist_deleteValue(aWorld->entities, aEntity);
}

void world_setGravity(World_t *aWorld, vec2_t aGravity)
{
    cpSpaceSetGravity(aWorld->cpSpace, VEC2_TO_CPV(aGravity));
}
vec2_t world_gravity(World_t *aWorld)
{
    cpVect gravity = cpSpaceGetGravity(aWorld->cpSpace);
    return CPV_TO_VEC2(gravity);
}

WorldEntity_t *world_pointQuery(World_t *aWorld, vec2_t aPoint)
{
    cpShape *cpShape = cpSpacePointQueryFirst(aWorld->cpSpace, VEC2_TO_CPV(aPoint), CP_ALL_LAYERS, CP_NO_GROUP);
    if(!cpShape)
        return NULL;
    WorldEntity_t *entity = cpShape->body->data;
    return entity;
}


#pragma mark - World entities

WorldEntity_t *worldEnt_create(World_t *aWorld, Obj_t *aOwner, GLMFloat aMass, GLMFloat amoment)
{
    WorldEntity_t *out = obj_create_autoreleased(&Class_WorldEntity);
    out->world = aWorld;
    out->owner = aOwner;
    out->cpBody = cpBodyNew(aMass, amoment);
    out->cpBody->data = out;

    out->shapes = obj_retain(llist_create((InsertionCallback_t)&obj_retain, &obj_release));
    
    out->luaUpdateHandler = -1;
    out->luaPreCollisionHandler = -1;
    out->luaCollisionHandler = -1;
    out->luaPostCollisionHandler = -1;
    
    return out;
}

void worldEnt_destroy(WorldEntity_t *aEntity)
{
    if(aEntity != aEntity->world->staticEntity)
        cpBodyFree(aEntity->cpBody);
    obj_release(aEntity->shapes);
}

vec2_t worldEnt_location(WorldEntity_t *aEntity)
{
    cpVect pos = cpBodyGetPos(aEntity->cpBody);
    return CPV_TO_VEC2(pos);
}

void worldEnt_setLocation(WorldEntity_t *aEntity, vec2_t aLocation)
{
    cpBodySetPos(aEntity->cpBody, VEC2_TO_CPV(aLocation));
}

GLMFloat worldEnt_angle(WorldEntity_t *aEntity)
{
    return cpBodyGetAngle(aEntity->cpBody);
}
void worldEnt_setAngle(WorldEntity_t *aEntity, GLMFloat aAngle)
{
    cpBodySetAngle(aEntity->cpBody, aAngle);
}

void worldEnt_applyForce(WorldEntity_t *aEntity, vec2_t aForce, vec2_t aOffset)
{
    cpBodyApplyForce(aEntity->cpBody, VEC2_TO_CPV(aForce), VEC2_TO_CPV(aOffset));
}
void worldEnt_applyImpulse(WorldEntity_t *aEntity, vec2_t aImpulse, vec2_t aOffset)
{
    cpBodyApplyImpulse(aEntity->cpBody, VEC2_TO_CPV(aImpulse), VEC2_TO_CPV(aOffset));
}

vec2_t worldEnt_velocity(WorldEntity_t *aEntity)
{
    cpVect vel = cpBodyGetVel(aEntity->cpBody);
    return CPV_TO_VEC2(vel);
}
void worldEnt_setVelocity(WorldEntity_t *aEntity, vec2_t aVelocity)
{
    cpBodySetVel(aEntity->cpBody, VEC2_TO_CPV(aVelocity));
}

WorldShape_t *worldEnt_addShape(WorldEntity_t *aEntity, WorldShape_t *aShape)
{
    cpShapeSetBody(aShape->cpShape, aEntity->cpBody);
    if(aEntity == aEntity->world->staticEntity) {
        cpSpaceAddShape(aEntity->world->cpSpace, aShape->cpShape);
    }
    llist_pushValue(aEntity->shapes, aShape);
    return aShape;
}

#pragma mark - World entity shapes

WorldShape_t *worldShape_createCircle(vec2_t aCenter, GLMFloat aRadius)
{
    WorldShape_t *out = obj_create_autoreleased(&Class_WorldShape);
    out->cpShape = cpCircleShapeNew(NULL, aRadius, VEC2_TO_CPV(aCenter));
    out->cpShape->data = out;
    return out;
}
WorldShape_t *worldShape_createSegment(vec2_t a, vec2_t b, GLMFloat aThickness)
{
    WorldShape_t *out = obj_create_autoreleased(&Class_WorldShape);
    out->cpShape = cpSegmentShapeNew(NULL, VEC2_TO_CPV(a), VEC2_TO_CPV(b), aThickness);
    out->cpShape->data = out;
    return out;
}
WorldShape_t *worldShape_createBox(vec2_t aSize)
{
    WorldShape_t *out = obj_create_autoreleased(&Class_WorldShape);
    out->cpShape = cpBoxShapeNew(NULL, aSize.w, aSize.h);
    out->cpShape->data = out;
    return out;
}
WorldShape_t *worldShape_createPoly(unsigned aVertCount, vec2_t *aVerts)
{
    WorldShape_t *out = obj_create_autoreleased(&Class_WorldShape);
    out->cpShape = cpPolyShapeNew(NULL, aVertCount, (cpVect*)aVerts, cpvzero);
    return out;
}

void worldShape_destroy(WorldShape_t *aEntity)
{
    cpShapeFree(aEntity->cpShape);
}

GLMFloat worldShape_friction(WorldShape_t *aEntity)
{
    return cpShapeGetFriction(aEntity->cpShape);
}
void worldShape_setFriction(WorldShape_t *aEntity, GLMFloat aVal)
{
    cpShapeSetFriction(aEntity->cpShape, aVal);
}

GLMFloat worldShape_elasticity(WorldShape_t *aEntity)
{
    return cpShapeGetElasticity(aEntity->cpShape);
}
void worldShape_setElasticity(WorldShape_t *aEntity, GLMFloat aVal)
{
    cpShapeSetElasticity(aEntity->cpShape, aVal);
}
WorldShapeGroup_t worldShape_group(WorldShape_t *aShape)
{
    return cpShapeGetGroup(aShape->cpShape);
}
void worldShape_setGroup(WorldShape_t *aShape, WorldShapeGroup_t aGroup)
{
    cpShapeSetGroup(aShape->cpShape, aGroup);
}
void worldShape_setCollides(WorldShape_t *aShape, bool aCollides)
{
    cpShapeSetSensor(aShape->cpShape, aCollides);
}
bool worldShape_collides(WorldShape_t *aShape)
{
    return cpShapeGetSensor(aShape->cpShape);
}


GLMFloat world_momentForCircle(GLMFloat aMass, GLMFloat aInnerRadius, GLMFloat aOuterRadius, vec2_t aOffset)
{
    return cpMomentForCircle(aMass, aInnerRadius, aOuterRadius, VEC2_TO_CPV(aOffset));
}
GLMFloat world_momentForSegment(GLMFloat aMass, vec2_t a, vec2_t b)
{
    return cpMomentForSegment(aMass, VEC2_TO_CPV(a), VEC2_TO_CPV(b));
}
GLMFloat world_momentForPoly(GLMFloat aMass, unsigned aVertCount, vec2_t *aVerts, vec2_t aOffset)
{
    return cpMomentForPoly(aMass, aVertCount, (cpVect*)aVerts, VEC2_TO_CPV(aOffset));
}
GLMFloat world_momentForBox(GLMFloat aMass, vec2_t aSize)
{
    return cpMomentForBox(aMass, aSize.w, aSize.h);
}

#pragma mark - Collision handling

static World_CollisionInfo _collisionInfoForArbiter(cpArbiter *aArbiter)
{
    cpBody *bodyA, *bodyB;
    cpArbiterGetBodies(aArbiter, &bodyA, &bodyB);
    WorldEntity_t *a = bodyA->data; dynamo_assert(a != NULL, "Incomplete collision");
    WorldEntity_t *b = bodyB->data; dynamo_assert(b != NULL, "Incomplete collision");
    
    cpContactPointSet cpPointSet = cpArbiterGetContactPointSet(aArbiter);
    World_ContactPointSet pointSet = *(World_ContactPointSet *)&cpPointSet;
    return (World_CollisionInfo){
        a, b,
        cpArbiterIsFirstContact(aArbiter),
        pointSet,
        aArbiter
    };
}

static void _callLuaCollisionHandler(int aCallback, WorldEntity_t *aCollider, World_CollisionInfo *aCollisionInfo)
{
    if(aCallback == -1)
        return;
    luaCtx_pushScriptHandler(GlobalLuaContext, aCallback);
    luaCtx_pushlightuserdata(GlobalLuaContext, aCollider);
    luaCtx_pushlightuserdata(GlobalLuaContext, aCollisionInfo);
    luaCtx_pcall(GlobalLuaContext, 2, 0, 0);
}

static int collisionWillBegin(cpArbiter *aArbiter, struct cpSpace *aSpace, void *aData)
{
    World_CollisionInfo collInfo = _collisionInfoForArbiter(aArbiter);
    if(collInfo.a->preCollisionHandler)
        collInfo.a->preCollisionHandler(collInfo.a, &collInfo);
    if(collInfo.b->preCollisionHandler)
        collInfo.b->preCollisionHandler(collInfo.b, &collInfo);
    
    _callLuaCollisionHandler(collInfo.a->luaPreCollisionHandler, collInfo.b, &collInfo);
    _callLuaCollisionHandler(collInfo.b->luaPreCollisionHandler, collInfo.a, &collInfo);

    return true;
}
static void collisionDidBegin(cpArbiter *aArbiter, struct cpSpace *aSpace, void *aData)
{
    World_CollisionInfo collInfo = _collisionInfoForArbiter(aArbiter);
    if(collInfo.a->collisionHandler)
        collInfo.a->collisionHandler(collInfo.a, &collInfo);
    if(collInfo.b->collisionHandler)
        collInfo.b->collisionHandler(collInfo.b, &collInfo);
    
    _callLuaCollisionHandler(collInfo.a->luaCollisionHandler, collInfo.b, &collInfo);
    _callLuaCollisionHandler(collInfo.b->luaCollisionHandler, collInfo.a, &collInfo);
}
static void collisionDidEnd(cpArbiter *aArbiter, struct cpSpace *aSpace, void *aData)
{
    World_CollisionInfo collInfo = _collisionInfoForArbiter(aArbiter);
    if(collInfo.a->postCollisionHandler)
        collInfo.a->postCollisionHandler(collInfo.a, &collInfo);
    if(collInfo.b->postCollisionHandler)
        collInfo.b->postCollisionHandler(collInfo.b, &collInfo);
    
    _callLuaCollisionHandler(collInfo.a->luaPostCollisionHandler, collInfo.b, &collInfo);
    _callLuaCollisionHandler(collInfo.b->luaPostCollisionHandler, collInfo.a, &collInfo);
}

#pragma mark -

static void _addShapeToSpace(WorldShape_t *aShape, World_t *aWorld)
{
    cpSpaceAddShape(aWorld->cpSpace, aShape->cpShape);
}
static void _removeShapeFromSpace(WorldShape_t *aShape, World_t *aWorld)
{
    cpSpaceRemoveShape(aWorld->cpSpace, aShape->cpShape);
}
static void _removeEntityFromWorld(WorldEntity_t *aEntity, World_t *aWorld)
{
    world_removeEntity(aWorld, aEntity);
    if(aEntity != aWorld->staticEntity)
        cpSpaceRemoveBody(aWorld->cpSpace, aEntity->cpBody);
}

static void _callEntityUpdateCallback(WorldEntity_t *aEntity, World_t *aWorld)
{
    if(aEntity->updateHandler)
        aEntity->updateHandler(aEntity);
    if(aEntity->luaUpdateHandler != -1) {
        luaCtx_pushScriptHandler(GlobalLuaContext, aEntity->luaUpdateHandler);
        luaCtx_pushlightuserdata(GlobalLuaContext, aEntity);
        luaCtx_pcall(GlobalLuaContext, 1, 0, 0);
    }
}

#pragma mark - Joints

static void world_destroyJoint(WorldConstraint_t *aJoint);
Class_t Class_WorldConstraint = {
    "WorldJoint",
    sizeof(WorldConstraint_t),
    (Obj_destructor_t)&world_destroyJoint
};

static void world_destroyJoint(WorldConstraint_t *aJoint)
{
    worldConstr_invalidate(aJoint);
    cpConstraintFree(aJoint->cpConstraint);
    obj_release(aJoint->a);
    obj_release(aJoint->b);
}

WorldConstraint_t *worldConstr_createPinJoint(WorldEntity_t *a, WorldEntity_t *b, vec2_t aAnchorA, vec2_t aAnchorB)
{
    dynamo_assert(a->world == b->world, "Entities are not in the same world");
    WorldConstraint_t *ret = obj_create_autoreleased(&Class_WorldConstraint);
    ret->world = a->world;
    ret->a = obj_retain(a);
    ret->b = obj_retain(b);
    ret->type = kWorldJointType_Pin;
    ret->cpConstraint = cpPinJointNew(a->cpBody, b->cpBody, VEC2_TO_CPV(aAnchorA), VEC2_TO_CPV(aAnchorB));
    cpSpaceAddConstraint(ret->world->cpSpace, ret->cpConstraint);
    return ret;
}

WorldConstraint_t *worldConstr_createSlideJoint(WorldEntity_t *a, WorldEntity_t *b, vec2_t aAnchorA, vec2_t aAnchorB,
                                     GLMFloat aMinDist, GLMFloat aMaxDist)
{
    dynamo_assert(a->world == b->world, "Entities are not in the same world");
    WorldConstraint_t *ret = obj_create_autoreleased(&Class_WorldConstraint);
    ret->world = a->world;
    ret->a = obj_retain(a);
    ret->b = obj_retain(b);
    ret->type = kWorldJointType_Slide;
    ret->cpConstraint = cpSlideJointNew(a->cpBody, b->cpBody, VEC2_TO_CPV(aAnchorA), VEC2_TO_CPV(aAnchorB), aMinDist, aMaxDist);
    cpSpaceAddConstraint(ret->world->cpSpace, ret->cpConstraint);
    return ret;
}
WorldConstraint_t *worldConstr_createPivotJoint(WorldEntity_t *a, WorldEntity_t *b, vec2_t aPivot)
{
    dynamo_assert(a->world == b->world, "Entities are not in the same world");
    WorldConstraint_t *ret = obj_create_autoreleased(&Class_WorldConstraint);
    ret->world = a->world;
    ret->a = obj_retain(a);
    ret->b = obj_retain(b);
    ret->type = kWorldJointType_Pivot;
    ret->cpConstraint = cpPivotJointNew(a->cpBody, b->cpBody, VEC2_TO_CPV(aPivot));
    cpSpaceAddConstraint(ret->world->cpSpace, ret->cpConstraint);
    return ret;
}
WorldConstraint_t *worldConstr_createGrooveJoint(WorldEntity_t *a, WorldEntity_t *b, vec2_t aGrooveStart, vec2_t aGrooveEnd,
                                      vec2_t aAnchorB)
{
    dynamo_assert(a->world == b->world, "Entities are not in the same world");
    WorldConstraint_t *ret = obj_create_autoreleased(&Class_WorldConstraint);
    ret->world = a->world;
    ret->a = obj_retain(a);
    ret->b = obj_retain(b);
    ret->type = kWorldJointType_Groove;
    ret->cpConstraint = cpGrooveJointNew(a->cpBody, b->cpBody, VEC2_TO_CPV(aGrooveStart), VEC2_TO_CPV(aGrooveEnd),
                                         VEC2_TO_CPV(aAnchorB));
    cpSpaceAddConstraint(ret->world->cpSpace, ret->cpConstraint);
    return ret;
}
WorldConstraint_t *worldConstr_createDampedSpringJoint(WorldEntity_t *a, WorldEntity_t *b, vec2_t aAnchorA, vec2_t aAnchorB,
                                            GLMFloat aRestLength, GLMFloat aStiffness, GLMFloat aDamping)
{
    dynamo_assert(a->world == b->world, "Entities are not in the same world");
    WorldConstraint_t *ret = obj_create_autoreleased(&Class_WorldConstraint);
    ret->world = a->world;
    ret->a = obj_retain(a);
    ret->b = obj_retain(b);
    ret->type = kWorldJointType_DampedSpring;
    ret->cpConstraint = cpDampedSpringNew(a->cpBody, b->cpBody, VEC2_TO_CPV(aAnchorA), VEC2_TO_CPV(aAnchorB), aRestLength, aStiffness, aDamping);
    cpSpaceAddConstraint(ret->world->cpSpace, ret->cpConstraint);
    return ret;
}
WorldConstraint_t *worldConstr_createDampedRotarySpringJoint(WorldEntity_t *a, WorldEntity_t *b,
                                                  GLMFloat aRestAngle, GLMFloat aStiffness, GLMFloat aDamping)
{
    dynamo_assert(a->world == b->world, "Entities are not in the same world");
    WorldConstraint_t *ret = obj_create_autoreleased(&Class_WorldConstraint);
    ret->world = a->world;
    ret->a = obj_retain(a);
    ret->b = obj_retain(b);
    ret->type = kWorldJointType_DampedRotarySpring;
    ret->cpConstraint = cpDampedRotarySpringNew(a->cpBody, b->cpBody, aRestAngle, aStiffness, aDamping);
    cpSpaceAddConstraint(ret->world->cpSpace, ret->cpConstraint);
    return ret;
}
WorldConstraint_t *worldConstr_createRotaryLimitJoint(WorldEntity_t *a, WorldEntity_t *b, GLMFloat aMinAngle, GLMFloat aMaxAngle)
{
    dynamo_assert(a->world == b->world, "Entities are not in the same world");
    WorldConstraint_t *ret = obj_create_autoreleased(&Class_WorldConstraint);
    ret->world = a->world;
    ret->a = obj_retain(a);
    ret->b = obj_retain(b);
    ret->type = kWorldJointType_RotaryLimit;
    ret->cpConstraint = cpRotaryLimitJointNew(a->cpBody, b->cpBody, aMinAngle, aMaxAngle);
    cpSpaceAddConstraint(ret->world->cpSpace, ret->cpConstraint);
    return ret;
}
WorldConstraint_t *worldConstr_createRatchetJoint(WorldEntity_t *a, WorldEntity_t *b, GLMFloat aPhase, GLMFloat aRatchet)
{
    dynamo_assert(a->world == b->world, "Entities are not in the same world");
    WorldConstraint_t *ret = obj_create_autoreleased(&Class_WorldConstraint);
    ret->world = a->world;
    ret->a = obj_retain(a);
    ret->b = obj_retain(b);
    ret->type = kWorldJointType_Ratchet;
    ret->cpConstraint = cpRatchetJointNew(a->cpBody, b->cpBody, aPhase, aRatchet);
    cpSpaceAddConstraint(ret->world->cpSpace, ret->cpConstraint);
    return ret;
}
WorldConstraint_t *worldConstr_createGearJoint(WorldEntity_t *a, WorldEntity_t *b, GLMFloat aPhase, GLMFloat aRatio)
{
    dynamo_assert(a->world == b->world, "Entities are not in the same world");
    WorldConstraint_t *ret = obj_create_autoreleased(&Class_WorldConstraint);
    ret->world = a->world;
    ret->a = obj_retain(a);
    ret->b = obj_retain(b);
    ret->type = kWorldJointType_Gear;
    ret->cpConstraint = cpGearJointNew(a->cpBody, b->cpBody, aPhase, aRatio);
    cpSpaceAddConstraint(ret->world->cpSpace, ret->cpConstraint);
    return ret;
}
WorldConstraint_t *worldConstr_createSimpleMotorJoint(WorldEntity_t *a, WorldEntity_t *b, GLMFloat aRate)
{
    dynamo_assert(a->world == b->world, "Entities are not in the same world");
    WorldConstraint_t *ret = obj_create_autoreleased(&Class_WorldConstraint);
    ret->world = a->world;
    ret->a = obj_retain(a);
    ret->b = obj_retain(b);
    ret->type = kWorldJointType_SimpleMotor;
    ret->cpConstraint = cpSimpleMotorNew(a->cpBody, b->cpBody, aRate);
    cpSpaceAddConstraint(ret->world->cpSpace, ret->cpConstraint);
    return ret;
}

void worldConstr_invalidate(WorldConstraint_t *aConstraint)
{
    if(!aConstraint->isValid)
        return;
    aConstraint->isValid = false;
    cpSpaceRemoveConstraint(aConstraint->world->cpSpace, aConstraint->cpConstraint);
}

vec2_t worldConstr_anchorA(WorldConstraint_t *aConstraint)
{
    cpVect ret;
    switch(aConstraint->type) {
        case kWorldJointType_Pin:
            ret = cpPinJointGetAnchr1(aConstraint->cpConstraint);
            break;
        case kWorldJointType_Slide:
            ret = cpSlideJointGetAnchr1(aConstraint->cpConstraint);
            break;
        case kWorldJointType_DampedSpring:
            ret = cpDampedSpringGetAnchr1(aConstraint->cpConstraint);
            break;
        default:
            dynamo_log("Invalid constraint");
            ret = cpv(-1,-1);
    }
    return CPV_TO_VEC2(ret);
}
void worldConstr_setAnchorA(WorldConstraint_t *aConstraint, vec2_t aAnchor)
{
    switch(aConstraint->type) {
        case kWorldJointType_Pin:
            cpPinJointSetAnchr1(aConstraint->cpConstraint, VEC2_TO_CPV(aAnchor));
            break;
        case kWorldJointType_Slide:
            cpSlideJointSetAnchr1(aConstraint->cpConstraint, VEC2_TO_CPV(aAnchor));
            break;
        case kWorldJointType_DampedSpring:
            cpDampedSpringSetAnchr1(aConstraint->cpConstraint, VEC2_TO_CPV(aAnchor));
            break;
        default:
            dynamo_log("Invalid constraint");
    }
}
vec2_t worldConstr_anchorB(WorldConstraint_t *aConstraint)
{
    cpVect ret;
    switch(aConstraint->type) {
        case kWorldJointType_Pin:
            ret = cpPinJointGetAnchr2(aConstraint->cpConstraint);
            break;
        case kWorldJointType_Slide:
            ret = cpSlideJointGetAnchr2(aConstraint->cpConstraint);
            break;
        case kWorldJointType_DampedSpring:
            ret = cpDampedSpringGetAnchr2(aConstraint->cpConstraint);
            break;
        default:
            dynamo_log("Invalid constraint");
            ret = cpv(-1,-1);
    }
    return CPV_TO_VEC2(ret);
}
void worldConstr_setAnchorB(WorldConstraint_t *aConstraint, vec2_t aAnchor)
{
    switch(aConstraint->type) {
        case kWorldJointType_Pin:
            cpPinJointSetAnchr2(aConstraint->cpConstraint, VEC2_TO_CPV(aAnchor));
            break;
        case kWorldJointType_Slide:
            cpSlideJointSetAnchr2(aConstraint->cpConstraint, VEC2_TO_CPV(aAnchor));
            break;
        case kWorldJointType_DampedSpring:
            cpDampedSpringSetAnchr2(aConstraint->cpConstraint, VEC2_TO_CPV(aAnchor));
            break;
        default:
            dynamo_log("Invalid constraint");
    }
}

