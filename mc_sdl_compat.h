#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef uint8_t Uint8;
typedef uint16_t Uint16;
typedef uint32_t Uint32;

void *SDL_malloc(unsigned int size) {
	return malloc(size);
}

void SDL_free(void *ptr) {
	free(ptr);
}

extern void mc_sleep(void);

extern void turtle_x(int);
extern void turtle_y(int);
extern void turtle_z(int);

extern void turtle_set(int);

extern void turtle_copy(void);
extern void turtle_paste(void);

void mc_present(void) {
	turtle_z(-20);
	turtle_x(129);
	turtle_y(1);

	turtle_set(9);
}

typedef struct {
	int x, y, w, h;
} SDL_Rect;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} SDL_Color;

typedef struct {
	Uint8 BitsPerPixel;
	Uint8 BytesPerPixel;
} SDL_PixelFormat;

typedef struct {
	SDL_PixelFormat *format;
	int w, h;
	int pitch;
	void *pixels;
	int locked;
	SDL_Rect clip_rect;
} SDL_Surface;

void SDL_FreeSurface(SDL_Surface *surf) {
	if (surf == NULL) {
		return;
	}

	if (surf->pixels != NULL) { SDL_free(surf->pixels); }
	if (surf->format != NULL) { SDL_free(surf->format); }

	SDL_free(surf);
}

int SDL_SetPalette(SDL_Surface *surf, int flags, SDL_Color *pal, int firstcolor, int ncolors) {
	return 0;
}

#define SDL_SRCCOLORKEY 1

int SDL_SetColorKey(SDL_Surface *surf, int yes, int key) {
	return 0;
}

Uint8 keyState[20] = { 0 };

Uint8 *SDL_GetKeyState(int *numkeys) {
	return keyState;
}

void SDL_Flip_screen() {

}

#define SCREEN_W 128
#define SCREEN_H 128

enum Block {
    AIR,
    COBBLESTONE,
    GRANITE,
    ANDESITE,
    DIORITE,
    LAPIS_BLOCK,
    IRON_BLOCK,
    GOLD_BLOCK,
    DIAMOND_BLOCK,
    REDSTONE_BLOCK,
    EMERALD_BLOCK,
    DIRT_BLOCK,
    OAK_LOG_BLOCK,
    OAK_LEAVES_BLOCK,
    COAL_BLOCK,
};

const enum Block palette_colors[16] = {
	COAL_BLOCK, LAPIS_BLOCK, GRANITE, OAK_LEAVES_BLOCK,
	DIRT_BLOCK, COBBLESTONE, ANDESITE, IRON_BLOCK,
	REDSTONE_BLOCK, GOLD_BLOCK, GOLD_BLOCK, EMERALD_BLOCK,
	DIAMOND_BLOCK, ANDESITE, GRANITE, COBBLESTONE,
};

void SDL_SetPixel_screen_palette(int x, int y, unsigned char palette_idx) {
	if (x < 0 || x >= 128) {
		return;
	}
	if (y < 0 || y >= 128) {
		return;
	}

	turtle_z(-20);
	turtle_x(128 - x);
	turtle_y(128 - y);

	palette_idx %= 16;

	turtle_set(palette_colors[palette_idx]);
}

void SDL_FillRect_screen_palette(SDL_Rect *rect, char palette_idx) {
	int x, y, w, h;
	if (rect) {
		x = rect->x;
		y = rect->y;
		w = rect->w;
		h = rect->h;

		if (x < 0) {
			w += x;
			x = 0;
		}

		if (y < 0) {
			h += y;
			y = 0;
		}

		if (x + w > SCREEN_W) {
			w -= (x + w - SCREEN_W);
		}
		if (y + h > SCREEN_H) {
			h -= (y + w - SCREEN_H);
		}
	} else {
		x = 0;
		y = 0;
		w = SCREEN_W;
		h = SCREEN_H;
	}

	if (w == 0 || h == 0) {
		return;
	}

	SDL_SetPixel_screen_palette(x, y, palette_idx);
	turtle_copy();

	for (int cur_y = y; cur_y < y + h; ++cur_y) {
		turtle_y(128 - cur_y);
		for (int cur_x = x; cur_x < x + w; ++cur_x) {
			turtle_x(128 - cur_x);
			turtle_paste();
		}
	}
}

#define SDL_INIT_AUDIO (1 << 0)
#define SDL_INIT_VIDEO (1 << 1)
#define SDL_INIT_GAMECONTROLLER (1 << 2)

int SDL_Init(Uint32 flags) {
	return 0;
}

int SDL_InitSubSystem(Uint32 system) {
	return 0;
}

#define SDL_SWSURFACE (1 << 0)
#define SDL_HWPALETTE (1 << 1)

SDL_Surface* SDL_CreateRGBSurface(
	Uint32 flags, int width, int height, int depth,
    Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	if (flags != SDL_SWSURFACE) {
		// TODO:
		return 0;
	}
	if (depth != 8 || Rmask != 0 || Gmask != 0 || Bmask != 0 || Amask != 0) {
		// TODO:
		return 0;
	}

	void *pixels = SDL_malloc(width * height);
	if (pixels == 0) {
		return 0;
	}

	SDL_Surface *surf = (SDL_Surface*)SDL_malloc(sizeof(SDL_Surface));
	if (surf == 0) {
		SDL_free(pixels);
		return 0;
	}

	SDL_PixelFormat *format = (SDL_PixelFormat*)SDL_malloc(sizeof(SDL_PixelFormat));
	if (format == 0) {
		SDL_free(pixels);
		SDL_free(surf);
	}

	format->BitsPerPixel = 8;
	format->BytesPerPixel = 1;

	surf->w = width;
	surf->h = height;

	surf->clip_rect.x = 0;
	surf->clip_rect.y = 0;
	surf->clip_rect.w = surf->w;
	surf->clip_rect.h = surf->h;

	surf->locked = 0;
	surf->pitch = width;
	surf->pixels = pixels;
	surf->format = format;

	return surf;
}

#define SDL_PHYSPAL (1 << 0)
#define SDL_LOGPAL (1 << 0)

const char *SDL_GetError() {
	return "";
}

#define SDL_QUIT 0
#define SDL_KEYDOWN 1
#define SDL_KEYUP 2

typedef struct {
	int type;
} SDL_Event;

#define SDLK_LEFT 	0
#define SDLK_RIGHT	1
#define SDLK_UP		2
#define SDLK_DOWN 	3
#define SDLK_z		4
#define SDLK_c		5
#define SDLK_n 		6
#define SDLK_x 		7
#define SDLK_v		8
#define SDLK_m		9
#define SDLK_F9		10

#include "mc_sdl_bmp.h"