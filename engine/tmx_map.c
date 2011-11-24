#include "tmx_map.h"
#include "various.h"
#include <stdlib.h>
#include <assert.h>
#include <mxml.h>


// Private helpers
static int *_parseCSV(const char *aCsvString, int aWidth, int aHeight);
static void _mxmlElementPrintAttrs(mxml_node_t *aNode);
static float _mxmlElementGetAttrAsFloat(mxml_node_t *aNode, const char *aAttrName, float aDefault);
static int _mxmlElementGetAttrAsInt(mxml_node_t *aNode, const char *aAttrName, int aDefault);
static mxml_node_t **_mxmlFindChildren(mxml_node_t *aNode, mxml_node_t *aTop, const char *aName, int *aoCount);


TMXMap_t *tmx_readMapFile(const char *aFilename)
{
	FILE *fp = fopen(aFilename, "rb");
	if(!fp) return NULL;
	mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_OPAQUE_CALLBACK);
	assert(tree != NULL);
	fclose(fp);

	TMXMap_t *out = malloc(sizeof(TMXMap_t));
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

	// Read the properties
	mxml_node_t *propertiesNode = mxmlFindElement(mapNode, tree, "properties", NULL, NULL, MXML_DESCEND_FIRST);
	if(propertiesNode) {
		mxml_node_t **propertyNodes = _mxmlFindChildren(propertiesNode, tree, "property", &out->numberOfProperties);
		out->properties = malloc(sizeof(TMXProperty_t)*out->numberOfProperties);
		for(int i = 0; i < out->numberOfTilesets; ++i) {
			tempNode = propertyNodes[i];
			strncpy(out->properties[i].name, mxmlElementGetAttr(tempNode, "name"), TMX_MAX_STRLEN);
			strncpy(out->properties[i].value, mxmlElementGetAttr(tempNode, "value"), TMX_MAX_STRLEN);
			mxmlRelease(tempNode);
		}
	}

	// Read the tilesets
	mxml_node_t **tilesetNodes = _mxmlFindChildren(mapNode, tree, "tileset", &out->numberOfTilesets);
	out->tilesets = malloc(sizeof(TMXTileset_t)*out->numberOfTilesets);
	for(int i = 0; i < out->numberOfTilesets; ++i) {
		tempNode = tilesetNodes[i];
		out->tilesets[i].firstTileId = _mxmlElementGetAttrAsInt(tempNode, "firstgid", 0);
		out->tilesets[i].tileWidth = _mxmlElementGetAttrAsInt(tempNode, "tilewidth", 0);
		out->tilesets[i].tileHeight = _mxmlElementGetAttrAsInt(tempNode, "tileheight", 0);
		out->tilesets[i].spacing = _mxmlElementGetAttrAsInt(tempNode, "spacing", 0);
		out->tilesets[i].margin = _mxmlElementGetAttrAsInt(tempNode, "margin", 0);

		mxml_node_t *imageNode = mxmlFindElement(tempNode, tree, "image", NULL, NULL, MXML_DESCEND_FIRST);
		assert(imageNode);
		out->tilesets[i].imageWidth = _mxmlElementGetAttrAsInt(imageNode, "width", 0);
		out->tilesets[i].imageHeight = _mxmlElementGetAttrAsInt(imageNode, "height", 0);
		strncpy(out->tilesets[i].imagePath, mxmlElementGetAttr(imageNode, "source"), TMX_MAX_STRLEN);

		mxmlRelease(imageNode);
		mxmlRelease(tempNode);
	}

	// Read the layers
	mxml_node_t **layerNodes = _mxmlFindChildren(mapNode, tree, "layer", &out->numberOfLayers);
	out->layers = malloc(sizeof(TMXTileset_t)*out->numberOfLayers);
	for(int i = 0; i < out->numberOfLayers; ++i) {
		tempNode = layerNodes[i];
		out->layers[i].opacity = _mxmlElementGetAttrAsFloat(tempNode, "opacity", 1.0);
		out->layers[i].isVisible = _mxmlElementGetAttrAsInt(tempNode, "visible", 1);
		// Parse the tile CSV
		mxml_node_t *dataNode = mxmlFindElement(tempNode, tree, "data", NULL, NULL, MXML_DESCEND_FIRST);
		assert(dataNode);
		mxml_node_t *csvNode = mxmlWalkNext(dataNode, tree, MXML_DESCEND);
		assert(csvNode);
		const char *tileCSV = csvNode->value.opaque;
		out->layers[i].tileIDs = _parseCSV(tileCSV, out->width, out->height);

		mxmlRelease(csvNode);
		mxmlRelease(dataNode);
		mxmlRelease(tempNode);
	}

	// Read the object groups
	mxml_node_t **objGroupNodes = _mxmlFindChildren(mapNode, tree, "objectgroup", &out->numberOfObjectGroups);
	out->objectGroups = malloc(sizeof(TMXObjectGroup_t)*out->numberOfObjectGroups);
	for(int i = 0; i < out->numberOfObjectGroups; ++i) {
		tempNode = objGroupNodes[i];
		strncpy(out->objectGroups[i].name, mxmlElementGetAttr(tempNode, "name"), TMX_MAX_STRLEN);
		// Read the objects in the current group
		mxml_node_t **objNodes = _mxmlFindChildren(tempNode, tree, "object", &out->objectGroups[i].numberOfObjects);
		out->objectGroups[i].objects = malloc(sizeof(TMXObject_t)*out->objectGroups[i].numberOfObjects);
		for(int j = 0; j < out->objectGroups[i].numberOfObjects; ++j) {
			strncpy(out->objectGroups[i].objects[j].name, mxmlElementGetAttr(objNodes[i], "name"), TMX_MAX_STRLEN);
			strncpy(out->objectGroups[i].objects[j].type, mxmlElementGetAttr(objNodes[i], "type"), TMX_MAX_STRLEN); 
			out->objectGroups[i].objects[j].x = _mxmlElementGetAttrAsInt(objNodes[i], "x", 0);
			out->objectGroups[i].objects[j].y = _mxmlElementGetAttrAsInt(objNodes[i], "y", 0);
			out->objectGroups[i].objects[j].width = _mxmlElementGetAttrAsInt(objNodes[i], "width", 0);
			out->objectGroups[i].objects[j].height = _mxmlElementGetAttrAsInt(objNodes[i], "height", 0);
			out->objectGroups[i].objects[j].tileId = _mxmlElementGetAttrAsInt(objNodes[i], "gid", -1);
			
			mxmlRelease(objNodes[i]);
		}
		mxmlRelease(tempNode);
	}

	// Clean up
	mxmlRelease(mapNode);
	mxmlRelease(tree);

	return out;
}

void tmx_destroyMap(TMXMap_t *aMap)
{
	for(int i = 0; i < aMap->numberOfLayers; ++i) free(aMap->layers[i].tileIDs);
	free(aMap->layers);
	for(int i = 0; i < aMap->numberOfObjectGroups; ++i) free(aMap->objectGroups[i].objects);
	free(aMap->objectGroups);
	free(aMap->tilesets);	

	free(aMap);
}


#pragma mark - Lookup helpers

TMXProperty_t *tmx_mapGetPropertyNamed(TMXMap_t *aMap, const char *aPropertyName)
{
	for(int i = 0; i < aMap->numberOfProperties; ++i) {
		if(strcmp(aMap->properties[i].name, aPropertyName) == 0)
			return &aMap->properties[i];
	}
	return NULL;
}

TMXTileset_t *tmx_mapGetTilesetForTileID(TMXMap_t *aMap, int aTileID)
{
	for(int i = 0; i < aMap->numberOfTilesets; ++i) {
		if(aMap->tilesets[i].firstTileId < aTileID)
			return &aMap->tilesets[i];
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


#pragma mark - Private helpers

// We know the dimensions of the map beforehand so we can just load the csv into a one dimensional array
static int *_parseCSV(const char *aCsvString, int aWidth, int aHeight)
{
	int *out = malloc(sizeof(int)*aWidth*aHeight);
	int i = 0; // Offset in output array
	int lastOffset = 0, currentOffset = 0; // Offset in CSV string
	do {
		currentOffset++;
		if(*(aCsvString+currentOffset) == '\n')
			lastOffset++;
		else if(*(aCsvString+currentOffset) == ',' || *(aCsvString+currentOffset) == '\0') {
			sscanf((aCsvString+lastOffset), "%d", &out[i++]);
			lastOffset = currentOffset+1;
		}
	} while(*(aCsvString+currentOffset) != '\0' && i < aWidth*aHeight);

	return out;
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
