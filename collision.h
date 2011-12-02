// A very simple collision detector using separation axes to determine if two objects are volliding

// Velocity is expressed in pixels per second

#include <stdbool.h>
#include "engine/GLMath/GLMath.h"
#include "spatial_hash.h"
#include "engine/renderer.h"

#ifndef _COLLISION_H_
#define _COLLISION_H_

typedef struct _CollisionPolyObject CollisionPolyObject_t;
struct _CollisionPolyObject {
	vec2_t center;
	int numberOfEdges;
	vec2_t *vertices; // Vertices relative to the center
	vec2_t *normals;
	rect_t boundingBox;
	float frictionCoef; // Friction coefficient: 0-1
	float bounceCoef; // Bounciness: 0-1
	vec2_t velocity;	
};

typedef struct _CollisionWorld {
	Renderable_t debugRenderable;
	vec2_t gravity;
	SpatialHash_t *spatialHash;
	CollisionPolyObject_t *character;
} CollisionWorld_t;


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
