#include "Gradient.h"

#include <SDL.h>
#include <SDL_image.h>

#include "logging.h"

Gradient::Gradient() 
 : surface(nullptr), min_in(0.0), max_in(1.0)
{}

Gradient::~Gradient() {
    if (surface) {
        SDL_FreeSurface(surface);
        surface = nullptr;
    }
}

bool Gradient::Load_From_Image(const char* filename) {
    surface = IMG_Load(filename);
    if (!surface) {
        log_fatal("Error loading image [%s]: %s", filename, SDL_GetError());
        return false;
    }

    return true;
}

void Gradient::Set_Mapping_Range(real32 min_in, real32 max_in) {
    this->min_in = min_in;
    this->max_in = max_in;
}

SDL_Color Gradient::Sample(real32 f) const {
    // f is on range of [min_in, max_in]
    // need to map in to the range of [0, surface->w]

    f = (f < min_in) ? min_in : f;
    f = (f > max_in) ? max_in : f;

    real32 x = (f - min_in) / (max_in - min_in) * surface->w + 0;
    int32 x_int = (int32)x;
    int32 y_int = 0;

    // read pixel from surface
    int bpp = surface->format->BytesPerPixel;
    uint8* p = (uint8*)surface->pixels + y_int *surface->pitch + x_int*bpp;

    uint32 pixel = 0;
    switch (bpp) {
        case 1: {
            pixel = *p;
        } break;

        case 2: {
            pixel = *(uint16*)p;
        } break;

        case 3: {
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                pixel = (p[0] << 16) | (p[1] << 8) | (p[2]);
            } else {
                pixel = (p[0]) | (p[1] << 8) | (p[2] << 16);
            }
        } break;

        case 4: {
            pixel = *(uint32*)p;
        } break;
    }

    // turn uint32 pixel into SDL_Color;
    SDL_Color color;
    SDL_GetRGBA(pixel, surface->format, &color.r, &color.g, &color.b, &color.a);
    return color;
}