#include "level.h"
#include "shared.h"
#include "engine/tmx_map.h"

static void _level_draw(Renderer_t *aRenderer, void *aOwner, double aTimeSinceLastFrame, double aInterpolation);
static CollisionPolyObject_t *_level_generateCollisionObjForTile(LevelTile_t aTile);

Level_t *level_load(const char *aFilename)
{
	Level_t *out = malloc(sizeof(Level_t));
	out->renderable.displayCallback = &_level_draw;
	out->character = NULL;
	out->bgm = NULL;

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

	const char *bgmPath = tmx_mapGetPropertyNamed(map, "BGM");
	if(bgmPath)
		out->bgm = sound_load(bgmPath);

	TMXTileset_t *tileset = &map->tilesets[0];
	Texture_t *tilesetTexture = texture_loadFromPng(tileset->imagePath, true, true);
	out->tileSize.w = (float)tileset->tileWidth;
	out->tileSize.h = (float)tileset->tileHeight;
	out->tileset = texAtlas_create(tilesetTexture, kVec2_zero, out->tileSize);
	vec2_t margin = { (float)tileset->spacing, (float)tileset->spacing };

	out->tileset->origin = margin;
	out->tileset->margin = margin;

	// Create the collision world
	vec2_t collisionWorldSize = { (float)out->size[0] * out->tileSize.w, (float)out->size[1] * out->tileSize.h };
	out->collisionWorld = collision_createWorld(vec2_create(0.0f, -880.0f), collisionWorldSize, out->tileSize.w);

	TMXLayer_t *layer = tmx_mapGetLayerNamed(map, "blocks");
	assert(layer);
	out->tiles = malloc(sizeof(LevelTile_t)*map->width*map->height);
	int tilesPerRow = (tileset->imageWidth - tileset->spacing) / (tileset->tileWidth + tileset->spacing);
	int rows = (tileset->imageHeight - tileset->spacing) / (tileset->tileHeight + tileset->spacing);
	int currTileId;
	LevelTile_t *currTile;
	for(int y =  0; y < map->height; ++y) {
		for(int x = 0; x < map->width; ++x) {
			currTile = &out->tiles[y*map->width + x];
			currTileId = layer->tiles[((map->height-1) - y) * map->width + x].id; // Flip the map vertically
			currTile->x = currTileId % tilesPerRow;
			currTile->y = (rows-1) - (currTileId / tilesPerRow); // Flip the y coordinate of the tile
			currTile->collisionObject =_level_generateCollisionObjForTile(out->tiles[y*map->width + x]);
			if(currTile->collisionObject) {
				vec2_t center = vec2_create(x*out->tileSize.w + out->tileSize.w/2.0f, y*out->tileSize.h + out->tileSize.h/2.0f);
				center = vec2_add(center, currTile->collisionObject->center);
				collision_setPolyObjectCenter(currTile->collisionObject, center);
				spatialHash_addItem(out->collisionWorld->spatialHash, currTile->collisionObject, currTile->collisionObject->boundingBox);
			}
		}
	}

	TMXObjectGroup_t *characterObjGroup = tmx_mapGetObjectGroupNamed(map, "character");
	if(characterObjGroup) {
		TMXObject_t *characterObj = &characterObjGroup->objects[0];
		vec3_t center = {
			characterObj->x + characterObj->width/2.0,
			(map->tileHeight*map->height - 1) - (characterObj->y - characterObj->height/2.0), 0.0f
		};
		vec2_t spriteSize = { characterObj->width, characterObj->height };
		TextureAtlas_t *atlas = texAtlas_create(texture_loadFromPng("levels/character.png", false, false), kVec2_zero, spriteSize);
		out->character = character_create();

		out->character->sprite = sprite_create(center, spriteSize, atlas, 7);
		out->character->sprite->animations[0] = sprite_createAnimation(3);
		out->character->sprite->animations[1] = sprite_createAnimation(1);
		out->character->sprite->animations[2] = sprite_createAnimation(1);
		out->character->sprite->animations[3] = sprite_createAnimation(3);
		out->character->sprite->animations[4] = sprite_createAnimation(6);
		out->character->sprite->animations[5] = sprite_createAnimation(8);
		out->character->sprite->animations[6] = sprite_createAnimation(1);
		out->character->sprite->activeAnimation = 6;

		vec2_t characterVerts[4] = {
			{ center.x - 16.0f, center.y - 16.0f },
			{ center.x - 16.0f, center.y + 16.0f },
			{ center.x + 16.0f, center.y + 16.0f },
			{ center.x + 16.0f, center.y - 16.0f }
		};

		out->character->collisionObject = collision_createPolyObject(4, characterVerts, 0.0f, 0.0f);
	}
	tmx_destroyMap(map);


	// Generate & store the tile mesh
	int numberOfTiles = out->size[0]*out->size[1];
	vec2_t *texOffsets = malloc(sizeof(vec2_t)*numberOfTiles);
	vec2_t *screenCoords = malloc(sizeof(vec2_t)*numberOfTiles);
	for(int y = 0; y < out->size[1]; ++y) {
		for(int x = 0; x < out->size[0]; ++x) {
			texOffsets[y*out->size[0] + x].x   = out->tiles[y*out->size[0] + x].x;
			texOffsets[y*out->size[0] + x].y   = out->tiles[y*out->size[0] + x].y;
			screenCoords[y*out->size[0] + x].x = (out->tileSize.w * (float)x) + out->tileSize.w / 2.0f;
			screenCoords[y*out->size[0] + x].y = (out->tileSize.h * (float)y) + out->tileSize.w / 2.0f;
		}
	}

	int numberOfVertices;
	int numberOfIndices;
	vec2_t *vertices;
	vec2_t *texCoords;
	GLuint *indices;

	draw_textureAtlas_getVertices(out->tileset, numberOfTiles, texOffsets, screenCoords, &vertices, &texCoords, &numberOfVertices, &indices, &numberOfIndices);
	free(texOffsets);
	free(screenCoords);

	glGenBuffers(1, &out->vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, out->vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2_t)*numberOfVertices, vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &out->texCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, out->texCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2_t)*numberOfVertices, texCoords, GL_STATIC_DRAW);

	out->numberOfMeshIndices = numberOfIndices;
	glGenBuffers(1, &out->indexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out->indexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*numberOfIndices, indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	free(vertices);
	free(texCoords);
	free(indices);

	return out;
}

void level_destroy(Level_t *aLevel)
{
	background_destroy(aLevel->background, true);
	texAtlas_destroy(aLevel->tileset, true);
	character_destroy(aLevel->character);
	if(aLevel->bgm) sound_destroy(aLevel->bgm);

	glDeleteBuffers(1, &aLevel->vertexVBO);
	glDeleteBuffers(1, &aLevel->texCoordVBO);
	glDeleteBuffers(1, &aLevel->indexVBO);

	free(aLevel);
}


#pragma mark - Drawing

static void _level_draw(Renderer_t *aRenderer, void *aOwner, double aTimeSinceLastFrame, double aInterpolation)
{
	Level_t *level = (Level_t *)aOwner;

	vec2_t center = vec2_floor(level->character->collisionObject->center); // Pixel aligned center

	// Draw the background
	if(level->background) {
		level->background->offset = center;
		level->background->renderable.displayCallback(aRenderer, level->background, aTimeSinceLastFrame, aInterpolation);
	}

	// Translate to the location of the player
	matrix_stack_push(aRenderer->worldMatrixStack);
	matrix_stack_translate(aRenderer->worldMatrixStack, center.x*-1.0f + 100.0f, -1.0f*center.y + 100.0f, 0.0f);

	// Draw the tiles
	shader_makeActive(gTexturedShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, level->tileset->texture->id);

	shader_updateMatrices(gTexturedShader, aRenderer);
	glUniform1i(gTexturedShader->uniforms[kShader_colormap0Uniform], 0);
	vec4_t white = {1.0, 1.0, 1.0, 1.0};
	glUniform4fv(gTexturedShader->uniforms[kShader_colorUniform], 1, white.f);

	glBindBuffer(GL_ARRAY_BUFFER, level->vertexVBO);
	glVertexAttribPointer(gTexturedShader->attributes[kShader_positionAttribute], 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(gTexturedShader->attributes[kShader_positionAttribute]);

	glBindBuffer(GL_ARRAY_BUFFER, level->texCoordVBO);
	glVertexAttribPointer(gTexturedShader->attributes[kShader_texCoord0Attribute], 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(gTexturedShader->attributes[kShader_texCoord0Attribute]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, level->indexVBO);
	glDrawElements(GL_TRIANGLES, level->numberOfMeshIndices, GL_UNSIGNED_INT, 0);

	shader_makeInactive(gTexturedShader);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	// Draw the character
	static int count = 0;
	if(level->character) {
		if(count++ % 4 == 0)
			sprite_step(level->character->sprite);
		level->character->sprite->location = vec3_create(center.x, center.y, 0.0f);
		level->character->sprite->angle = level->character->collisionObject->orientation;
		level->character->sprite->renderable.displayCallback(aRenderer, level->character->sprite, aTimeSinceLastFrame, aInterpolation);
	}
	//level->collisionWorld->debugRenderable.displayCallback(aRenderer, level->collisionWorld, aTimeSinceLastFrame, aInterpolation);
	matrix_stack_pop(aRenderer->worldMatrixStack);
}


#pragma mark - Tools

// Returns a collision shape centered on (0,0) if matching one is found
// TODO: Make this use a resource file instead
static CollisionPolyObject_t *_level_generateCollisionObjForTile(LevelTile_t aTile)
{
	float bounce = 0.2f;
	float friction = 0.1f;
	vec2_t bl = { -16.0f, -16.0f };
	vec2_t tl = { -16.0f,  16.0f };
	vec2_t tr = {  16.0f,  16.0f };
	vec2_t br = {  16.0f, -16.0f };
	vec2_t vertices[4] = { bl, tl, tr, br };

	int x = aTile.x;
	int y = aTile.y;

	// Rectangle shape portions
	if(((y >= 4  && x >= 7) ||
	   (y <= 2 && x == 6) ||
	   (y == 1 && x >= 7))
	   && !(y == 5 && x == 8)) {
		return collision_createPolyObject(4, vertices, friction, bounce);
	}
	
	// Diagonal endpoints
	if(y == 0 && x == 3) {
		vertices[0] = kVec2_zero;
		vertices[1] = tl;
		vertices[2] = tr;
		return collision_createPolyObject(3, vertices, friction, bounce);
	}
	if(y == 3 && x == 0) {
		vertices[0] = br;
		vertices[1] = kVec2_zero;
		vertices[2] = tr;
		return collision_createPolyObject(3, vertices, friction, bounce);
	}
	if(y == 6 && x == 3) {
		vertices[0] = bl;
		vertices[1] = kVec2_zero;
		vertices[2] = br;
		return collision_createPolyObject(3, vertices, friction, bounce);
	}
	if(y == 3 && x == 6) {
		vertices[0] = bl;
		vertices[1] = tl;
		vertices[2] = kVec2_zero;
		return collision_createPolyObject(3, vertices, friction, bounce);
	}


	// Diagonals
	if((y == 1 && x == 2) || (y == 2 && x == 1)) {
		vertices[0] = br;
		vertices[1] = tl;
		vertices[2] = tr;
		return collision_createPolyObject(3, vertices, friction, bounce);
	}
	if((y == 1 && x == 4) || (y == 2 && x == 5)) {
		vertices[0] = bl;
		vertices[1] = tl;
		vertices[2] = tr;
		return collision_createPolyObject(3, vertices, friction, bounce);
	}

	if((y == 5 && x == 2) || (y == 4 && x == 1)) {
		vertices[0] = bl;
		vertices[1] = tr;
		vertices[2] = br;
		return collision_createPolyObject(3, vertices, friction, bounce);
	}
	if((y == 5 && x == 4) || (y == 4 && x == 5)) {
		vertices[0] = bl;
		vertices[1] = tl;
		vertices[2] = br;
		return collision_createPolyObject(3, vertices, friction, bounce);
	}


	return NULL;
}
