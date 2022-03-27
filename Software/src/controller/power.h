#ifndef __POWER_H
#define __POWER_H

#include <Arduino.h>

/**********************************
 * 可调电源控制器
 */

class Power
{

private:
    char m_name[16];      // 控制器名字
    uint8_t m_enPin;      // 电源的使能端
    uint8_t m_pwmPin;     // 电源的FB引脚（DAC或者PWM引脚）
    uint8_t m_pwmChannel; // PWM通道
    uint8_t m_volt;       // 电源的电压值

public:
    Power(const char *name, uint8_t enPin,
          uint8_t pwmPin, uint8_t pwmChannel);
    ~Power();
    bool start();
    bool setVolt(uint8_t volt);
    bool end();
};

#endif