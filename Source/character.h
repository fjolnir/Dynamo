#include "engine/sprite.h"
#include "collision.h"
#include "engine/object.h"

#ifndef _CHARACTER_H
#define _CHARACTER_H

typedef struct _Character {
	OBJ_GUTS
	Sprite_t *sprite;
	CollisionPolyObject_t *collisionObject;
	vec2_t spriteOffset;
} Character_t;

extern Character_t *character_create();
#endif

