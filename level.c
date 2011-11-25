#include "level.h"
#include "shared.h"
#include "engine/tmx_map.h"

static void _level_draw(Renderer_t *aRenderer, void *aOwner);

Level_t *level_load(const char *aFilename)
{
	Level_t *out = malloc(sizeof(Level_t));
	out->renderable.displayCallback = &_level_draw;

	// Load the level file
	TMXMap_t *map = tmx_readMapFile(aFilename);
	assert(map);

	out->size[0] = map->width;
	out->size[1] = map->height;
	
	out->background = NULL;
	const char *hasBg = tmx_mapGetPropertyNamed(map, "Has background");
	if(hasBg && strcmp(hasBg, "true") == 0) {
		out->background = background_create();
		out->background->layers[0] = background_createLayer(texture_loadFromPng(tmx_mapGetPropertyNamed(map, "BG1"), true, false),
															(float)atof(tmx_mapGetPropertyNamed(map, "BG1-depth")));
		out->background->layers[1] = background_createLayer(texture_loadFromPng(tmx_mapGetPropertyNamed(map, "BG2"), true, false),
															(float)atof(tmx_mapGetPropertyNamed(map, "BG2-depth")));
		out->background->layers[2] = background_createLayer(texture_loadFromPng(tmx_mapGetPropertyNamed(map, "BG3"), true, false),
															(float)atof(tmx_mapGetPropertyNamed(map, "BG3-depth")));
		out->background->layers[3] = background_createLayer(texture_loadFromPng(tmx_mapGetPropertyNamed(map, "BG4"), true, false),
															(float)atof(tmx_mapGetPropertyNamed(map, "BG4-depth")));
	}

	TMXTileset_t *tileset = &map->tilesets[0];
	Texture_t *tilesetTexture = texture_loadFromPng(tileset->imagePath, true, true);
	out->tileSize.w = (float)tileset->tileWidth;
	out->tileSize.h = (float)tileset->tileHeight;
	out->tileset = texAtlas_create(tilesetTexture, kVec2_zero, out->tileSize);
	vec2_t margin = { (float)tileset->spacing, (float)tileset->spacing };

	out->tileset->origin = margin;
	out->tileset->margin = margin;


	TMXLayer_t *layer = tmx_mapGetLayerNamed(map, "blocks");
	assert(layer);
	out->tiles = malloc(sizeof(LevelTile_t)*map->width*map->height);
	int tilesPerRow = (tileset->imageWidth - tileset->spacing) / (tileset->tileWidth + tileset->spacing);
	int rows = (tileset->imageHeight - tileset->spacing) / (tileset->tileHeight + tileset->spacing);
	int currTileId;
	for(int y =  0; y < map->height; ++y) {
		for(int x = 0; x < map->width; ++x) {
			currTileId = layer->tiles[((map->height-1) - y) * map->width + x].id; // Flip the map vertically
			out->tiles[y*map->width + x].x = currTileId % tilesPerRow;
			out->tiles[y*map->width + x].y = (rows-1) - (currTileId / tilesPerRow); // Flip the y coordinate of the tile
		}
	}

	tmx_destroyMap(map);
	return out;
}

void level_destroy(Level_t *aLevel)
{
	background_destroy(aLevel->background, true);
	texAtlas_destroy(aLevel->tileset, true);
	character_destroy(aLevel->character);
	free(aLevel);
}


#pragma mark - Drawing

static void _level_draw(Renderer_t *aRenderer, void *aOwner)
{

	Level_t *level = (Level_t *)aOwner;
	// Draw the background
	if(level->background)
		level->background->renderable.displayCallback(aRenderer, level->background);
	
	// Draw the tiles
	int numberOfTiles = level->size[0]*level->size[1];
	vec2_t *texOffsets = malloc(sizeof(vec2_t)*numberOfTiles);
	vec2_t *screenCoords = malloc(sizeof(vec2_t)*numberOfTiles);
	for(int y = 0; y < level->size[1]; ++y) {
		for(int x = 0; x < level->size[0]; ++x) {
			texOffsets[y*level->size[0] + x].x = level->tiles[y*level->size[0] + x].x;
			texOffsets[y*level->size[0] + x].y = level->tiles[y*level->size[0] + x].y;
			screenCoords[y*level->size[0] + x].x = (level->tileSize.w * (float)x) + level->tileSize.w / 2.0f;
			screenCoords[y*level->size[0] + x].y = (level->tileSize.h * (float)y) + level->tileSize.w / 2.0f;
		}
	}
	draw_textureAtlas(level->tileset, numberOfTiles, texOffsets, screenCoords);
	free(texOffsets);
	free(screenCoords);
}
