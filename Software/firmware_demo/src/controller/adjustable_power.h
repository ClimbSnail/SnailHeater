#ifndef __ADJUSTABLE_POWER_H
#define __ADJUSTABLE_POWER_H

#include <Arduino.h>
#include "controller_base.h"

/**********************************
 * 可调电源控制器
 */
class AdjustablePower : public ControllerBase
{

private:
    uint8_t m_enPin;      // 电源的使能端
    uint8_t m_pwmPin;     // 电源的FB引脚（DAC或者PWM引脚）
    uint8_t m_pwmChannel; // PWM通道
    uint16_t m_volt;      // 电源的电压值

public:
    AdjustablePower(const char *name, SuperManager *m_manager,
                    uint8_t enPin, uint8_t pwmPin, uint8_t pwmChannel);
    ~AdjustablePower();
    bool start();
    // 主处理程序
    bool process();
    bool setVolt(uint16_t volt);
    bool end();
    // 消息处理
    bool message_handle(const char *from, const char *to,
                        SUPER_MESSAGE_TYPE type, void *message,
                        void *ext_info);
};

#endif