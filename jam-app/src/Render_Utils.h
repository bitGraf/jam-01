#pragma once

#include <laml/laml.hpp>

#include <SDL.h>
#include <SDL_ttf.h>

#include "Sprite.h"
#include "Entity.h"

void Render_Text(SDL_Renderer* renderer, TTF_Font* font, SDL_Color color, SDL_Rect dest, const char* fmt, ...);
void Draw_Circle(SDL_Renderer* renderer, int32 c_x, int32 c_y ,int32 radius);
void Draw_Sprite(SDL_Renderer* renderer, const Sprite& sprite, laml::Vec2 screen_pos, real32 angle);

void Draw_Tilemap(SDL_Renderer* renderer, const Tilemap* map, const Indexed_Tilesheet* tilesheet, laml::Vec2 screen_pos, int16 tile_width, int16 tile_height, int16 ground_level);
void Draw_Tilemap_Debug(SDL_Renderer* renderer, const Tilemap* map, laml::Vec2 screen_pos, int16 tile_width, int16 tile_height);