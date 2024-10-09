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
// Option State
///////////////////////////////////////////////////////////////////////////////

enum Option_Actions {
    OptionAction_None = 0,
    OptionAction_MasterVolume,
    OptionAction_MusicVolume,
    OptionAction_Back
};

Option_State::Option_State() {
    log_trace("Option_State::Option_State()");

    this->menu_options.push_back(Menu_Item("Master Volume: ", OptionAction_MasterVolume));
    this->menu_options.push_back(Menu_Item("Music Volume: ", OptionAction_MusicVolume));
    this->menu_options.push_back(Menu_Item("Back", OptionAction_Back));

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
        const Menu_Item& item = this->menu_options[n];
        const std::string& opt_text = item.text;

        // volume
        const char* text = opt_text.c_str();
        char buffer[1024];
        if (n == 0) {
            text = buffer;
            snprintf(buffer, 1024, "Master Volume: %d", g_game.Get_Config().master_volume);
        } else if (n == 1) {
            text = buffer;
            snprintf(buffer, 1024, "Music Volume: %d", g_game.Get_Config().music_volume);
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
        const Menu_Item& item = this->menu_options[current_menu_item];
        log_info("Enter pressed [%s]", item.text.c_str());

        switch (item.action) {
            case OptionAction_Back: { // Back
                g_game.Pop_State();
                break;
            };
        }

        return true;
    } else if (action.action == Action_Left && action.pressed) {
        const Menu_Item& item = this->menu_options[current_menu_item];

        switch (item.action) {
            case OptionAction_MasterVolume: { // Master Volume
                g_game.Get_Config().master_volume -= 5;
                if (g_game.Get_Config().master_volume <= 0) {
                    g_game.Get_Config().master_volume = 0;
                }
                
                g_game.Update_Volume(g_game.Get_Config().master_volume, g_game.Get_Config().music_volume);
                break;
            };

            case OptionAction_MusicVolume: { // Music Volume
                g_game.Get_Config().music_volume -= 5;
                if (g_game.Get_Config().music_volume <= 0) {
                    g_game.Get_Config().music_volume = 0;
                }
                
                g_game.Update_Volume(g_game.Get_Config().master_volume, g_game.Get_Config().music_volume);
                break;
            };
        }

        return true;
    } else if (action.action == Action_Right && action.pressed) {
        const Menu_Item& item = this->menu_options[current_menu_item];

        switch (item.action) {
            case OptionAction_MasterVolume: { // Master Volume
                g_game.Get_Config().master_volume += 5;
                if (g_game.Get_Config().master_volume >= 100) {
                    g_game.Get_Config().master_volume = 100;
                }

                g_game.Update_Volume(g_game.Get_Config().master_volume, g_game.Get_Config().music_volume);
                break;
            };

            case OptionAction_MusicVolume: { // Music Volume
                g_game.Get_Config().music_volume += 5;
                if (g_game.Get_Config().music_volume >= 100) {
                    g_game.Get_Config().music_volume = 100;
                }

                g_game.Update_Volume(g_game.Get_Config().master_volume, g_game.Get_Config().music_volume);
                break;
            };
        }

        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////