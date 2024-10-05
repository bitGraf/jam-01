#pragma once

#include <laml/Data_types.hpp>

enum Input_Actions {
    Action_None     = 0,
    Action_Up       = 1,
    Action_Down     = 2,
    Action_Left     = 3,
    Action_Right    = 4,
    Action_Pause    = 5,
    Action_Back     = 6,
    Action_A        = 7,
    Action_B        = 8,
    Action_X        = 9,
    Action_Y        = 10,
    Action_LB       = 11,
    Action_RB       = 12
};

struct Action_Event {
    Input_Actions action = Action_None;
    bool pressed = false;
};

Action_Event Action_Event_From_Hat_Event(uint8 hat_code);
Action_Event Action_Event_From_Button_Event(uint8 button, bool pressed);
Action_Event Action_Event_From_Key_Event(int32 key_code, bool pressed);
const char* Action_Event_String(Input_Actions action);

enum Input_Axes {
    Axis_Left_Vert = 0,
    Axis_Left_Horiz,
    Axis_Right_Vert,
    Axis_Right_Horiz,

    Axis_Left_Trigger,
    Axis_Right_Trigger,

    AXIS_NUM
};

struct Axis_Mapping {
    void Update_Keys(const uint8* keystate, int num_keys);
    real32 axes[AXIS_NUM];
};