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
    int fineAdjTemp; // 精调温度

    SolderUtilConfig utilConfig;     // 通用设置
    SolderCoreConfig coreConfig;     // 发热芯参数
    SolderCoreConfig editCoreConfig; // 正在编辑的发热芯参数

    int16_t curTemp;                       // 当前的温度
    uint16_t powerRatio;                   // 供电能量占比
    INFO_MANAGE_ACTION manageCoreAction;   // 发热芯的管理动作
    INFO_MANAGE_ACTION manageConfigAction; // 配置文件的管理动作
    // 软件开关
    // 唤醒模式为 None 时，可控制使能状态，其余唤醒模式保持使能开启
    ENABLE_STATE softwareSwitch;

    char curCoreName[16];     // 发热名字 格式 id+'_'+type
    const char *coreNameList; // 发热芯的名字列表

    uint16_t typeValue; // 烙铁类型值
    uint16_t volValue;  // 测试得到的电压
};

extern struct SolderModel solderModel;

struct AirhotModel
{
    unsigned char workState; // 工作状态

    HA_UtilConfig utilConfig;     // 通用设置
    HA_CoreConfig coreConfig;     // 发热芯参数
    HA_CoreConfig editCoreConfig; // 正在编辑的发热芯参数

    int curTemp;                           // 当前的温度
    uint8_t curAirSpeed;                   // 当前运行的风速
    unsigned long curRunTime;              // 当前运行时间
    uint16_t powerRatio;                   // 供电能量占比
    INFO_MANAGE_ACTION manageCoreAction;   // 发热芯的管理动作
    INFO_MANAGE_ACTION manageConfigAction; // 配置文件的管理动作

    char curCoreName[16];     // 发热名字 格式 id+'_'+type
    const char *coreNameList; // 发热芯的名字列表
};

extern struct AirhotModel airhotModel;

struct HeatplatformModel
{
    unsigned char workState; // 工作状态
    unsigned char enable;    // 使能状态

    HP_UtilConfig utilConfig;     // 通用设置
    HP_CoreConfig coreConfig;     // 发热芯设置
    HP_CoreConfig editCoreConfig; // 正在编辑的发热芯参数
    HP_CurveConfig curveConfig;   // 温度曲线

    int curTemp;                           // 当前的温度
    uint8_t curAirSpeed;                   // 当前运行的风速
    int m_curveTargetTemp;                 // 回流焊的目标温度
    unsigned long curRunTime;              // 当前运行时间(ms)
    uint16_t powerRatio;                   // 供电能量占比
    INFO_MANAGE_ACTION manageCurveAction;  // 曲线管理动作
    INFO_MANAGE_ACTION manageCoreAction;   // 发热芯的管理动作
    INFO_MANAGE_ACTION manageConfigAction; // 配置文件的管理动作
    uint16_t curveAllTime;                 // 回流焊总时间(s)
    uint16_t curveRunTime;                 // 回流焊运行时间(s)
    uint16_t curveStartTimeStamp;          // 回流焊开始时间(s)
    uint16_t curveEndTime;                 // 回流焊结束时间(s)
};

extern struct HeatplatformModel heatplatformModel;

struct AdjPowerModel
{
    unsigned char workState; // 工作状态

    AdjPowerConfig utilConfig;             // 通用配置
    int32_t voltage;                       // 当前电压
    int32_t current;                       // 当前的电流
    int32_t capacity;                      // 功率
    bool curToZeroFlag;                    // 电流归零
    INFO_MANAGE_ACTION manageConfigAction; // 配置文件的管理动作
};

extern struct AdjPowerModel adjPowerModel;

struct StopWelderModel
{
    unsigned char workState;         // 工作状态
    unsigned char forcedOpenMosFlag; // 控制mos是否强制打开

    ENABLE_STATE manualTriggerFlag; // 手动触发标志

    uint16_t voltage; // 当前电容的电压
    uint16_t count;   // 点焊计数

    SpotWelderConfig utilConfig; // 通用配置
};

extern struct StopWelderModel stopWelderModel;

struct SignalGeneratorModel
{
    unsigned char workState; // 工作状态

    uint32_t freqStep;       // 频率调节的不进
    uint32_t realFreqPwm;    // 方波真实频率 hz
    SignalConfig utilConfig; // 通用配置
};

extern struct SignalGeneratorModel signalGeneratorModel;

struct SysInfoModel
{
    char sn[32];
    char softwareVersion[16]; // 软件版本

    SysCoreConfig coreConfig;
    SysUtilConfig utilConfig;
    INFO_MANAGE_ACTION manageConfigAction; // 配置文件的管理动作

    bool lockScreenFlag;     // 是否锁屏
    bool hasInfo;            // 是否有系统消息
    bool resetFlag;          // 重置标志位
    int16_t sysInfoDispTime; // 系统消息显示的时长 ms
    char sysInfo[128];       // 系统消息的具体内容

    // 设置保存的回调函数
    int (*saveConfAPI)(void *param);

    // 设置旋钮智能加速
    bool (*setIntellectRateFlag)(bool flag);
    // std::function<bool(bool)> setIntellectRateFlag;
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

// 信号发生器
int setSignalGeneratorInfo(SignalGeneratorModel *mode);
int setSignalGeneratorWorkState(unsigned char workState);

// 系统给设置
int setSysInfo(SysInfoModel *model);
int setKnobsDir(KNOBS_DIR knobDir);

#endif