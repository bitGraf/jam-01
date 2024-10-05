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

Menu_State::Menu_State() {
    log_trace("Menu_State::Menu_State()");

    this->menu_options.push_back("Play");
    this->menu_options.push_back("Options");
    this->menu_options.push_back("Quit");

    this->current_menu_item = 0;
}

Menu_State::~Menu_State() {
    log_trace("Menu_State::~Menu_State()");
}

void Menu_State::Update_And_Render(SDL_Renderer* renderer, real32 dt) {
    SDL_Rect pos = { 10, 200, 0, 000 };

    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Color back_color = { 0, 0, 0, 255 };
    SDL_Color select_color = {255, 255, 0, 255};

    int num_options = menu_options.size();
    current_menu_item = (current_menu_item >= num_options) ? num_options : current_menu_item;
    for (int n = 0; n < num_options; n++) {
        const std::string& item = this->menu_options[n];

        Render_Text(renderer, g_large_font, back_color, pos, item.c_str());

        pos.x -= offset;
        pos.y -= offset;

        //render selected text differently
        if (n == current_menu_item) {
            Render_Text(renderer, g_large_font, select_color, pos, item.c_str());
        } else {
            Render_Text(renderer, g_large_font, color, pos, item.c_str());
        }

        pos.y += g_font_size_large;
    }
}

bool Menu_State::On_Action_Event(Action_Event action) {
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
        log_info("Enter pressed [%s]", this->menu_options[current_menu_item].c_str());

        switch (current_menu_item) {
            case 0: { // Play
                Game_State* play = new Level_State("data/levels/level_1.json");
                g_game.Push_New_State(play);
                break;
            };

            case 1: { // Options
                Game_State* opt = new Option_State();
                g_game.Push_New_State(opt);
                break;
            };

            case 2: { // Quit
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