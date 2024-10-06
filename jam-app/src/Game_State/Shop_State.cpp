#include "Game_State.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include "Render_Utils.h"
#include "logging.h"

extern TTF_Font* g_large_font;
extern int32 g_font_size_large;
extern TTF_Font* g_medium_font;
extern int32 g_font_size_medium;
extern TTF_Font* g_small_font;
extern int32 g_font_size_small;
const int32 offset = 2;

#include "Game_App.h"
extern Game_App g_game;

// json parser
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

const char* Ability_Names[] = {
    "<none>",
    "Hibernate",
    "Dash"
};

Upgrade::Upgrade() {
    name = "<upgrade>";
    description = "<description>";
    cost = 0;
    speed = 0;
    strength = 0;
    extraction = 0;
    ability = Ability_None;

    bought = true;
}

Shop_Handover::Shop_Handover(int16& food,uint8& dig_speed,uint8& dig_strength,uint8& extraction,uint8& abilities)
 : food(food), dig_speed(dig_speed), dig_strength(dig_strength), extraction(extraction), abilities(abilities)
{}

Shop_State::Shop_State(Shop_Handover& handover)
 : handover(handover) {
    log_trace("Shop_State::Shop_State()");

    current_menu_item = 0;

    if (!Read_Config()) {
        log_error("Failed to load shop config!");
    }

    num_options = upgrades.size() + 1;
}

Shop_State::~Shop_State() {
    log_trace("Shop_State::~Shop_State()");
}

void Shop_State::Update_And_Render(SDL_Renderer* renderer, real32 dt) {
    SDL_Rect rect = { 10, 50, 0, 0 };

    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Color back_color = { 0, 0, 0, 255 };
    SDL_Color select_color = {255, 255, 0, 255};
    SDL_Color bought_color = {255, 0, 255, 255};

    char buffer[256];

    // Shop Title
    Render_Text(renderer, g_large_font, back_color, rect, shop_title.c_str());
    rect.x -= offset;
    rect.y -= offset;
    Render_Text(renderer, g_large_font, color, rect, shop_title.c_str());

    // Print food/stats
    rect.y = 150;
    rect.x = 400;
    snprintf(buffer, 32, "Food: %d", handover.food);
    Render_Text(renderer, g_medium_font, back_color, rect, buffer);
    rect.x -= offset;
    rect.y -= offset;
    Render_Text(renderer, g_medium_font, color, rect, buffer);
    rect.x += offset;
    rect.y += (g_font_size_medium - offset);
    snprintf(buffer, 32, "Dig Speed: %d", handover.dig_speed);
    Render_Text(renderer, g_medium_font, back_color, rect, buffer);
    rect.x -= offset;
    rect.y -= offset;
    Render_Text(renderer, g_medium_font, color, rect, buffer);
    rect.x += offset;
    rect.y += (g_font_size_medium - offset);
    snprintf(buffer, 32, "Dig Strength: %d", handover.dig_strength);
    Render_Text(renderer, g_medium_font, back_color, rect, buffer);
    rect.x -= offset;
    rect.y -= offset;
    Render_Text(renderer, g_medium_font, color, rect, buffer);
    rect.y += (g_font_size_medium - offset);
    rect.x += offset;
    snprintf(buffer, 32, "Extraction: %d", handover.extraction);
    Render_Text(renderer, g_medium_font, back_color, rect, buffer);
    rect.x -= offset;
    rect.y -= offset;
    Render_Text(renderer, g_medium_font, color, rect, buffer);
    rect.y += (g_font_size_medium - offset);
    rect.x += offset;

    // Start Menu options, begining with Leave
    rect.y = 150;
    rect.x = 10;
    Render_Text(renderer, g_medium_font, back_color, rect, "Leave");
    rect.x -= offset;
    rect.y -= offset;
    if (current_menu_item == 0)
        Render_Text(renderer, g_medium_font, select_color, rect, "Leave");
    else
        Render_Text(renderer, g_medium_font, color, rect, "Leave");
    rect.y += (g_font_size_medium - offset);

    // List upgrades as additional options
    for (int n = 0; n < upgrades.size(); n++) {
        const Upgrade& upgrade = upgrades[n];
        rect.x = 10;

        std::string up_name = upgrade.name;
        if (upgrade.bought) {
            up_name = "Bought: " + up_name;
        }

        Render_Text(renderer, g_medium_font, back_color, rect, up_name.c_str());
        rect.x -= offset;
        rect.y -= offset;
        if (current_menu_item == (n+1))
            Render_Text(renderer, g_medium_font, select_color, rect, up_name.c_str());
        else
            Render_Text(renderer, g_medium_font, color, rect, up_name.c_str());
        rect.y += (g_font_size_medium - offset);
    }

    // show upgrade stats if a upgrade is hovered
    if (current_menu_item != 0) {
        const Upgrade& upgrade = upgrades[current_menu_item - 1];

        rect.y = 300;
        rect.x = 450;

        Render_Text(renderer, g_medium_font, back_color, rect, upgrade.name.c_str());
        rect.x -= offset;
        rect.y -= offset;
        Render_Text(renderer, g_medium_font, color, rect, upgrade.name.c_str());
        rect.y += (g_font_size_medium - offset);

        snprintf(buffer, 32, "Cost: %d", upgrade.cost);
        Render_Text(renderer, g_medium_font, back_color, rect, buffer);
        rect.x -= offset;
        rect.y -= offset;
        Render_Text(renderer, g_medium_font, color, rect, buffer);
        rect.y += (g_font_size_medium - offset);

        if (upgrade.speed != 0) {
            snprintf(buffer, 32, "Speed +%d", upgrade.speed);
            Render_Text(renderer, g_medium_font, back_color, rect, buffer);
            rect.x -= offset;
            rect.y -= offset;
            Render_Text(renderer, g_medium_font, color, rect, buffer);
            rect.y += (g_font_size_medium - offset);
        }

        if (upgrade.strength != 0) {
            snprintf(buffer, 32, "Strength +%d", upgrade.strength);
            Render_Text(renderer, g_medium_font, back_color, rect, buffer);
            rect.x -= offset;
            rect.y -= offset;
            Render_Text(renderer, g_medium_font, color, rect, buffer);
            rect.y += (g_font_size_medium - offset);
        }

        if (upgrade.extraction != 0) {
            snprintf(buffer, 32, "Extraction +%d", upgrade.extraction);
            Render_Text(renderer, g_medium_font, back_color, rect, buffer);
            rect.x -= offset;
            rect.y -= offset;
            Render_Text(renderer, g_medium_font, color, rect, buffer);
            rect.y += (g_font_size_medium - offset);
        }

        if (upgrade.ability != 0) {
            snprintf(buffer, 32, "[%s]", Ability_Names[upgrade.ability]);
            Render_Text(renderer, g_medium_font, back_color, rect, buffer);
            rect.x -= offset;
            rect.y -= offset;
            Render_Text(renderer, g_medium_font, color, rect, buffer);
            rect.y += (g_font_size_medium - offset);
        }

        snprintf(buffer, 256, "%s", upgrade.description.c_str());
        Render_Text(renderer, g_small_font, back_color, rect, buffer);
        rect.x -= offset;
        rect.y -= offset;
        Render_Text(renderer, g_small_font, color, rect, buffer);
        rect.y += (g_font_size_small - offset);
    }
}

bool Shop_State::On_Action_Event(Action_Event action) {
    if (action.pressed) {
        if (action.action == Action_Back) {
            Leave_Shop();
            return true;
        } else if (action.action == Action_Up) {
            current_menu_item--;
            if (current_menu_item >= num_options) {
                current_menu_item = num_options-1;
            }
            return true;
        } else if (action.action == Action_Down) {
            current_menu_item++;
            if (current_menu_item >= num_options) {
                current_menu_item = 0;
            }
            return true;
        } else if (action.action == Action_A) {
            if (current_menu_item == 0) {
                log_info("Enter pressed [Leave]");
                Leave_Shop();
                return true;
            }

            uint8 upgrade_num = current_menu_item - 1;
            Upgrade& upgrade = upgrades[upgrade_num];
            log_info("Enter pressed [%s]", upgrade.name.c_str());

            if (!upgrade.bought && handover.food >= upgrade.cost) {
                handover.food -= upgrade.cost;
                upgrade.bought = true;

                handover.dig_speed += upgrade.speed;
                handover.dig_strength += upgrade.strength;
                handover.extraction += upgrade.extraction;
                handover.abilities |= upgrade.ability;
                
                //upgrade.name = "Bought: " + upgrade.name;
            }

            return true;
        }
    }


    return false;
}

void Shop_State::Leave_Shop() {
    //if (!Write_Config()) {
    //    log_error("Failed to write shop config!");
    //}

    g_game.Pop_State();
}

bool Shop_State::Read_Config() {
    std::ifstream init_shop("data/shop_init.json");
    std::ifstream saved_shop("shop.json");

    try {
        json data = json::parse(init_shop);
        log_info("Loading config from data/init.json");

        // Shop Title
        if (data.find("Title") != data.end()) {
            shop_title = data["Title"].get<std::string>();
        }

        // check if save file exists, and if so read options from that.
        json upgrade_list;
        if (saved_shop.is_open()) {
            json conf = json::parse(saved_shop);
            log_info("Loading upgrades from shop.json");
            upgrade_list = conf["Upgrades"];
        } else {
            upgrade_list = data["Upgrades"];
        }

        int num_upgrades = upgrade_list.size();        
        for (int n = 0; n < num_upgrades; n++) {
            json up = upgrade_list[n];

            Upgrade upgrade;
            upgrade.name        = up["Name"].get<std::string>();        // required
            upgrade.cost        = up["Cost"].get<int16>();              // required
            upgrade.description = up["Description"].get<std::string>(); // required

            if (up.find("Speed") != up.end())       upgrade.speed = up["Speed"].get<uint8>();
            if (up.find("Strength") != up.end())    upgrade.strength = up["Strength"].get<uint8>();
            if (up.find("Extraction") != up.end())  upgrade.extraction = up["Extraction"].get<uint8>();
            if (up.find("Ability") != up.end())     upgrade.ability = up["Ability"].get<uint8>();
            
            upgrade.bought   = false;
            this->upgrades.push_back(upgrade);
        }
    } catch (json::parse_error e) {
        log_error("Json parse exception: [%s]", e.what());
        return false;
    } catch (json::type_error e) {
        log_error("Json type exception: [%s]", e.what());
        return false;
    } catch (json::other_error e) {
        log_error("Json other exception: [%s]", e.what());
        return false;
    }

    return true;
}

bool Shop_State::Write_Config() {
    // try to re-serialze to disk
    try {
        json upgrade_list = json::array();

        for (const Upgrade& upgrade : upgrades) {
            if (upgrade.bought) {
                continue;
            }

            json up = {
                {"Name", upgrade.name},
                {"Cost", upgrade.cost},
                {"Description", upgrade.description},
            };
            if (upgrade.speed != 0)      up["Speed"] = upgrade.speed;
            if (upgrade.strength != 0)   up["Strength"] = upgrade.strength;
            if (upgrade.extraction != 0) up["Extraction"] = upgrade.extraction;
            if (upgrade.ability != 0)    up["Ability"] = upgrade.ability;

            upgrade_list.push_back(up);
        }

        json data = {
            {"Upgrades", upgrade_list}
        };

        std::ofstream fp("shop.json");
        fp << std::setw(2) << data << std::endl;
        fp.close();
        log_info("Options written to shop.json");

    } catch (json::parse_error e) {
        log_error("Json parse exception: [%s]", e.what());
        return false;
    } catch (json::type_error e) {
        log_error("Json type exception: [%s]", e.what());
        return false;
    } catch (json::other_error e) {
        log_error("Json other exception: [%s]", e.what());
        return false;
    }

    return true;
}
