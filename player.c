//
// Created by thiabaud on 16/05/20.
//

#include "player.h"
#include "constants.h"

#define PL_MOVE_SPEED 5

void pl_handle_input(struct Player *player, SDL_Event *event) {
	if (event->type == SDL_KEYDOWN && event->key.repeat == 0) {
		switch (event->key.keysym.sym) {
		case SDLK_UP:
			player->v_y = -PL_MOVE_SPEED;
			break;
		case SDLK_DOWN:
			player->v_y = +PL_MOVE_SPEED;
			break;
		case SDLK_LEFT:
			player->v_x = -PL_MOVE_SPEED;
			break;
		case SDLK_RIGHT:
			player->v_x = +PL_MOVE_SPEED;
			break;
		}
	}

	if (event->type == SDL_KEYUP && event->key.repeat == 0) {
		switch (event->key.keysym.sym) {
		case SDLK_UP:
		case SDLK_DOWN:
			player->v_y = 0;
			break;
		case SDLK_LEFT:
		case SDLK_RIGHT:
			player->v_x = 0;
			break;
		}
	}
}

void pl_update(struct Player *player) {
	player->rect.x += player->v_x;
	player->rect.y += player->v_y;
	if (player->rect.x < 0 || player->rect.x + player->rect.w > SCREEN_WIDTH) player->rect.x -= player->v_x;
	if (player->rect.y < 0 || player->rect.y + player->rect.h > SCREEN_HEIGHT) player->rect.y -= player->v_y;
}