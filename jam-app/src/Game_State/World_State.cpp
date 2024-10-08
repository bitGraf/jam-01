#include "Game_State.h"

#include "utils.h"
#include "Render_Utils.h"
#include "logging.h"

extern TTF_Font* g_small_font;
extern int32 g_font_size_small;
const int32 offset = 2;

#include "Game_App.h"
extern Game_App g_game;
extern int32 g_window_width;
extern int32 g_window_height;

// json parser
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

static const char* world_init_filename = "data/init/world_init.json";
static const char* world_filename = "save/world.json";

const int16 world_width = 25;
const int16 world_height = 60;
const real32 day_start = 6;  //  6:00 am
const real32 day_end = 22.0; // 10:00 pm

int16 ground_level;
int16 spawn_x;
int16 spawn_y;

real32 cam_min_y;
real32 cam_max_y;

enum Move_Reason {
    MOVE_NOT_POSSIBLE = 0,
    MOVE_POSSIBLE,
    MOVE_AND_BREAK,
    DEPOSIT_FOOD,
    WITHDRAW_FOOD,
    ENTER_SHOP,
    SLEEP
};

enum Tileset_Types {
    Type_Empty              =  0,
    Type_Bedrock            =  1,
    Type_Dirt               =  2,
    Type_Sap                =  3,
    Type_Deposit            =  4,
    Type_Withdraw           =  5,
    Type_Shop_TL            =  6,
    Type_Shop_TR            =  7,
    Type_Bed                =  8,
    //Type_Empty            =  9,   // Unused!
    //Type_Empty            = 10,   // Unused!
    //Type_Empty            = 11,   // Unused!
    //Type_Empty            = 12,   // Unused!
    //Type_Empty            = 13,   // Unused!
    Type_Shop_BL            = 14,
    Type_Shop_Entrance      = 15,
};

World_State::World_State(const char* filename) 
 : handover(colony_food, dig_speed, dig_strength, extraction, abilities)
{
    log_trace("World_State::World_State(%s)", filename);
    level_name = filename;

    if (!Read_Config(world_init_filename, world_filename)) {
        log_error("Failed to load world config!");
    }

    ground_level = 8;
    spawn_x = 13;
    spawn_y = ground_level;

    // Initialize world origin
    world.origin = laml::Vec2(0.0, g_window_height);
    world.Init_Grid("data/tilesheet.png", world_width, world_height, 32, 32);
    world.cam_world_pos.x = -world.grid_size_x/2;
    world.cam_world_pos.y = g_window_height - world.grid_size_y/2;

    cam_min_y = (g_window_height - world.grid_size_y/2);
    cam_max_y = (world_height*32 - world.grid_size_y/2);

    world.grid.Fill(Tile_Data(Type_Bedrock));                          // fill whole with bedrock
    world.grid.Fill(1, 1, world_width-2, ground_level, Tile_Data(Type_Empty,1));  // empty out burrow
    
    // fill ground with dirt, becoming tougher with depth
    for (int x = 1; x < (world_width-1); x++) {
        Tile_Data* col_data = world.grid[x];
        for (int y = ground_level+1; y < (world_height-1); y++) {
            int16 depth = y - ground_level;
            depth = (depth < 0) ? 0 : depth;

            col_data[y].type = Type_Dirt;
            col_data[y].data_1 = 1+(depth / 5);

            //if (x == 1) {
            //    log_debug("depth: %d  | strength: %d", depth, col_data[y].data_1);
            //}
        }
    }

    world.grid.Fill(1, ground_level+1, 4, 1, Tile_Data(Type_Bedrock));  // put floors down in burrow
    world.grid.Fill(11, ground_level+1, 13, 1, Tile_Data(Type_Bedrock));  // put floors down in burrow
    world.grid.Fill(15, ground_level-4, 8, 5, Tile_Data(Type_Bedrock));  // build house
    world.grid[15][ground_level] = Tile_Data(Type_Deposit);    // depositer
    world.grid[14][ground_level+1] = Tile_Data(Type_Withdraw);  // withdrawer
    world.grid[12][ground_level+1] = Tile_Data(Type_Bed);  // bed

    // build upgrade hut
    world.grid[2][ground_level-1] = Tile_Data(Type_Shop_TL);
    world.grid[3][ground_level-1] = Tile_Data(Type_Shop_TR);
    world.grid[2][ground_level]   = Tile_Data(Type_Shop_BL);
    world.grid[3][ground_level]   = Tile_Data(Type_Shop_Entrance);

    // place dew drops
    int num_to_place = 20;
    for (int n = 0; n < num_to_place; n++) {
        int16 x = rand_int(1, (world_width-2));
        int16 y = rand_int(ground_level+5, (world_height-2));

        int16 depth = y - ground_level;
        depth = (depth < 0) ? 0 : depth;

        world.grid[x][y].type = Type_Sap;
        world.grid[x][y].data_1 = 1;
        world.grid[x][y].data_2 = 1 + (depth / 5);
    }

    // place boulders
    num_to_place = 200;
    for (int n = 0; n < num_to_place; n++) {
        int16 x = rand_int(1, (world_width-2));
        int16 y = rand_int(ground_level+5, (world_height-2));

        world.grid[x][y] = Tile_Data(Type_Bedrock);
    }

    // Load sprite-sheet
    player.sprite.Load_Sprite_Sheet_From_Meta(g_game.GetRenderer(), "data/sprites/bug.sprite");
    player.world_x = spawn_x;
    player.world_y = spawn_y;
    player.angle = 0.0;
    dig_strength = 1;
    dig_speed = 1;
    extraction = 1;
    abilities = 0;
    can_hibernate = false;
    can_dash = false;

    // day-night cycles
    time_gradient.Load_From_Image("data/sky_gradient.png");
    time_gradient.Set_Mapping_Range(0.0, 24.0);
    time_rate = 0.5; // 30 mins every second?
    fast_forward_ratio = 10.0f;
    time_of_day = day_start;
    day_number = 1;
    fast_forward = false;
    pan_to_colony = false;

    // Hunger
    hunger = 0.0;
    hunger_rate = 0.001;
    move_cost = 0.002;
    break_cost = 0.01;
    colony_hunger = 0.0;
    carrying_food = 0;
    colony_food = 1000;
    colony_size = 5;
    field_eat_ratio = 0.25;
    colony_eat_ratio = 1.0;

    show_debug = false;
}

World_State::~World_State() {
    if (!Write_Config(world_filename)) {
        log_error("Failed to write world save!");
    }

    log_trace("World_State::~World_State(%s)", level_name.c_str());
}

void World_State::Update_And_Render(SDL_Renderer* renderer, real32 dt) {
    int32 depth = player.world_y - ground_level;
    bool in_colony = (depth <= 0);

    // Read inputs
    real32 l_stick_x = g_game.GetAxes()->axes[Axis_Left_Horiz];
    real32 l_stick_y = g_game.GetAxes()->axes[Axis_Left_Vert];

    real32 r_stick_x = g_game.GetAxes()->axes[Axis_Right_Horiz];
    real32 r_stick_y = g_game.GetAxes()->axes[Axis_Right_Vert];

    real32 l_trigger = g_game.GetAxes()->axes[Axis_Left_Trigger];
    real32 r_trigger = g_game.GetAxes()->axes[Axis_Right_Trigger];

    can_hibernate = (abilities & ABILITY_HIBERNATE);
    can_dash      = (abilities & ABILITY_DASH);

    // Update Time
    real32 time_advance_rate = fast_forward ? (time_rate*fast_forward_ratio) : time_rate;
    if (!fast_forward) time_advance_rate *= 1.0f + (r_trigger*9.0); // not pressed->1.0, max press->10.0
    time_of_day += time_advance_rate * dt;
    if (time_of_day > 24.0f) {
        day_number++;
        time_of_day -= 24.0f;
    }
    SDL_Color bg = time_gradient.Sample(time_of_day);

    if ((time_of_day > day_end) && !fast_forward) {
        if (in_colony || can_hibernate) {
            log_info("End of day! go to sleep.");
            Next_Day();
        } else {
            log_info("End of day! death.");
            Death();
        }
    }

    if (fast_forward && (fast_forward_day==day_number) && (time_of_day > day_start)) {
        if (pan_to_colony) {
            player.world_x = spawn_x;
            player.world_y = spawn_y;
            player.angle = 0.0;
        }
        fast_forward = false;
        pan_to_colony = false;
        this->player.sprite.Set_Sequence(0, 0);
    }

    // clear background
    //SDL_SetRenderDrawColor(renderer, 120, 90, 255, 255);
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(renderer);

    // Update sprite
    player.sprite.Update(dt);

    // Update hunger stats    
    hunger += (hunger_rate * dt);
    if (hunger > 1.0) {
        if (in_colony) {
            colony_food--;
            hunger -= colony_eat_ratio;
        } else {
            carrying_food--;
            hunger -= field_eat_ratio;
        }
    }
    colony_hunger += (hunger_rate * (colony_size-1) * dt);  // -1 because your hunger is tracked separatly
    if (colony_hunger > 1.0) {
        colony_food--;
        colony_hunger-=1.0;
    }
    
    // Respond to inputs
    //player.world_x += l_stick_x * (1.0 * dt);
    //player.world_y += l_stick_y * (1.0 * dt);

    //player.angle += (r_trigger - l_trigger) * 360.0*dt;
    //player.angle = laml::map(player.angle, 0.0, 360.0);

    if (pan_to_colony) {
        real32 start_time = time_of_day;
        real32 end_time = day_start;
        real32 ttg = (start_time > day_end) ? (24.0 - start_time + end_time) : (end_time - start_time);
        real32 ttg_s = ttg / (fast_forward_ratio*time_rate);
        real32 ey = (world.cam_world_pos.y - cam_min_y);
        real32 rate = (ey / ttg_s);

        //log_trace("%.3f -> %.3f  |  ttg%.3f  |  e%.3f  | r%.3f", world.cam_world_pos.y, cam_min_y, ttg_s, ey, rate);

        world.cam_world_pos.y -= rate * dt;
    } else {
        world.cam_world_pos = world.cam_world_pos + laml::Vec2(0.0, -r_stick_y) * (1500.0 * dt);
        if (world.cam_world_pos.y < cam_min_y) {
            world.cam_world_pos.y = cam_min_y;
        }
        if (world.cam_world_pos.y > cam_max_y) {
            world.cam_world_pos.y = cam_max_y;
        }
    }


    //////////// Render
    // tilemap
    Draw_Tilemap(renderer, &world.grid, &world.tilesheet, world.Get_Origin_Screen_Pos(), world.grid_size_x, world.grid_size_y, ground_level);
    if (show_debug)
        Draw_Tilemap_Debug(renderer, &world.grid, world.Get_Origin_Screen_Pos(), world.grid_size_x, world.grid_size_y);

    // origin
    Draw_Sprite(renderer, world.sprite_origin, world.Get_Origin_Screen_Pos(), 0.0);

    // camera
    Draw_Sprite(renderer, world.sprite_cam, world.Get_Screen_Pos(world.cam_world_pos), 0.0);

    // sprite
    laml::Vec2 world_pos = world.Get_World_Pos(player.world_x, player.world_y);
    laml::Vec2 screen_pos = world.Get_Screen_Pos(player.world_x, player.world_y);
    Draw_Sprite(renderer, player.sprite, screen_pos, player.angle);

    SDL_Rect rect = { 4, 50, 0, 0 };
    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Color back_color = { 0, 0, 0, 255 };
    char buffer[256];

    /*
    */
    snprintf(buffer, 256, "Colony: %d", colony_size);
    Render_Text(renderer, g_small_font, color, rect, buffer);
    rect.y += g_font_size_small;
    //snprintf(buffer, 256, "  Hunger: %.6f", colony_hunger);
    //Render_Text(renderer, g_small_font, color, rect, buffer);
    //rect.y += g_font_size_small;
    snprintf(buffer, 256, "  Food: %d", colony_food);
    Render_Text(renderer, g_small_font, color, rect, buffer);
    rect.y += g_font_size_small;

    snprintf(buffer, 256, "[You]");
    Render_Text(renderer, g_small_font, color, rect, buffer);
    rect.y += g_font_size_small;
    //snprintf(buffer, 256, "  Hunger: %.6f", hunger);
    //Render_Text(renderer, g_small_font, color, rect, buffer);
    //rect.y += g_font_size_small;
    snprintf(buffer, 256, "  Food: %d", carrying_food);
    Render_Text(renderer, g_small_font, color, rect, buffer);
    rect.y += g_font_size_small;

    snprintf(buffer, 256, "Dig Speed: %d", this->dig_speed);
    Render_Text(renderer, g_small_font, color, rect, buffer);
    rect.y += g_font_size_small;
    snprintf(buffer, 256, "Dig Strength: %d", this->dig_strength);
    Render_Text(renderer, g_small_font, color, rect, buffer);
    rect.y += g_font_size_small;
    snprintf(buffer, 256, "Extraction: %d", this->extraction);
    Render_Text(renderer, g_small_font, color, rect, buffer);
    rect.y += g_font_size_small;

    /*
    // world info
    snprintf(buffer, 256, "Origin: <%.0f, %.0f>", world.origin.x, world.origin.y);
    Render_Text(renderer, g_small_font, color, rect, buffer);
    rect.y += g_font_size_small;
    snprintf(buffer, 256, "Camera: <%.0f, %.0f>", world.cam_world_pos.x, world.cam_world_pos.y);
    Render_Text(renderer, g_small_font, color, rect, buffer);
    rect.y += g_font_size_small;
    snprintf(buffer, 256, "iWorld: <%d, %d>", player.world_x, player.world_y);
    Render_Text(renderer, g_small_font, color, rect, buffer);
    rect.y += g_font_size_small;
    snprintf(buffer, 256, "World: <%.0f, %.0f>", world_pos.x, world_pos.y);
    Render_Text(renderer, g_small_font, color, rect, buffer);
    rect.y += g_font_size_small;
    snprintf(buffer, 256, "Screen: <%.0f, %.0f>", screen_pos.x, screen_pos.y);
    Render_Text(renderer, g_small_font, color, rect, buffer);
    rect.y += g_font_size_small;
    snprintf(buffer, 256, "Rotation: %.1f", player.angle);
    Render_Text(renderer, g_small_font, color, rect, buffer);
    rect.y += g_font_size_small;
    snprintf(buffer, 256, "Depth: %d", depth);
    Render_Text(renderer, g_small_font, color, rect, buffer);
    */


    // Time info
    rect.x = 300;
    rect.y = 10;

    snprintf(buffer, 256, "Day: %d", day_number);
    Render_Text(renderer, g_small_font, color, rect, buffer);
    rect.y += g_font_size_small;

    int32 time_hr = time_of_day;
    int32 time_min = ((time_of_day - (real32)time_hr) * 60.0f);
    char* am_pm = "am";
    if (time_hr >= 12) {
        time_hr -= 12;
        am_pm = "pm";
    }
    if (time_hr == 0) time_hr = 12;
    snprintf(buffer, 256, "Time: %2d:%02d %s", time_hr, time_min, am_pm);
    Render_Text(renderer, g_small_font, color, rect, buffer);
    rect.y += g_font_size_small;
}

bool World_State::On_Action_Event(Action_Event action) {
    if (action.action == Action_Pause && action.pressed) {
        Game_State* pause = new Pause_State();
        g_game.Push_New_State(pause);
        return true;
    } else if (action.action == Action_Back && action.pressed) {
        show_debug = !show_debug;
        return true;
    }
    
    if (action.pressed && !fast_forward) {
        if (action.action == Action_Y) {
            // hibernate
            if (can_hibernate) {
                this->player.sprite.Set_Sequence(1, 0);
                Next_Day();
                return true;
            }
            return false;
        } else if (action.action == Action_X) {
            // Dash Left
            if (can_dash) {
                this->player.sprite.Set_Sequence(2, 0);
                return true;
            }
            return false;
        } else if (action.action == Action_B) {
            // Dash Right
            if (can_dash) {
                this->player.sprite.Set_Sequence(3, 0);
                return true;
            }
            return false;
        } else if (action.action == Action_A) {
            // Dash Down
            if (can_dash) {
                this->player.sprite.Set_Sequence(4, 0);
                return true;
            }
            return false;
        }
    
        int16 move_x = 0, move_y = 0;
        if (action.action == Action_Right) {
            move_x = 1;
        } else if (action.action == Action_Left) {
            move_x = -1;
        } else if (action.action == Action_Up) {
            if (player.world_y > ground_level)
                move_y = -1;
        } else if (action.action == Action_Down) {
            move_y = 1;
        }

        if (move_x || move_y) {
            uint8 reason = Move_Entity(player.world_x, player.world_y, move_x, move_y);
            switch (reason) {
                case MOVE_POSSIBLE: {
                    this->player.world_x += move_x;
                    this->player.world_y += move_y;
                    hunger += move_cost;

                    world.grid[player.world_x][player.world_y].data_1 = 1; // to denote that we have visited this place before.
                } break;

                case MOVE_AND_BREAK: {
                    this->player.world_x += move_x;
                    this->player.world_y += move_y;
                    hunger += break_cost;

                    world.grid[player.world_x][player.world_y].data_1 = 1; // to denote that we have visited this place before.

                    Break_Block(player.world_x, player.world_y);
                } break;

                case DEPOSIT_FOOD: {
                    if (this->carrying_food > 0) {
                        this->carrying_food--;
                        this->colony_food++;
                    }
                } break;

                case WITHDRAW_FOOD: {
                    if (this->colony_food > 0) {
                        this->colony_food--;
                        this->carrying_food++;
                    }
                } break;

                case SLEEP: {
                    log_info("Going to sleep!");
                    bool allowed_to_sleep = true;
                    if (allowed_to_sleep) {
                        this->player.sprite.Set_Sequence(1, 0);
                        Next_Day();
                    }
                } break;

                case ENTER_SHOP: {
                    Game_State* new_state = new Shop_State(handover);
                    g_game.Push_New_State(new_state);
                }
            }

            return true;
        }
    }

    return false;
}

uint8 World_State::Move_Entity(int16 start_x, int16 start_y, int16 move_x, int16 move_y) {
    if ((abs(move_x) + abs(move_y)) > 1) {
        log_error("Move_Entity() should only move 1 block at a time");
        return MOVE_NOT_POSSIBLE;
    }

    int16 end_x = start_x + move_x;
    int16 end_y = start_y + move_y;

    Tile_Data& start_cell = world.grid[start_x][start_y];
    Tile_Data& end_cell = world.grid[end_x][end_y];

    if (end_cell.type == Type_Empty) {
        if (move_y < 0 && end_cell.data_1 != 1) {
            return MOVE_NOT_POSSIBLE;
        }

        return MOVE_POSSIBLE;
    }

    // cannot dig upwards
    if (move_y < 0) {
        return MOVE_NOT_POSSIBLE;
    }

    if (end_cell.type == Type_Dirt) { // dirt
        if (dig_speed > end_cell.data_1) end_cell.data_1 = 0;
        else end_cell.data_1 -= dig_speed;

        if (end_cell.data_1 == 0) 
            return MOVE_AND_BREAK;
        else
            return MOVE_NOT_POSSIBLE;
    }

    if (end_cell.type == Type_Sap) {
        if (dig_speed > end_cell.data_1) end_cell.data_1 = 0;
        else end_cell.data_1 -= dig_speed;

        if (end_cell.data_1 == 0) 
            return MOVE_AND_BREAK;
        else
            return MOVE_NOT_POSSIBLE;
    }

    if (end_cell.type == Type_Deposit) {
        return DEPOSIT_FOOD;
    }

    if (end_cell.type == Type_Withdraw) {
        return WITHDRAW_FOOD;
    }

    if (end_cell.type == Type_Bed) {
        return SLEEP;
    }

    if (end_cell.type == Type_Shop_Entrance) {
        return ENTER_SHOP;
    }

    return MOVE_NOT_POSSIBLE;
}

uint8 World_State::Break_Block(int16 world_x, int16 world_y) {
    Tile_Data cell = world.grid[world_x][world_y];
    log_trace("destroy cell: %d", cell.type);

    if (cell.type == Type_Sap) {
        log_info("Got some food");
        carrying_food += (cell.data_2 * extraction * extraction);
    }

    world.grid[world_x][world_y].type = Type_Empty;

    return 0;
}

void World_State::Next_Day() {
    fast_forward = true;
    fast_forward_day = day_number + 1;
}

void World_State::Death() {
    colony_size--;
    carrying_food = 0;
    hunger = 0;
    
    log_info("Spawning as new bug");

    pan_to_colony = true;

    player.angle = 90.0;
    this->player.sprite.Set_Sequence(2, 0);
    Next_Day();
}


bool World_State::Read_Config(const char* init_filename, const char* filename) {
    std::ifstream init_world(init_filename);
    std::ifstream saved_world(filename);

    try {
        json data = json::parse(init_world);
        log_info("Loading world config from '%s'", init_filename);

        // check if save file exists, and if so read options from that.
        if (saved_world.is_open()) {
            json saved = json::parse(saved_world);
            log_info("Loading world save from '%s'", filename);
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

bool World_State::Write_Config(const char* filename) {
    // try to re-serialze to disk
    try {
        json data = {
            {"placeholder", 0}
        };

        std::ofstream fp(filename);
        fp << std::setw(2) << data << std::endl;
        fp.close();
        log_info("World Save written to '%s'", filename);

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