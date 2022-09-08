#ifndef __ADJUSTABLE_POWER_H
#define __ADJUSTABLE_POWER_H

#include <Arduino.h>
#include "common.h"
#include "../base/controller_base.h"

// 配置文件中的参数
struct AdjPowerConfig
{
    uint8_t mode;            // 模式 （0恒压 1横流）
    uint32_t settingVoltage; // 设定的电压（mv）
    uint32_t settingCurrent; // 设定的电流（mA）
    uint32_t stepByStep;     // 调节的不进（mV/A）
};

/**********************************
 * 可调电源控制器
 */
class AdjustablePower : public ControllerBase
{

private:
    AdjPowerConfig m_config; // 配置信息
    uint8_t m_enPin;         // 电源的使能端
    uint8_t m_dacPin;        // 电源的FB引脚（DAC或者PWM引脚）
    uint8_t m_pwmPinChannel; // pwm通道
    uint8_t m_curADCPin;     // 检测电流的ADC引脚（可以为空）
    uint8_t m_oVADCPin;      // 检测电源实际输出电压的ADC引脚（可以为空）
    uint16_t m_volt;         // 电源的电压值

    double volCoefficient;     // 电压换算的系数
    double currentCoefficient; // 电流换算的系数

public:
    AdjustablePower(const char *name, SnailManager *m_manager,
                    uint8_t enPin, uint8_t dacPin, uint8_t pwmPinChannel,
                    uint8_t curADCPin = EMPTY_PIN, uint8_t oVADCPin = EMPTY_PIN);
    ~AdjustablePower();
    bool start();
    // 主处理程序
    bool process();
    bool setVolt(uint16_t volt);
    uint16_t getVolt();
    bool end();
    // 消息处理
    bool message_handle(const char *from, const char *to,
                        CTRL_MESSAGE_TYPE type, void *message,
                        void *ext_info);

    void write_config(AdjPowerConfig *cfg);
    void read_config(AdjPowerConfig *cfg);
};

#endif