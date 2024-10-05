#include "Entity.h"

laml::Vec2 World::Get_Screen_Pos(laml::Vec2 world_pos) {
    return laml::Vec2(origin.x + (world_pos.x - cam_world_pos.x), origin.y - (world_pos.y - cam_world_pos.y));
}

laml::Vec2 World::Get_World_Pos(laml::Vec2 screen_pos) {
    return laml::Vec2(screen_pos.x - origin.x + cam_world_pos.x, origin.y + cam_world_pos.y - screen_pos.y);
}

laml::Vec2 World::Get_Origin_Screen_Pos() {
    return laml::Vec2(origin.x - cam_world_pos.x, origin.y + cam_world_pos.y);
}