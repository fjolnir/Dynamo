#include "engine/sprite.h"

#ifndef _CHARACTER_H
#define _CHARACTER_H

typedef struct _Character {
	Sprite_t *sprite;
} Character_t;

extern Character_t *character_create();
extern void character_destroy(Character_t *aCharacter);
#endif

