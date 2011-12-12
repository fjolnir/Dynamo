#include "engine/sprite.h"
#include "collision.h"

#ifndef _CHARACTER_H
#define _CHARACTER_H

typedef struct _Character {
	Sprite_t *sprite;
	CollisionPolyObject_t *collisionObject;
} Character_t;

extern Character_t *character_create();
extern void character_destroy(Character_t *aCharacter);
#endif

