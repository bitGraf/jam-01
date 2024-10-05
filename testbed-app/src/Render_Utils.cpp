#include "Render_Utils.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <stdio.h>  // for vsnprintf
#include <stdarg.h> // for va_args

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