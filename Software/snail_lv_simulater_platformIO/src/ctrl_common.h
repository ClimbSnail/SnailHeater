#ifndef CTRL_COMMON_H
#define CTRL_COMMON_H

#include <stdint.h>
#include "stdio.h"

#define DISCONNCT_TEMP 600
#define DAC_DEFAULT_RESOLUTION 4096

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

enum ADJ_POWER_OPEN_STATE
{
    ADJ_POWER_OPEN_STATE_CLOSE = 0, // 自动状态关闭
    ADJ_POWER_OPEN_STATE_OPEN,      // 自动状态开启
};

enum ADJ_POWER_MODE
{
    ADJ_POWER_MODE_NONE = 0,
    ADJ_POWER_MODE_VOL, // 恒压模式
    ADJ_POWER_MODE_CUR, // 恒流模式
};


enum SOLDER_SHAKE_TYPE
{
    SOLDER_SHAKE_TYPE_NONE = 0, // 无振动开关
    SOLDER_SHAKE_TYPE_HIGH,     // 高电平休眠
    SOLDER_SHAKE_TYPE_LOW,      // 低电平休眠
    SOLDER_SHAKE_TYPE_CHANGE    // 高低变化休眠
};

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


#endif