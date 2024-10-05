#pragma once

#include <laml/laml.hpp>

#include <SDL.h>
#include <SDL_ttf.h>

#include "Sprite.h"

void Render_Text(SDL_Renderer* renderer, TTF_Font* font, SDL_Color color, SDL_Rect dest, const char* fmt, ...);
void Draw_Circle(SDL_Renderer* renderer, int32 c_x, int32 c_y ,int32 radius);
void Draw_Sprite(SDL_Renderer* renderer, const Sprite& sprite, laml::Vec2 screen_pos, real32 angle);