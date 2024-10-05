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

laml::Vec2 World::Get_World_Pos(int16 world_x, int16 world_y) {
    return laml::Vec2(world_x*grid_x, world_y*grid_y);
}

laml::Vec2 World::Get_Screen_Pos(int16 world_x, int16 world_y) {
    laml::Vec2 world_pos(world_x*grid_x, world_y*grid_y);
    return Get_Screen_Pos(world_pos);
}

// Tilemap

Tilemap::Tilemap() 
 : map_width(0), map_height(0), init(false), map(nullptr)
{}
Tilemap::~Tilemap() {
    if (init) {
        Destroy();
    }
}

void Tilemap::Create(int16 map_x, int16 map_y) {
    this->map_width = map_x;
    this->map_height = map_y;

    this->map = new uint8*[map_x];
    for (int y = 0; y < map_y; y++) {
        this->map[y] = new uint8[map_x];

        for (int x = 0; x < map_x; x++) {
            this->map[y][x] = 0;
        }
    }

    this->init = true;
}

void Tilemap::Destroy() {
    for (int y = 0; y < map_height; y++) {
        delete [] this->map[y];
        this->map[y] = nullptr;
    }
    delete [] this->map;
    this->map = nullptr;

    this->map_width = 0;
    this->map_height = 0;

    init = false;
}