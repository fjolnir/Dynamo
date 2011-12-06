#include "collision.h"
#include <float.h>
#include "engine/various.h"
#include "engine/drawutils.h"
#include "engine/GLMath/GLMath.h"


static float _collision_intersectLine(vec2_t aRayOrigin, vec2_t aRayDirection, vec2_t aLineOrigin, vec2_t aLineNormal);
static vec2_t _collision_closestPointOnLineSeg(vec2_t aQueryPoint, vec2_t aPointA, vec2_t aPointB);
static vec2_t _collision_projectPolyShape(CollisionPolyObject_t *aInputObject, vec2_t aLineNormal);
static float _collision_overlapOfProjections(vec2_t aLeft, vec2_t aRight);
static bool _collision_doProjectionsOverlap(vec2_t aLeft, vec2_t aRight);
static bool _collision_objectIsInContact(CollisionPolyObject_t *aObject);
// Draws a wireframe of the collision world to aid with debugging
static void _collision_drawDebugView(Renderer_t *aRenderer, void *aOwner, double aTimeSinceLastFrame, double aInterpolation);

CollisionWorld_t *collision_createWorld(vec2_t aGravity, vec2_t aSize, float aCellSize)
{
	CollisionWorld_t *out = malloc(sizeof(CollisionWorld_t));
	out->debugRenderable.displayCallback = &_collision_drawDebugView;
	out->debugRenderable.owner = out;
	out->gravity = aGravity;
	out->spatialHash = spatialHash_create(aSize, aCellSize);
	out->collisionCallback = NULL;

	return out;
}

void collision_destroyWorld(CollisionWorld_t *aWorld)
{
	spatialHash_destroy(aWorld->spatialHash);
	free(aWorld);
}

CollisionPolyObject_t *collision_createPolyObject(int aNumberOfEdges, vec2_t *aVertices, float aFriction, float aBounce)
{
	CollisionPolyObject_t *out = malloc(sizeof(CollisionPolyObject_t));
	out->numberOfEdges = aNumberOfEdges;
	out->velocity = kVec2_zero;
	out->frictionCoef = aFriction;
	out->bounceCoef = aBounce;
	out->collisionCallback = NULL;
	out->quat = *(quat_t *)&kVec4_zero;
	out->angularVelocity = 0.0f;
	out->vertices = malloc(aNumberOfEdges * sizeof(vec2_t));
	memcpy(out->vertices, aVertices, aNumberOfEdges * sizeof(vec2_t));
	out->normals = malloc(aNumberOfEdges * sizeof(vec2_t));

	int currSide = 0;
	vec2_t pointA, pointB;
	float minX, minY, maxX, maxY; // For creating the bounding box
	float totalX = 0.0f, totalY = 0.0f; // For calculating the center of the polygon
	while(collision_getPolyObjectEdges(out, currSide, &pointA, &pointB, NULL)) {
		if(currSide == 0 || minX > pointA.x) minX = pointA.x;
		if(currSide == 0 || minX > pointB.x) minX = pointB.x;
		if(currSide == 0 || minY > pointA.y) minY = pointA.y;
		if(currSide == 0 || minY > pointB.y) minY = pointB.y;
		if(currSide == 0 || maxX < pointA.x) maxX = pointA.x;
		if(currSide == 0 || maxX < pointB.x) maxX = pointB.x;
		if(currSide == 0 || maxY < pointA.y) maxY = pointA.y;
		if(currSide == 0 || maxY < pointB.y) maxY = pointB.y;

		totalX += pointA.x;
		totalY += pointA.y;

		// Calculate the normal vector
		vec2_t aToB = vec2_sub(pointB, pointA);
		out->normals[currSide] = vec2_normalize(vec2_create(-1.0f*aToB.y, aToB.x));

		++currSide;
	}
	out->boundingBox = rect_create(minX, minY, maxX, maxY);
	out->center = vec2_create(totalX/(float)out->numberOfEdges, totalY/(float)out->numberOfEdges);

	// Make the vertices relative to the center
	for(int i = 0; i < out->numberOfEdges; ++i)
		out->vertices[i] = vec2_sub(out->vertices[i], out->center);

	return out;
}

// Sets the center & updates the bounding box
void collision_setPolyObjectCenter(CollisionPolyObject_t *aObject, vec2_t aCenter)
{
	vec2_t delta = vec2_sub(aCenter, aObject->center);
	aObject->boundingBox = rect_translate(aObject->boundingBox, delta);
	aObject->center = aCenter;
}

#pragma mark - Collision detection

bool collision_getPolyObjectEdges(CollisionPolyObject_t *aObject, int aEdgeIndex,
                                  vec2_t *aoPointA, vec2_t *aoPointB, vec2_t *aoNormal)
{
	// Bail for invalid indices
	if(aEdgeIndex < 0 || aEdgeIndex > (aObject->numberOfEdges - 1))
		return false;

	vec4_t pointA = { aObject->vertices[aEdgeIndex].x, aObject->vertices[aEdgeIndex].y, 0.0f, 1.0f };
	int indexB = aEdgeIndex+1;
	if(indexB == aObject->numberOfEdges) indexB = 0;
	vec4_t pointB = { aObject->vertices[indexB].x, aObject->vertices[indexB].y, 0.0f, 1.0f };
	vec4_t normal = { aObject->normals[aEdgeIndex].x, aObject->normals[aEdgeIndex].y, 0.0f, 1.0f };

	if(aObject->quat.x != 0.0f || aObject->quat.y != 0.0f || aObject->quat.z != 0.0f) {
		pointA = quat_rotatePoint(aObject->quat, pointA);
		pointB = quat_rotatePoint(aObject->quat, pointB);
		normal = quat_rotatePoint(aObject->quat, normal);
	}

	if(aoPointA) *aoPointA = *(vec2_t*)&pointA;
	if(aoPointB) *aoPointB = *(vec2_t*)&pointB;
	if(aoNormal) *aoNormal = *(vec2_t*)&normal;
	//if(aoPointA) *aoPointA = aObject->vertices[aEdgeIndex];
	//int indexB = aEdgeIndex+1;
	//if(indexB == aObject->numberOfEdges) indexB = 0;
	//if(aoPointB) *aoPointB = aObject->vertices[indexB];

	//if(aoNormal) *aoNormal = aObject->normals[aEdgeIndex];

	return true;
}

// Returns the distance along the ray to the intersection point with the given line
static float _collision_intersectLine(vec2_t aRayOrigin, vec2_t aRayDirection, vec2_t aLineOrigin, vec2_t aLineNormal)
{
	float numer = vec2_dot(vec2_sub(aLineOrigin, aRayOrigin), aLineNormal);
	float denom = vec2_dot(aRayDirection, aLineNormal);

	return numer/denom;
}

static vec2_t _collision_closestPointOnLineSeg(vec2_t aQueryPoint, vec2_t aPointA, vec2_t aPointB)
{
	vec2_t lineSegVec = vec2_sub(aPointB, aPointA);
	vec2_t lineDir = vec2_normalize(lineSegVec);
	float lineLen = vec2_mag(lineSegVec);
	vec2_t c = vec2_sub(aQueryPoint, aPointA);
	float t = vec2_dot(lineDir, c);

	if(t <= 0.0f) return aPointA;
	else if(t >= lineLen) return aPointB;

	return vec2_add(aPointA, vec2_scalarMul(lineDir, t));
}

// X component: min, Y component: max
static vec2_t _collision_projectPolyShape(CollisionPolyObject_t *aInputObject, vec2_t aLineNormal)
{
	float min, max;
	int side = 0;
	vec2_t point;
	float p;
	while(collision_getPolyObjectEdges(aInputObject, side, &point, NULL, NULL)) {
		p = vec2_dot(vec2_add(aInputObject->center, point), aLineNormal);
		min = (side == 0) ? p : MIN(min, p);
		max = (side == 0) ? p : MAX(max, p);
		++side;
	}
	vec2_t out = { min, max };
	return out;
}

static float _collision_overlapOfProjections(vec2_t aLeft, vec2_t aRight)
{
	if(aLeft.y > aRight.y)
		return aRight.y - aLeft.x;
	else
		return aLeft.y - aLeft.x;
}
static bool _collision_doProjectionsOverlap(vec2_t aLeft, vec2_t aRight)
{
	float total = MAX(aLeft.y, aLeft.y) - MIN(aLeft.x, aRight.x);
	float leftLength = aLeft.y - aLeft.x;
	float rightLength = aRight.y - aRight.x;
	
	return total <= (leftLength+rightLength);
}

bool collision_intersectObjects(CollisionPolyObject_t *aObjectA, CollisionPolyObject_t *aObjectB,
                                float *aoOverlap, vec2_t *aoOverlapAxis)
{
	float overlap = FLT_MAX;
	vec2_t overlapAxis;

	int side = 0;
	vec2_t normal;
	vec2_t projectionA, projectionB;
	float currOverlap;
	while(collision_getPolyObjectEdges(aObjectB, side++, NULL, NULL, &normal)) {
		projectionA = _collision_projectPolyShape(aObjectA, normal);
		projectionB = _collision_projectPolyShape(aObjectB, normal);
		if(!_collision_doProjectionsOverlap(projectionA, projectionB))
			return false;
		
		currOverlap = _collision_overlapOfProjections(projectionA, projectionB);
		if(currOverlap < overlap) {
			overlap = currOverlap;
			overlapAxis = normal;
		}
	}
	side = 0;
	while(collision_getPolyObjectEdges(aObjectA, side++, NULL, NULL, &normal)) {
		projectionA = _collision_projectPolyShape(aObjectA, normal);
		projectionB = _collision_projectPolyShape(aObjectB, normal);
		if(!_collision_doProjectionsOverlap(projectionA, projectionB))
			return false;

		float currOverlap = _collision_overlapOfProjections(projectionA, projectionB);
		if(currOverlap < overlap) {
			overlap = currOverlap;
			overlapAxis = normal;
		}
	}
	if(aoOverlap) *aoOverlap = overlap;
	if(aoOverlapAxis) *aoOverlapAxis = overlapAxis;

	return true;
}

// Checks the difference in position in the direction of the last collision, and if it's too small we assume the object
// is in contact
static bool _collision_objectIsInContact(CollisionPolyObject_t *aObject)
{
	float current = vec2_mag(vec2_mul(aObject->lastCollision.direction, aObject->center));
	float last = vec2_mag(vec2_mul(aObject->lastCollision.direction, aObject->lastCollision.objectACenter));
	return fabs(current - last) < 2.0; // 2 is just a magic number that seems to work fine
}

bool collision_step(CollisionWorld_t *aWorld, CollisionPolyObject_t *aInputObject, float aTimeDelta)
{
	vec2_t displacement = vec2_scalarMul(aInputObject->velocity, aTimeDelta);
	float distanceToTravel = vec2_mag(displacement);
	float orientationChange = aInputObject->angularVelocity * aTimeDelta;

	//if(distanceToTravel < FLT_EPSILON)
		//return false;

	collision_setPolyObjectCenter(aInputObject, vec2_add(aInputObject->center, displacement));
	aInputObject->orientation += orientationChange;

	if(aInputObject->orientation > 2.0f*M_PI)
		aInputObject->orientation -= 2.0f*M_PI;
	else if(aInputObject->orientation < -2.0f*M_PI)
		aInputObject->orientation += 2.0f*M_PI;
	aInputObject->quat = quat_makef(0.0f, 0.0f, 1.0f, aInputObject->orientation);
	rect_t newBoundingBox = rect_translate(aInputObject->boundingBox, displacement);
	CollisionPolyObject_t **potentialColliders;
	int numberOfPotentialColliders;
	potentialColliders = (CollisionPolyObject_t **)spatialHash_query(aWorld->spatialHash, newBoundingBox, &numberOfPotentialColliders);
	if(numberOfPotentialColliders == 0) {
		aInputObject->inContact = _collision_objectIsInContact(aInputObject);
		return false;
	}

	// A single collider can be present in multiple cells so we must keep track of which ones we have tested already
	Array_t *testedColliders = array_create(numberOfPotentialColliders);

	CollisionPolyObject_t *nearestCollider = NULL;
	float overlap = 0.0f;
	vec2_t overlapAxis;

	// Process each of the possibly colliding polygons
	bool didCollide = false;
	float currOverlap;
	vec2_t currOverlapAxis;
	CollisionPolyObject_t *currentCollider;
	for(int i = 0; i < numberOfPotentialColliders; ++i) {
		currentCollider = potentialColliders[i];
		// Check if we've already tested against this object
		if(array_containsPtr(testedColliders, currentCollider) || currentCollider == aInputObject)
			continue;
		array_push(testedColliders, currentCollider);
		
		if(collision_intersectObjects(aInputObject, currentCollider, &currOverlap, &currOverlapAxis)) {
			if(currOverlap > overlap) {
				didCollide = true;
				nearestCollider = currentCollider;
				overlap = currOverlap;
				overlapAxis = currOverlapAxis;
			}
		}
	}
	free(potentialColliders);
	array_destroy(testedColliders);
	if(!didCollide) {
		aInputObject->inContact = _collision_objectIsInContact(aInputObject);
		return false;
	}

	// Separate the objects
	collision_setPolyObjectCenter(aInputObject, vec2_add(aInputObject->center, vec2_scalarMul(overlapAxis, overlap)));

	// Apply some physics magic
	float projection = vec2_dot(overlapAxis, displacement);

	float frictionCoef = 1.0f - nearestCollider->frictionCoef;
	float bounceCoef = nearestCollider->bounceCoef;

	// Only apply collision response when heading into the collision (not out)
	if(projection < 0.0f) {
		vec2_t normalVelocity = vec2_scalarMul(overlapAxis, projection);
		vec2_t tangentVelocity = vec2_sub(displacement, normalVelocity);

		vec2_t newVel = vec2_add(vec2_scalarMul(normalVelocity, -bounceCoef), vec2_scalarMul(tangentVelocity, frictionCoef));
		// Convert back to to px/s
		aInputObject->velocity = vec2_scalarDiv(newVel, aTimeDelta);
		
		// Determine the contact edge on the input object by casting a ray from its center
		// in the direction opposite to the overlap on to each of it's edges
		vec2_t pointA, pointB, normal;
		vec2_t rayDir = vec2_negate(overlapAxis);
		float t, smallestT = FLT_MAX;
		vec2_t collisionEdge, collisionEdgeNormal;
		int side = 0;
		while(collision_getPolyObjectEdges(aInputObject, side++, &pointA, &pointB, &normal)) {
			t = _collision_intersectLine(kVec2_zero, rayDir, pointA, normal);
			if(t >= 0.0f && t < smallestT) {
				smallestT = t;
				collisionEdge = vec2_sub(pointB, pointA);
				collisionEdgeNormal = vec2_negate(normal);
			}
		}
		// Rotate the object so that the contact edge matches the collision angle
		float edgeAngle = atan2(collisionEdgeNormal.y, collisionEdgeNormal.x);
		float overlapAngle = atan2(overlapAxis.y, overlapAxis.x);
		aInputObject->angularVelocity = MAX(1.0f, overlap)*4.0f*(overlapAngle- edgeAngle);
	}

	// Let interested parties know a collision occurred
	aInputObject->inContact = true;
	Collision_t collisionInfo = {
		aInputObject,    aInputObject->velocity,    aInputObject->center,
		nearestCollider, nearestCollider->velocity, nearestCollider->center,
		overlapAxis, overlap
	};
	if(aInputObject->collisionCallback)
		aInputObject->collisionCallback(aWorld, collisionInfo);
	aInputObject->lastCollision = collisionInfo;

	if(aWorld->collisionCallback)
		aWorld->collisionCallback(aWorld, collisionInfo);

	collisionInfo.objectA = nearestCollider;
	collisionInfo.objectB = aInputObject;
	if(nearestCollider->collisionCallback) 
		nearestCollider->collisionCallback(aWorld, collisionInfo);
	nearestCollider->lastCollision = collisionInfo;

	return true;
}


#pragma mark - Debug drawing
static void _collision_drawDebugView(Renderer_t *aRenderer, void *aOwner, double aTimeSinceLastFrame, double aInterpolation)
{
	CollisionWorld_t *world = (CollisionWorld_t *)aOwner;
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw the cells
	
	vec4_t whiteColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	vec4_t redColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	vec4_t blueColor = { 0.0f, 0.0f, 1.0f, 1.0f };
	vec4_t greenColor = { 0.0f, 1.0f, 0.0f, 1.0f };
	vec4_t yellowColor = { 1.0f, 0.8f, 0.0f, 1.0f };

	rect_t cellRect = { 0.0f, 0.0f, world->spatialHash->cellSize, world->spatialHash->cellSize };
	SpatialHash_cell_t *currCell;
	CollisionPolyObject_t *currPoly;

	// Draw the grid
	for(int y = 0; y < (int)world->spatialHash->sizeInCells.h; ++y) {
		cellRect.origin.y = y*world->spatialHash->cellSize;
		for(int x = 0; x < (int)world->spatialHash->sizeInCells.w; ++x) {
			cellRect.origin.x = x*world->spatialHash->cellSize;
			draw_rect(cellRect, 0.0f, whiteColor, false);
		}
	}
	for(int y = 0; y < (int)world->spatialHash->sizeInCells.h; ++y) {
		cellRect.origin.y = y*world->spatialHash->cellSize;
		for(int x = 0; x < (int)world->spatialHash->sizeInCells.w; ++x) {
			cellRect.origin.x = x*world->spatialHash->cellSize;
			currCell = world->spatialHash->cells[y*(int)world->spatialHash->sizeInCells.w + x];
			if(currCell == NULL) continue;
			// Redraw populated cells in blue
			if(currCell->objects->count > 0)
				draw_rect(cellRect, 0.0f, blueColor, false);

			// Draw the polyon
			for(int i = 0; i < currCell->objects->count; ++i) {
				currPoly = currCell->objects->items[i];
				draw_circle(currPoly->center, 1.0f, 3.0, greenColor, true);

				matrix_stack_push(aRenderer->worldMatrixStack);
				matrix_stack_translate(aRenderer->worldMatrixStack, currPoly->center.x, currPoly->center.y, 0.0f);
				draw_polygon(currPoly->numberOfEdges, currPoly->vertices, redColor, false);
				// Draw the polygon's edge normals
				int currEdge = 0;
				vec2_t normal;
				vec2_t pointA, pointB;
				while(collision_getPolyObjectEdges(currPoly, currEdge++, &pointA, &pointB, &normal)) {
					draw_circle(pointA, 1.0f, 3.0, greenColor, true);
					// Find the edge midpoint
					vec2_t lineSeg = vec2_scalarDiv(vec2_sub(pointB, pointA), 2.0f);
					lineSeg = vec2_add(pointA, lineSeg);
					normal = vec2_add(lineSeg, vec2_scalarMul(vec2_normalize(normal), 10.0f));
					draw_lineSeg(lineSeg, normal, greenColor);
				}
				matrix_stack_pop(aRenderer->worldMatrixStack);
			}
		}
	}

	// Draw the character
	if(world->character) {
		//draw_rect(world->character->boundingBox, 0.0f, blueColor, false);
		draw_circle(world->character->center, 1.0f, 3.0, greenColor, true);			
		matrix_stack_push(aRenderer->worldMatrixStack);
		matrix_stack_translate(aRenderer->worldMatrixStack, world->character->center.x, world->character->center.y, 0.0f);
		matrix_stack_push_item(aRenderer->worldMatrixStack, mat4_mul(matrix_stack_get_mat4(aRenderer->worldMatrixStack), quat_to_ortho(world->character->quat)));
			draw_polygon(world->character->numberOfEdges, world->character->vertices, world->character->inContact ? yellowColor : redColor, false);
		matrix_stack_pop(aRenderer->worldMatrixStack);
		// Draw the polygon's edge normals
		int currEdge = 0;
		vec2_t normal;
		vec2_t pointA, pointB;
		while(collision_getPolyObjectEdges(world->character, currEdge++, &pointA, &pointB, &normal)) {
			draw_circle(pointA, 1.0f, 3.0, greenColor, true);
			// Find the edge midpoint
			vec2_t lineSeg = vec2_scalarDiv(vec2_sub(pointB, pointA), 2.0f);
			lineSeg = vec2_add(pointA, lineSeg);
			normal = vec2_add(lineSeg, vec2_scalarMul(normal, 10.0f));
			draw_lineSeg(lineSeg, normal, currEdge == 2 ? whiteColor : greenColor);
		}
		// Draw the velocity
		draw_lineSeg(kVec2_zero, vec2_scalarDiv(world->character->velocity, 20.0f), greenColor);
		matrix_stack_pop(aRenderer->worldMatrixStack);
	}
}
