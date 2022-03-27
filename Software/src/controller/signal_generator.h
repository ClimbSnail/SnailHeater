#ifndef __SIGNAL_GENERATOR_H
#define __SIGNAL_GENERATOR_H

#include <Arduino.h>

/**********************************
 * 函数发生器
 */

class SignalGenerator
{

private:
    char m_name[16];      // 控制器名字
    uint8_t m_channelPin; // 模拟输出引脚（DAC引脚）

public:
    SignalGenerator(const char *name, uint8_t channelPin);
    ~SignalGenerator();
    bool start();
    bool process();
    bool end();
};

#endif