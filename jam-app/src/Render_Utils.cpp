#include "Render_Utils.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <stdio.h>  // for vsnprintf
#include <stdarg.h> // for va_args

extern TTF_Font* g_tiny_font;
extern int32 g_font_size_tiny;
const int32 offset = 2;

extern uint32 g_window_pixel_format;

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

void Draw_Tilemap(SDL_Renderer* renderer, const Tilemap* map, const Indexed_Tilesheet* tilesheet, 
                  laml::Vec2 screen_pos, int16 tile_width, int16 tile_height, int16 ground_level) {
    int num_x = map->map_width;
    int num_y = map->map_height;

    char buffer[16] = {0};
    SDL_Rect dst_rect = {0,0,tilesheet->tile_x, tilesheet->tile_y};

    uint8 r_start = 86, g_start = 37, b_start = 0, a_start = 0;
    uint8 r_end   = 86, g_end   = 37, b_end   = 0, a_end   = 200;
    int16 start_depth = 10;
    int16 end_depth = 45;

    SDL_PixelFormat* pf = SDL_AllocFormat(g_window_pixel_format);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (int x = 0; x < num_x; x++) {
        const Tile_Data* col_data = map->operator[](x);
        dst_rect.x = (screen_pos.x + tile_width*x) - tile_width/2;

        for (int y = 0; y < num_y; y++) {
            const Tile_Data cell = col_data[y];
            dst_rect.y = (screen_pos.y + tile_height*y) - tile_height/2;

            if (cell.type) {
                Sprite_Frame frame = tilesheet->Get_Sprite_Frame(cell.type);

                SDL_Rect src_rect = {frame.x, frame.y, frame.w, frame.h};
                SDL_RenderCopy(renderer, tilesheet->texture, &src_rect, &dst_rect);

                //
                int16 depth = y - ground_level;

                depth = (depth < start_depth) ? start_depth : ((depth>end_depth) ? end_depth : depth);

                real32 f = (real32)(depth - start_depth) / (end_depth - start_depth);
                uint8 r = (r_start*(1-f) + r_end*f);
                uint8 g = (g_start*(1-f) + g_end*f);
                uint8 b = (b_start*(1-f) + b_end*f);
                uint8 a = (a_start*(1-f) + a_end*f);
                SDL_SetRenderDrawColor(renderer, r, g, b, a);
                SDL_RenderFillRect(renderer, &dst_rect);
            }
        }
    }
}

void Draw_Tilemap_Debug(SDL_Renderer* renderer, const Tilemap* map, laml::Vec2 screen_pos, int16 tile_width, int16 tile_height) {
    int num_x = map->map_width;
    int num_y = map->map_height;

    char buffer[16] = {0};
    SDL_Rect rect;
    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Color back_color = { 0, 0, 0, 255 };

    for (int x = 0; x < num_x; x++) {
        const Tile_Data* col_data = map->operator[](x);
        rect.x = (screen_pos.x + tile_width*x) - tile_width/2;

        for (int y = 0; y < num_y; y++) {
            const Tile_Data cell = col_data[y];
            rect.y = (screen_pos.y + tile_height*y) - tile_height/2;

            if (cell.type) {
                snprintf(buffer, 16, "%d", cell.type);
                Render_Text(renderer, g_tiny_font, color, rect, buffer);

                rect.x += tile_width/2;
                snprintf(buffer, 16, "%d", cell.data);
                Render_Text(renderer, g_tiny_font, color, rect, buffer);
                rect.x -= tile_width/2;
            }
        }
    }
}