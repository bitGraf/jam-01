#pragma once

#include <laml/laml.hpp>

enum Mouse_Buttons {
    MOUSE1 = 1,
    MOUSE3 = 2,
    MOUSE2 = 3,
    MOUSE4 = 4,
    MOUSE5 = 5,

    NUM_MOUSE_BUTTONS
};

struct Mouse {
    Mouse();

    void Update(real32 dt);

    void Update_Motion(int32 x_, int32 y_, int32 dx_, int32 dy_);
    void Button_Event(uint8 button, bool pressed);
    void Wheel_Event(int32 course_y, real32 precise_y);

    bool operator[](uint8 button) const;

    // position
    int32 x = 0;
    int32 y = 0;
    int32 dx = 0;
    int32 dy = 0;

    // scroll-wheel
    int32 wheel_y = 0;
    real32 wheel_y_precise = 0.0;

    bool buttons[Mouse_Buttons::NUM_MOUSE_BUTTONS];
};