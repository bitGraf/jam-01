#include "Render_Utils.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <stdio.h>  // for vsnprintf
#include <stdarg.h> // for va_args

extern TTF_Font* g_small_font;
extern int32 g_font_size_small;
const int32 offset = 2;

void Render_Text(SDL_Renderer* renderer, TTF_Font* font, SDL_Color color, SDL_Rect dest, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char BUFFER1[2048];
    vsnprintf(BUFFER1, 2048, fmt, args);
    va_end(args);

	SDL_Surface* surf = TTF_RenderText_Solid(font, BUFFER1, color);

	dest.w = surf->w;
	dest.h = surf->h;

	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);

	SDL_RenderCopy(renderer, tex, NULL, &dest);
	SDL_DestroyTexture(tex);
	SDL_FreeSurface(surf);
}

void Draw_Circle(SDL_Renderer* renderer, int32 c_x, int32 c_y ,int32 radius) {
    const int32_t diameter = (radius * 2);

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);

    while (x >= y) {
        // Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, c_x + x, c_y - y);
        SDL_RenderDrawPoint(renderer, c_x + x, c_y + y);
        SDL_RenderDrawPoint(renderer, c_x - x, c_y - y);
        SDL_RenderDrawPoint(renderer, c_x - x, c_y + y);
        SDL_RenderDrawPoint(renderer, c_x + y, c_y - x);
        SDL_RenderDrawPoint(renderer, c_x + y, c_y + x);
        SDL_RenderDrawPoint(renderer, c_x - y, c_y - x);
        SDL_RenderDrawPoint(renderer, c_x - y, c_y + x);

        if (error <= 0) {
            y++;
            error += ty;
            ty += 2;
        }

        if (error > 0){
            x--;
            tx += 2;
            error += (tx - diameter);
        }
    }
}

void Draw_Sprite(SDL_Renderer* renderer, const Sprite& sprite, laml::Vec2 screen_pos, real32 angle) {
    const Sprite_Frame& frame = sprite.Current_Frame();

    SDL_Rect src_rect = {frame.x, frame.y, frame.w, frame.h};
    SDL_Rect dst_rect = {(int32)(screen_pos.x - sprite.anchor.x), (int32)(screen_pos.y - sprite.anchor.y), frame.w, frame.h};
    //SDL_RenderCopy(renderer, sprite.texture, &src_rect, &dst_rect);

    //SDL_Point rot_center = { (int32)screen_pos.x, (int32)screen_pos.y};
    SDL_Point rot_center = { (int32)sprite.anchor.x, (int32)sprite.anchor.y};
    SDL_RenderCopyEx(renderer, sprite.texture, &src_rect, &dst_rect, angle, &rot_center, SDL_FLIP_NONE);
}

void Draw_Tilemap_Debug(SDL_Renderer* renderer, const Tilemap* map, laml::Vec2 screen_pos, int16 tile_width, int16 tile_height) {
    int num_x = map->map_width;
    int num_y = map->map_height;

    char buffer[16] = {0};
    SDL_Rect rect;
    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Color back_color = { 0, 0, 0, 255 };

    for (int y = 0; y < num_y; y++) {
        const uint8* row_data = map->map[y];
        rect.y = (screen_pos.y + tile_height*y) - tile_height/2;

        for (int x = 0; x < num_x; x++) {
            const uint8 cell = row_data[x];
            rect.x = (screen_pos.x + tile_width*x) - tile_width/2;

            snprintf(buffer, 16, "%d", cell);
            Render_Text(renderer, g_small_font, color, rect, buffer);
        }
    }
}