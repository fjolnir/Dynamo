// Note: Assumes cpFloat is the same type as GLMFloat
#include "world.h"

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
    
    out->staticEntity = obj_create(&Class_WorldEntity);
    out->staticEntity->world = out;
    out->staticEntity->owner = out;
    out->staticEntity->cpBody = out->cpSpace->staticBody;
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
    cpSpaceFree(aWorld->cpSpace), aWorld->cpSpace = NULL;
    obj_release(aWorld->entities);
    obj_release(aWorld->staticEntity);
}

void world_addEntity(World_t *aWorld, WorldEntity_t *aEntity)
{
    assert(aEntity != aWorld->staticEntity);
    cpSpaceAddBody(aWorld->cpSpace, aEntity->cpBody);
    llist_apply(aEntity->shapes, (LinkedListApplier_t)&_addShapeToSpace, aWorld);
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


#pragma mark - World entities

WorldEntity_t *worldEnt_create(World_t *aWorld, Obj_t *aOwner, GLMFloat aMass, GLMFloat aMomentum)
{
    WorldEntity_t *out = obj_create_autoreleased(&Class_WorldEntity);
    out->world = aWorld;
    out->owner = aOwner;
    out->cpBody = cpBodyNew(aMass, aMomentum);
    out->cpBody->data = out;

    out->shapes = obj_retain(llist_create((InsertionCallback_t)&obj_retain, &obj_release));
    
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

void worldEnt_addShape(WorldEntity_t *aEntity, WorldShape_t *aShape)
{
    cpShapeSetBody(aShape->cpShape, aEntity->cpBody);
    if(aEntity == aEntity->world->staticEntity) {
        cpSpaceAddShape(aEntity->world->cpSpace, aShape->cpShape);
    }
    llist_pushValue(aEntity->shapes, aShape);
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

static void worldShape_destroy(WorldShape_t *aEntity)
{
    cpShapeFree(aEntity->cpShape);
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
    WorldEntity_t *a = bodyA->data; assert(a != NULL);
    WorldEntity_t *b = bodyB->data; assert(b != NULL);
    
    cpContactPointSet cpPointSet = cpArbiterGetContactPointSet(aArbiter);
    World_ContactPointSet pointSet = *(World_ContactPointSet *)&cpPointSet;
    return (World_CollisionInfo){
        a, b,
        cpArbiterIsFirstContact(aArbiter),
        pointSet,
        aArbiter
    };
}
static int collisionWillBegin(cpArbiter *aArbiter, struct cpSpace *aSpace, void *aData)
{
    World_CollisionInfo collInfo = _collisionInfoForArbiter(aArbiter);
    World_t *world = aSpace->data;
    if(collInfo.a->preCollisionHandler)
        collInfo.a->preCollisionHandler(collInfo.a, world, collInfo);
    if(collInfo.b->preCollisionHandler)
        collInfo.b->preCollisionHandler(collInfo.b, world, collInfo);
    return true;
}
static void collisionDidBegin(cpArbiter *aArbiter, struct cpSpace *aSpace, void *aData)
{
    World_CollisionInfo collInfo = _collisionInfoForArbiter(aArbiter);
    World_t *world = aSpace->data;
    if(collInfo.a->collisionHandler)
        collInfo.a->collisionHandler(collInfo.a, world, collInfo);
    if(collInfo.b->collisionHandler)
        collInfo.b->collisionHandler(collInfo.b, world, collInfo);
}
static void collisionDidEnd(cpArbiter *aArbiter, struct cpSpace *aSpace, void *aData)
{
    World_CollisionInfo collInfo = _collisionInfoForArbiter(aArbiter);
    World_t *world = aSpace->data;
    if(collInfo.a->postCollisionHandler)
        collInfo.a->postCollisionHandler(collInfo.a, world, collInfo);
    if(collInfo.b->postCollisionHandler)
        collInfo.b->postCollisionHandler(collInfo.b, world, collInfo);
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
    llist_apply(aEntity->shapes, (LinkedListApplier_t)_removeShapeFromSpace, aWorld);
    cpSpaceRemoveBody(aWorld->cpSpace, aEntity->cpBody);
}

static void _callEntityUpdateCallback(WorldEntity_t *aEntity, World_t *aWorld)
{
    if(aEntity->updateHandler)
        aEntity->updateHandler(aEntity, aWorld);
}