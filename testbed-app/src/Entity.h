#pragma once

#include <laml/laml.hpp>
#include "Sprite.h"

struct World {
    laml::Vec2 origin;
    laml::Vec2 cam_world_pos;
    Sprite sprite_origin;
    Sprite sprite_cam;

    laml::Vec2 Get_Screen_Pos(laml::Vec2 world_pos);
    laml::Vec2 Get_Origin_Screen_Pos();
    laml::Vec2 Get_World_Pos(laml::Vec2 screen_pos);
};

struct Entity {
    Sprite sprite;
    laml::Vec2 world_pos;
    real32 angle;
};