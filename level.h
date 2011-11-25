#include "engine/background.h"
#include "engine/texture_atlas.h"

#include "character.h"

#ifndef _LEVEL_H_
#define _LEVEL_H_

typedef struct _LevelTile {
	// Tile coordinates within the texture atlas
	int x;
	int y;
} LevelTile_t;

typedef struct _Level {
	Renderable_t renderable;
	Background_t *background;
	TextureAtlas_t *tileset;
	LevelTile_t *tiles;
	int size[2]; // In tiles
	vec2_t tileSize; // In pixels
	Character_t *character;
} Level_t;

extern Level_t *level_load(const char *aFilename);
extern void level_destroy(Level_t *aLevel);
#endif
