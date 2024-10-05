#include "Entity.h"

#include "logging.h"

#include "Game_App.h"
extern Game_App g_game;

laml::Vec2 World::Get_Screen_Pos(laml::Vec2 world_pos) {
    return laml::Vec2(origin.x + (world_pos.x - cam_world_pos.x), origin.y + (world_pos.y - cam_world_pos.y));
}

laml::Vec2 World::Get_World_Pos(laml::Vec2 screen_pos) {
    return laml::Vec2(screen_pos.x - origin.x + cam_world_pos.x, screen_pos.y - origin.y + cam_world_pos.y);
}

laml::Vec2 World::Get_Origin_Screen_Pos() {
    return laml::Vec2(origin.x - cam_world_pos.x, origin.y - cam_world_pos.y);
}

laml::Vec2 World::Get_World_Pos(int16 world_x, int16 world_y) {
    return laml::Vec2(world_x*grid_size_x, world_y*grid_size_y);
}

laml::Vec2 World::Get_Screen_Pos(int16 world_x, int16 world_y) {
    laml::Vec2 world_pos(world_x*grid_size_x, world_y*grid_size_y);
    return Get_Screen_Pos(world_pos);
}

void World::Init_Grid(int16 num_cells_x, int16 num_cells_y, int16 cell_x, int16 cell_y) {
    cam_world_pos = laml::Vec2(0.0, 0.0);
    sprite_origin.Load_Sprite_Sheet(g_game.GetRenderer(), "data/plus.png", 32, 32, 16, 16);
    sprite_cam.Load_Sprite_Sheet(g_game.GetRenderer(), "data/plus2.png", 32, 32, 16, 16);

    grid.Create(num_cells_x, num_cells_y);
    grid_size_x = cell_x;
    grid_size_y = cell_y;

    tilesheet.Load_Tilesheet(g_game.GetRenderer(), "data/tilesheet.png", 32, 32);
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

uint8* Tilemap::operator[](int16 x) {
    if (x < 0) {
        log_error("Tilemap[%d] out of bounds!", x);
        return map[0];
    }
    if (x >= this->map_width) {
        log_error("Tilemap[%d] out of bounds!", x);
        return map[map_width-1];
    }

    return map[x];
}
const uint8* Tilemap::operator[](int16 x) const {
    if (x < 0) {
        log_error("Tilemap[%d] out of bounds!", x);
        return map[0];
    }
    if (x >= this->map_width) {
        log_error("Tilemap[%d] out of bounds!", x);
        return map[map_width-1];
    }

    return map[x];
}

void Tilemap::Create(int16 map_x, int16 map_y) {
    this->map_width = map_x;
    this->map_height = map_y;

    this->map = new uint8*[map_x];
    for (int x = 0; x < map_x; x++) {
        this->map[x] = new uint8[map_y];

        for (int y = 0; y < map_y; y++) {
            this->map[x][y] = 0;
        }
    }

    this->init = true;
}

void Tilemap::Destroy() {
    for (int x = 0; x < map_width; x++) {
        delete [] this->map[x];
        this->map[x] = nullptr;
    }
    delete [] this->map;
    this->map = nullptr;

    this->map_width = 0;
    this->map_height = 0;

    init = false;
}

void Tilemap::Fill(uint8 cell_value) {
    for (int x = 0; x < map_width; x++) {
        uint8* col_data = map[x];
        for (int y = 0; y < map_height; y++) {
            col_data[y] = cell_value;
        }
    }
}

void Tilemap::Fill(int16 start_x, int16 start_y, int16 width, int16 height, uint8 cell_value) {
    int end_x = start_x + width;
    int end_y = start_y + height;
    for (int x = start_x; x < end_x; x++) {
        uint8* col_data = map[x];
        for (int y = start_y; y < end_y; y++) {
            col_data[y] = cell_value;
        }
    }
}