#include "character.h"

static void character_destroy(Character_t *aCharacter);

Character_t *character_create()
{
	Character_t *out = obj_create_autoreleased(sizeof(Character_t), (Obj_destructor_t)&character_destroy);
	out->collisionObject = NULL;
	out->sprite = NULL;
	out->spriteOffset = kVec2_zero;

	return out;
}

void character_destroy(Character_t *aCharacter)
{
	obj_release(aCharacter->sprite);
}
