#include "Game_Pad.h"

// Copied from SDL_Joystick.h
#define SDL_HAT_UP          0x01
#define SDL_HAT_RIGHT       0x02
#define SDL_HAT_DOWN        0x04
#define SDL_HAT_LEFT        0x08

#define SDL_JOYSTICK_AXIS_MAX    32767
#define SDL_JOYSTICK_AXIS_MIN   -32768

Game_Pad_Axis::Game_Pad_Axis() 
 : min_raw(SDL_JOYSTICK_AXIS_MIN), 
   max_raw(SDL_JOYSTICK_AXIS_MAX),
   min_scaled(-1.0), 
   max_scaled(1.0)
{}
Game_Pad_Axis::Game_Pad_Axis(int16 min_r, int16 max_r, real32 min_s, real32 max_s) 
 : min_raw(min_r), 
   max_raw(max_r), 
   min_scaled(min_s), 
   max_scaled(max_s)
{}

void Game_Pad_Axis::Update(int16 raw_value, bool negate) {
    const real32 r_diff = (real32)(raw_value - min_raw);
    const real32 r_range = (real32)(max_raw - min_raw);
    this->scaled = min_scaled + ((max_scaled-min_scaled) * (r_diff/r_range));
    if (negate)
        this->scaled *= -1.0;
}

Game_Pad::Game_Pad() {
    // configure standard axes
    this->axes[Joystick_Axes::LEFT_STICK_X] = Game_Pad_Axis(SDL_JOYSTICK_AXIS_MIN, SDL_JOYSTICK_AXIS_MAX, -1.0, 1.0);
    this->axes[Joystick_Axes::LEFT_STICK_Y] = Game_Pad_Axis(SDL_JOYSTICK_AXIS_MIN, SDL_JOYSTICK_AXIS_MAX, -1.0, 1.0);
    this->axes[Joystick_Axes::RIGHT_STICK_X] = Game_Pad_Axis(SDL_JOYSTICK_AXIS_MIN, SDL_JOYSTICK_AXIS_MAX, -1.0, 1.0);
    this->axes[Joystick_Axes::RIGHT_STICK_Y] = Game_Pad_Axis(SDL_JOYSTICK_AXIS_MIN, SDL_JOYSTICK_AXIS_MAX, -1.0, 1.0);

    this->axes[Joystick_Axes::LEFT_TRIGGER] = Game_Pad_Axis(SDL_JOYSTICK_AXIS_MIN, SDL_JOYSTICK_AXIS_MAX, 0.0, 1.0);
    this->axes[Joystick_Axes::RIGHT_TRIGGER] = Game_Pad_Axis(SDL_JOYSTICK_AXIS_MIN, SDL_JOYSTICK_AXIS_MAX, 0.0, 1.0);

    for (int n = 0; n < Joystick_Buttons::NUM_BUTTONS; n++) {
        this->buttons[n] = false;
    }

    // deadzone defaults
    left_stick_deadzone    = 0.5;
    right_stick_deadzone   = 0.5;
    left_trigger_deadzone  = 0.5;
    right_trigger_deadzone = 0.5;
}

void Game_Pad::Set_Deadzones(real32 left_stick, real32 right_stick, real32 left_trigger, real32 right_trigger) {

}

void Game_Pad::Hat_Event(uint8 hat_code) {
    buttons[DPAD_UP] = (hat_code & SDL_HAT_UP);
    buttons[DPAD_DOWN] = (hat_code & SDL_HAT_DOWN);
    buttons[DPAD_LEFT] = (hat_code & SDL_HAT_LEFT);
    buttons[DPAD_RIGHT] = (hat_code & SDL_HAT_RIGHT);
}

void Game_Pad::Button_Event(uint8 button, bool pressed) {
    this->buttons[button] = pressed;
}

void Game_Pad::Update_Axis(uint8 axis, int16 raw_value, bool negate) {
    this->axes[axis].Update(raw_value, negate);
}

bool Game_Pad::operator[](uint8 idx) const {
    return this->buttons[idx];
}

real32 Game_Pad::Get_Axis(uint8 axis) const {
    return this->axes[axis].scaled;
}

void Game_Pad::Get_Left_Stick(real32& x, real32& y) const {
    // get values
    real32 stick_x = Get_Axis(Joystick_Axes::LEFT_STICK_X);
    real32 stick_y = Get_Axis(Joystick_Axes::LEFT_STICK_Y);

    // remove deadzone -> circular deadzone
    real32 mag = sqrt(stick_x*stick_x + stick_y*stick_y);
    if (mag < left_stick_deadzone) {
        x = 0.0;
        y = 0.0;
    } else {
        // remap from mag -> 1.0
        real32 new_mag = (mag - left_stick_deadzone) / (1.0 - left_stick_deadzone);
        x = stick_x*new_mag/mag;
        y = stick_y*new_mag/mag;
    }
}

void Game_Pad::Get_Right_Stick(real32& x, real32& y) const {
    // get values
    real32 stick_x = Get_Axis(Joystick_Axes::RIGHT_STICK_X);
    real32 stick_y = Get_Axis(Joystick_Axes::RIGHT_STICK_Y);

    // remove deadzone -> circular deadzone
    real32 mag = sqrt(stick_x*stick_x + stick_y*stick_y);
    if (mag < right_stick_deadzone) {
        x = 0.0;
        y = 0.0;
    } else {
        // remap from mag -> 1.0
        real32 new_mag = (mag - right_stick_deadzone) / (1.0 - right_stick_deadzone);
        x = stick_x*new_mag/mag;
        y = stick_y*new_mag/mag;
    }
}