#ifndef DISPLAY_H
#define DISPLAY_H

#include <lvgl.h>

#define LCD_BL_PWM_CHANNEL 0

class Display
{
public:
    void init();
    void routine();
    void setBackLight(float);
};

#endif
