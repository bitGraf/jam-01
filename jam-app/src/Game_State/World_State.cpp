#include "Game_State.h"

#include "Render_Utils.h"
#include "logging.h"

extern TTF_Font* g_small_font;
extern int32 g_font_size_small;
const int32 offset = 2;

#include "Game_App.h"
extern Game_App g_game;
extern int32 g_window_width;
extern int32 g_window_height;

enum Move_Reason {
    MOVE_NOT_POSSIBLE = 0,
    MOVE_POSSIBLE = 1,
    MOVE_AND_BREAK = 2
};

uint8 Move_Entity(int16 start_x, int16 start_y, int16 move_x, int16 move_y, const World& world);
uint8 Break_Block(int16 world_x, int16 world_y, World& world);

World_State::World_State(const char* filename) {
    log_trace("World_State::World_State(%s)", filename);
    level_name = filename;

    ground_level = 8;

    // Initialize world origin
    world.origin = laml::Vec2(0.0, g_window_height);
    world.Init_Grid(25, 60, 32, 32);
    world.cam_world_pos.x = -16;
    world.cam_world_pos.y = g_window_height - 16;

    world.grid.Fill(1);
    world.grid.Fill(1, 1, 23, ground_level, 0);
    world.grid.Fill(1, ground_level+1, 23, world.grid.map_height - ground_level - 2, 2);
    world.grid.Fill(15, ground_level-4, 8, 5, 1);

    // Load sprite-sheet
    player.sprite.Load_Sprite_Sheet_From_Meta(g_game.GetRenderer(), "data/thingy.sprite");
    player.world_x = 6;
    player.world_y = ground_level;
    player.angle = 0.0;
}

World_State::~World_State() {
    log_trace("World_State::~World_State(%s)", level_name.c_str());
}

void World_State::Update_And_Render(SDL_Renderer* renderer, real32 dt) {
    SDL_SetRenderDrawColor(renderer, 120, 90, 255, 255);
    SDL_RenderClear(renderer);

    // Draw sprite
    player.sprite.Update(dt);
    real32 stick_x = 0.0, stick_y = 0.0;
    
    //stick_x = g_game.GetAxes()->axes[Axis_Left_Horiz];
    //stick_y = g_game.GetAxes()->axes[Axis_Left_Vert];
    //player.world_x += stick_x * (1.0 * dt);
    //player.world_y += stick_y * (1.0 * dt);

    real32 l_trigger = g_game.GetAxes()->axes[Axis_Left_Trigger];
    real32 r_trigger = g_game.GetAxes()->axes[Axis_Right_Trigger];
    player.angle += (r_trigger - l_trigger) * 360.0*dt;
    player.angle = laml::map(player.angle, 0.0, 360.0);

    stick_x =  0.0 * g_game.GetAxes()->axes[Axis_Right_Horiz];
    stick_y = -1.0 * g_game.GetAxes()->axes[Axis_Right_Vert];
    world.cam_world_pos = world.cam_world_pos + laml::Vec2(stick_x, stick_y) * (500.0 * dt);
    if (world.cam_world_pos.y < (g_window_height - 16)) {
        world.cam_world_pos.y = g_window_height - 16;
    }

    int32 depth = player.world_y - ground_level;


    //////////// Render
    // tilemap
    Draw_Tilemap(renderer, &world.grid, &world.tilesheet, world.Get_Origin_Screen_Pos(), world.grid_size_x, world.grid_size_y);
    //Draw_Tilemap_Debug(renderer, &world.grid, world.Get_Origin_Screen_Pos(), world.grid_x, world.grid_y);

    // origin
    Draw_Sprite(renderer, world.sprite_origin, world.Get_Origin_Screen_Pos(), 0.0);

    // camera
    Draw_Sprite(renderer, world.sprite_cam, world.Get_Screen_Pos(world.cam_world_pos), 0.0);

    // sprite
    laml::Vec2 world_pos = world.Get_World_Pos(player.world_x, player.world_y);
    laml::Vec2 screen_pos = world.Get_Screen_Pos(player.world_x, player.world_y);
    Draw_Sprite(renderer, player.sprite, screen_pos, player.angle);

    // world info
    SDL_Rect rect = { 4, 100, 0, 0 };
    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Color back_color = { 0, 0, 0, 255 };
    char buffer[256];

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
}

bool World_State::On_Action_Event(Action_Event action) {
    if (action.action == Action_Pause && action.pressed) {
        Game_State* pause = new Pause_State();
        g_game.Push_New_State(pause);
        return true;
    } else if (action.action == Action_X && action.pressed) {
        this->player.sprite.Set_Sequence(0, 0);
        return true;
    } else if (action.action == Action_Y && action.pressed) {
        this->player.sprite.Set_Sequence(1, 0);
        return true;
    } else if (action.action == Action_B && action.pressed) {
        this->player.sprite.Set_Sequence(2, 0);
        return true;
    }
    
    int16 move_x = 0, move_y = 0;
    if (action.action == Action_Right && action.pressed) {
        move_x = 1;
    } else if (action.action == Action_Left && action.pressed) {
        move_x = -1;
    } else if (action.action == Action_Up && action.pressed) {
        move_y = -1;
    } else if (action.action == Action_Down && action.pressed) {
        move_y = 1;
    }

    if (move_x || move_y) {
        uint8 reason = Move_Entity(player.world_x, player.world_y, move_x, move_y, world);
        switch (reason) {
            case MOVE_POSSIBLE: {
                this->player.world_x += move_x;
                this->player.world_y += move_y;
            } break;

            case MOVE_AND_BREAK: {
                this->player.world_x += move_x;
                this->player.world_y += move_y;

                Break_Block(player.world_x, player.world_y, world);
            } break;
        }

        return true;
    }

    return false;
}

uint8 Move_Entity(int16 start_x, int16 start_y, int16 move_x, int16 move_y, const World& world) {
    int16 end_x = start_x + move_x;
    int16 end_y = start_y + move_y;

    uint8 start_cell = world.grid[start_x][start_y];
    uint8 end_cell = world.grid[end_x][end_y];

    if (end_cell == 0) {
        return MOVE_POSSIBLE;
    }

    if (end_cell == 2) {
        return MOVE_AND_BREAK;
    }

    return MOVE_NOT_POSSIBLE;
}

uint8 Break_Block(int16 world_x, int16 world_y, World& world) {
    uint8 cell = world.grid[world_x][world_y];
    log_debug("destroy cell: %d", cell);

    world.grid[world_x][world_y] = 0;

    return 0;
}