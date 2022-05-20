#ifndef __PWM_CONTROLLER_H
#define __PWM_CONTROLLER_H

#include <Arduino.h>
#include "controller_base.h"

/**********************************
 * 高精度PWM控制器
 */

class PwmController : public ControllerBase
{

private:
    uint8_t m_pwmPin;     // 电源的FB引脚（PWM引脚）
    uint8_t m_pwmChannel; // PWM通道
    uint8_t m_bits;       // 精度（多少位）
    double m_duty;        // 占空比
    double m_freq;        // 频率

public:
    PwmController(const char *name, SuperManager *m_manager,
                  uint8_t pwmPin, uint8_t pwmChannel,
                  double freq = 1000, double duty = 50, uint8_t bits = 10);
    ~PwmController();
    bool start();
    // 主处理程序
    bool process();
    bool setDuty(double duty);
    bool end();
    // 消息处理
    bool message_handle(const char *from, const char *to,
                        SUPER_MESSAGE_TYPE type, void *message,
                        void *ext_info);
};

#endif