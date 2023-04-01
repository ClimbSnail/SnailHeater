#ifndef DESKTOP_MODEL_H
#define DESKTOP_MODEL_H

#include "driver/knobs.h"

#ifdef SIMULATOR
#include "ctrl_common.h"
#else
#include "sys/snail_manager.h"
#include "controller/solder/solder.h"
#include "controller/hot_air/hot_air.h"
#include "controller/heat_platform/heat_platform.h"
#include "controller/power/adjustable_power.h"
#include "controller/spot_welder/spot_welder.h"
#endif

struct SolderModel
{
    unsigned char workState; // 工作状态

    // 使能标志位 + 三组预设温度 + 一组精调温度
    union TempEnable
    {
        // 用于批量操作
        unsigned char allValue; // 用于批量操作， 比如所有位清空 all=0 重新设置前必须清空

        // 用于独立操作 注：重新设置前必须清空（allValue=0）
        struct BitValue
        {
            unsigned char quickSetupTempEnable_0 : 1; // 温度有效标志位 快速设定 0 默认 ENABLE_STATE_CLOSE
            unsigned char quickSetupTempEnable_1 : 1; // 温度有效标志位 快速设定 1
            unsigned char quickSetupTempEnable_2 : 1; // 温度有效标志位 快速设定 2
            unsigned char fineAdjTempEnable : 1;      // 温度有效标志位 精调
            unsigned char : 4;                        // 预留 实现内存对齐
        } bitValue;
    } tempEnable;

    int quickSetupTemp_0; // 快速设定温度0
    int quickSetupTemp_1; // 快速设定温度1
    int quickSetupTemp_2; // 快速设定温度2
    int fineAdjTemp;      // 精调温度

    int targetTemp;      // 设定的温度
    int curTemp;         // 当前的温度
    uint16_t powerRatio; // 供电能量占比

    int16_t easySleepTemp;      // 浅度休眠的温度
    int32_t enterEasySleepTime; // 进入浅休眠的时间
    int32_t enterDeepSleepTime; // 进入深度休眠的时间
    int16_t alarmValue;         // 超温报警阈值
    uint16_t coreCnt;           // 烙铁芯数量
    uint16_t coreIDRecord;      // 使用位标志烙铁芯id编号 1~31位
    uint16_t curCoreID;         // 当前选择的烙铁芯ID

    char curCoreName[16];     // 烙铁名字 格式 id+'_'+type
    const char *coreNameList; // 烙铁芯的名字列表
    unsigned char type;       // 烙铁类型
    uint8_t wakeType;         // 唤醒类型（震动开关）
};

extern struct SolderModel solderModel;

struct AirhotModel
{
    unsigned char workState; // 工作状态

    // 使能标志位 + 三组预设温度 + 一组精调温度
    union TempEnable
    {
        // 用于批量操作
        unsigned char allValue; // 用于批量操作， 比如所有位清空 all=0 重新设置前必须清空

        // 用于独立操作 注：重新设置前必须清空（allValue=0）
        struct BitValue
        {
            unsigned char quickSetupTempEnable_0 : 1; // 温度有效标志位 快速设定 0 默认 ENABLE_STATE_CLOSE
            unsigned char quickSetupTempEnable_1 : 1; // 温度有效标志位 快速设定 1
            unsigned char quickSetupTempEnable_2 : 1; // 温度有效标志位 快速设定 2
            unsigned char fineAdjTempEnable : 1;      // 温度有效标志位 精调
            unsigned char : 4;                        // 预留 实现内存对齐
        } bitValue;
    } tempEnable;
    int quickSetupTemp_0; // 快速设定温度0
    int quickSetupTemp_1; // 快速设定温度1
    int quickSetupTemp_2; // 快速设定温度2
    int fineAdjTemp;      // 精调温度

    int targetTemp;            // 设定的目标温度
    int curTemp;               // 当前的温度
    unsigned int workAirSpeed; // 工作状态下的风速
    uint16_t powerRatio;       // 供电能量占比

    unsigned int coolingAirSpeed;     // 冷却时的风速
    int16_t coolingFinishTemp;        // 冷却结束的温度
    int16_t alarmValue;               // 超温报警阈值
    unsigned int kp;                  // PID参数
    unsigned int ki;                  // PID参数
    unsigned int kd;                  // PID参数
    unsigned int kt;                  // PID参数
    unsigned int pid_start_value_low; // PID调控的最低值
    unsigned int pid_end_value_high;  // PID调控的最高值
};

extern struct AirhotModel airhotModel;

struct HeatplatformModel
{
    unsigned char workState; // 工作状态
    unsigned char enable;    // 使能状态

    // 使能标志位 + 三组预设温度 + 一组精调温度
    union TempEnable
    {
        // 用于批量操作
        unsigned char allValue; // 用于批量操作， 比如所有位清空 all=0 重新设置前必须清空

        // 用于独立操作 注：重新设置前必须清空（allValue=0）
        struct BitValue
        {
            unsigned char quickSetupTempEnable_0 : 1; // 温度有效标志位 快速设定 0 默认 ENABLE_STATE_CLOSE
            unsigned char quickSetupTempEnable_1 : 1; // 温度有效标志位 快速设定 1
            unsigned char quickSetupTempEnable_2 : 1; // 温度有效标志位 快速设定 2
            unsigned char fineAdjTempEnable : 1;      // 温度有效标志位 精调
            unsigned char : 4;                        // 预留 实现内存对齐
        } bitValue;
    } tempEnable;
    int quickSetupTemp_0; // 快速设定温度0
    int quickSetupTemp_1; // 快速设定温度1
    int quickSetupTemp_2; // 快速设定温度2
    int fineAdjTemp;      // 精调温度

    int targetTemp;            // 设定的温度
    int curTemp;               // 当前的温度
    unsigned int workAirSpeed; // 工作状态下的风速
    uint16_t powerRatio;       // 供电能量占比

    unsigned int coolingAirSpeed;     // 冷却时的风速
    int16_t coolingFinishTemp;        // 冷却结束的温度
    int16_t alarmValue;               // 超温报警阈值
    unsigned int kp;                  // PID参数
    unsigned int ki;                  // PID参数
    unsigned int kd;                  // PID参数
    unsigned int kt;                  // PID参数
    unsigned int pid_start_value_low; // PID调控的差值(开始值)
    unsigned int pid_end_value_high;  // PID调控的差值（结束值）
};

extern struct HeatplatformModel heatplatformModel;

struct AdjPowerModel
{
    unsigned char workState; // 工作状态

    int volDacValue;  // 控制电压的DAC值
    int curDacValue;  // 控制电流的DAC值
    uint8_t mode;     // 模式
    int32_t voltage;  // 当前电压
    int32_t current;  // 当前的电流
    int32_t capacity; // 功率

    uint8_t autoOpen;    // 开机自动开启输出
    uint32_t stepByStep; // 调节的不进（mV/mA）
};

extern struct AdjPowerModel adjPowerModel;

struct StopWelderModel
{
    unsigned char workState; // 工作状态
    uint8_t mode;            // 点焊模式
    uint16_t pulseWidth;     // 脉宽 ms
    uint16_t interval;       // 触发最小间隔 ms
    uint8_t capNumber;       // 电容数
    uint16_t singleCapVol;   // 单电容额定电压 mv
    uint16_t alarmVol;       // 总电压报警值
    uint16_t voltage;        // 当前电容电压
};

extern struct StopWelderModel stopWelderModel;

struct SysInfoModel
{
    char sn[32];
    VERSION_INFO srceenVersion;
    VERSION_INFO coreVersion;
    VERSION_INFO outBoardVersion;
    char softwareVersion[16];    // 软件版本
    KNOBS_DIR knobDir;           // 旋钮方向
    UI_PARAM_INFO uiGlobalParam; // 关于UI的全局配置参数，会持久化保存
};

extern struct SysInfoModel sysInfoModel;

// 电烙铁
int setSolderInfo(SolderModel *model);
int setSolderCurTempAndPowerDuty(int curTemp, uint16_t powerRatio);
int setSolderWorkState(unsigned char workState);

// 热风枪
int setAirhotInfo(AirhotModel *model);
int setAirhotCurTempAndPowerDuty(int curTemp, uint16_t powerRatio);
int setAirhotWorkState(unsigned char workState);

// 加热台
int setHeatplatformInfo(HeatplatformModel *model);
int setHeatplatformCurTempAndPowerDuty(int curTemp, uint16_t powerRatio);
int setHeatplatformEnable(unsigned char enable);

// 可调电源
int setAdjPowerInfo(AdjPowerModel *model);
int setAdjPowerVoltage(int32_t voltage);
int setAdjPowerCurrent(int32_t current);
int setAdjPowerCapacity(int32_t capacity);
int setAdjPowerWorkState(unsigned char workState);

// 点焊机
int setStopWelderInfo(StopWelderModel *mode);
int setStopWelderVoltage(uint16_t voltage);
int setStopWelderWorkState(unsigned char workState);

// 系统给设置
int setSysInfo(SysInfoModel *model);
int setKnobsDir(KNOBS_DIR knobDir);

#endif