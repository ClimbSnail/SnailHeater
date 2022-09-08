#ifndef __HOT_AIR_H
#define __HOT_AIR_H

#include <Arduino.h>
#include "pid.h"
#include "../base/controller_base.h"
#include "../base/heat_ctrl_base.h"

enum HOTAIR_STATE : unsigned char
{
    HOTAIR_STATE_SLEEP = 0,   // 休眠状态
    HOTAIR_STATE_INTER_SLEEP, // 预休眠状态
    HOTAIR_STATE_NORMAL       // pid控制
};

enum HOT_HEATING_MODE : unsigned char
{
    HOT_HEATING_MODE_NONE = 0,
    HOT_HEATING_MODE_PWM,  // pwm加热
    HOT_HEATING_MODE_FULL, // 全速加热
};

enum AIR_HEATING_MODE : unsigned char
{
    AIR_HEATING_MODE_NONE = 0,
    AIR_HEATING_MODE_PWM,  // pwm空速
    AIR_HEATING_MODE_FULL, // 全速控制
};

// 配置文件中的参数
struct HA_Config
{
    unsigned int id;                   // 风枪的ID
    int16_t settingTemp;               // 设定的温度
    unsigned int workAirSpeed;         // 工作状态下的风速
    unsigned int coolingAirSpeed;      // 冷却时的风速
    int16_t coolingFinishTemp;         // 冷却结束的温度
    unsigned int kp;                   // PID参数
    unsigned int ki;                   // PID参数
    unsigned int kd;                   // PID参数
    unsigned int kt;                   // PID参数
    unsigned int pid_start_low_value;  // PID调控的最低值
    unsigned int pid_start_high_value; // PID调控的最高值
    int16_t calibrationError_100;      // 系统显示100度时，热电偶的电动势
    int16_t calibrationError_135;      // 系统显示135度时，热电偶的电动势
    int16_t calibrationError_170;      // 系统显示170度时，热电偶的电动势
};

/**********************************
 * 热风枪控制器
 */
class HotAir : public ControllerBase
{
private:
    PID *m_pidContorller; // pid控制对象
    HA_Config m_config;   // 配置信息
    uint8_t m_powerPin;   // 主要的功率输出引脚（可控硅）
    uint8_t m_powerPinChannel;
    uint8_t m_powerDuty; // 占空比 0-100
    uint8_t m_airPin;    // 风枪的控制引脚
    uint8_t m_airPinChannel;
    uint8_t m_airDuty;                     // 占空比 0-100
    uint8_t m_temperaturePin;              // 热电偶的读取引脚（一定是ADC引脚）
    uint8_t m_shakePin;                    // 振动开关的检查引脚
    static unsigned long s_wakeMillis_0;   // 上一次唤醒的时间
    static unsigned long s_wakeMillis_1;   // 上一次唤醒的时间
    int16_t m_targetTemp;                  // 目标温度
    double m_curTemp;                      // 当前温度
    double m_tempBuf[TEMPERATURE_BUF_LEN]; // 温度缓冲区
    double m_tempBufSum;                   // Buffer的总和
    int16_t m_bufIndex;
    HOTAIR_STATE m_workState;       // 工作状态
    HOT_HEATING_MODE m_heatingMode; // 加热模式
    AIR_HEATING_MODE m_airMode;     // 风扇控制模式

    uint8_t curHotAirObjInd;       // 当前对象的下标
    static uint8_t s_hotAirObjNum; // 风枪对象的数量

public:
    HotAir(const char *name, SnailManager *m_manager,
           uint8_t powerPin, uint8_t powerPinChannel,
           uint8_t airPin, uint8_t airPinChannel,
           uint8_t temperaturePin, uint8_t shakePin);
    ~HotAir();
    bool start();
    bool process();
    bool end();
    // 消息处理
    bool message_handle(const char *from, const char *to,
                        CTRL_MESSAGE_TYPE type, void *message,
                        void *ext_info);

    double getCurTemperature(bool isReal = false); // 获取当前温度
    bool setTargetTemp(int16_t temperature);       // 设置温度
    int16_t getTargetTemp();
    uint8_t getPowerDuty();
    bool setAirDuty(uint8_t duty);   // 占空比 0-100
    bool enbleAirDuty(uint8_t duty); // 占空比 0-100
    uint8_t getAirDuty();
    uint8_t getWorkState();
    static void hotAirSwInterruptCallBack_0(void); // 静态中断函数
    static void hotAirSwInterruptCallBack_1(void); // 静态中断函数

    void write_config(HA_Config *cfg, int num);
    void read_config(HA_Config *cfg, int num);

private:
    bool setHeatingMode(HOT_HEATING_MODE mode, bool force = false);
    bool setAirgMode(AIR_HEATING_MODE mode, bool force = false); // 设置热风枪模式
    bool setPowerDuty(uint16_t duty);                            // 占空比 0-100
};

#endif