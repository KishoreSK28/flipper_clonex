// control_mode.h

#ifndef CONTROL_MODE_H
#define CONTROL_MODE_H

enum ControlMode {
    PHYSICAL,
    APP
};

extern ControlMode currentControlMode;

void displayControlModeSelection();
void waitForControlModeSelection();

#endif // CONTROL_MODE_H
