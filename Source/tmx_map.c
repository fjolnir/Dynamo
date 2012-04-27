#include "tmx_map.h"
#include "util.h"
#include <stdlib.h>
#include <assert.h>
#include <mxml.h>
#include <string.h>
#include "drawutils.h"

const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
const unsigned FLIPPED_VERTICALLY_FLAG   = 0x40000000;

static void tmx_destroyMap(TMXMap_t *aMap);

// Private helpers
static void _mxmlElementPrintAttrs(mxml_node_t *aNode);
static char *_mxmlElementCopyAttr(mxml_node_t *aNode, const char *aAttrName);
static float _mxmlElementGetAttrAsFloat(mxml_node_t *aNode, const char *aAttrName, float aDefault);
static int _mxmlElementGetAttrAsInt(mxml_node_t *aNode, const char *aAttrName, int aDefault);
static mxml_node_t **_mxmlFindChildren(mxml_node_t *aNode, mxml_node_t *aTop, const char *aName, int *aoCount);
static TMXProperty_t *_tmx_readPropertiesFromMxmlNode(mxml_node_t *aParentNode, mxml_node_t *aTopNode, int *aoCount);

static TMXTile_t _tmx_mapCreateTileForTileGID(TMXMap_t *aMap, int aTileGID);
static TMXTileset_t *_tmx_mapGetTilesetForTileGID(TMXMap_t *aMap, int aTileID);

Class_t Class_TMXMap = {
	"TMXMap",
	sizeof(TMXMap_t),
	(Obj_destructor_t)&tmx_destroyMap
};

TMXMap_t *tmx_readMapFile(const char *aFilename)
{
	FILE *fp = fopen(aFilename, "rb");
	if(!fp) return NULL;
	mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_OPAQUE_CALLBACK);
	assert(tree != NULL);
	fclose(fp);

	TMXMap_t *out = obj_create_autoreleased(&Class_TMXMap);
	// Start walking through the file
	mxml_node_t *mapNode = mxmlFindElement(tree, tree, "map", NULL, NULL, MXML_DESCEND);

	const char *mapOrientation = mxmlElementGetAttr(mapNode, "orientation");
	assert(mapOrientation);
	out->orientation = (strstr(mapOrientation, "isometric") != NULL) ? kTMXMap_isometric : kTMXMap_orthogonal;

	out->width      = _mxmlElementGetAttrAsInt(mapNode, "width", 0);
	out->height     = _mxmlElementGetAttrAsInt(mapNode, "height", 0);
	out->tileWidth  = _mxmlElementGetAttrAsInt(mapNode, "tilewidth", 0);
	out->tileHeight = _mxmlElementGetAttrAsInt(mapNode, "tileheight", 0);

	mxml_node_t *tempNode;

	// Read the map properties
	out->properties = _tmx_readPropertiesFromMxmlNode(mapNode, tree, &out->numberOfProperties);

	// Read the tilesets
	mxml_node_t **tilesetNodes = _mxmlFindChildren(mapNode, tree, "tileset", &out->numberOfTilesets);
	out->tilesets = malloc(sizeof(TMXTileset_t)*out->numberOfTilesets);
	for(int i = 0; i < out->numberOfTilesets; ++i) {
		tempNode = tilesetNodes[i];
		out->tilesets[i].firstTileGid = _mxmlElementGetAttrAsInt(tempNode, "firstgid", 0);
		out->tilesets[i].tileWidth = _mxmlElementGetAttrAsInt(tempNode, "tilewidth", 0);
		out->tilesets[i].tileHeight = _mxmlElementGetAttrAsInt(tempNode, "tileheight", 0);
		out->tilesets[i].spacing = _mxmlElementGetAttrAsInt(tempNode, "spacing", 0);
		out->tilesets[i].margin = _mxmlElementGetAttrAsInt(tempNode, "margin", 0);
		mxml_node_t *imageNode = mxmlFindElement(tempNode, tree, "image", NULL, NULL, MXML_DESCEND_FIRST);
		assert(imageNode);
		out->tilesets[i].imageWidth = _mxmlElementGetAttrAsInt(imageNode, "width", 0);
		out->tilesets[i].imageHeight = _mxmlElementGetAttrAsInt(imageNode, "height", 0);
		out->tilesets[i].imagePath = strdup(mxmlElementGetAttr(imageNode, "source"));
	}
	free(tilesetNodes);

	// Read the layers
	mxml_node_t **layerNodes = _mxmlFindChildren(mapNode, tree, "layer", &out->numberOfLayers);
	out->layers = malloc(sizeof(TMXLayer_t)*out->numberOfLayers);
	for(int i = 0; i < out->numberOfLayers; ++i) {
		tempNode = layerNodes[i];
		out->layers[i].name = _mxmlElementCopyAttr(tempNode, "name");
		out->layers[i].opacity = _mxmlElementGetAttrAsFloat(tempNode, "opacity", 1.0);
		out->layers[i].isVisible = _mxmlElementGetAttrAsInt(tempNode, "visible", 1);
		out->layers[i].properties = _tmx_readPropertiesFromMxmlNode(tempNode, tree, &out->layers[i].numberOfProperties);

		// Parse the tiles and resolve the global tile ids
		mxml_node_t *dataNode = mxmlFindElement(tempNode, tree, "data", NULL, NULL, MXML_DESCEND_FIRST);
		assert(dataNode);
        mxml_node_t **tileNodes = _mxmlFindChildren(dataNode, tree, "tile", &out->layers[i].numberOfTiles);
        assert(out->layers[i].numberOfTiles == out->width*out->height);

        out->layers[i].tiles = malloc(sizeof(TMXTile_t)*out->layers[i].numberOfTiles);
        for(int j = 0; j < out->layers[i].numberOfTiles; ++j) {
            int gid = _mxmlElementGetAttrAsInt(tileNodes[j], "gid", 0);
            out->layers[i].tiles[j] = _tmx_mapCreateTileForTileGID(out, gid);
        }
        free(tileNodes);
	}
	free(layerNodes);

	// Read the object groups
	mxml_node_t **objGroupNodes = _mxmlFindChildren(mapNode, tree, "objectgroup", &out->numberOfObjectGroups);
	out->objectGroups = malloc(sizeof(TMXObjectGroup_t)*out->numberOfObjectGroups);
	for(int i = 0; i < out->numberOfObjectGroups; ++i) {
		tempNode = objGroupNodes[i];
		out->objectGroups[i].name = _mxmlElementCopyAttr(tempNode, "name");
		out->objectGroups[i].properties = _tmx_readPropertiesFromMxmlNode(tempNode, tree, &out->objectGroups[i].numberOfProperties);
		// Read the objects in the current group
		mxml_node_t **objNodes = _mxmlFindChildren(tempNode, tree, "object", &out->objectGroups[i].numberOfObjects);
		out->objectGroups[i].objects = malloc(sizeof(TMXObject_t)*out->objectGroups[i].numberOfObjects);
		for(int j = 0; j < out->objectGroups[i].numberOfObjects; ++j) {
			out->objectGroups[i].objects[j].name = _mxmlElementCopyAttr(objNodes[i], "name");
			out->objectGroups[i].objects[j].type = _mxmlElementCopyAttr(objNodes[i], "type");
			out->objectGroups[i].objects[j].x = _mxmlElementGetAttrAsInt(objNodes[i], "x", 0);
			out->objectGroups[i].objects[j].y = _mxmlElementGetAttrAsInt(objNodes[i], "y", 0);
			out->objectGroups[i].objects[j].width = _mxmlElementGetAttrAsInt(objNodes[i], "width", 0);
			out->objectGroups[i].objects[j].height = _mxmlElementGetAttrAsInt(objNodes[i], "height", 0);
			int tileGid = _mxmlElementGetAttrAsInt(objNodes[i], "gid", -1);
			out->objectGroups[i].objects[j].tile = _tmx_mapCreateTileForTileGID(out, tileGid);
		}
		free(objNodes);
	}
	free(objGroupNodes);

	// Clean up
	mxmlRelease(tree);

	return out;
}

void tmx_destroyMap(TMXMap_t *aMap)
{
	for(int i = 0; i < aMap->numberOfLayers; ++i) {
		for(int j = 0; aMap->layers[i].numberOfProperties; ++j) {
			free(aMap->layers[i].properties[j].name);
			free(aMap->layers[i].properties[j].value);
		}
		if(aMap->layers[i].properties) free(aMap->layers[i].properties);
		free(aMap->layers[i].name);
		free(aMap->layers[i].tiles);
	}
	free(aMap->layers);

	for(int i = 0; i < aMap->numberOfObjectGroups; ++i) {
		for(int j = 0; j < aMap->objectGroups[i].numberOfObjects; ++j) {
			free(aMap->objectGroups[i].objects[j].name);
			free(aMap->objectGroups[i].objects[j].type);
		}
		free(aMap->objectGroups[i].objects);
		for(int j = 0; aMap->objectGroups[i].numberOfProperties; ++j) {
			free(aMap->objectGroups[i].properties[j].name);
			free(aMap->objectGroups[i].properties[j].value);
		}
		if(aMap->objectGroups[i].properties) free(aMap->objectGroups[i].properties);

		free(aMap->objectGroups[i].name);
	}
	free(aMap->objectGroups);

	for(int i = 0; i < aMap->numberOfTilesets; ++i) free(aMap->tilesets[i].imagePath);
	free(aMap->tilesets);

	for(int i = 0; i < aMap->numberOfProperties; ++i) {
		free(aMap->properties[i].name);
		free(aMap->properties[i].value);
	}
	if(aMap->properties) free(aMap->properties);
	free(aMap);
}


#pragma mark - Lookup helpers

const char *tmx_mapGetPropertyNamed(TMXMap_t *aMap, const char *aPropertyName)
{
	for(int i = 0; i < aMap->numberOfProperties; ++i) {
		if(strcmp(aMap->properties[i].name, aPropertyName) == 0)
			return aMap->properties[i].value;
	}
	return NULL;
}

TMXLayer_t *tmx_mapGetLayerNamed(TMXMap_t *aMap, const char *aLayerName)
{
	for(int i = 0; i < aMap->numberOfLayers; ++i) {
		if(strcmp(aMap->layers[i].name, aLayerName) == 0)
			return &aMap->layers[i];
	}
	return NULL;
}

TMXObjectGroup_t *tmx_mapGetObjectGroupNamed(TMXMap_t *aMap, const char *aGroupName)
{
	for(int i = 0; i < aMap->numberOfObjectGroups; ++i) {
		if(strcmp(aMap->objectGroups[i].name, aGroupName) == 0)
			return &aMap->objectGroups[i];
	}
	return NULL;
}

TMXObject_t *tmx_objGroupGetObjectNamed(TMXObjectGroup_t *aGroup, const char *aObjName)
{
	for(int i = 0; i < aGroup->numberOfObjects; ++i) {
		if(strcmp(aGroup->objects[i].name, aObjName) == 0)
			return &aGroup->objects[i];
	}
	return NULL;
}

#pragma mark - Private lookup helpers

static TMXTile_t _tmx_mapCreateTileForTileGID(TMXMap_t *aMap, int aTileGID)
{
	TMXTile_t out = { NULL, -1, 0, 0 };
	if(aTileGID == -1) return out;

	out.flippedHorizontally = (aTileGID & FLIPPED_HORIZONTALLY_FLAG);
    out.flippedVertically = (aTileGID & FLIPPED_VERTICALLY_FLAG);
    // Clear the flags
    aTileGID &= ~(FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG);
	TMXTileset_t *tileset = _tmx_mapGetTilesetForTileGID(aMap, aTileGID);
	if(!tileset) return out;
	out.tileset = tileset;
	out.id = aTileGID - tileset->firstTileGid;

	return out;
}

static TMXTileset_t *_tmx_mapGetTilesetForTileGID(TMXMap_t *aMap, int aTileGID)
{
	for(int i = 0; i < aMap->numberOfTilesets; ++i) {
		if(aMap->tilesets[i].firstTileGid <= aTileGID)
			return &aMap->tilesets[i];
	}
	return NULL;
}

static void _mxmlElementPrintAttrs(mxml_node_t *aNode)
{
	mxml_element_t *element = &aNode->value.element;
	mxml_attr_t *attr;
	for(int i = 0; i < element->num_attrs; ++i) {
		attr = &element->attrs[i];
		debug_log("%s: %s=%s", element->name, attr->name, attr->value);
	}
}


#pragma mark - XML Parsing

static char *_mxmlElementCopyAttr(mxml_node_t *aNode, const char *aAttrName)
{
	const char *attr = mxmlElementGetAttr(aNode, aAttrName);
	if(!attr) return NULL;
	return strdup(attr);
}

static float _mxmlElementGetAttrAsFloat(mxml_node_t *aNode, const char *aAttrName, float aDefault)
{
	float out;
	const char *value = mxmlElementGetAttr(aNode, aAttrName);
	if(value == NULL) return aDefault;
	sscanf(value, "%f", &out);
	return out;
}

static int _mxmlElementGetAttrAsInt(mxml_node_t *aNode, const char *aAttrName, int aDefault)
{
	int out;
	const char *value = mxmlElementGetAttr(aNode, aAttrName);
	if(value == NULL) return aDefault;
	sscanf(value, "%d", &out);
	return out;
}

static mxml_node_t **_mxmlFindChildren(mxml_node_t *aNode, mxml_node_t *aTop, const char *aName, int *aoCount)
{
	int count = 0;
	int capacity = 24;
	mxml_node_t **nodes = malloc(sizeof(mxml_node_t*)*capacity);
	mxml_node_t *currNode;
	for(currNode = mxmlFindElement(aNode, aTop, aName, NULL, NULL, MXML_DESCEND_FIRST);
	    currNode != NULL;
	    currNode = mxmlFindElement(currNode, aTop, aName, NULL, NULL, MXML_NO_DESCEND), ++count)
	{
		if(count >= capacity) {
			capacity *= 2;
			nodes = realloc(nodes, sizeof(mxml_node_t*)*capacity);
		}
		nodes[count] = currNode;
	}
	if(aoCount != NULL) *aoCount = count;

	return nodes;
}

TMXProperty_t *_tmx_readPropertiesFromMxmlNode(mxml_node_t *aParentNode, mxml_node_t *aTopNode, int *aoCount)
{
	mxml_node_t *propertiesNode = mxmlFindElement(aParentNode, aTopNode, "properties", NULL, NULL, MXML_DESCEND_FIRST);
	if(propertiesNode) {
		mxml_node_t **propertyNodes = _mxmlFindChildren(propertiesNode, aTopNode, "property", aoCount);
		TMXProperty_t *properties = malloc(sizeof(TMXProperty_t)* (*aoCount));
		for(int i = 0; i < *aoCount; ++i) {
			properties[i].name = _mxmlElementCopyAttr(propertyNodes[i], "name");
			properties[i].value = _mxmlElementCopyAttr(propertyNodes[i], "value");
			mxmlRelease(propertyNodes[i]);
		}
		free(propertyNodes);
		if(*aoCount == 0) {
			free(properties);
			return NULL;
		}
		return properties;
	}
	else if(aoCount != NULL)
		*aoCount = 0;

	return NULL;
}

static void tmx_destroyLayerRenderable(TMXLayerRenderable_t *aRenderable)
{
    glDeleteBuffers(1, &aRenderable->posVBO);
    glDeleteBuffers(1, &aRenderable->texCoordVBO);
    glDeleteBuffers(1, &aRenderable->indexVBO);
    obj_release(aRenderable->map);
}

Class_t Class_TMXLayerRenderable = {
	"TMXLayerRenderable",
	sizeof(TMXLayerRenderable_t),
	(Obj_destructor_t)&tmx_destroyLayerRenderable
};

static void tmx_drawLayerRenderable(Renderer_t *aRenderer, TMXLayerRenderable_t *aRenderable, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation)
{
   shader_makeActive(gTexturedShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, aRenderable->atlas->texture->id);
    
	shader_updateMatrices(gTexturedShader, aRenderer);
	glUniform1i(gTexturedShader->uniforms[kShader_colormap0Uniform], 0);
	vec4_t white = {1.0, 1.0, 1.0, 1.0};
	glUniform4fv(gTexturedShader->uniforms[kShader_colorUniform], 1, white.f);
    
	glBindBuffer(GL_ARRAY_BUFFER, aRenderable->posVBO);
	glVertexAttribPointer(gTexturedShader->attributes[kShader_positionAttribute], 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(gTexturedShader->attributes[kShader_positionAttribute]);
    
	glBindBuffer(GL_ARRAY_BUFFER, aRenderable->texCoordVBO);
	glVertexAttribPointer(gTexturedShader->attributes[kShader_texCoord0Attribute], 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(gTexturedShader->attributes[kShader_texCoord0Attribute]);
    
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, aRenderable->indexVBO);
	glDrawElements(GL_TRIANGLES, aRenderable->indexCount, GL_UNSIGNED_INT, 0);
    
	shader_makeInactive(gTexturedShader);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

#pragma mark -

vec2_t tmx_tileset_texCoordFromId(TMXTileset_t *aTileset, int id)
{
    int tilesPerRow = (aTileset->imageWidth - aTileset->spacing) / (aTileset->tileWidth + aTileset->spacing);
    int u = id % tilesPerRow;
    int v = id / tilesPerRow;
    return vec2_create(u, v);
}

TMXLayerRenderable_t *tmx_createRenderableForLayer(TMXMap_t *aMap, unsigned int aLayerIdx)
{
    assert(aMap != NULL);
    assert(aLayerIdx < aMap->numberOfLayers);
    obj_retain(aMap);
    
    TMXLayerRenderable_t *out = obj_create_autoreleased(&Class_TMXLayerRenderable);
    out->map = aMap;
    out->layer = &aMap->layers[aLayerIdx];
    out->displayCallback = (RenderableDisplayCallback_t)&tmx_drawLayerRenderable;
    
    // Find the first used tile and use its tileset
    TMXTileset_t *tileset = NULL; 
    for(int i = 0; i < out->layer->numberOfTiles && tileset == NULL; ++i) {
        tileset = out->layer->tiles[i].tileset;
    }
    assert(tileset != NULL);
    
    // Open a texture atlas for the tileset
    vec2_t tileSize = vec2_create(out->map->tileWidth, out->map->tileHeight);
    char texPath[512];
    util_pathForResource(tileset->imagePath, NULL, NULL, texPath, 512);
    Texture_t *tex = texture_loadFromPng((const char*)texPath, false, false);
    assert(tex != NULL);
    TextureAtlas_t *atlas = texAtlas_create(tex, vec2_create(tileset->margin, tileset->margin), tileSize);
    atlas->margin = vec2_create(tileset->spacing, tileset->spacing);
    assert(atlas != NULL);
    out->atlas = obj_retain(atlas);
    
    // Generate & store the tile mesh
	vec2_t *texOffsets = malloc(sizeof(vec2_t)*out->layer->numberOfTiles);
	vec2_t *screenCoords = malloc(sizeof(vec2_t)*out->layer->numberOfTiles);
	for(int y = 0; y < out->map->height; ++y) {
		for(int x = 0; x < out->map->width; ++x) {
            int idx = y*out->map->width + x;
            TMXTile_t *tile = &out->layer->tiles[idx];
            texOffsets[idx] = tmx_tileset_texCoordFromId(tileset, tile->id);
			screenCoords[idx].x = (out->map->tileWidth * (float)x) + out->map->tileWidth / 2.0f;
			screenCoords[idx].y = (out->map->tileHeight * (float)y) + out->map->tileWidth / 2.0f;
		}
	}

    int numberOfVertices;
	int numberOfIndices;
	vec2_t *vertices;
	vec2_t *texCoords;
	GLuint *indices;
    draw_textureAtlas_getVertices(atlas, out->layer->numberOfTiles, texOffsets, screenCoords, &vertices, &texCoords, &numberOfVertices, &indices, &numberOfIndices);
    free(texOffsets);
	free(screenCoords);
    
    glGenBuffers(1, &out->posVBO);
    glBindBuffer(GL_ARRAY_BUFFER, out->posVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2_t)*numberOfVertices, vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &out->texCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, out->texCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2_t)*numberOfVertices, texCoords, GL_STATIC_DRAW);
    
	out->indexCount = numberOfIndices;
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
