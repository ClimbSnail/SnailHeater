#ifndef __SOLDER_H
#define __SOLDER_H

#include <Arduino.h>
#include "pid.h"
#include "../base/controller_base.h"
#include "../base/heat_ctrl_base.h"

enum SOLDER_TYPE : unsigned char
{
    SOLDER_TYPE_T12 = 0,
    SOLDER_TYPE_JBC210,
    SOLDER_TYPE_JBC245
};

enum SOLDER_STATE : unsigned char
{
    SOLDER_STATE_DEEP_SLEEP = 0, // 深度休眠
    SOLDER_STATE_SLEEP,          // 轻度休眠
    SOLDER_STATE_NORMAL          // 正常工作状态
};

enum HEATING_MODE : unsigned char
{
    HEATING_MODE_NONE = 0,
    HEATING_MODE_PWM,  // pwm加热
    HEATING_MODE_FULL, // 全速加热
};

// 烙铁配置文件中的参数
struct SolderConfig
{
    unsigned int wakeSensitivity; // 唤醒灵敏度
    int16_t settingTemp;          // 设定的温度
    int16_t easySleepTemp;        // 浅度休眠的温度
    int32_t enterEasySleepTime;   // 进入浅休眠的时间
    int32_t enterDeepSleepTime;   // 进入深度休眠的时间
    int16_t alarmValue;           // 超温报警阈值
    int16_t heaterCnt;            // 烙铁芯数量
    int16_t curID;                // 当前选择的烙铁芯ID
};

// 烙铁芯配置文件中的参数
struct SolderHeaterConfig
{
    unsigned int id;                     // 烙铁芯id
    SOLDER_TYPE solderType;              // 烙铁类型
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
    int16_t calibrationError_350;        // 校准的偏差
    int16_t calibrationError_400;        // 校准的偏差
};

/**********************************
 * T12烙铁控制器 (只允许实例化一个T12控制器)
 */
class Solder : public ControllerBase
{
private:
    PID *m_pidContorller;              // pid控制对象
    SolderConfig m_config;             // 烙铁的设置信息
    SolderHeaterConfig m_heaterConfig; // 烙铁头的配置信息
    SOLDER_TYPE m_type;                // 烙铁的类型
    uint8_t m_powerPin;                // 主要的功率输出引脚（MOS管）
    uint8_t m_powerPinChannel;
    uint8_t m_powerDuty; // 占空比 0-100

    uint8_t m_temperaturePin;              // 热电偶的读取引脚（一定是ADC引脚）
    uint8_t m_shakePin;                    // 振动开关的检查引脚
    uint8_t s_wakeSignalCnt;               // 唤醒信号计数
    static unsigned long s_wakeMillis;     // 上一次唤醒的时间
    int16_t m_targetTemp;                  // 目标温度
    double m_curTemp;                      // 当前温度
    double m_tempBuf[TEMPERATURE_BUF_LEN]; // 温度缓冲区
    double m_tempBufSum;                   // Buffer的总和
    int16_t m_bufIndex;
    bool m_sleepFlag;           // 休眠状态
    SOLDER_STATE m_workState;   // 工作状态
    HEATING_MODE m_heatingMode; // 加热模式

    double volCoefficient;  // 热电偶电压换算的系数
    double tempCoefficient; // 热电偶温度换算的系数

public:
    Solder(const char *name, SnailManager *m_manager,
           SOLDER_TYPE type, uint8_t powerPin, uint8_t powerPinChannel,
           uint8_t temperaturePin, uint8_t shakePin);
    ~Solder();
    bool start();
    bool process();
    bool end();
    // 消息处理
    bool message_handle(const char *from, const char *to,
                        CTRL_MESSAGE_TYPE type, void *message,
                        void *ext_info);

    double getCurTemperature(bool force = false); // 获取实时温度
    bool setTargetTemp(int16_t temperature);      // 设置温度
    int16_t getTargetTemp();
    uint8_t getPowerDuty();
    SOLDER_STATE nextState(void);                // 下一个状态判断
    static void solderSwInterruptCallBack(void); // 友元中断函数
    unsigned long getInterval(void);

    void write_config(SolderConfig *cfg);
    void read_config(SolderConfig *cfg);

private:
    bool setPowerDuty(uint16_t duty);                                // 占空比 0-100
    bool setHeatingMode(HEATING_MODE mode, bool changeMode = false); // 设置加热状态
    bool tmpCloseOutPut(void);                                       // 临时关闭输出
    void sleepCheck(void);                                           // 休眠检测
};

#endif