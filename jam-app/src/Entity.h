#pragma once

#include <laml/laml.hpp>
#include "Sprite.h"

struct Tilemap {
    int16 map_width, map_height; // num tiles
    uint8** map; // actual data

    void Create(int16 map_x, int16 map_y);
    void Destroy();
    
    Tilemap();
    ~Tilemap();

private:
    bool init;
};

struct World {
    laml::Vec2 origin;
    laml::Vec2 cam_world_pos;
    Sprite sprite_origin;
    Sprite sprite_cam;

    int16 grid_x, grid_y;
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