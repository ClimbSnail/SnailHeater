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

    SolderUtilConfig utilConfig; // 通用设置
    SolderCoreConfig coreConfig; // 发热芯参数

    int16_t curTemp;     // 当前的温度
    uint16_t powerRatio; // 供电能量占比

    char curCoreName[16];     // 发热名字 格式 id+'_'+type
    const char *coreNameList; // 发热芯的名字列表

    uint16_t typeValue; // 烙铁类型值
    uint16_t volValue;  // 测试得到的电压
};

extern struct SolderModel solderModel;

struct AirhotModel
{
    unsigned char workState; // 工作状态

    HA_UtilConfig utilConfig; // 通用设置
    HA_CoreConfig coreConfig; // 发热芯参数

    int curTemp;         // 当前的温度
    uint16_t powerRatio; // 供电能量占比

    char curCoreName[16];     // 发热名字 格式 id+'_'+type
    const char *coreNameList; // 发热芯的名字列表
};

extern struct AirhotModel airhotModel;

struct HeatplatformModel
{
    unsigned char workState; // 工作状态
    unsigned char enable;    // 使能状态

    HP_UtilConfig utilConfig; // 通用设置

    int curTemp;         // 当前的温度
    uint16_t powerRatio; // 供电能量占比
};

extern struct HeatplatformModel heatplatformModel;

struct AdjPowerModel
{
    unsigned char workState; // 工作状态

    AdjPowerConfig utilConfig; // 通用配置
    int32_t voltage;           // 当前电压
    int32_t current;           // 当前的电流
    int32_t capacity;          // 功率
    bool curToZeroFlag;        // 电流归零
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
    char softwareVersion[16]; // 软件版本
    uint8_t touchFlag;        // 触摸开关
    ENABLE_STATE sysTone;     // 系统提示音（关闭后 旋钮设置将不起作用）
    ENABLE_STATE knobTone;    // 旋钮反馈音
    KNOBS_DIR knobDir;        // 旋钮方向

    uint16_t lockScreenDelayTime; // 锁屏延时时间(s)
    uint16_t wallpaperSwitchTime; // 锁屏切换时间(s)

    UI_PARAM_INFO uiGlobalParam; // 关于UI的全局配置参数，会持久化保存
    bool lockScreenFlag;         // 是否锁屏
    bool hasInfo;                // 是否有系统消息
    int16_t sysInfoDispTime;     // 系统消息显示的时长 ms
    char sysInfo[128];           // 系统消息的具体内容

    // 设置保存的回调函数
    int (*saveConfAPI)(void *param);
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