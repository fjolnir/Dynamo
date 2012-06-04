#include "font.h"
#include "util.h"

static void _font_destroy(Font_t *aFont);
static char *_itoa(int i);

Class_t Class_Font = {
    "Font",
    sizeof(Font_t),
    (Obj_destructor_t)&_font_destroy
};

void _font_parse(size_t aLen, const char *aData)
{
    
}

Font_t *font_load(const char *aPath)
{
	FILE *fd = fopen(aPath, "r");
    if(!fd) {
        dynamo_log("No font found at %s", aPath);
        return NULL;
    }
    
    Font_t *out = obj_create_autoreleased(&Class_Font);
    out->glyphs = dict_create(NULL, &free);
    
    int maxLen = 1024;
    char *line = malloc(maxLen);
    int charCount = -1;
        
    while(fgets(line, maxLen, fd) != NULL) {
        if(strstr(line, "info face") == line) {
            char *face, *charset;
            assert(sscanf(line, "info face=\"%s\"", face));
            assert(sscanf(line, "size=%f", &out->info.size));
            assert(sscanf(line, "bold=%d", &out->info.isBold));
            assert(sscanf(line, "italic=%d", &out->info.isItalic));
            assert(sscanf(line, "charset=\"%s\"", charset));
            assert(sscanf(line, "unicode=%d", &out->info.isUnicode));
            assert(sscanf(line, "stretchH=%f", &out->info.heightStretch));
            assert(sscanf(line, "smooth=%d", &out->info.isSmooth));
            assert(sscanf(line, "aa=%d", &out->info.isAntiAliased));
            assert(sscanf(line, "padding=%f,%f,%f,%f", &out->info.padding.x, &out->info.padding.y, &out->info.padding.z, &out->info.padding.w));
            assert(sscanf(line, "spacing=%f,%f", &out->info.spacing.x, &out->info.spacing.y));
            
            out->info.face = strdup(face);
            out->info.charset = strdup(charset);
        } else if(strstr(line, "common lineHeight") == line) {
            FontPage_t page;
            assert(sscanf(line, "id=%d", &page.startId));
            char *filename;
            assert(sscanf(line, "filename=%d", &filename));
            Texture_t *tex = texture_loadFromPng(filename, false, false);
            assert(tex);
            page.atlas = texAtlas_create(tex, GLMVec2_zero, GLMVec2_zero);
        } else if(strstr(line, "chars count") == line) {
            assert(sscanf(line, "count=%d", &charCount));
        } else if(strstr(line, "char") == line) {
            assert(charCount != -1);
            FontGlyph_t *glyph = malloc(sizeof(FontGlyph_t));
            assert(sscanf(line, "char id=%d", &glyph->id));
            assert(sscanf(line, "x=%f", &glyph->location.u));
            assert(sscanf(line, "y=%f", &glyph->location.v));
            assert(sscanf(line, "width=%f", &glyph->size.w));
            assert(sscanf(line, "height=%f", &glyph->size.h));
            assert(sscanf(line, "xoffset=%f", &glyph->offset.x));
            assert(sscanf(line, "yoffset=%f", &glyph->offset.y));
            assert(sscanf(line, "xadvance=%f", &glyph->xadvance));
            assert(sscanf(line, "page=%f", &glyph->page));
            assert(sscanf(line, "chnl=%f", &glyph->channel));
            
            dict_set(out->glyphs, _itoa(glyph->id), glyph);
        } else if(strstr(line, "kernings count") == line) {
            // TODO: Support kerning
        } else if(strstr(line, "kerning first") == line) {
            // TODO: Support kerning
        }
    }
    fclose(fd);
    
    return out;
}

void _font_destroy(Font_t *aFont)
{
    obj_release(aFont->atlas);
    obj_release(aFont->glyphs);
}

int font_kerning(Font_t *aFont, char a, char b)
{
    // TODO: Support kerning
    return 0;
}

SpriteBatch_t *font_renderableForString(Font_t *aFont, const char *aStr, size_t aLen)
{
    SpriteBatch_t *out = spriteBatch_create();

    FontGlyph_t *glyph;
    for(int i = 0; i < aLen; ++i, ++aStr) {
        glyph = dict_get(aFont->glyphs, "");
        if(!glyph)
            continue;
        
    }
    return out;
}

#define INT_DIGITS 19		// enough for a 64 bit integer
char *_itoa(int i) // Borrowed from groff
{
    // Room for INT_DIGITS digits, - and '\0'
    static char buf[INT_DIGITS + 2];
    char *p = buf + INT_DIGITS + 1;	// points to terminating '\0' 
    if (i >= 0) {
        do {
            *--p = '0' + (i % 10);
            i /= 10;
        } while (i != 0);
        return p;
    } else { // i < 0
        do {
            *--p = '0' - (i % 10);
            i /= 10;
        } while (i != 0);
        *--p = '-';
    }
    return p;
}