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

Option_State::Option_State() {
    log_trace("Option_State::Option_State()");

    this->menu_options.push_back("Master Volume: ");
    this->menu_options.push_back("Music Volume: ");
    this->menu_options.push_back("Back");

    this->current_menu_item = 0;
}

Option_State::~Option_State() {
    log_trace("Option_State::~Option_State()");
}

void Option_State::Update_And_Render(SDL_Renderer* renderer, real32 dt) {
    SDL_Rect rect = { 10, 200, 0, 0 };

    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Color back_color = { 0, 0, 0, 255 };
    SDL_Color select_color = {255, 255, 0, 255};

    int num_options = menu_options.size();
    current_menu_item = (current_menu_item >= num_options) ? num_options : current_menu_item;
    for (int n = 0; n < num_options; n++) {
        const std::string& item = this->menu_options[n];

        // volume
        const char* text = item.c_str();
        char buffer[1024];
        if (n == 0) {
            text = buffer;
            snprintf(buffer, 1024, "Master Volume: %d", g_game.Get_Options().master_volume);
        } else if (n == 1) {
            text = buffer;
            snprintf(buffer, 1024, "Music Volume: %d", g_game.Get_Options().music_volume);
        }

        Render_Text(renderer, g_large_font, back_color, rect, text);

        rect.x -= offset;
        rect.y -= offset;

        //render selected text differently
        if (n == current_menu_item) {
            Render_Text(renderer, g_large_font, select_color, rect, text);
        } else {
            Render_Text(renderer, g_large_font, color, rect, text);
        }

        rect.y += g_font_size_large;
    }
}

bool Option_State::On_Action_Event(Action_Event action) {
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
            case 0: { // Master Volume
                break;
            };

            case 1: { // Music Volume
                break;
            };

            case 2: { // Back
                g_game.Pop_State();
                break;
            };
        }

        return true;
    } else if (action.action == Action_Left && action.pressed) {

        switch (current_menu_item) {
            case 0: { // Master Volume
                g_game.Get_Options().master_volume -= 5;
                if (g_game.Get_Options().master_volume <= 0) {
                    g_game.Get_Options().master_volume = 0;
                }
                
                g_game.Update_Volume(g_game.Get_Options().master_volume, g_game.Get_Options().music_volume);
                break;
            };

            case 1: { // Music Volume
                g_game.Get_Options().music_volume -= 5;
                if (g_game.Get_Options().music_volume <= 0) {
                    g_game.Get_Options().music_volume = 0;
                }
                
                g_game.Update_Volume(g_game.Get_Options().master_volume, g_game.Get_Options().music_volume);
                break;
            };
        }

        return true;
    } else if (action.action == Action_Right && action.pressed) {

        switch (current_menu_item) {
            case 0: { // Master Volume
                g_game.Get_Options().master_volume += 5;
                if (g_game.Get_Options().master_volume >= 100) {
                    g_game.Get_Options().master_volume = 100;
                }

                g_game.Update_Volume(g_game.Get_Options().master_volume, g_game.Get_Options().music_volume);
                break;
            };

            case 1: { // Music Volume
                g_game.Get_Options().music_volume += 5;
                if (g_game.Get_Options().music_volume >= 100) {
                    g_game.Get_Options().music_volume = 100;
                }

                g_game.Update_Volume(g_game.Get_Options().master_volume, g_game.Get_Options().music_volume);
                break;
            };
        }

        return true;
    }
    return false;
}