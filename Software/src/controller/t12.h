#ifndef __T12_H
#define __T12_H

#include <Arduino.h>
#include "pid.h"
#include "controller_base.h"
#include "heat_ctrl_base.h"

/**********************************
 * T12烙铁控制器 (只允许实例化一个T12控制器)
 */
class T12 : public ControllerBase
{
private:
    PID *m_pidContorller; // pid控制对象
    uint8_t m_powerPin;   // 主要的功率输出引脚（MOS管）
    uint8_t m_powerPinChannel;
    uint8_t m_powerDuty; // 占空比 0-100

    uint8_t m_temperaturePin;              // 热电偶的读取引脚（一定是ADC引脚）
    uint8_t m_shakePin;                    // 振动开关的检查引脚
    int16_t m_targetTemp;                  // 目标温度
    double m_nowTemp;                      // 当前温度
    double m_tempBuf[TEMPERATURE_BUF_LEN]; // 温度缓冲区
    double m_tempBufCnt;                   // Buffer的总和
    int16_t m_bufIndex;
    bool m_sleepFlag; // 休眠状态

    volatile int m_swInterruptCounter;
    portMUX_TYPE m_swMux;

public:
    T12(const char *name, SuperManager *m_manager,
        uint8_t powerPin, uint8_t powerPinChannel,
        uint8_t temperaturePin, uint8_t shakePin);
    ~T12();
    bool start();
    bool process();
    bool end();
    // 消息处理
    bool message_handle(const char *from, const char *to,
                        SUPER_MESSAGE_TYPE type, void *message,
                        void *ext_info);

    double getCurTemperature(bool flag = false); // 获取实时温度
    bool setTargetTemp(int16_t temperature);     // 设置温度
    int16_t getTargetTemp();
    uint8_t getPowerDuty();
    uint8_t getWorkState();
    void t12SwInterruptCallBack(); // 友元中断函数

private:
    bool setPowerDuty(uint8_t duty); // 占空比 0-100
};

static void interruptCallBack();

#endif