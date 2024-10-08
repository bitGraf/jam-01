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
    uint8 Move_Entity(int16 start_x, int16 start_y, int16 move_x, int16 move_y);
    uint8 Move_Entity(laml::Vec2 start, laml::Vec2 end);
    uint8 Break_Block(int16 world_x, int16 world_y);

private:
    // World
    World world;
    int16 ground_level;

    // Player
    Entity player;
    real32 move_speed;
    real32 fall_speed;
    real32 max_fall_speed;
    bool falling;

    // Hunger
    real32 hunger;
    real32 hunger_rate;
    real32 move_cost;
    real32 break_cost;
    int16 carrying_food;
    int16 colony_food;
    int16 colony_size;
    real32 colony_hunger;
    real32 field_eat_ratio;
    real32 colony_eat_ratio;

    // debug
    std::string level_name;
};