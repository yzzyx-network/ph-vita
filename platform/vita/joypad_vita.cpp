#include "joypad_vita.h"
#include "core/variant.h"

static const SceCtrlButtons pad_mapping[] = {
    SCE_CTRL_CROSS, SCE_CTRL_CIRCLE, SCE_CTRL_SQUARE, SCE_CTRL_TRIANGLE,
    SCE_CTRL_L2, SCE_CTRL_R2, SCE_CTRL_L1, SCE_CTRL_R1,
    (SceCtrlButtons)0, (SceCtrlButtons)0, SCE_CTRL_SELECT, SCE_CTRL_START,
    SCE_CTRL_UP, SCE_CTRL_DOWN, SCE_CTRL_LEFT, SCE_CTRL_RIGHT
};

JoypadVita::JoypadVita(InputDefault *in) {
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
    button_count = sizeof(pad_mapping) / sizeof(*pad_mapping);
    input = in;
}

JoypadVita::~JoypadVita() {}

void JoypadVita::process() {
    static SceCtrlData old_pad_input;
    sceCtrlPeekBufferPositive(0, &pad_input, 1);
    uint64_t changed;

    float lx, ly, rx, ry;
    lx = ((pad_input.lx) / 255.0f) * 2.0 - 1.0;
    ly = ((pad_input.ly) / 255.0f) * 2.0 - 1.0;
    rx = ((pad_input.rx) / 255.0f) * 2.0 - 1.0;
    ry = ((pad_input.ry) / 255.0f) * 2.0 - 1.0;

    input->set_joy_axis(0, JOY_AXIS_0, lx);
    input->set_joy_axis(0, JOY_AXIS_1, ly);
    input->set_joy_axis(0, JOY_AXIS_2, rx);
    input->set_joy_axis(0, JOY_AXIS_3, ry);

    changed = old_pad_input.buttons ^ pad_input.buttons;
    old_pad_input = pad_input;
    if (changed) {
        for(int i = 0; i < button_count; i++) {
            if (changed & pad_mapping[i]) {
                input->joy_button(0, i, (bool)(pad_input.buttons & pad_mapping[i]));
            }
        }
    }
}