#include "Input_Mapping.h"

#include "Game_Pad.h"
#include "SDL_keycode.h"

#define SDL_HAT_UP          0x01
#define SDL_HAT_RIGHT       0x02
#define SDL_HAT_DOWN        0x04
#define SDL_HAT_LEFT        0x08

static bool dpad_up = false;
static bool dpad_right = false;
static bool dpad_down = false;
static bool dpad_left = false;

Action_Event Action_Event_From_Hat_Event(uint8 hat_code) {
    // todo!
    Action_Event event;
    event.action = Action_None;
    event.pressed = false;

    if (hat_code & SDL_HAT_UP) {
        event.action = Action_Up;
        event.pressed = true;
    } else if (hat_code & SDL_HAT_RIGHT) {
        event.action = Action_Right;
        event.pressed = true;
    } else if (hat_code & SDL_HAT_LEFT) {
        event.action = Action_Left;
        event.pressed = true;
    } else if (hat_code & SDL_HAT_DOWN) {
        event.action = Action_Down;
        event.pressed = true;
    }

    return event;
}

Action_Event Action_Event_From_Button_Event(uint8 button, bool pressed) {
    Action_Event event;

    event.pressed = pressed;
    event.action = Action_None;
    switch (button) {
        case Joystick_Buttons::BUTTON_A: event.action = Action_A; break;
        case Joystick_Buttons::BUTTON_B: event.action = Action_B; break;
        case Joystick_Buttons::BUTTON_X: event.action = Action_X; break;
        case Joystick_Buttons::BUTTON_Y: event.action = Action_Y; break;

        case Joystick_Buttons::LEFT_BUMPER:  event.action = Action_LB; break;
        case Joystick_Buttons::RIGHT_BUMPER: event.action = Action_RB; break;

        case Joystick_Buttons::BUTTON_BACK:  event.action = Action_Back; break;
        case Joystick_Buttons::BUTTON_START: event.action = Action_Pause; break;
    }
    return event;
}

Action_Event Action_Event_From_Key_Event(int32 key_code, bool pressed) {
    Action_Event event;

    event.pressed = pressed;
    event.action = Action_None;
    switch (key_code) {
        case SDLK_RETURN: event.action = Action_A; break;
        case SDLK_BACKSPACE: event.action = Action_B; break;

        case SDLK_j: event.action = Action_A; break;
        case SDLK_k: event.action = Action_B; break;
        case SDLK_u: event.action = Action_X; break;
        case SDLK_i: event.action = Action_Y; break;

        case SDLK_1: event.action = Action_A; break;
        case SDLK_2: event.action = Action_B; break;
        case SDLK_3: event.action = Action_X; break;
        case SDLK_4: event.action = Action_Y; break;

        case SDLK_s: event.action = Action_Down; break;
        case SDLK_d: event.action = Action_Right; break;
        case SDLK_a: event.action = Action_Left; break;
        case SDLK_w: event.action = Action_Up; break;

        case SDLK_DOWN: event.action = Action_Down; break;
        case SDLK_RIGHT: event.action = Action_Right; break;
        case SDLK_LEFT: event.action = Action_Left; break;
        case SDLK_UP: event.action = Action_Up; break;

        case SDLK_q: event.action = Action_LB; break;
        case SDLK_e: event.action = Action_RB; break;

        case SDLK_TAB: event.action = Action_Back; break;
        case SDLK_ESCAPE: event.action = Action_Pause; break;
    }
    return event;
}

void Axis_Mapping::Update_Keys(const uint8* keystate, int num_keys) {
    // left stick -> wasd
    int left_up    = keystate[SDL_SCANCODE_W];
    int left_down  = keystate[SDL_SCANCODE_S];
    int left_left  = keystate[SDL_SCANCODE_A];
    int left_right = keystate[SDL_SCANCODE_D];
    
    int vert = left_up - left_down;
    int horz = left_right - left_left;

    if (vert) {
        this->axes[Axis_Left_Vert] = (real32)vert;
    }
    if (horz) {
        this->axes[Axis_Left_Horiz] = (real32)horz;
    }

    // right stick -> arrow keys?
    int right_up    = keystate[SDL_SCANCODE_UP];
    int right_down  = keystate[SDL_SCANCODE_DOWN];
    int right_left  = keystate[SDL_SCANCODE_LEFT];
    int right_right = keystate[SDL_SCANCODE_RIGHT];
    
    vert = right_up - right_down;
    horz = right_right - right_left;

    if (vert) {
        this->axes[Axis_Right_Vert] = (real32)vert;
    }
    if (horz) {
        this->axes[Axis_Right_Horiz] = (real32)horz;
    }

    // Triggers
    int lt = keystate[SDL_SCANCODE_Q];
    if (lt) {
        this->axes[Axis_Left_Trigger] = (real32)lt;
    }

    int rt = keystate[SDL_SCANCODE_E];
    if (rt) {
        this->axes[Axis_Right_Trigger] = (real32)rt;
    }
}

const char* enum_string[] = {
    "Action_None",
    "Action_Up",
    "Action_Down",
    "Action_Left",
    "Action_Right",
    "Action_Pause",
    "Action_Back",
    "Action_A",
    "Action_B",
    "Action_X",
    "Action_Y",
    "Action_LB",
    "Action_RB"
};

const char* Action_Event_String(Input_Actions action) {
    return enum_string[action];
}
