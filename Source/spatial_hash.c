#include "spatial_hash.h"
#include "engine/various.h"

SpatialHash_t *spatialHash_create(vec2_t aSize, float aCellSize)
{
	SpatialHash_t *out = malloc(sizeof(SpatialHash_t));
	out->size = aSize;
	out->cellSize = aCellSize;
	out->sizeInCells = vec2_create(ceilf(aSize.w/aCellSize), ceilf(aSize.h/aCellSize));
	out->numberOfCells = (int)(out->sizeInCells.w)*(int)(out->sizeInCells.h);
	out->cells = calloc(out->numberOfCells, sizeof(SpatialHash_cell_t*));

	return out;
}

void spatialHash_destroy(SpatialHash_t *aHash)
{
	for(int i = 0; i < aHash->numberOfCells; ++i)
		spatialHash_destroyCell(aHash->cells[i]);
	free(aHash->cells);
	free(aHash);
}

SpatialHash_cell_t *spatialHash_createCell()
{
	SpatialHash_cell_t *out = malloc(sizeof(SpatialHash_cell_t));
	out->objects = array_create(8);

	return out;
}

void spatialHash_destroyCell(SpatialHash_cell_t *aCell)
{
	if(aCell) {
		array_destroy(aCell->objects);
		free(aCell);
	}
}

// Clears the hash making it ready for reuse
void spatialHash_clear(SpatialHash_t *aHash)
{
	for(int i = 0; i < aHash->numberOfCells; ++i)
		if(aHash->cells[i] != NULL) aHash->cells[i]->objects->count = 0;
}

Array_t *spatialHash_cellsForBoundingBox(SpatialHash_t *aHash, rect_t aBoundingBox, bool aShouldCreate)
{
	vec2_t cellDimensions = {
		ceilf(( ((int)aBoundingBox.o.x%(int)aHash->cellSize) + aBoundingBox.s.w) / aHash->cellSize),
		ceilf(( ((int)aBoundingBox.o.y%(int)aHash->cellSize) + aBoundingBox.s.h) / aHash->cellSize)
	};
	int numberOfCells = cellDimensions.w*cellDimensions.h;
	Array_t *cells = array_create(numberOfCells);

	int firstIndex = floorf(aBoundingBox.origin.x/aHash->cellSize) + (floorf(aBoundingBox.origin.y/aHash->cellSize) * aHash->sizeInCells.w);

	SpatialHash_cell_t *cell;
	int index;
	for(int y = 0; y < (int)cellDimensions.h; ++y) {
		for(int x = 0; x < (int)cellDimensions.w; ++x) {
			index = firstIndex+ (y*aHash->sizeInCells.w)+x;
			if(index < 0 || index >= aHash->sizeInCells.w * aHash->sizeInCells.h)
				continue;
			if(!aHash->cells[index] && aShouldCreate)
				aHash->cells[index] = spatialHash_createCell();
			cell = aHash->cells[index];
			if(cell)
				array_push(cells, cell);
		}
	}
	return cells;
}

bool spatialHash_addItem(SpatialHash_t *aHash, void *aItem, rect_t aBoundingBox)
{
	Array_t *cells = spatialHash_cellsForBoundingBox(aHash, aBoundingBox, true);
	SpatialHash_cell_t *currentCell;
	for(int i = 0; i < cells->count; ++i) {
		currentCell = cells->items[i];
		array_push(currentCell->objects, aItem);
	}
	array_destroy(cells);

	return (cells->count > 0);
}

int spatialHash_getCellForPoint(SpatialHash_t *aHash, vec2_t aPoint, bool aShouldCreate)
{
	int index =  floorf(aPoint.x/aHash->cellSize) + (floorf(aPoint.y/aHash->cellSize) * aHash->sizeInCells.w);
	if(index < 0 || index >= aHash->numberOfCells)
		return -1;
	
	if(aHash->cells[index] == NULL) {
		if(aShouldCreate)
			aHash->cells[index] = spatialHash_createCell();
		else
			return -1;
	}
	return index;
}

void **spatialHash_query(SpatialHash_t *aHash, rect_t aBoundingBox, int *aoCount)
{
	Array_t *cells = spatialHash_cellsForBoundingBox(aHash, aBoundingBox, false);
	if(!cells) return NULL;

	int outLength = 0;
	SpatialHash_cell_t *cell;
	for(int i = 0; i < cells->count; ++i) {
		cell = cells->items[i];
		outLength += cell->objects->count;
	}
	if(outLength == 0) {
		if(aoCount) *aoCount = 0;
		array_destroy(cells);
		return NULL;
	}
	void **out = malloc(sizeof(void*)*outLength);
	int currIdx = 0;
	for(int i = 0; i < cells->count; ++i) {
		cell = cells->items[i];
		if(cell == NULL) continue;
		for(int j = 0; j < cell->objects->count; ++j)
			out[currIdx++] = cell->objects->items[j];
	}
	array_destroy(cells);
	if(aoCount) *aoCount = currIdx;

	return out;
}
