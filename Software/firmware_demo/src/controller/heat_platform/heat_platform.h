#ifndef __HEAT_PLATFORM_H
#define __HEAT_PLATFORM_H

#include <Arduino.h>
#include "pid.h"
#include "../base/controller_base.h"
#include "../base/heat_ctrl_base.h"

// 配置文件中的参数
struct HPConfig
{
    int16_t settingTemp;                 // 设定的温度
    unsigned int kp;                     // PID参数
    unsigned int ki;                     // PID参数
    unsigned int kd;                     // PID参数
    unsigned int kt;                     // PID参数
    unsigned int pid_start_d_value_low;  // PID调控的差值
    unsigned int pid_start_d_value_high; // PID调控的差值
    int16_t calibrationError_50;         // 校准的偏差
    int16_t calibrationError_100;        // 校准的偏差
    int16_t calibrationError_150;        // 校准的偏差
    int16_t calibrationError_200;        // 校准的偏差
    int16_t calibrationError_250;        // 校准的偏差
    int16_t calibrationError_300;        // 校准的偏差
};

/**********************************
 * 加热台控制器
 */
class HeatPlatform : public ControllerBase
{
private:
    PID *m_pidContorller; // pid控制对象
    HPConfig m_HpConfig;  // 烙铁的设置信息
    uint8_t m_powerPin;   // 主要的功率输出引脚（MOS管）
    uint8_t m_powerPinChannel;
    uint8_t m_powerDuty; // 占空比 0-100

    uint8_t m_temperaturePin;              // 热电偶的读取引脚（一定是ADC引脚）
    uint8_t m_shakePin;                    // 振动开关的检查引脚
    int16_t m_targetTemp;                  // 目标温度
    double m_nowTemp;                      // 当前温度
    double m_tempBuf[TEMPERATURE_BUF_LEN]; // 温度缓冲区
    double m_tempBufSum;                   // Buffer的总和
    int16_t m_bufIndex;
    bool m_sleepFlag;                  // 休眠状态
    static unsigned long s_wakeMillis; // 上一次唤醒的时间

public:
    HeatPlatform(const char *name, SnailManager *m_manager,
                 uint8_t powerPin, uint8_t powerPinChannel,
                 uint8_t temperaturePin, uint8_t shakePin = 255);
    ~HeatPlatform();
    bool start();
    bool process();
    bool end();
    // 消息处理
    bool message_handle(const char *from, const char *to,
                        CTRL_MESSAGE_TYPE type, void *message,
                        void *ext_info);

    double getCurTemperature(bool flag = false); // 获取实时温度
    bool setTargetTemp(int16_t temperature);     // 设置温度
    int16_t getTargetTemp();
    uint8_t getPowerDuty();
    uint8_t getWorkState();
    static void swInterruptCallBack(void); // 友元中断函数

    void write_config(HPConfig *cfg);
    void read_config(HPConfig *cfg);

private:
    bool setPowerDuty(uint8_t duty); // 占空比 0-100
};

#endif