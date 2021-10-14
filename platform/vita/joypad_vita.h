#include "main/input_default.h"
#include <psp2/ctrl.h>
#include <psp2/touch.h>

class JoypadVita {
public:
    JoypadVita(InputDefault *in);
    ~JoypadVita();
    void process();
private:
    InputDefault *input;
    SceCtrlData pad_input;
    int button_count;
};