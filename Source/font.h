/*!
 @header Font
 @abstract
 @discussion Font class implements rendering of strings using bitmap fonts
 */

#ifndef _FONT_H_
#define _FONT_H_

#include "object.h"
#include "renderer.h"
#include "texture_atlas.h"
#include "dictionary.h"
#include "sprite.h"

/*!
 A Font
 */
typedef struct _Font {
	OBJ_GUTS
    RENDERABLE_GUTS
	TextureAtlas_t *atlas;
    GLMFloat lineHeight;
    Dictionary_t *glyphs;
    
    struct {
        const char *face, *charset;
        bool isBold, isItalic, isUnicode, isSmooth, isAntiAliased;
        vec4_t padding;
        vec2_t spacing;
        GLMFloat size, heightStretch;
    } info;
} Font_t;


/*!
 A page in a font
 */
typedef struct _FontPage {
    int startId;
    TextureAtlas_t *atlas;
} FontPage_t;

/*!
 A glyph in a font
*/
typedef struct _FontGlyph {
    int id;
    vec2_t location;
    vec2_t size;
    vec2_t offset;
    GLMFloat xadvance;
    int page;
    int channel;
} FontGlyph_t;
extern Class_t Class_Font;
#endif