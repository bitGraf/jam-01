#pragma once

#include <laml/laml.hpp>
#include "Sprite.h"

struct Tilemap {
    int16 map_width, map_height; // num tiles

    uint8* operator[](int16 x);
    const uint8* operator[](int16 x) const;

    void Create(int16 map_x, int16 map_y);
    void Destroy();

    void Fill(uint8 cell_value);
    void Fill(int16 start_x, int16 start_y, int16 width, int16 height, uint8 cell_value);
    
    Tilemap();
    ~Tilemap();

private:
    uint8** map; // actual data
    bool init;
};

struct World {
    laml::Vec2 origin;
    laml::Vec2 cam_world_pos;
    Sprite sprite_origin;
    Sprite sprite_cam;

    Indexed_Tilesheet tilesheet;

    void Init_Grid(int16 num_cells_x, int16 num_cells_y, int16 cell_x, int16 cell_y);

    int16 grid_size_x, grid_size_y;
    Tilemap grid;

    laml::Vec2 Get_Screen_Pos(laml::Vec2 world_pos);
    laml::Vec2 Get_Origin_Screen_Pos();
    laml::Vec2 Get_World_Pos(laml::Vec2 screen_pos);

    laml::Vec2 Get_World_Pos(int16 world_x, int16 world_y);
    laml::Vec2 Get_Screen_Pos(int16 world_x, int16 world_y);
};

struct Entity {
    Sprite sprite;
    real32 angle;

    int16 world_x, world_y;
};