// Utilities for simple drawing

#include "GLMath/GLMath.h"
#include "texture.h"
#include "texture_atlas.h"
#include "shader.h"
#include "renderer.h"

#ifndef __DRAWUTILS_H_
#define __DRAWUTILS_H_

#define kDraw_defaultEllipseSubDivs 25

extern  Shader_t *gTexturedShader;
extern Shader_t *gColoredShader;

// Initializes the drawing utils
extern void draw_init(Renderer_t *aDefaultRenderer);
extern void draw_cleanup();

// Draws a textured quad on the X/Y plane
extern void draw_quad(vec3_t aCenter, vec2_t aSize, Texture_t *aTexture, TextureRect_t aTextureArea, vec4_t aColor, float aAngle, bool aFlipHorizontal, bool aFlipVertical);

// Draws a specified portion of a texture onto a quad of the same size as the portion sampled
extern void draw_texturePortion(vec3_t aCenter, Texture_t *aTexture, TextureRect_t aTextureArea, float aScale, float aAngle, bool aFlipHorizontal, bool aFlipVertical);

// Draws a texture onto a quad of the same size
extern void draw_texture(vec3_t aCenter, Texture_t *aTexture, float aScale, float aAngle, bool aFlipHorizontal, bool aFlipVertical);

// Draws multiple subtextures at different locations using a texture atlas (In a single draw call, useful for performing
// multiple simple draws, such as when drawing a tiled level)
// aOffsets: an array of [x,y] offsets in the texture atlas (Cast to int)
// aCenterPoints: an array of points to draw the tiles at
extern void draw_textureAtlas(TextureAtlas_t *aAtlas, int aNumberOfTiles, vec2_t *aOffsets, vec2_t *aCenterPoints);

// Generates the vertices used by the above function (use this if you want to store your vertices in a VBO when handling
// larger meshes
extern void draw_textureAtlas_getVertices(TextureAtlas_t *aAtlas, int aNumberOfTiles, vec2_t *aOffsets, vec2_t *aCenterPoints,
	vec2_t **aoVertices, vec2_t **aoTexCoords, int *aoNumberOfVertices, GLuint **aoIndices, int *aoNumberOfIndices);


// Draws an untextured rectangle
extern void draw_rect(rect_t aRect, float aAngle, vec4_t aColor, bool aShouldFill);

// Draws an untextured ellipse
extern void draw_ellipse(vec2_t aCenter, vec2_t aRadii, int aSubdivisions, float aAngle, vec4_t aColor, bool aShouldFill);

// Draws an untextured circle
extern void draw_circle(vec2_t aCenter, float radius, int aSubdivisions, vec4_t aColor, bool aShouldFill);

// Draws an untextured polygon
extern void draw_polygon(int aNumberOfVertices, vec2_t *aVertices, vec4_t aColor, bool aShouldFill);

// Draws a line segment
extern void draw_lineSeg(vec2_t aPointA, vec2_t aPointB, vec4_t aColor);
#endif
