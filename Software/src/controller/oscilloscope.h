#ifndef __OSCILLOSCOPE_H
#define __OSCILLOSCOPE_H

#include <Arduino.h>

/**********************************
 * 示波器控制器
 */

class Oscilloscope
{

private:
    char m_name[16];        // 控制器名字
    uint8_t m_channelPin_0; // 示波器通道0（ADC引脚）
    uint8_t m_channelPin_1; // 示波器通道1（ADC引脚）

public:
    Oscilloscope(const char *name, uint8_t channelPin_0, uint8_t channelPin_1);
    ~Oscilloscope();
    bool start();
    bool process();
    bool end();
};

#endif