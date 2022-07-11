#ifndef __SIGNAL_GENERATOR_H
#define __SIGNAL_GENERATOR_H

#include <Arduino.h>
#include "controller_base.h"

/**********************************
 * 函数发生器
 */
class SignalGenerator : public ControllerBase
{

private:
    uint8_t m_channelPin; // 模拟输出引脚（DAC引脚）

public:
    SignalGenerator(const char *name, SuperManager *m_manager,
                    uint8_t channelPin);
    ~SignalGenerator();
    bool start();
    bool process();
    bool end();
    // 消息处理
    bool message_handle(const char *from, const char *to,
                        SUPER_MESSAGE_TYPE type, void *message,
                        void *ext_info);
};

#endif