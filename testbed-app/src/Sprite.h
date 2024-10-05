#pragma once

#include <laml/laml.hpp>
#include <vector>
#include <string>

struct SDL_Texture;
struct SDL_Renderer;

struct Sprite_Frame {
    int x_index, y_index;
    int x, y, w, h;
};

struct Sprite_Sequence {
    bool looping = true;
    uint8 current_frame;
    uint8 num_frames;
    
    real32 frame_rate;
    real32 sequence_time;

    std::string name;
    std::vector<Sprite_Frame> frames;

public:
    void Update(real32 dt);
    const Sprite_Frame& Current_Frame() const;
};

struct Sprite {
    Sprite();

    bool Load_Sprite_Sheet(SDL_Renderer* renderer, const char* filename, int32 sprite_w, int32 sprite_h, real32 anchor_x, real32 anchor_y);
    bool Load_Sprite_Sheet_From_Meta(SDL_Renderer* renderer, const char* filename);
    void Destroy();

    void Update(real32 dt);
    const Sprite_Frame& Current_Frame() const;

    void Set_Sequence(uint8 sequence_idx, uint8 flags);

    SDL_Texture* texture;
    laml::Vec2 size;
    laml::Vec2 anchor;
    uint8 current_sequence;
    std::vector<Sprite_Sequence> sequences;
};