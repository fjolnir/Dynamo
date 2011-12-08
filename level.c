#include "level.h"
#include "shared.h"
#include "engine/tmx_map.h"

static void _level_draw(Renderer_t *aRenderer, void *aOwner, double aTimeSinceLastFrame, double aInterpolation);

Level_t *level_load(const char *aFilename)
{
	Level_t *out = malloc(sizeof(Level_t));
	out->renderable.displayCallback = &_level_draw;
	out->character = NULL;

	// Load the level file
	TMXMap_t *map = tmx_readMapFile(aFilename);
	assert(map);

	out->size[0] = map->width;
	out->size[1] = map->height;
	
	out->background = NULL;
	const char *hasBg = tmx_mapGetPropertyNamed(map, "Has background");
	if(hasBg && strcmp(hasBg, "true") == 0) {
		const char *path;
		out->background = background_create();
		path = tmx_mapGetPropertyNamed(map, "BG1");
		if(path != NULL)
			out->background->layers[0] = background_createLayer(texture_loadFromPng(path, true, true),
																(float)atof(tmx_mapGetPropertyNamed(map, "BG1-depth")));
		path = tmx_mapGetPropertyNamed(map, "BG2");
		if(path != NULL)
			out->background->layers[1] = background_createLayer(texture_loadFromPng(path, true, true),
																(float)atof(tmx_mapGetPropertyNamed(map, "BG2-depth")));
		path = tmx_mapGetPropertyNamed(map, "BG3");
		if(path != NULL)
			out->background->layers[2] = background_createLayer(texture_loadFromPng(path, true, true),
																(float)atof(tmx_mapGetPropertyNamed(map, "BG3-depth")));
		path = tmx_mapGetPropertyNamed(map, "BG4");
		if(path != NULL)
			out->background->layers[3] = background_createLayer(texture_loadFromPng(path, true, true),
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

	TMXObjectGroup_t *characterObjGroup = tmx_mapGetObjectGroupNamed(map, "character");
	if(characterObjGroup) {
		TMXObject_t *characterObj = &characterObjGroup->objects[0];
		vec3_t center = { characterObj->x + characterObj->width/2.0, (map->tileHeight*map->height - 1) - (characterObj->y + characterObj->height/2.0), 0.0f };
		
		vec2_t spriteSize = { characterObj->width, characterObj->height };
		TextureAtlas_t *atlas = texAtlas_create(texture_loadFromPng("textures/sonic.png", false, false), kVec2_zero, spriteSize);
		out->character = character_create();

		out->character->sprite = sprite_create(center, spriteSize, atlas, 1);
		out->character->sprite->animations[0] = sprite_createAnimation(11);
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

static void _level_draw(Renderer_t *aRenderer, void *aOwner, double aTimeSinceLastFrame, double aInterpolation)
{
	Level_t *level = (Level_t *)aOwner;

	// Draw the background
	if(level->background)
		level->background->renderable.displayCallback(aRenderer, level->background, aTimeSinceLastFrame, aInterpolation);
	
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
	matrix_stack_push(aRenderer->worldMatrixStack);
	matrix_stack_translate(aRenderer->worldMatrixStack, -1.0*level->background->offset.x, 0.0, 0.0);
	draw_textureAtlas(level->tileset, numberOfTiles, texOffsets, screenCoords);
	matrix_stack_pop(aRenderer->worldMatrixStack);
	free(texOffsets);
	free(screenCoords);

	// Draw the character
	static int count = 0;
	if(level->character) {
		if(count++ % 2 == 0)
			sprite_step(level->character->sprite);
		level->character->sprite->renderable.displayCallback(aRenderer, level->character->sprite, aTimeSinceLastFrame, aInterpolation);
	}
}
