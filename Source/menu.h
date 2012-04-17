#include "engine/renderer.h"
#include "engine/texture_atlas.h"
#include "engine/input.h"
#include "engine/background.h"
#include "engine/object.h"

#ifndef _MENU_H_
#define _MENU_H_

typedef struct MainMenu MainMenu_t;

struct MainMenu {
	OBJ_GUTS
	Renderable_t renderable;
	TextureAtlas_t *itemAtlas;
	Background_t *background;
	int selectedItem;
	int numberOfItems;
	bool disabled;

	vec2_t backgroundVelocity;
	bool fadingOut;
	float opacity;

	InputObserver_t *arrowUpObserver;
	InputObserver_t *arrowDownObserver;
	InputObserver_t *enterObserver;

	void (*selectionCallback)(MainMenu_t *aMenu, int aSelection, void *metaData);
	void (*fadeCallback)(MainMenu_t *aMenu, void *metaData);
	void *metaData;
};

MainMenu_t *mainMenu_create();
void mainMenu_update(MainMenu_t *aMenu, double aTimeDelta);
void mainMenu_fadeOut(MainMenu_t *aMenu);

#endif
