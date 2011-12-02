// Spatial hash
//
// Used to accelerate collision detection
// Objects are split into cells(buckets) by their positions&bounding rectangles.
// The case where an object is too large, and appears in more than one cell is handled by calculating the hash for
// each corner and then adding the object to each of the appropriate cells. (This breaks down for large objects, but
// you should adjust cell size to fit your needs)

// The cell index of an object is calculated with p.x/cellSize + (p.y/cellSize) * (sceneWidth/cellSize))

#include "engine/GLMath/GLMath.h"
#include "array.h"

#ifndef _SPATIALHASH_H
#define _SPATIALHASH_H

typedef struct _SpatialHash_cell {
	Array_t *objects;
} SpatialHash_cell_t;

typedef struct _SpatialHash {
	vec2_t size; // Make sure to set to a multiple of the cell size
	vec2_t sizeInCells;
	float cellSize; // Cells are square
	int numberOfCells;
	SpatialHash_cell_t **cells;
} SpatialHash_t;

extern SpatialHash_t *spatialHash_create(vec2_t aSize, float aCellSize);
extern void spatialHash_destroy(SpatialHash_t *aHash);
extern SpatialHash_cell_t *spatialHash_createCell();
extern void spatialHash_destroyCell(SpatialHash_cell_t *aCell);

// Clears the hash making it ready for reuse
extern void spatialHash_clear(SpatialHash_t *aHash);
extern bool spatialHash_addItem(SpatialHash_t *aHash, void *aItem, rect_t aBoundingBox);
extern int spatialHash_getCellForPoint(SpatialHash_t *aHash, vec2_t aPoint, bool aShouldCreate);
// Returns an array of items in the cells matching aBoundingBox
extern void **spatialHash_query(SpatialHash_t *aHash, rect_t aBoundingBox, int *aoCount);

#endif
