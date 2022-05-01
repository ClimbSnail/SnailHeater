#ifndef __HOT_AIR_H
#define __HOT_AIR_H

#include <Arduino.h>
#include "pid.h"

#define TEMPERATURE_BUF_LEN 10

/**********************************
 * 热风枪控制器
 */
class HotAir
{
private:
    char m_name[16];      // 控制器名字
    PID *m_pidContorller; // pid控制对象
    uint8_t m_powerPin;   // 主要的功率输出引脚（可控硅）
    uint8_t m_powerPinChannel;
    uint8_t m_powerDuty; // 占空比 0-100
    uint8_t m_airPin;    // 风枪的控制引脚
    uint8_t m_airPinChannel;
    uint8_t m_airDuty;                     // 占空比 0-100
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
    HotAir(const char *name, uint8_t powerPin, uint8_t powerPinChannel,
           uint8_t airPin, uint8_t airPinChannel,
           uint8_t temperaturePin, uint8_t shakePin);
    ~HotAir();
    bool start();
    bool process();
    double getTemperature(int flag = false); // 获取实时温度
    bool setTargetTemp(int16_t temperature); // 设置温度
    int16_t getTargetTemp();
    uint8_t getPowerDuty();
    bool setAirDuty(uint8_t duty, bool flag = false); // 占空比 0-100
    uint8_t getAirDuty();
    void swInterruptCallBack(); // 友元中断函数
    bool end();

private:
    bool setPowerDuty(uint8_t duty); // 占空比 0-100
    bool disableAir(void);
};

// extern HotAir hotAir;

// static void interruptCallBack();

#endif