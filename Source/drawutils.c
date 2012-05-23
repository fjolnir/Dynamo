#include "drawutils.h"
#include "util.h"

Shader_t *gTexturedShader = NULL;
Shader_t *gColoredShader = NULL;
static Renderer_t *_renderer = NULL;
static const vec4_t kColorWhite = { 1.0f, 1.0f, 1.0f, 1.0f };

void draw_init(Renderer_t *aDefaultRenderer)
{
	_renderer = aDefaultRenderer;
	const int maxLen = 1024;
	char texturedVSH[maxLen], texturedFSH[maxLen], coloredVSH[maxLen], coloredFSH[maxLen];

	dynamo_assert(util_pathForResource("textured", "vsh", "DynamoShaders", texturedVSH, maxLen), "Could not find textured.vsh");
	dynamo_assert(util_pathForResource("textured", "fsh", "DynamoShaders", texturedFSH, maxLen), "Could not find textured.fsh");
	dynamo_assert(util_pathForResource("colored", "vsh", "DynamoShaders", coloredVSH, maxLen), "Could not find colored.vsh");
	dynamo_assert(util_pathForResource("colored", "fsh", "DynamoShaders", coloredFSH, maxLen), "Could not find colored.fsh");

	if(!gTexturedShader) {
		gTexturedShader = obj_retain(shader_loadFromFiles((const char*)texturedVSH, (const char*)texturedFSH));
		gTexturedShader->uniforms[kShader_colorUniform] = shader_getUniformLocation(gTexturedShader, "u_color");
	}
	if(!gColoredShader) {
		gColoredShader = obj_retain(shader_loadFromFiles((const char*)coloredVSH, (const char*)coloredFSH));
		gColoredShader->attributes[kShader_colorAttribute] = shader_getAttributeLocation(gColoredShader, "a_color");
	}
}

void draw_cleanup()
{
	if(gTexturedShader) obj_release(gTexturedShader);
	if(gColoredShader) obj_release(gColoredShader);
}


#pragma mark - Texture drawing

void draw_quad(vec3_t aCenter, vec2_t aSize, Texture_t *aTexture, TextureRect_t aTextureArea, vec4_t aColor, float aAngle, bool aFlipHorizontal, bool aFlipVertical)
{
	GLfloat vertices[4*3] = {
		0.0f,    0.0f,    0.0f,
		0.0f,    aSize.h, 0.0f,
		aSize.w, 0.0f,    0.0f,
		aSize.w, aSize.h, 0.0f
	};

	float maxTexX = aTextureArea.origin.x + aTextureArea.size.w;
	float maxTexY = aTextureArea.origin.y + aTextureArea.size.h;
	GLfloat texCoords[4*2] = {
		aFlipHorizontal ? maxTexX : aTextureArea.origin.x, aFlipVertical ? maxTexY : aTextureArea.origin.y,
		aFlipHorizontal ? maxTexX : aTextureArea.origin.x, aFlipVertical ? aTextureArea.origin.y : maxTexY,
		aFlipHorizontal ? aTextureArea.origin.x  :maxTexX, aFlipVertical ? maxTexY : aTextureArea.origin.y,
		aFlipHorizontal ? aTextureArea.origin.x : maxTexX, aFlipVertical ? aTextureArea.origin.y : maxTexY
	};

	// Translate&rotate the quad into it's target location
	aCenter = vec3_floor(aCenter);
	matrix_stack_push(_renderer->worldMatrixStack);
	matrix_stack_translate(_renderer->worldMatrixStack, aCenter.x, aCenter.y, aCenter.z);
	matrix_stack_rotate(_renderer->worldMatrixStack, aAngle, 0.0f, 0.0f, 1.0f);
	matrix_stack_translate(_renderer->worldMatrixStack, floor(aSize.w/-2.0f), floor(aSize.h/-2.0f), 0.0f);

	shader_makeActive(gTexturedShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, aTexture->id);

	shader_updateMatrices(gTexturedShader, _renderer);
	glUniform1i(gTexturedShader->uniforms[kShader_colormap0Uniform], 0);
	glUniform4fv(gTexturedShader->uniforms[kShader_colorUniform], 1, aColor.f);

	glVertexAttribPointer(gTexturedShader->attributes[kShader_positionAttribute], 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(gTexturedShader->attributes[kShader_positionAttribute]);
	glVertexAttribPointer(gTexturedShader->attributes[kShader_texCoord0Attribute], 2, GL_FLOAT, GL_FALSE, 0, texCoords);
	glEnableVertexAttribArray(gTexturedShader->attributes[kShader_texCoord0Attribute]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	shader_makeInactive(gTexturedShader);
	matrix_stack_pop(_renderer->worldMatrixStack);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void draw_texturePortion(vec3_t aCenter, Texture_t *aTexture, TextureRect_t aTextureArea, float aScale, float aAngle, bool aFlipHorizontal, bool aFlipVertical)
{
	vec2_t quadSize = vec2_mul(aTexture->size, aTextureArea.size);
	quadSize = vec2_scalarMul(quadSize, aScale);
	draw_quad(aCenter, quadSize, aTexture, aTextureArea, kColorWhite, aAngle, aFlipHorizontal, aFlipVertical);
}

void draw_texture(vec3_t aCenter, Texture_t *aTexture, float aScale, float aAngle, bool aFlipHorizontal, bool aFlipVertical)
{
	draw_texturePortion(aCenter, aTexture, kTextureRectEntire, aScale, aAngle, aFlipHorizontal, aFlipVertical);
}

void draw_textureAtlas_getVertices(TextureAtlas_t *aAtlas, int aNumberOfTiles, vec2_t *aOffsets, vec2_t *aCenterPoints,
	vec2_t **aoVertices, vec2_t **aoTexCoords, int *aoNumberOfVertices, GLuint **aoIndices, int *aoNumberOfIndices)
{
	int numberOfVertices = 4*aNumberOfTiles;
	int numberOfIndices = 6*aNumberOfTiles;
	vec2_t *vertices = calloc(numberOfVertices, sizeof(vec3_t));
	vec2_t *texCoords = calloc(numberOfVertices, sizeof(vec2_t));
	GLuint *indices = calloc(numberOfIndices, sizeof(GLuint));
	
	int lastIndex = 0;
	TextureRect_t currTexRect;
	for(int i = 0; i < aNumberOfTiles; ++i) {
		currTexRect = texAtlas_getTextureRect(aAtlas, (int)aOffsets[i].x, (int)aOffsets[i].y);

		vertices[(4*i)+0].x = aCenterPoints[i].x - aAtlas->size.x/2.0f; vertices[(4*i)+0].y = aCenterPoints[i].y - aAtlas->size.y/2.0f;
		vertices[(4*i)+1].x = aCenterPoints[i].x - aAtlas->size.x/2.0f; vertices[(4*i)+1].y = aCenterPoints[i].y + aAtlas->size.y/2.0f;
		vertices[(4*i)+2].x = aCenterPoints[i].x + aAtlas->size.x/2.0f; vertices[(4*i)+2].y = aCenterPoints[i].y + aAtlas->size.y/2.0f;
		vertices[(4*i)+3].x = aCenterPoints[i].x + aAtlas->size.x/2.0f; vertices[(4*i)+3].y = aCenterPoints[i].y - aAtlas->size.y/2.0f;

		texCoords[(4*i)+0]   = currTexRect.origin;
		texCoords[(4*i)+1].u = currTexRect.origin.u;                      texCoords[(4*i)+1].v = currTexRect.origin.v + currTexRect.size.v;
		texCoords[(4*i)+2].u = currTexRect.origin.u + currTexRect.size.u; texCoords[(4*i)+2].v = currTexRect.origin.v + currTexRect.size.v;
		texCoords[(4*i)+3].u = currTexRect.origin.u + currTexRect.size.u; texCoords[(4*i)+3].v = currTexRect.origin.v;

		indices[lastIndex++] = (4*i)+0;
		indices[lastIndex++] = (4*i)+1;
		indices[lastIndex++] = (4*i)+2;
		indices[lastIndex++] = (4*i)+0;
		indices[lastIndex++] = (4*i)+2;
		indices[lastIndex++] = (4*i)+3;
	}
	
	if(aoVertices) *aoVertices = vertices;
	if(aoTexCoords) *aoTexCoords = texCoords;
	if(aoNumberOfVertices) *aoNumberOfVertices = numberOfVertices;
	if(aoIndices) *aoIndices = indices;
	if(aoNumberOfIndices) *aoNumberOfIndices = numberOfIndices;
}

void draw_textureAtlas(TextureAtlas_t *aAtlas, int aNumberOfTiles, vec2_t *aOffsets, vec2_t *aCenterPoints)
{
	int numberOfVertices;
	int numberOfIndices;
	vec2_t *vertices;
	vec2_t *texCoords;
	GLuint *indices;

	draw_textureAtlas_getVertices(aAtlas, aNumberOfTiles, aOffsets, aCenterPoints, &vertices, &texCoords, &numberOfVertices, &indices, &numberOfIndices);

	matrix_stack_push(_renderer->worldMatrixStack);

	shader_makeActive(gTexturedShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, aAtlas->texture->id);

	shader_updateMatrices(gTexturedShader, _renderer);
	glUniform1i(gTexturedShader->uniforms[kShader_colormap0Uniform], 0);
	vec4_t white = {1.0, 1.0, 1.0, 1.0};
	glUniform4fv(gTexturedShader->uniforms[kShader_colorUniform], 1, white.f);

	glVertexAttribPointer(gTexturedShader->attributes[kShader_positionAttribute], 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(gTexturedShader->attributes[kShader_positionAttribute]);
	glVertexAttribPointer(gTexturedShader->attributes[kShader_texCoord0Attribute], 2, GL_FLOAT, GL_FALSE, 0, texCoords);
	glEnableVertexAttribArray(gTexturedShader->attributes[kShader_texCoord0Attribute]);

	glDrawElements(GL_TRIANGLES, numberOfIndices, GL_UNSIGNED_INT, indices);

	shader_makeInactive(gTexturedShader);
	matrix_stack_pop(_renderer->worldMatrixStack);

	glBindTexture(GL_TEXTURE_2D, 0);

	free(vertices);
	free(texCoords);
	free(indices);
}


#pragma mark - Primitive drawing (Debug drawing)

void draw_rect(vec2_t aCenter, vec2_t aSize, float aAngle, vec4_t aColor, bool aShouldFill)
{
    vec2_t size = vec2_floor(aSize);
	size = vec2_floor(size);
	GLfloat vertices[4*2] = {
		-size.w/2.0, -aSize.h/2.0,
		-size.w/2.0,  aSize.h/2.0,
		 size.w/2.0,  size.h/2.0,
		 size.w/2.0, -aSize.h/2.0
	};
	vec4_t colors[4] = { aColor, aColor, aColor, aColor };

	// Translate&rotate the rectangle into it's target location
	matrix_stack_push(_renderer->worldMatrixStack);
	matrix_stack_rotate(_renderer->worldMatrixStack, aAngle, 0.0f, 0.0f, 1.0f);
	matrix_stack_translate(_renderer->worldMatrixStack, floorf(aCenter.x), floorf(aCenter.y), 0.0);
    
	shader_makeActive(gColoredShader);

	shader_updateMatrices(gColoredShader, _renderer);

	glVertexAttribPointer(gColoredShader->attributes[kShader_positionAttribute], 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(gColoredShader->attributes[kShader_positionAttribute]);
	glVertexAttribPointer(gColoredShader->attributes[kShader_colorAttribute], 4, GL_FLOAT, GL_FALSE, 0, colors);
	glEnableVertexAttribArray(gColoredShader->attributes[kShader_colorAttribute]);

	glDrawArrays(aShouldFill ? GL_TRIANGLE_FAN : GL_LINE_LOOP, 0, 4);

	shader_makeInactive(gColoredShader);
	matrix_stack_pop(_renderer->worldMatrixStack);

}

void draw_ellipse(vec2_t aCenter, vec2_t aRadii, int aSubdivisions, float aAngle, vec4_t aColor, bool aShouldFill)
{
	aSubdivisions = MAX(6, aSubdivisions);
	float vertices[aSubdivisions*2] ;
	vec4_t colors[aSubdivisions];
	float twoPi = 2.0f*M_PI;
	int count = 0;
	for(float theta = 0.0f; (twoPi - theta) > 0.001; theta += twoPi/(float)aSubdivisions) {
		colors[count/2] = aColor;
		vertices[count++] = cosf(theta) * aRadii.w;
		vertices[count++] = sinf(theta) * aRadii.h;
	}


	// Translate&rotate the ellipse into it's target location
	matrix_stack_push(_renderer->worldMatrixStack);
	matrix_stack_translate(_renderer->worldMatrixStack, floorf(aCenter.x), floorf(aCenter.y), 0.0);
	matrix_stack_rotate(_renderer->worldMatrixStack, aAngle, 0.0f, 0.0f, 1.0f);

	shader_makeActive(gColoredShader);
	shader_updateMatrices(gColoredShader, _renderer);

	glVertexAttribPointer(gColoredShader->attributes[kShader_positionAttribute], 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(gColoredShader->attributes[kShader_positionAttribute]);
	glVertexAttribPointer(gColoredShader->attributes[kShader_colorAttribute], 4, GL_FLOAT, GL_FALSE, 0, colors);
	glEnableVertexAttribArray(gColoredShader->attributes[kShader_colorAttribute]);

	glDrawArrays(aShouldFill ? GL_TRIANGLE_FAN : GL_LINE_LOOP, 0, aSubdivisions);

	shader_makeInactive(gColoredShader);
	matrix_stack_pop(_renderer->worldMatrixStack);
}

void draw_circle(vec2_t aCenter, float aRadius, int aSubdivisions, vec4_t aColor, bool aShouldFill)
{
	vec2_t size = { aRadius, aRadius };
	draw_ellipse(aCenter, size, aSubdivisions, 0.0f, aColor, aShouldFill);
}
void draw_circleShowingAngle(vec2_t aCenter, float aRadius, float aAngle, int aSubdivisions, vec4_t aColor, bool aShouldFill)
{
    draw_circle(aCenter, aRadius, aSubdivisions, aColor, aShouldFill);
    
    vec3_t edgePoint = vec3_create(aRadius, 0, 0);
    quat_t quat = quat_createf(0, 0, 1, aAngle);
    edgePoint = quat_rotateVec3(quat, edgePoint);
    draw_lineSeg(aCenter, vec2_add(aCenter, edgePoint.xy), vec4_create(1, 1, 1, 1));
}

void draw_point(vec2_t aLocation, vec4_t aColor)
{
    draw_circle(aLocation, 4, 6, aColor, true);
}

void draw_polygon(int aNumberOfVertices, vec2_t *aVertices, vec4_t aColor, bool aShouldFill)
{
	vec4_t colors[aNumberOfVertices];
	for(int i = 0; i < aNumberOfVertices; ++i) colors[i] = aColor;

	shader_makeActive(gColoredShader);
	shader_updateMatrices(gColoredShader, _renderer);

	glVertexAttribPointer(gColoredShader->attributes[kShader_positionAttribute], 2, GL_FLOAT, GL_FALSE, 0, aVertices);
	glEnableVertexAttribArray(gColoredShader->attributes[kShader_positionAttribute]);
	glVertexAttribPointer(gColoredShader->attributes[kShader_colorAttribute], 4, GL_FLOAT, GL_FALSE, 0, colors);
	glEnableVertexAttribArray(gColoredShader->attributes[kShader_colorAttribute]);

	glDrawArrays(aShouldFill ? GL_TRIANGLE_FAN : GL_LINE_LOOP, 0, aNumberOfVertices);

	shader_makeInactive(gColoredShader);
}

void draw_lineSeg(vec2_t aPointA, vec2_t aPointB, vec4_t aColor)
{
	vec2_t vertices[2] = { vec2_floor(aPointA), vec2_floor(aPointB) };
	vec4_t colors[2] = { aColor, aColor };

	shader_makeActive(gColoredShader);
	shader_updateMatrices(gColoredShader, _renderer);

	glVertexAttribPointer(gColoredShader->attributes[kShader_positionAttribute], 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(gColoredShader->attributes[kShader_positionAttribute]);
	glVertexAttribPointer(gColoredShader->attributes[kShader_colorAttribute], 4, GL_FLOAT, GL_FALSE, 0, colors);
	glEnableVertexAttribArray(gColoredShader->attributes[kShader_colorAttribute]);

	glDrawArrays(GL_LINE_STRIP, 0, 2);

	shader_makeInactive(gColoredShader);
}

#pragma mark - World shape debug drawing

void draw_worldShape(WorldShape_t *aShape, WorldEntity_t *aEntity, bool aDrawBB)
{   
    if(aDrawBB) {
        vec4_t bbColor = { 0,1,0,1 };
        cpBB bb = aShape->cpShape->bb;
        vec2_t verts[] = {
            { bb.l, bb.b },
            { bb.l, bb.t },
            { bb.r, bb.t },
            { bb.r, bb.b }
        };
        draw_polygon(4, verts, bbColor, false);
    }
    vec4_t shapeColor = { 0.4, 0.4, 0.8,1 };
    switch(aShape->cpShape->klass_private->type) {
        case CP_CIRCLE_SHAPE: {
            GLMFloat angle = aEntity->cpBody->a;
            cpCircleShape *circle = (cpCircleShape *)aShape->cpShape;
            draw_circleShowingAngle(*(vec2_t*)&circle->tc, circle->r, angle, 30, shapeColor, true);
            break;
        } case CP_POLY_SHAPE: {
            cpPolyShape *poly = (cpPolyShape *)aShape->cpShape;
            draw_polygon(poly->numVerts, (vec2_t*)poly->tVerts, shapeColor, true);
            break;
        } case CP_SEGMENT_SHAPE: {
            cpSegmentShape *seg = (cpSegmentShape *)aShape->cpShape;
            draw_lineSeg(*(vec2_t *)&seg->ta, *(vec2_t *)&seg->tb, shapeColor);
            break;
        } default:
            break;
    }
}

struct _World_drawCtx { WorldEntity_t *entity; bool shouldDrawBB; };
static void _drawShapeHandler(WorldShape_t *aShape, struct _World_drawCtx *aCtx)
{
    draw_worldShape(aShape, aCtx->entity, aCtx->shouldDrawBB);
}
void draw_worldEntity(WorldEntity_t *aEntity, bool aDrawBB)
{
    struct _World_drawCtx ctx = { aEntity, aDrawBB };
    llist_apply(aEntity->shapes, (LinkedListApplier_t)_drawShapeHandler, &ctx);
}

static void _drawConstraint(cpConstraint *constraint, void *data)
{
    cpBody *body_a = constraint->a;
	cpBody *body_b = constraint->b;
    
    vec4_t color = vec4_create(1, 0, 0, 1);
    const cpConstraintClass *klass = constraint->klass_private;
	if(klass == cpPinJointGetClass()){
		cpPinJoint *joint = (cpPinJoint *)constraint;
        
		cpVect a = cpvadd(body_a->p, cpvrotate(joint->anchr1, body_a->rot));
		cpVect b = cpvadd(body_b->p, cpvrotate(joint->anchr2, body_b->rot));
        draw_lineSeg(vec2_create(a.x, a.y), vec2_create(b.x, b.y), color);
	} else if(klass == cpSlideJointGetClass()){
		cpSlideJoint *joint = (cpSlideJoint *)constraint;
        
		cpVect a = cpvadd(body_a->p, cpvrotate(joint->anchr1, body_a->rot));
		cpVect b = cpvadd(body_b->p, cpvrotate(joint->anchr2, body_b->rot));
        draw_lineSeg(vec2_create(a.x, a.y), vec2_create(b.x, b.y), color);
	} else if(klass == cpPivotJointGetClass()){
		cpPivotJoint *joint = (cpPivotJoint *)constraint;
		cpVect a = cpvadd(body_a->p, cpvrotate(joint->anchr1, body_a->rot));
		cpVect b = cpvadd(body_b->p, cpvrotate(joint->anchr2, body_b->rot));
        draw_point(vec2_create(a.x, a.y), color);
        draw_point(vec2_create(b.x, b.y), color);
	} else if(klass == cpGrooveJointGetClass()){
		cpGrooveJoint *joint = (cpGrooveJoint *)constraint;
        
		cpVect a = cpvadd(body_a->p, cpvrotate(joint->grv_a, body_a->rot));
		cpVect b = cpvadd(body_a->p, cpvrotate(joint->grv_b, body_a->rot));
		cpVect c = cpvadd(body_b->p, cpvrotate(joint->anchr2, body_b->rot));
        draw_lineSeg(vec2_create(a.x, a.y), vec2_create(b.x, b.y), color);
        draw_lineSeg(vec2_create(b.x, b.y), vec2_create(c.x, c.y), color);
	} else if(klass == cpDampedSpringGetClass()){
		//drawSpring((cpDampedSpring *)constraint, body_a, body_b);
	}
}

static void _drawEntityHandler(WorldEntity_t *aEntity, struct _World_drawCtx *aCtx)
{
    draw_worldEntity(aEntity, aCtx->shouldDrawBB);
}
void draw_world(World_t *aWorld, bool aDrawBB)
{
    draw_worldEntity(aWorld->staticEntity, aDrawBB);
    struct _World_drawCtx ctx = { NULL, aDrawBB };
    llist_apply(aWorld->entities, (LinkedListApplier_t)&_drawEntityHandler, &ctx);
    cpSpaceEachConstraint(aWorld->cpSpace, &_drawConstraint, NULL);
}
