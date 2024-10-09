#pragma once

#include <stack>

#include <laml/laml.hpp>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include "Save_Data.h"
#include "Sprite.h"
#include "Game_Pad.h"
#include "Mouse.h"
#include "Game_State.h"
#include "Input_Mapping.h"

struct Game_App {
public:
    Game_App();
    ~Game_App();

    bool Run();

    void Goto_Main_Menu();
    void Push_New_State(Game_State* state);
    void Pop_State();

    void Update_Volume(uint8 new_master_volume, uint8 new_music_volume);

    SDL_Renderer* GetRenderer() { return renderer; }
    const SDL_Renderer* GetRenderer() const { return renderer; }

    Sys_Config_Data& Get_Config() { return sys_config; }
    const Sys_Config_Data& Get_Config() const { return sys_config; }

    Game_Save_Data& Get_Save_Data() { return save_data; }
    const Game_Save_Data& Get_Save_Data() const { return save_data; }
    void Reset_Save_Data();

    const Game_Pad* GetGamePad() const { return &gamepad; }
    const Axis_Mapping* GetAxes() const { return &axes; }

private:
    bool Read_Config(const char* config_filename);
    bool Write_Config(const char* config_filename) const;

    bool Read_Save_Data(const char* save_filename);
    bool Write_Save_Data(const char* save_filename) const;

    bool Init();
    bool Shutdown();
    SDL_Joystick* Update_Joysticks();
    bool Fixed_Update_And_Render(float dt);
    bool Load_Assets();

    void On_Key_Event(int32 key_code, bool pressed);
    void On_Action_Event(Action_Event event);

    void Draw_Gamepad();

private:
    int32 version_number;
    Sys_Config_Data sys_config;
    Game_Save_Data save_data;

    // Pointers to our window and renderer
	SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    // Textures
    SDL_Texture* cursor = NULL;

    // Sounds
    Mix_Chunk* sound = NULL;
    Mix_Music* music = NULL;

    // Controller
    Game_Pad gamepad;
    Axis_Mapping axes;

    // Mouse
    Mouse mouse;

    //
    std::stack<Game_State*> game_state;
    Game_State* new_state = nullptr;
    bool return_to_main = false;
    bool pop_state = false;
};