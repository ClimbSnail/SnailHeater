#ifndef DESKTOP_MODEL_H
#define DESKTOP_MODEL_H

#include "driver/knobs.h"
#include "ctrl_common.h"


struct SolderModel
{
    unsigned char type;      // 烙铁
    unsigned char wakeType;  // 唤醒类型（震动开关）
    int targetTemp;          // 设定的温度
    int curTemp;             //当前的温度
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
    int adcValue;            // 控制电压的ADC值
    int32_t voltage;         // 当前电压
    int32_t current;         // 当前的电流
    int32_t capacity;        // 功率
    unsigned char workState; // 工作状态
};

extern struct AdjPowerModel adjPowerModel;

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
int setAdjPowerInfo(int adcValue, int32_t voltage,
                    int32_t current, int32_t capacity,
                    unsigned char workState);
int setVoltage(int32_t voltage);
int setCurrent(int32_t current);
int setCapacity(int32_t capacity);
int setWorkState(unsigned char workState);

// 系统给设置
int setSysInfo(const char *sn, const char *srceenVersion,
               const char *coreVersion, const char *outBoardVersion,
               const char *softwareVersion, KNOBS_DIR knobDir);
int setKnobsDir(KNOBS_DIR knobDir);

#endif