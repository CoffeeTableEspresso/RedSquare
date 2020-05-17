//
// Created by thiabaud on 16/05/20.
//

#ifndef SDL_EXAMPLE_PLAYER_H
#define SDL_EXAMPLE_PLAYER_H

#include "SDL2/SDL.h"

struct Player {
	SDL_Rect rect;
	int v_x, v_y;
};

void pl_handle_input(struct Player *player, SDL_Event *event);
void pl_update(struct Player *player);

#endif //SDL_EXAMPLE_PLAYER_H
