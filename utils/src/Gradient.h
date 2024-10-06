#pragma once

#include "laml/Data_types.hpp"
#include <SDL_pixels.h>

struct SDL_Surface;

struct Gradient {
    Gradient();
    ~Gradient();

    bool Load_From_Image(const char* filename);
    void Set_Mapping_Range(real32 min_in, real32 max_in);
    SDL_Color Sample(real32 f) const;

    SDL_Surface* surface;
    real32 min_in, max_in;
};