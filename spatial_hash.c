#include "spatial_hash.h"

SpatialHash_t *spatialHash_create(vec2_t aSize, float aCellSize)
{
	SpatialHash_t *out = malloc(sizeof(SpatialHash_t));
	out->size = aSize;
	out->cellSize = aCellSize;
	out->numberOfCells = (aSize.w/aCellSize)*(aSize.h/aCellSize);
	out->cells = malloc(sizeof(SpatialHash_cell_t)*out->numberOfCells);

	return out;
}

void spatialHash_destroy(SpatialHash_t *aHash)
{
	for(int i = 0; i < aHash->numberOfCells; ++i)
		free(aHash->cells[i].objects);
	free(aHash->cells);
	free(aHash);
}

// Clears the hash making it ready for reuse
void spatialHash_clear(SpatialHash_t *aHash)
{
	for(int i = 0; i < aHash->numberOfCells; ++i)
		aHash->cells[i].objects->count = 0;
}

Array_t *_spatialHash_cellsForBoundingBox(SpatialHash_t *aHash, rect_t aBoundingBox)
{
	Array_t *cells = array_create(4);

	int cellIndex;
	vec2_t point;

	point = aBoundingBox.origin;
	cellIndex = spatialHash_getCellForPoint(aHash, point);
	if(cellIndex != -1)
		array_push(cells, &aHash->cells[cellIndex]);

	point = vec2_add(aBoundingBox.origin, vec2_create(aBoundingBox.size.h, 0.0));
	cellIndex = spatialHash_getCellForPoint(aHash, point);
	if(cellIndex != -1 && !array_containsPtr(cells, &aHash->cells[cellIndex]))
		array_push(cells, &aHash->cells[cellIndex]);

	point = vec2_add(aBoundingBox.origin, aBoundingBox.size);
	cellIndex = spatialHash_getCellForPoint(aHash, point);
	if(cellIndex != -1 && !array_containsPtr(cells, &aHash->cells[cellIndex]))
		array_push(cells, &aHash->cells[cellIndex]);

	point = vec2_add(aBoundingBox.origin, vec2_create(0.0, aBoundingBox.size.w));
	cellIndex = spatialHash_getCellForPoint(aHash, point);
	if(cellIndex != -1 && !array_containsPtr(cells, &aHash->cells[cellIndex]))
		array_push(cells, &aHash->cells[cellIndex]);
	
	return cells;
}

bool spatialHash_addItem(SpatialHash_t *aHash, void *aItem, rect_t aBoundingBox)
{
	Array_t *cells = _spatialHash_cellsForBoundingBox(aHash, aBoundingBox);
	bool didAdd = (cells->count > 0);
	for(int i = 0; i < cells->count; ++i) {
		array_push(aHash->cells[i].objects, aItem);
	}
	array_destroy(cells);
	return didAdd;
}

int spatialHash_getCellForPoint(SpatialHash_t *aHash, vec2_t aPoint)
{
	int index =  aPoint.x/aHash->cellSize + ((aPoint.y/aHash->cellSize) * (aHash->size.w/aHash->cellSize));
	if(index < 0 || index >= aHash->numberOfCells)
		return -1;
	return index;
}

void **spatialHash_query(SpatialHash_t *aHash, rect_t aBoundingBox, int *aoCount)
{
	Array_t *cells = _spatialHash_cellsForBoundingBox(aHash, aBoundingBox);
	int outLength = 0;
	SpatialHash_cell_t *cell;
	for(int i = 0; i < cells->count; ++i) {
		cell = cells->items[i];
		outLength += cell->objects->count;
	}
	if(outLength == 0) {
		array_destroy(cells);
		return NULL;
	}

	void **out = malloc(sizeof(void*)*outLength);
	int currIdx = 0;
	for(int i = 0; i < cells->count; ++i) {
		cell = &aHash->cells[i];
		for(int j = 0; j < cell->objects->count; ++j)
			out[currIdx++] = cell->objects->items[j];
	}
	array_destroy(cells);
	return out;
}
