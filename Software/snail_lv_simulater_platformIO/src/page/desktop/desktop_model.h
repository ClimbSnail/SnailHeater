#ifndef DESKTOP_MODEL_H
#define DESKTOP_MODEL_H

#include "driver/knobs.h"
#include "ctrl_common.h"

// #include "controller/solder/solder.h"
// #include "controller/hot_air/hot_air.h"
// #include "controller/heat_platform/heat_platform.h"
// #include "controller/power/adjustable_power.h"
// #include "controller/spot_welder/spot_welder.h"

struct SolderModel
{
    unsigned char type;      // 烙铁
    unsigned char wakeType;  // 唤醒类型（震动开关）
    int targetTemp;          // 设定的温度
    int curTemp;             // 当前的温度
    unsigned char workState; // 工作状态
    uint16_t powerRatio;     // 供电能量占比
};

extern struct SolderModel solderModel;

struct AirhotModel
{
    int targetTemp;            // 设定的目标温度
    int curTemp;               // 当前的温度
    unsigned int workAirSpeed; // 工作状态下的风速
    unsigned char workState;   // 工作状态
    uint16_t powerRatio;       // 供电能量占比
};

extern struct AirhotModel airhotModel;

struct HeatplatformModel
{
    int targetTemp;       // 设定的温度
    int curTemp;          // 当前的温度
    unsigned char enable; // 使能状态
    uint16_t powerRatio;  // 供电能量占比
};

extern struct HeatplatformModel heatplatformModel;

struct AdjPowerModel
{
    int volAdcValue;         // 控制电压的ADC值
    int curAdcValue;         // 控制电流的ADC值
    uint8_t mode;            // 模式
    int32_t voltage;         // 当前电压
    int32_t current;         // 当前的电流
    int32_t capacity;        // 功率
    unsigned char workState; // 工作状态
};

extern struct AdjPowerModel adjPowerModel;

struct StopWelderModel
{
    uint8_t mode;            // 点焊模式
    uint16_t pulseWidth;     // 脉宽 ms
    uint16_t interval;       // 触发最小间隔 ms
    uint8_t capNumber;       // 电容数
    uint16_t singleCapVol;   // 单电容额定电压 mv
    uint16_t alarmVol;       // 总电压报警值
    uint16_t voltage;        // 当前电容电压
    unsigned char workState; // 工作状态
};

extern struct StopWelderModel stopWelderModel;

struct SysInfoModel
{
    char sn[32];
    char srceenVersion[16];
    char coreVersion[16];
    char outBoardVersion[16];
    char softwareVersion[16]; // 软件版本
    KNOBS_DIR knobDir;        // 旋钮方向
};

extern struct SysInfoModel sysInfoModel;

// 电烙铁
int setSolderInfo(unsigned char type, unsigned char wakeType,
                  int targetTemp, int curTemp,
                  unsigned char workState, uint16_t powerRatio);
int setSolderCurTempAndPowerDuty(int curTemp, uint16_t powerRatio);
int setSolderWorkState(unsigned char workState);

// 热风枪
int setAirhotInfo(int targetTemp, int curTemp,
                  unsigned int workAirSpeed, unsigned char workState,
                  uint16_t powerRatio);
int setAirhotCurTempAndPowerDuty(int curTemp, uint16_t powerRatio);
int setAirhotWorkState(unsigned char workState);

// 加热台
int setHeatplatformInfo(int targetTemp, int curTemp,
                        unsigned char enable, uint16_t powerRatio);
int setHeatplatformCurTempAndPowerDuty(int curTemp, uint16_t powerRatio);
int setHeatplatformEnable(unsigned char enable);

// 可调电源
int setAdjPowerInfo(int volAdcValue, int curAdcValue, unsigned char mode,
                    int32_t voltage, int32_t current,
                    int32_t capacity, unsigned char workState);
int setAdjPowerVoltage(int32_t voltage);
int setAdjPowerCurrent(int32_t current);
int setAdjPowerCapacity(int32_t capacity);
int setAdjPowerWorkState(unsigned char workState);

// 点焊机
int setStopWelderInfo(uint8_t mode, uint16_t pulseWidth, uint16_t interval,
                      uint8_t capNumber, uint16_t singleCapVol,
                      uint16_t alarmVol, uint16_t voltage,
                      unsigned char workState);
int setStopWelderVoltage(uint16_t voltage);
int setStopWelderWorkState(unsigned char workState);

// 系统给设置
int setSysInfo(const char *sn, const char *srceenVersion,
               const char *coreVersion, const char *outBoardVersion,
               const char *softwareVersion, KNOBS_DIR knobDir);
int setKnobsDir(KNOBS_DIR knobDir);

#endif