#ifndef __HEAT_PLATFORM_H
#define __HEAT_PLATFORM_H

#include <Arduino.h>
#include "pid.h"
#include "controller_base.h"
#include "heat_ctrl_base.h"

/**********************************
 * 加热台控制器
 */
class HeatPlatform : public ControllerBase
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

public:
    HeatPlatform(const char *name, SuperManager *m_manager,
                 uint8_t powerPin, uint8_t powerPinChannel,
                 uint8_t temperaturePin, uint8_t shakePin = -1);
    ~HeatPlatform();
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

private:
    bool setPowerDuty(uint8_t duty); // 占空比 0-100
};

#endif