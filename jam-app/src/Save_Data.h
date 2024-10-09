#pragma once

#include <laml/Data_types.hpp>
#include <string>

//#include "Game_State.h"

// things like resolution, volume, keybindings, etc.
struct Sys_Config_Data {
    bool Serialize_From_Json(const char* filename);
    bool Serialize_To_Json(const char* filename) const;

    // windowing
    std::string window_title = "<window_title>";
    uint16 window_width = 800;
    uint16 window_height = 600;

    // options
    uint8 master_volume = 50;
    uint8 music_volume = 50;
};

struct Game_Save_Data {
    bool Serialize_From_Json(const char* filename);
    bool Serialize_To_Json(const char* filename) const;

    // World State
    int16 Food = 10;
};