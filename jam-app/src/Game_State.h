#pragma once

#include <laml/laml.hpp>
#include <vector>
#include <string>

#include "Sprite.h"
#include "Entity.h"
#include "Input_Mapping.h"
#include "Gradient.h"

struct SDL_Renderer;

///////////////////////////////////////////////////////////////////////////////
// Base Game State
///////////////////////////////////////////////////////////////////////////////

struct Game_State {
public:
    Game_State() = default;
    virtual ~Game_State() = default;

    virtual void Update_And_Render(SDL_Renderer* renderer, real32 dt) = 0;
    virtual bool On_Action_Event(Action_Event action) = 0;
};


///////////////////////////////////////////////////////////////////////////////
// Main Menu State
///////////////////////////////////////////////////////////////////////////////

struct Menu_Item {
    //Menu_Item(const std::string& text_);
    Menu_Item(const std::string& text_, uint32 action_);

    std::string text;
    uint32 action;
};

struct Menu_State : public Game_State {
    Menu_State(bool has_save);
    virtual ~Menu_State() override;

    virtual void Update_And_Render(SDL_Renderer* renderer, real32 dt) override;
    virtual bool On_Action_Event(Action_Event action) override;

private:
    uint8 current_menu_item;
    std::vector<Menu_Item> menu_options;
};

///////////////////////////////////////////////////////////////////////////////
// Options Menu State
///////////////////////////////////////////////////////////////////////////////

struct Option_State : public Game_State {
    Option_State();
    virtual ~Option_State() override;

    virtual void Update_And_Render(SDL_Renderer* renderer, real32 dt) override;
    virtual bool On_Action_Event(Action_Event action) override;

private:
    uint8 current_menu_item;
    std::vector<Menu_Item> menu_options;
};

///////////////////////////////////////////////////////////////////////////////
// Pause Menu State
///////////////////////////////////////////////////////////////////////////////

struct Pause_State : public Game_State {
    Pause_State();
    virtual ~Pause_State() override;

    virtual void Update_And_Render(SDL_Renderer* renderer, real32 dt) override;
    virtual bool On_Action_Event(Action_Event action) override;

private:
    uint8 current_menu_item;
    std::vector<Menu_Item> menu_options;
};

///////////////////////////////////////////////////////////////////////////////
// World/Shop Handover data
///////////////////////////////////////////////////////////////////////////////

struct Shop_Handover {
    Shop_Handover(int16& food,uint8& dig_speed,uint8& dig_strength,uint8& extraction,uint8& abilities);

    int16& food;
    uint8& dig_speed;
    uint8& dig_strength;
    uint8& extraction;
    uint8& abilities;
};

///////////////////////////////////////////////////////////////////////////////
// World State
///////////////////////////////////////////////////////////////////////////////

struct World_State : public Game_State {
    World_State(const char* filename);
    virtual ~World_State() override;

    virtual void Update_And_Render(SDL_Renderer* renderer, real32 dt) override;
    virtual bool On_Action_Event(Action_Event action) override;

private:
    bool Read_Config(const char* init_filename, const char* filename);
    bool Write_Config(const char* filename);
    uint8 Move_Entity(int16 start_x, int16 start_y, int16 move_x, int16 move_y);
    uint8 Break_Block(int16 world_x, int16 world_y);

    void Next_Day();
    void Death();

private:
    Shop_Handover handover;

    // World
    World world;
    Gradient time_gradient;
    real32 time_rate;
    real32 time_of_day;
    uint8 day_number;
    uint8 fast_forward_day;

    real32 fast_forward_ratio;
    bool pan_to_colony;
    bool fast_forward;

    // Player
    Entity player;
    uint8 dig_strength; // how tough things u can break
    uint8 dig_speed;    // speed at which you break them
    uint8 extraction;
    uint8 abilities;
    bool can_hibernate;
    bool can_dash;

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
    bool show_debug;
    std::string level_name;
};

///////////////////////////////////////////////////////////////////////////////
// Shop State
///////////////////////////////////////////////////////////////////////////////

struct Shop_Upgrade {
    Shop_Upgrade();

    std::string name;
    std::string description;
    int16 cost;
    uint8 speed;
    uint8 strength;
    uint8 extraction;
    uint8 ability;

    bool bought;
};

enum Ability_Enum {
    Ability_None = 0,
    Ability_Hibernate = 1,
    Ability_Dash = 2
};

#define ABILITY_HIBERNATE 0x01
#define ABILITY_DASH 0x02

struct Shop_State : public Game_State {
    Shop_State(Shop_Handover& handover);
    virtual ~Shop_State() override;

    virtual void Update_And_Render(SDL_Renderer* renderer, real32 dt) override;
    virtual bool On_Action_Event(Action_Event action) override;

private:
    bool Read_Config(const char* init_filename, const char* filename);
    bool Write_Config(const char* filename);
    void Leave_Shop();

private:
    std::string shop_title;
    std::vector<Shop_Upgrade> upgrades;
    uint8 num_options;
    uint8 current_menu_item;

    // handles to the game state
    Shop_Handover& handover;
};