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

World_State::World_State(const char* filename) {
    log_trace("World_State::World_State(%s)", filename);
    level_name = filename;

    // Initialize world origin
    world.origin = laml::Vec2(g_window_width/2.0, g_window_height/2.0);
    world.cam_world_pos = laml::Vec2(0.0, 0.0);
    world.sprite_origin.Load_Sprite_Sheet(g_game.GetRenderer(), "data/plus.png", 32, 32, 16, 16);
    world.sprite_cam.Load_Sprite_Sheet(g_game.GetRenderer(), "data/plus2.png", 32, 32, 16, 16);
    world.grid_x = 32;
    world.grid_y = 32;
    world.grid.Create(8, 8);
    world.grid.map[1][3] = 5;

    // Load sprite-sheet
    player.sprite.Load_Sprite_Sheet_From_Meta(g_game.GetRenderer(), "data/thingy.sprite");
    player.world_x = 0;
    player.world_y = 0;
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
    real32 stick_x, stick_y;
    
    stick_x = g_game.GetAxes()->axes[Axis_Left_Horiz];
    stick_y = g_game.GetAxes()->axes[Axis_Left_Vert];
    //player.world_x += stick_x * (1.0 * dt);
    //player.world_y += stick_y * (1.0 * dt);
    real32 l_trigger = g_game.GetAxes()->axes[Axis_Left_Trigger];
    real32 r_trigger = g_game.GetAxes()->axes[Axis_Right_Trigger];
    player.angle += (r_trigger - l_trigger) * 360.0*dt;
    player.angle = laml::map(player.angle, 0.0, 360.0);

    stick_x = g_game.GetAxes()->axes[Axis_Right_Horiz];
    stick_y = g_game.GetAxes()->axes[Axis_Right_Vert];
    world.cam_world_pos = world.cam_world_pos + laml::Vec2(stick_x, stick_y) * (250.0 * dt);


    //////////// Render
    // tilemap
    Draw_Tilemap_Debug(renderer, &world.grid, world.Get_Origin_Screen_Pos(), world.grid_x, world.grid_y);

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
    } else if (action.action == Action_Right && action.pressed) {
        this->player.world_x++;
        return true;
    } else if (action.action == Action_Left && action.pressed) {
        this->player.world_x--;
        return true;
    } else if (action.action == Action_Up && action.pressed) {
        this->player.world_y++;
        return true;
    } else if (action.action == Action_Down && action.pressed) {
        this->player.world_y--;
        return true;
    }

    return false;
}