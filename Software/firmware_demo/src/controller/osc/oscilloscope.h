#ifndef __OSCILLOSCOPE_H
#define __OSCILLOSCOPE_H

#include <Arduino.h>
#include "../base/controller_base.h"

/**********************************
 * 示波器控制器
 */

class Oscilloscope : public ControllerBase
{

private:
    uint8_t m_channelPin_0; // 示波器通道0（ADC引脚）
    uint8_t m_channelPin_1; // 示波器通道1（ADC引脚）
    uint16_t m_channel0_val;
    uint16_t m_channel1_val;

    double channel0Coefficient; // 通道0换算的系数
    double channel1Coefficient; // 通道1换算的系数

public:
    Oscilloscope(const char *name, SnailManager *m_manager,
                 uint8_t channelPin_0, uint8_t channelPin_1);
    ~Oscilloscope();
    uint16_t getChanne0();
    uint16_t getChanne1();
    bool start();
    bool process();
    bool end();
    // 消息处理
    bool message_handle(const char *from, const char *to,
                        CTRL_MESSAGE_TYPE type, void *message,
                        void *ext_info);
};

#endif