#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>

#include "player.h"
#include "constants.h"

#define ASSET(x) ("assets/" #x)

enum KeyPressSurfaces
{
	KEY_PRESS_SURFACE_DEFAULT,
	KEY_PRESS_SURFACE_UP,
	KEY_PRESS_SURFACE_DOWN,
	KEY_PRESS_SURFACE_LEFT,
	KEY_PRESS_SURFACE_RIGHT,
	KEY_PRESS_SURFACE_TOTAL
};

struct Window_State {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Surface *screen;
	SDL_Texture *current;
	SDL_Texture *key_press_surfaces[KEY_PRESS_SURFACE_TOTAL];
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

	state->screen = SDL_GetWindowSurface(state->window);
	return true;
}

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

void wclose(struct Window_State *state) {
	for (unsigned i = 0; i < KEY_PRESS_SURFACE_TOTAL; i++) {
		SDL_DestroyTexture(state->key_press_surfaces[i]);
	}

	SDL_DestroyRenderer(state->renderer);
	SDL_DestroyWindow(state->window);

	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char *argv[]) {
	struct Window_State state;

	winit(&state);

	load_media(&state);

	bool quit = false;
	SDL_Event event_handler;

	const int step_size = 1;

	struct Player player = {
		{
			SCREEN_WIDTH / 2,
			SCREEN_HEIGHT / 2,
			SCREEN_WIDTH / 25,
			SCREEN_WIDTH / 25
		},
		0,
		0
	};

	while (!quit) {
		printf("(x,y): (%d, %d)\n", player.rect.x, player.rect.y);
		while (SDL_PollEvent(&event_handler) != 0) {
			if (event_handler.type == SDL_QUIT) quit = true;
			pl_handle_input(&player, &event_handler);
		}

		pl_update(&player);

		SDL_SetRenderDrawColor(state.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(state.renderer);
		SDL_SetRenderDrawColor(state.renderer, 0xFF, 0x00, 0x00, 0xFF);
		SDL_RenderFillRect(state.renderer, &player.rect);
		SDL_RenderPresent(state.renderer);
	}

	wclose(&state);
	return 0;
}

