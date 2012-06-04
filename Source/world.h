/*!
	@header Physics World
	@abstract
	@discussion A wrapper around chipmunk physics.
*/

#ifndef __WORLD_H_
#define __WORLD_H_

#include "renderer.h"
#include "linkedlist.h"
#include "gametimer.h"
#include <chipmunk/chipmunk.h>

typedef struct _World World_t;
typedef struct _WorldShape WorldShape_t;
typedef struct _WorldEntity WorldEntity_t;

/*!
	A set of contact points of a collision
*/
typedef struct _World_ContactPointSet {
	int count;
	struct {
		vec2_t point;
		vec2_t normal;
		GLMFloat depth;
	} points[CP_MAX_CONTACTS_PER_ARBITER];
} World_ContactPointSet;

/*!
	Information about a collision.

	@field a Entity A
	@field b Entity B
	@field firstContact Indicates whether or not this is the initial contact of the two objects
	@field contactPoints The set of points at which the object touch
*/
typedef struct _World_CollisionInfo {
    WorldEntity_t *a;
    WorldEntity_t *b;
    bool firstContact;
    World_ContactPointSet contactPoints;
    cpArbiter *cpArbiter;
} World_CollisionInfo;

/*!
	A collision handler
*/
typedef void (*WorldEntity_CollisionHandler)(WorldEntity_t *aEntity, World_CollisionInfo *aCollisionInfo);
/*!
	An entity's update handler
*/
typedef void (*WorldEntity_UpdateHandler)(WorldEntity_t *aEntity);

/*!
	A Game entity is an object that can be rendered and/or included in the physics simulation

	@field world The world which contains the entity
	@field owner An arbitrary pointer to the "owner" of the entity (Useful to attach the entity to for example a sprite)
	@field updateHandler A function that is called every time the entity's state changes
	@field preCollisionHandler A function that is called right before the entity collides
	@field collisionHandler A function that is called during a collision of the entity
	@field postCollisionHandler A function that is called when the entity separates from whatever entity it was in collision with
*/
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
extern Class_t Class_WorldEntity;

/*!
	A collision shape, attached to one and only one(!) entity
*/
struct _WorldShape {
    OBJ_GUTS
    cpShape *cpShape;
};

/*!
	A shape group.

	Shapes in the same group do not collide.
*/
typedef cpGroup WorldShapeGroup_t;

/*!
	A game world

	Can contain multiple entites

	@field staticEntity The static entity of the world. Shapes attached to this entity are static.
*/
struct _World {
    OBJ_GUTS
    cpSpace *cpSpace;
    LinkedList_t *entities;
    WorldEntity_t *staticEntity;
};

/*!
	Joint types
*/
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

/*!
	A constraint

	Used to connect entities together

	@field world The world which contains the constraint
	@field a Entity A
	@field b Entity B
	@field type The joint type
	@field isValid This is set to false when the joint is removed from it's world.
*/
typedef struct _WorldConstraint {
    World_t *world; // Weak
    WorldEntity_t *a, *b;
    WorldJointType_t type;
    cpConstraint *cpConstraint;
    bool isValid;
} WorldConstraint_t;
extern Class_t Class_WorldConstraint;

/*!
	@functiongroup World
*/

/*!
	Creates a world.
*/
extern World_t *world_create(void);
/*!
	Steps the world state .
*/
extern void world_step(World_t *aWorld, GameTimer_t *aTimer);
/*!
	Sets the gravity in a world.
*/
extern void world_setGravity(World_t *aWorld, vec2_t aGravity);
/*!
	Gets the gravity in a world.
*/
extern vec2_t world_gravity(World_t *aWorld);
/*!
	Adds an entity to the given world.
*/
extern void world_addEntity(World_t *aWorld, WorldEntity_t *aEntity);
/*!
	Removes an entity from the given world.
*/
extern void world_removeEntity(World_t *aWorld, WorldEntity_t *aEntity);
/*!
	Returns the entity at the given point in the given world (if any)
*/
extern WorldEntity_t *world_pointQuery(World_t *aWorld, vec2_t aPoint);

extern void world_addJoint(World_t *aWorld, WorldConstraint_t *aJoint);
extern void world_removeJoint(World_t *aWorld, WorldConstraint_t *aJoint);


/*!
	@functiongroup Entities
*/

/*!
	Creates a world entity.
*/
extern WorldEntity_t *worldEnt_create(World_t *aWorld, Obj_t *aOwner, GLMFloat aMass, GLMFloat amoment);
/*!
	Gets the location of an entity.
*/
extern vec2_t worldEnt_location(WorldEntity_t *aEntity);
/*!
	Sets the location of an entity.
*/
extern void worldEnt_setLocation(WorldEntity_t *aEntity, vec2_t aLocation);
/*!
	Gets the angle of an entity.
*/
extern GLMFloat worldEnt_angle(WorldEntity_t *aEntity);
/*!
	Sets the angle of an entity.
*/
extern void worldEnt_setAngle(WorldEntity_t *aEntity, GLMFloat aAngle);
/*!
	Applies a force to an entity.
*/
extern void worldEnt_applyForce(WorldEntity_t *aEntity, vec2_t aForce, vec2_t aOffset);
/*!
	Applies an impulse to an entity.
*/
extern void worldEnt_applyImpulse(WorldEntity_t *aEntity, vec2_t aImpulse, vec2_t aOffset);
/*!
	Gets the velocity of an entity
*/
extern vec2_t worldEnt_velocity(WorldEntity_t *aEntity);
/*!
	Sets the velocity of an entity.
*/
extern void worldEnt_setVelocity(WorldEntity_t *aEntity, vec2_t aVelocity);
/*!
	Adds a shape to an en entity
*/
extern WorldShape_t *worldEnt_addShape(WorldEntity_t *aEntity, WorldShape_t *aShape);

/*!
	@functiongroup Shapes
*/

/*!
	Creates a circle shape.
*/
extern WorldShape_t *worldShape_createCircle(vec2_t aCenter, GLMFloat aRadius);
/*!
	Creates a line segment shape.
*/
extern WorldShape_t *worldShape_createSegment(vec2_t a, vec2_t b, GLMFloat aThickness);
/*!
	Creates a box shape.
*/
extern WorldShape_t *worldShape_createBox(vec2_t aSize);
/*!
	Creates a convex polygon shape.

	@param aVerts An array of counter clockwise wound vertices
*/
extern WorldShape_t *worldShape_createPoly(unsigned aVertCount, vec2_t *aVerts);
/*!
	Gets the friction of a shape.
*/
extern GLMFloat worldShape_friction(WorldShape_t *aEntity);
/*!
	Sets the friction of a shape.
*/
extern void worldShape_setFriction(WorldShape_t *aEntity, GLMFloat aVal);
/*!
	Gets the elasticty of a shape.
*/
extern GLMFloat worldShape_elasticity(WorldShape_t *aEntity);
/*!
	Sets the elasticty of as shape.
*/
extern void worldShape_setElasticity(WorldShape_t *aEntity, GLMFloat aVal);
/*!
	Gets the group of a shape.
*/
extern WorldShapeGroup_t worldShape_group(WorldShape_t *aShape);
/*!
	Sets the group of a shape.
*/
extern void worldShape_setGroup(WorldShape_t *aShape, WorldShapeGroup_t aGroup);

/*!
	Sets whether or not a shape generates collisions (even if not, it still calls it's collision callbacks)
*/
extern void worldShape_setCollides(WorldShape_t *aShape, bool aCollides);

/*!
	Gets whether or not a shape generates collisions (even if not, it still calls it's collision callbacks)
*/
extern bool worldShape_collides(WorldShape_t *aShape);

/*!
	Calculates the correct moment of inertia for a circle shape.
*/
extern GLMFloat world_momentForCircle(GLMFloat aMass, GLMFloat aInnerRadius, GLMFloat aOuterRadius, vec2_t aOffset);
/*!
	Calculates the correct moment of inertia for a line segment shape.
*/
extern GLMFloat world_momentForSegment(GLMFloat aMass, vec2_t a, vec2_t b);
/*!
	Calculates the correct moment of inertia for a polygon shape.
*/
extern GLMFloat world_momentForPoly(GLMFloat aMass, unsigned aVertCount, vec2_t *aVerts, vec2_t aOffset);
/*!
	Calculates the correct moment of inertia for a box shape.
*/

extern GLMFloat world_momentForBox(GLMFloat aMass, vec2_t aSize);

/*!
	@functiongroup Joints
*/

/*!
	Creates a pin joint
*/
extern WorldConstraint_t *worldConstr_createPinJoint(WorldEntity_t *a, WorldEntity_t *b, vec2_t aAnchorA, vec2_t aAnchorB);
/*!
	Creates a slide joint
*/
extern WorldConstraint_t *worldConstr_createSlideJoint(WorldEntity_t *a, WorldEntity_t *b, vec2_t aAnchorA, vec2_t aAnchorB, GLMFloat minDist, GLMFloat maxDist);
/*!
	Creates a pivot joint.
*/
extern WorldConstraint_t *worldConstr_createPivotJoint(WorldEntity_t *a, WorldEntity_t *b, vec2_t aPivot);
/*!
	Creates a groove joint.
*/
extern WorldConstraint_t *worldConstr_createGrooveJoint(WorldEntity_t *a, WorldEntity_t *b, vec2_t aGrooveStart, vec2_t aGrooveEnd, vec2_t aAnchorB);
/*!
	Creates a damped spring joint.
*/
extern WorldConstraint_t *worldConstr_createDampedSpringJoint(WorldEntity_t *a, WorldEntity_t *b, vec2_t aAnchorA, vec2_t aAnchorB, GLMFloat aRestLength, GLMFloat aStiffness, GLMFloat aDamping);
/*!
	Creates a damped rotary spring constraint.
*/
extern WorldConstraint_t *worldConstr_createDampedRotarySpringJoint(WorldEntity_t *a, WorldEntity_t *b, GLMFloat restAngle, GLMFloat aStiffness, GLMFloat aDamping);
/*!
	Creates a rotary limit constraint.
*/
extern WorldConstraint_t *worldConstr_createRotaryLimitJoint(WorldEntity_t *a, WorldEntity_t *b, GLMFloat aMinAngle, GLMFloat aMaxAngle);
/*!
	Creates a ratchet joint.
*/
extern WorldConstraint_t *worldConstr_createRatchetJoint(WorldEntity_t *a, WorldEntity_t *b, GLMFloat aPhase, GLMFloat aRatchet);
/*!
	Creates a gear constraint.
*/
extern WorldConstraint_t *worldConstr_createGearJoint(WorldEntity_t *a, WorldEntity_t *b, GLMFloat aPhase, GLMFloat aRatio);
/*!
	Creates a simple motor constraint.
*/
extern WorldConstraint_t *worldConstr_createSimpleMotorJoint(WorldEntity_t *a, WorldEntity_t *b, GLMFloat aRate);
/*!
	Deactivates a joint. (cannot be reactivated again)
*/
extern void worldConstr_invalidate(WorldConstraint_t *aConstraint);
#endif
