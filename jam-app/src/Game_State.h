#pragma once

#include <laml/laml.hpp>
#include <vector>
#include <string>

#include "Sprite.h"
#include "Entity.h"
#include "Input_Mapping.h"

struct SDL_Renderer;

struct Game_State {
    Game_State() = default;
    virtual ~Game_State() = default;

    virtual void Update_And_Render(SDL_Renderer* renderer, real32 dt) = 0;
    virtual bool On_Action_Event(Action_Event action) = 0;
};

struct Menu_State : public Game_State {
    Menu_State();
    virtual ~Menu_State() override;

    virtual void Update_And_Render(SDL_Renderer* renderer, real32 dt) override;
    virtual bool On_Action_Event(Action_Event action) override;

private:
    uint8 current_menu_item;
    std::vector<std::string> menu_options;
};

struct Option_State : public Game_State {
    Option_State();
    virtual ~Option_State() override;

    virtual void Update_And_Render(SDL_Renderer* renderer, real32 dt) override;
    virtual bool On_Action_Event(Action_Event action) override;

private:
    uint8 current_menu_item;
    std::vector<std::string> menu_options;
};

struct Pause_State : public Game_State {
    Pause_State();
    virtual ~Pause_State() override;

    virtual void Update_And_Render(SDL_Renderer* renderer, real32 dt) override;
    virtual bool On_Action_Event(Action_Event action) override;

private:
    uint8 current_menu_item;
    std::vector<std::string> menu_options;
};

struct World_State : public Game_State {
    World_State(const char* filename);
    virtual ~World_State() override;

    virtual void Update_And_Render(SDL_Renderer* renderer, real32 dt) override;
    virtual bool On_Action_Event(Action_Event action) override;

private:
    // World
    World world;

    // Player Sprite
    Entity player;

    // debug
    std::string level_name;
};