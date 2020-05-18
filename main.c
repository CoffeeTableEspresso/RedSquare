#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <yasl/yasl.h>

#include <stdio.h>
#include <stdbool.h>

#include "player.h"
#include "constants.h"
#include "vect.h"

VECT_DECL(SDL_Rect)

// #define ASSET(x) ("assets/" #x)

/*
enum KeyPressSurfaces
{
	KEY_PRESS_SURFACE_DEFAULT,
	KEY_PRESS_SURFACE_UP,
	KEY_PRESS_SURFACE_DOWN,
	KEY_PRESS_SURFACE_LEFT,
	KEY_PRESS_SURFACE_RIGHT,
	KEY_PRESS_SURFACE_TOTAL
};
*/

struct Window_State {
	SDL_Window *window;
	SDL_Renderer *renderer;
	// SDL_Surface *screen;
	// SDL_Texture *current;
	// SDL_Texture *key_press_surfaces[KEY_PRESS_SURFACE_TOTAL];
};

bool winit(struct Window_State *state) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL Could not be initialized: %s\n", SDL_GetError());
		return false;
	}

	if (!SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		fprintf(stderr, "Linear texture filtering could not be enabled: %s\n", SDL_GetError());
		return false;
	}

	state->window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
					 SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (!state->window) {
		fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
		return false;
	}

	state->renderer = SDL_CreateRenderer( state->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!state->renderer) {
		fprintf(stderr, "Renderer could not be created: %s\n", SDL_GetError());
		return false;
	}

	SDL_SetRenderDrawColor(state->renderer, 0xFF, 0xFF, 0xFF, 0XFF);

	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)) {
		fprintf(stderr, "SDL_image could not be initialized: %s\n", IMG_GetError());
		return false;
	}

	if (TTF_Init() == -1) {
		fprintf(stderr, "SDL_ttf could not be initialized: %s\n", TTF_GetError());
		return false;
	}

	// state->screen = SDL_GetWindowSurface(state->window);
	return true;
}

/*
SDL_Texture *load_texture(struct Window_State *state, const char *path) {
	SDL_Surface *surface = IMG_Load(path);
	if (!surface) {
		fprintf(stderr, "Unable to load image %s: %s\n", path, IMG_GetError());
		return NULL;
	}

	SDL_Texture *texture = SDL_CreateTextureFromSurface(state->renderer, surface);
	SDL_FreeSurface(surface);
	if (!texture) {
		fprintf(stderr, "Unable to create texture from %s: %s\n", path, IMG_GetError());
	}

	return texture;
}
*/

/*
bool load_media(struct Window_State *state) {
	state->key_press_surfaces[KEY_PRESS_SURFACE_DEFAULT] = load_texture(state, ASSET(loaded.png));
	state->key_press_surfaces[KEY_PRESS_SURFACE_UP] = load_texture(state, ASSET(up.bmp));
	state->key_press_surfaces[KEY_PRESS_SURFACE_DOWN] = load_texture(state, ASSET(down.bmp));
	state->key_press_surfaces[KEY_PRESS_SURFACE_LEFT] = load_texture(state, ASSET(left.bmp));
	state->key_press_surfaces[KEY_PRESS_SURFACE_RIGHT] = load_texture(state, ASSET(right.bmp));

	state->current = state->key_press_surfaces[KEY_PRESS_SURFACE_DEFAULT];
	for (unsigned i = 0; i < KEY_PRESS_SURFACE_TOTAL; i++) {
		if (!state->key_press_surfaces[i]) return false;
	}
	return true;
}
*/

void wclose(struct Window_State *state) {
	//for (unsigned i = 0; i < KEY_PRESS_SURFACE_TOTAL; i++) {
	//	SDL_DestroyTexture(state->key_press_surfaces[i]);
	//}

	SDL_DestroyRenderer(state->renderer);
	SDL_DestroyWindow(state->window);

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

bool collide(SDL_Rect *a, SDL_Rect *b) {
	return (a->x) < (b->x + b->w) &&
		(a->x + a->w) > (b->x) &&
		(a->y + a->h) > (b->y) &&
		(a->y) < (b->y + b->h);
}

bool collide_any(Vect(SDL_Rect) *v, SDL_Rect *r) {
	for (size_t i = 0; i < v->count; i++) {
		if (collide(&v->items[i], r)) return true;
	}
	return false;
}

static struct Player player;
static Vect(SDL_Rect) enemies;
static SDL_Rect goal;

int AddPlayer(struct YASL_State *S) {
	yasl_int y = YASL_popint(S);
	yasl_int x = YASL_popint(S);
	player.rect.x = x;
	player.rect.y = y;
	return YASL_SUCCESS;
}

int AddEnemy(struct YASL_State *S) {
	yasl_int h = YASL_popint(S);
	yasl_int w = YASL_popint(S);
	yasl_int y = YASL_popint(S);
	yasl_int x = YASL_popint(S);
	v_push(SDL_Rect)(&enemies, (SDL_Rect){ x, y, w, h});
	return YASL_SUCCESS;
}

int AddGoal(struct YASL_State *S) {
	yasl_int y = YASL_popint(S);
	yasl_int x = YASL_popint(S);
	goal.x = x;
	goal.y = y;
	return YASL_SUCCESS;
}

int main(int argc, char *argv[]) {
	struct Window_State state;
	winit(&state);

	// load_media(&state);

	bool quit = false;
	SDL_Event event_handler;

	while (!quit) {
		bool dead = false;
		enemies = v_init(SDL_Rect)();
		player.rect = (SDL_Rect) {0, 0, SCREEN_WIDTH / 25, SCREEN_WIDTH / 25};
		goal = (SDL_Rect) {0, 0, SCREEN_WIDTH / 50, SCREEN_WIDTH / 50};

		struct YASL_State *S = YASL_newstate("level.yasl");
		if (!S) {
			fprintf(stderr, "Failed to load level\n");
		}

		YASL_declglobal(S, "SCREEN_WIDTH");
		YASL_pushinteger(S, SCREEN_WIDTH);
		YASL_setglobal(S, "SCREEN_WIDTH");

		YASL_declglobal(S, "SCREEN_HEIGHT");
		YASL_pushinteger(S, SCREEN_HEIGHT);
		YASL_setglobal(S, "SCREEN_HEIGHT");

		YASL_declglobal(S, "AddPlayer");
		YASL_pushcfunction(S, AddPlayer, 2);
		YASL_setglobal(S, "AddPlayer");

		YASL_declglobal(S, "AddEnemy");
		YASL_pushcfunction(S, AddEnemy, 4);
		YASL_setglobal(S, "AddEnemy");

		YASL_declglobal(S, "AddGoal");
		YASL_pushcfunction(S, AddGoal, 2);
		YASL_setglobal(S, "AddGoal");

		YASL_execute(S);

		while (!dead && !quit) {
			while (SDL_PollEvent(&event_handler) != 0) {
				if (event_handler.type == SDL_QUIT) quit = true;
				pl_handle_input(&player, &event_handler);
			}

			pl_update(&player);

			SDL_SetRenderDrawColor(state.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderClear(state.renderer);
			SDL_SetRenderDrawColor(state.renderer, 0x00, 0x00, 0x00, 0xFF);
			for (size_t i = 0; i < enemies.count; i++) {
				SDL_RenderFillRect(state.renderer, &enemies.items[i]);
			}
			SDL_SetRenderDrawColor(state.renderer, 0x00, 0xFF, 0x00, 0xFF);
			SDL_RenderFillRect(state.renderer, &goal);
			SDL_SetRenderDrawColor(state.renderer, 0xFF, 0x00, 0x00, 0xFF);
			SDL_RenderFillRect(state.renderer, &player.rect);
			SDL_RenderPresent(state.renderer);

			if (collide_any(&enemies, &player.rect)) {
				// TODO move this set up to init.
				SDL_SetRenderDrawColor(state.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(state.renderer);
				SDL_SetRenderDrawColor(state.renderer, 0x88, 0x00, 0x00, 0xFF);
				SDL_RenderFillRect(state.renderer, &player.rect);
				SDL_Color color = {0xFF, 0x00, 0x00};
				TTF_Font *font = TTF_OpenFont("fonts/lazy.ttf", 20);
				SDL_Surface *text = TTF_RenderText_Solid(font, "YOU DIED :(", color);
				SDL_Texture *texture = SDL_CreateTextureFromSurface(state.renderer, text);
				SDL_FreeSurface(text);
				TTF_CloseFont(font);
				while (!quit && !dead) {
					while (SDL_PollEvent(&event_handler) != 0) {
						if (event_handler.type == SDL_QUIT) quit = true;
						if (event_handler.type == SDL_KEYDOWN &&
						    event_handler.key.repeat == 0 &&
						    event_handler.key.keysym.sym == SDLK_SPACE)
							dead = true;
						pl_handle_input(&player, &event_handler);
					}
					SDL_SetRenderDrawColor(state.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
					SDL_RenderClear(state.renderer);
					SDL_SetRenderDrawColor(state.renderer, 0x88, 0x00, 0x00, 0xFF);
					SDL_RenderFillRect(state.renderer, &player.rect);
					SDL_RenderCopy(state.renderer, texture, NULL, NULL);
					SDL_RenderPresent(state.renderer);
				}
			} else if (collide(&goal, &player.rect)) {
				// TODO move this set up to init.
				SDL_SetRenderDrawColor(state.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(state.renderer);
				SDL_SetRenderDrawColor(state.renderer, 0x00, 0x88, 0x00, 0xFF);
				SDL_RenderFillRect(state.renderer, &player.rect);
				SDL_Color color = {0x00, 0xFF, 0x00};
				TTF_Font *font = TTF_OpenFont("fonts/lazy.ttf", 20);
				SDL_Surface *text = TTF_RenderText_Solid(font, "YOU WIN :)", color);
				SDL_Texture *texture = SDL_CreateTextureFromSurface(state.renderer, text);
				SDL_FreeSurface(text);
				TTF_CloseFont(font);
				while (!quit && !dead) {
					while (SDL_PollEvent(&event_handler) != 0) {
						if (event_handler.type == SDL_QUIT) quit = true;
						if (event_handler.type == SDL_KEYDOWN &&
						    event_handler.key.repeat == 0 &&
						    event_handler.key.keysym.sym == SDLK_SPACE)
							dead = true;
						pl_handle_input(&player, &event_handler);
					}
					SDL_SetRenderDrawColor(state.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
					SDL_RenderClear(state.renderer);
					SDL_SetRenderDrawColor(state.renderer, 0x00, 0x88, 0x00, 0xFF);
					SDL_RenderFillRect(state.renderer, &player.rect);
					SDL_RenderCopy(state.renderer, texture, NULL, NULL);
					SDL_RenderPresent(state.renderer);
				}
			}
		}

		YASL_delstate(S);
		v_cleanup(SDL_Rect)(&enemies);
	}

	wclose(&state);
	return 0;
}

