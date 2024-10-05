#include "Mouse.h"

#include "logging.h"

Mouse::Mouse() {
    for (int n = 0; n < Mouse_Buttons::NUM_MOUSE_BUTTONS; n++) {
        this->buttons[n-1] = false;
    }
}

void Mouse::Update(real32 dt) {
    this->wheel_y = 0;
    this->wheel_y_precise = 0.0;
}

void Mouse::Update_Motion(int32 x_, int32 y_, int32 dx_, int32 dy_) {
    this->x = x_;
    this->y = y_;
    this->dx = x_;
    this->dy = dy_;
}

void Mouse::Button_Event(uint8 button, bool pressed) {
    log_info("Button: %d [%s]", button, pressed ? "pressed" : "released");
    this->buttons[button-1] = pressed;
}

void Mouse::Wheel_Event(int32 course_y, real32 precise_y) {
    this->wheel_y = course_y;
    this->wheel_y_precise = precise_y;
}

bool Mouse::operator[](uint8 button) const {
    return this->buttons[button-1];
}