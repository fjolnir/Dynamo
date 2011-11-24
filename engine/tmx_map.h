// A quick&dirty TMX map loader
//
// Properties are only loaded for the map
// Doesn't support external tilesets
// Tilesets are assumed to only have a single image
// Ignores the transparency color of images (We use the alpha channel)
//
// And probably some other parts. I only implemented what my project required.
// IMPORTANT: Requires you to save your maps with the data layer as CSV (Settable in the preference window)

#include <stdbool.h>

#ifndef _TMXMAP_H_
#define _TMXMAP_H_

#define TMX_MAX_STRLEN 256

typedef enum _TMXMap_orientation {
	kTMXMap_orthogonal,
	kTMXMap_isometric
} TMXMap_orientation;

typedef struct _TMXProperty {
	char name[TMX_MAX_STRLEN];
	char value[TMX_MAX_STRLEN];
} TMXProperty_t;

typedef struct _TMXTileset {
	int firstTileId;
	int imageWidth, imageHeight;
	int tileWidth, tileHeight;
	int spacing;
	int margin;
	char imagePath[TMX_MAX_STRLEN];
} TMXTileset_t;

typedef struct _TMXLayer {
	char name[TMX_MAX_STRLEN];
	float opacity;
	bool isVisible;
	int numberOfTiles;
	int *tileIDs;
} TMXLayer_t;

typedef struct _TMXObject {
	char name[TMX_MAX_STRLEN];
	char type[TMX_MAX_STRLEN];
	int x, y; // in pixels
	int width, height; // in pixels
	int tileId; // Default -1
} TMXObject_t;

typedef struct _TMXObjectGroup {
	char name[TMX_MAX_STRLEN];
	int numberOfObjects;
	TMXObject_t *objects;
} TMXObjectGroup_t;

typedef struct _TMXMap {
	TMXMap_orientation orientation;
	int width, height; // Dimensions in tiles
	int tileWidth, tileHeight; // Dimensions of tiles in pixels
	
	int numberOfLayers;
	TMXLayer_t *layers;
	int numberOfTilesets;
	TMXTileset_t *tilesets;
	int numberOfObjectGroups;
	TMXObjectGroup_t *objectGroups;
	int numberOfProperties;
	TMXProperty_t *properties;
} TMXMap_t;

extern TMXMap_t *tmx_readMapFile(const char *aFilename);
extern void tmx_destroyMap(TMXMap_t *aMap);

// Lookup helpers
extern TMXProperty_t *tmx_mapGetPropertyNamed(TMXMap_t *aMap, const char *aPropertyName);
extern TMXTileset_t *tmx_mapGetTilesetForTileID(TMXMap_t *aMap, int aTileID);
extern TMXLayer_t *tmx_mapGetLayerNamed(TMXMap_t *aMap, const char *aLayerName);
extern TMXObjectGroup_t *tmx_mapGetObjectGroupNamed(TMXMap_t *aMap, const char *aGroupName);
extern TMXObject_t *tmx_objGroupGetObjectNamed(TMXObjectGroup_t *aGroup, const char *aObjName);
#endif
