// A very simple collision detector using separation axes to determine if two objects are volliding

// Velocity is expressed in pixels per second

#include <stdbool.h>
#include "engine/GLMath/GLMath.h"
#include "spatial_hash.h"
#include "engine/renderer.h"

#ifndef _COLLISION_H_
#define _COLLISION_H_

typedef struct _Collision Collision_t;
typedef struct _CollisionPolyObject CollisionPolyObject_t;
typedef struct _CollisionWorld CollisionWorld_t;

typedef void (*CollisionCallback_t)(CollisionWorld_t *aWorld, Collision_t collisionInfo);

struct _Collision {
	CollisionPolyObject_t *objectA;
	vec2_t objectAVelocity; // Velocity of object A at the time of collision
	vec2_t objectACenter; // Center of object A at the time of collision
	CollisionPolyObject_t *objectB;
	vec2_t objectBCenter; // Center of object B at the time of collision
	vec2_t objectBVelocity; // Velocity of object B at the time of collision
	vec2_t direction; // Unit vector in the direction of the collision
	float magnitude; // Scalar indicating the strength of the collision
};

struct _CollisionPolyObject {
	vec2_t center;
	int numberOfEdges;
	vec2_t *vertices; // Vertices relative to the center
	vec2_t *normals;
	rect_t boundingBox;
	float frictionCoef; // Friction coefficient: 0-1
	float bounceCoef; // Bounciness: 0-1
	vec2_t velocity;

	CollisionCallback_t collisionCallback; // Called for every collision this object is in. This object will always be objectA
	Collision_t lastCollision;
	bool inContact; // True if the object was in contact with any other object during the last cycle

	void *info; // An arbitrary pointer you can use to identify objects
};

struct _CollisionWorld {
	Renderable_t debugRenderable;
	vec2_t gravity;
	SpatialHash_t *spatialHash;
	CollisionPolyObject_t *character;

	CollisionCallback_t collisionCallback; // Called for every collision in the world
};


extern CollisionWorld_t *collision_createWorld(vec2_t aGravity, vec2_t aSize, float aCellSize);
extern void collision_destroyWorld(CollisionWorld_t *aWorld);

// Create a poly objects from a set of vertices with a clockwise winding
extern CollisionPolyObject_t *collision_createPolyObject(int aNumberOfEdges, vec2_t *aVertices, float aFriction, float aBounce);
extern void collision_setPolyObjectCenter(CollisionPolyObject_t *aObject, vec2_t aCenter);
// Returns true & sets aPointA&aPointB for edge indices in the valid range
extern bool collision_getPolyObjectEdges(CollisionPolyObject_t *aObject, int aEdgeIndex,
                                  vec2_t *aoPointA, vec2_t *aoPointB, vec2_t *aoNormal);

// Tests the input shape against all the objects in the passed world and performs collisions where appropriate
extern bool collision_step(CollisionWorld_t *aWorld, CollisionPolyObject_t *aInputObject, float aTimeDelta);

#endif
