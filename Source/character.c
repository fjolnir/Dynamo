#include "character.h"

Character_t *character_create()
{
	Character_t *out = malloc(sizeof(Character_t));
	out->collisionObject = NULL;
	out->sprite = NULL;
	out->spriteOffset = kVec2_zero;

	return out;
}

void character_destroy(Character_t *aCharacter)
{
	free(aCharacter);
}
