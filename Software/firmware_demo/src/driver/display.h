#ifndef DISPLAY_H
#define DISPLAY_H

#include <lvgl.h>

class Display
{
public:
    void init();
    void routine();
    void setBackLight(float);
};

#endif
