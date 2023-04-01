#ifndef CTRL_COMMON_H
#define CTRL_COMMON_H

#include <stdint.h>
#include "stdio.h"

#define DISCONNCT_TEMP 600
#define DAC_DEFAULT_RESOLUTION 4096

// 版本信息
enum VERSION_INFO
{
    VERSION_INFO_SRCEEN_V20 = 0,
    VERSION_INFO_CORE_V20,
    VERSION_INFO_OUT_BOARD_V20,

    VERSION_INFO_SRCEEN_V21,
    VERSION_INFO_CORE_V21,
    VERSION_INFO_OUT_BOARD_V21,

    VERSION_INFO_SRCEEN_V25,
    VERSION_INFO_CORE_V25,
    VERSION_INFO_OUT_BOARD_V25,

    VERSION_INFO_MAX
};

const char version_info[VERSION_INFO_MAX][8] = {
    "v2.0", "v2.0", "v2.0",
    "v2.1", "v2.1", "v2.1",
    "v2.5", "v2.5", "v2.5"};

enum ENABLE_STATE
{
    ENABLE_STATE_CLOSE = 0, // 关闭
    ENABLE_STATE_OPEN,      // 开启
    ENABLE_STATE_NONE
};

struct UI_PARAM_INFO
{
    // ENABLE_STATE_CLOSE or ENABLE_STATE_OPEN
    unsigned char solderGridEnable : 1;   // 是否开启烙铁页面曲线图网格
    unsigned char airhotGridEnable : 1;   // 是否开启风枪页面曲线图网格
    unsigned char heatplatGridEnable : 1; // 是否开启加热台页面曲线图网格
    unsigned char : 3;                    // 预留 实现内存对齐
    unsigned char hpAutoHeatEnable : 1;   // 加热台是否自动加热
    unsigned char whiteThemeEnable : 1;   // 白色主题是否有效
};

enum HEAT_PLATFORM_STATE
{
    HP_STATE_SLEEP = 0,   // 休眠状态
    HP_STATE_INTER_SLEEP, // 预休眠状态
    HP_STATE_NORMAL,      // pid控制
    HP_STATE_CONNECT,     // 连接
    HP_STATE_DISCONNECT   // 断开连接
};

enum HP_HEATING_MODE
{
    HP_HOT_HEATING_MODE_NONE = 0,
    HP_HOT_HEATING_MODE_PWM,  // pwm加热
    HP_HOT_HEATING_MODE_FULL, // 全速加热
};

enum HP_AIR_MODE
{
    HP_AIR_MODE_NONE = 0,
    HP_AIR_MODE_PWM,  // pwm空速
    HP_AIR_MODE_FULL, // 全速控制
};

enum HOTAIR_STATE
{
    HOTAIR_STATE_SLEEP = 0,   // 休眠状态
    HOTAIR_STATE_INTER_SLEEP, // 预休眠状态
    HOTAIR_STATE_NORMAL,      // pid控制
    HOTAIR_STATE_CONNECT,     // 连接
    HOTAIR_STATE_DISCONNECT   // 断开连接
};

enum HA_HEATING_MODE
{
    HOT_HEATING_MODE_NONE = 0,
    HOT_HEATING_MODE_PWM,  // pwm加热
    HOT_HEATING_MODE_FULL, // 全速加热
};

enum HA_AIR_MODE
{
    AIR_MODE_NONE = 0,
    AIR_MODE_PWM,  // pwm空速
    AIR_MODE_FULL, // 全速控制
};

enum ADJ_POWER_MODE
{
    ADJ_POWER_MODE_CV = 0, // 恒压模式
    ADJ_POWER_MODE_CC,     // 恒流模式

    ADJ_POWER_MODE_MAX_NUM
};

enum SOLDER_SHAKE_TYPE
{
    SOLDER_SHAKE_TYPE_NONE = 0, // 无振动开关
    SOLDER_SHAKE_TYPE_HIGH,     // 高电平休眠
    SOLDER_SHAKE_TYPE_LOW,      // 低电平休眠
    SOLDER_SHAKE_TYPE_CHANGE    // 高低变化休眠
};

const char solder_type_str[3][8] = {
    "T12", "C210", "C245"};

enum SOLDER_TYPE
{
    SOLDER_TYPE_T12 = 0,
    SOLDER_TYPE_JBC210,
    SOLDER_TYPE_JBC245
};

enum SOLDER_STATE
{
    SOLDER_STATE_DEEP_SLEEP = 0, // 深度休眠
    SOLDER_STATE_SLEEP,          // 轻度休眠
    SOLDER_STATE_NORMAL          // 正常工作状态
};

enum HEATING_MODE
{
    HEATING_MODE_NONE = 0,
    HEATING_MODE_PWM,  // pwm加热
    HEATING_MODE_FULL, // 全速加热
};

enum SPOTWELDER_MODE
{
    SPOTWELDER_MODE_SINGLE = 0, // 单脉冲
    SPOTWELDER_MODE_DOUBLE,     // 双脉冲
    SPOTWELDER_MODE_MAX,        //
};

enum SPOTWELDER_STATE
{
    SPOTWELDER_STATE_NONE = 0,
    SPOTWELDER_STATE_TRIGGERING, // 触发中
    SPOTWELDER_STATE_WAIT,       // 等待中
};

#endif