/*************************************************************************/
/*  joypad_vita.cpp                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/* Copyright (c) 2022 Jaylon Gowie                                       */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "joypad_vita.h"
#include "core/variant.h"
#include "core/os/os.h"

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

void JoypadVita::process_joypads() {
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