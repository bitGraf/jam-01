#include "Game_State.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include "Render_Utils.h"
#include "logging.h"

extern TTF_Font* g_large_font;
extern int32 g_font_size_large;
const int32 offset = 2;

#include "Game_App.h"
extern Game_App g_game;

///////////////////////////////////////////////////////////////////////////////
// Pause State
///////////////////////////////////////////////////////////////////////////////

enum Pause_Actions {
    PauseAction_None = 0,
    PauseAction_Resume,
    PauseAction_Options,
    PauseAction_MainMenu,
    PauseAction_Quit
};

Pause_State::Pause_State() {
    log_trace("Pause_State::Pause_State()");

    this->menu_options.push_back(Menu_Item("Resume", PauseAction_Resume));
    this->menu_options.push_back(Menu_Item("Options", PauseAction_Options));
    this->menu_options.push_back(Menu_Item("Main Menu", PauseAction_MainMenu));
    this->menu_options.push_back(Menu_Item("Quit Game", PauseAction_Quit));

    this->current_menu_item = 0;
}

Pause_State::~Pause_State() {
    log_trace("Pause_State::~Pause_State()");
}

void Pause_State::Update_And_Render(SDL_Renderer* renderer, real32 dt) {
    SDL_Rect rect = { 10, 200, 0, 0 };

    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Color back_color = { 0, 0, 0, 255 };
    SDL_Color select_color = {255, 255, 0, 255};

    int num_options = menu_options.size();
    current_menu_item = (current_menu_item >= num_options) ? num_options : current_menu_item;
    for (int n = 0; n < num_options; n++) {
        const Menu_Item& item = this->menu_options[n];
        const std::string& menu_text = item.text;

        Render_Text(renderer, g_large_font, back_color, rect, menu_text.c_str());

        rect.x -= offset;
        rect.y -= offset;

        //render selected text differently
        if (n == current_menu_item) {
            Render_Text(renderer, g_large_font, select_color, rect, menu_text.c_str());
        } else {
            Render_Text(renderer, g_large_font, color, rect, menu_text.c_str());
        }

        rect.y += g_font_size_large;
    }
}

bool Pause_State::On_Action_Event(Action_Event action) {
    if (action.action == Action_Up && action.pressed) {
        current_menu_item--;
        if (current_menu_item >= menu_options.size()) {
            current_menu_item = menu_options.size()-1;
        }
        return true;
    } else if (action.action == Action_Down && action.pressed) {
        current_menu_item++;
        if (current_menu_item >= menu_options.size()) {
            current_menu_item = 0;
        }
        return true;
    } else if (action.action == Action_A && action.pressed) {
        const Menu_Item& item = this->menu_options[current_menu_item];
        log_info("Enter pressed [%s]", item.text.c_str());

        switch (item.action) {
            case PauseAction_Resume: { // Resume
                g_game.Pop_State();
                break;
            };

            case PauseAction_Options: { // Options
                Game_State* pause = new Option_State();
                g_game.Push_New_State(pause);
                break;
            };

            case PauseAction_MainMenu: { // Main Menu
                g_game.Goto_Main_Menu();
                break;
            };

            case PauseAction_Quit: { // Quit Game
                SDL_Event quit_event;
                quit_event.type = SDL_QUIT;
                SDL_PushEvent(&quit_event);
                break;
            };
        }

        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////