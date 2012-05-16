// A wrapper around chipmunk

#ifndef __WORLD_H_
#define __WORLD_H_

#include "renderer.h"
#include "linkedlist.h"
#include "gametimer.h"
#include <chipmunk/chipmunk.h>

typedef struct _World World_t;
typedef struct _WorldShape WorldShape_t;
typedef struct _WorldEntity WorldEntity_t;

typedef struct _World_ContactPointSet {
	int count;
	struct {
		vec2_t point;
		vec2_t normal;
		GLMFloat depth;
	} points[CP_MAX_CONTACTS_PER_ARBITER];
} World_ContactPointSet;

typedef struct _World_CollisionInfo {
    WorldEntity_t *a;
    WorldEntity_t *b;
    bool firstContact;
    World_ContactPointSet contactPoints;
    cpArbiter *cpArbiter;
} World_CollisionInfo;

typedef void (*WorldEntity_CollisionHandler)(WorldEntity_t *aEntity, World_CollisionInfo *aCollisionInfo);
typedef void (*WorldEntity_UpdateHandler)(WorldEntity_t *aEntity);

// A Game entity is an object that can be rendered and/or included in the physics simulation
extern Class_t Class_WorldEntity;
struct _WorldEntity {
    OBJ_GUTS
    World_t *world; // Weak reference
    Obj_t *owner; // Weak reference (Owner should retain the entity)
    cpBody *cpBody;
    LinkedList_t *shapes;
    WorldEntity_UpdateHandler updateHandler;
    WorldEntity_CollisionHandler preCollisionHandler;
    WorldEntity_CollisionHandler collisionHandler;    
    WorldEntity_CollisionHandler postCollisionHandler;
};

// A collision shape, attached to one and only one(!) entity
struct _WorldShape {
    OBJ_GUTS
    cpShape *cpShape;
};

struct _World {
    OBJ_GUTS
    cpSpace *cpSpace;
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
    cpConstraint *cpConstraint;
} WorldConstraint_t;

extern World_t *world_create(void);
extern void world_step(World_t *aWorld, GameTimer_t *aTimer);
extern void world_setGravity(World_t *aWorld, vec2_t aGravity);
extern vec2_t world_gravity(World_t *aWorld);
extern void world_addEntity(World_t *aWorld, WorldEntity_t *aEntity);
extern void world_removeEntity(World_t *aWorld, WorldEntity_t *aEntity);
extern WorldEntity_t *world_pointQuery(World_t *aWorld, vec2_t aPoint);
extern void world_addJoint(World_t *aWorld, WorldConstraint_t *aJoint);
extern void world_removeJoint(World_t *aWorld, WorldConstraint_t *aJoint);

extern WorldEntity_t *worldEnt_create(World_t *aWorld, Obj_t *aOwner, GLMFloat aMass, GLMFloat aMomentum);
extern vec2_t worldEnt_location(WorldEntity_t *aEntity);
extern void worldEnt_setLocation(WorldEntity_t *aEntity, vec2_t aLocation);
extern GLMFloat worldEnt_angle(WorldEntity_t *aEntity);
extern void worldEnt_setAngle(WorldEntity_t *aEntity, GLMFloat aAngle);
extern void worldEnt_applyForce(WorldEntity_t *aEntity, vec2_t aForce, vec2_t aOffset);
extern void worldEnt_applyImpulse(WorldEntity_t *aEntity, vec2_t aImpulse, vec2_t aOffset);
extern vec2_t worldEnt_velocity(WorldEntity_t *aEntity);
extern void worldEnt_setVelocity(WorldEntity_t *aEntity, vec2_t aVelocity);
extern void worldEnt_addShape(WorldEntity_t *aEntity, WorldShape_t *aShape);


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
#endif
