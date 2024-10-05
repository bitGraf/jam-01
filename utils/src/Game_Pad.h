#pragma once

#include <laml/laml.hpp>

enum Joystick_Buttons {
    BUTTON_A = 0,
    BUTTON_B = 1,
    BUTTON_X = 2,
    BUTTON_Y = 3,
    LEFT_BUMPER = 4,
    RIGHT_BUMPER = 5,
    BUTTON_BACK = 6,
    BUTTON_START = 7,
    BUTTON_LEFT_STICK = 8,
    BUTTON_RIGHT_STICK = 9,
    BUTTON_CENTER = 10,
    DPAD_UP = 11,
    DPAD_DOWN = 12,
    DPAD_LEFT = 13,
    DPAD_RIGHT = 14,

    NUM_BUTTONS
};

enum Joystick_Axes {
    LEFT_STICK_X  = 0,
    LEFT_STICK_Y  = 1,
    RIGHT_STICK_X = 2,
    RIGHT_STICK_Y = 3,
    LEFT_TRIGGER  = 4,
    RIGHT_TRIGGER = 5,

    NUM_AXES
};

struct Game_Pad_Axis {
    Game_Pad_Axis();
    Game_Pad_Axis(int16 min_r, int16 max_r, real32 min_s, real32 max_s);
    void Update(int16 raw_value, bool negate);

    int16 raw;
    real32 scaled;

    int16 min_raw, max_raw;
    real32 min_scaled, max_scaled;
};

struct Game_Pad {
    Game_Pad();

    void Set_Deadzones(real32 left_stick, real32 right_stick, real32 left_trigger, real32 right_trigger);

    void Hat_Event(uint8 hat_code);
    void Button_Event(uint8 button, bool pressed);
    void Update_Axis(uint8 axis, int16 raw_value, bool negate = false);

    bool operator[](uint8 idx) const;
    real32 Get_Axis(uint8 axis) const;

    void Get_Left_Stick(real32& x, real32& y) const;
    void Get_Right_Stick(real32& x, real32& y) const;

private:
    bool buttons[Joystick_Buttons::NUM_BUTTONS];
    Game_Pad_Axis axes[Joystick_Axes::NUM_AXES];

    real32 left_stick_deadzone;
    real32 right_stick_deadzone;
    real32 left_trigger_deadzone;
    real32 right_trigger_deadzone;
};