#ifndef CTRL_COMMON_H
#define CTRL_COMMON_H

#include <stdint.h>
#include "stdio.h"
#include "driver/knobs.h"

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

    VERSION_INFO_SRCEEN_V26,
    VERSION_INFO_CORE_V26,
    VERSION_INFO_OUT_BOARD_V26,

    VERSION_INFO_SRCEEN_V27,
    VERSION_INFO_CORE_V27,
    VERSION_INFO_OUT_BOARD_V27,

    VERSION_INFO_MAX
};

// 控制器工作活跃标志
enum CTRL_WORK_EN : unsigned char
{
    HOTAIR0_CTRL_WORK_EN = 0x1,
    HOTAIR1_CTRL_WORK_EN = 0x2,
    HP_CTRL_WORK_EN = 0x4,

    SOLDER_CTRL_WORK_EN = 0x8,
    ADJPOWER_CTRL_WORK_EN = 0x10,

    CTRL_WORK_EN_MAX = 5
};

const char version_info[VERSION_INFO_MAX][8] = {
    "v2.0", "v2.0", "v2.0",
    "v2.1", "v2.1", "v2.1",
    "v2.5", "v2.5", "v2.5",
    "v2.6", "v2.6", "v2.6",
    "v2.7", "v2.7", "v2.7"};

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

// 系统的重要配置的参数
struct SysCoreConfig
{
    // String sn;
    uint8_t activateState;        // 激活状态 0未激活 1激活
    VERSION_INFO srceenVersion;   // 屏幕版版本号
    VERSION_INFO coreVersion;     // 核心板版本号
    VERSION_INFO outBoardVersion; // 功率板版本号

    uint8_t reserveParam1; // 预留参数1
    uint8_t reserveParam2; // 预留参数2
    uint8_t reserveParam3; // 预留参数3
};

// 系统的通用配置参数
struct SysUtilConfig
{
    uint8_t isFirstStart;   // 是否第一次启动
    uint8_t backLight;      // 屏幕背光亮度
    uint8_t pilotLampLight; // 指示灯亮度
    uint8_t pilotLampSpeed; // 指示灯渐变速度
    uint8_t touchFlag;      // 触摸开关
    ENABLE_STATE sysTone;   // 系统提示音（关闭后 旋钮设置将不起作用）
    ENABLE_STATE knobTone;  // 旋钮反馈音
    KNOBS_DIR knobDir;      // 旋钮方向

    ENABLE_STATE isStartupAnim;          // 是否启动开机动画 0不启动 1启动
    ENABLE_STATE isStartupBell;          // 是否开启开机铃声
    ENABLE_STATE isRunAnim;              // 是否开启运行动画
    ENABLE_STATE enableWallpaper;        // 壁纸总开关
    ENABLE_STATE enableStaticWallpaper;  // 静态壁纸
    ENABLE_STATE enableDynamicWallpaper; // 动态壁纸
    ENABLE_STATE activeCloseSolder;      // 开启可调电源时是否主动关闭烙铁
    uint16_t lockScreenDelayTime;        // 锁屏延迟时间(s)
    uint16_t wallpaperSwitchTime;        // 锁屏切换时间(s)

    uint16_t adcVrefVoltage; // ADC的基准电压（mv）
    uint16_t sysVoltage;     // 系统的供电电压（mv）

    union UI_Param
    {
        // 用于批量操作
        unsigned char allValue; // 用于批量读写操作

        // 用于独立操作 注：重新设置前必须清空（allSet=0）
        UI_PARAM_INFO uiGlobalParam; // 关于UI的全局配置参数，会持久化保存
    } uiParam;
};

#define DISPLAY_TEMP_STEP 100

enum DISPLAY_TEMP : unsigned char
{
    DISPLAY_TEMP_0 = 0,
    // DISPLAY_TEMP_50, // 显示温度为 50 的时候
    DISPLAY_TEMP_100,
    // DISPLAY_TEMP_150,
    DISPLAY_TEMP_200,
    // DISPLAY_TEMP_250,
    DISPLAY_TEMP_300,
    // DISPLAY_TEMP_350,
    DISPLAY_TEMP_400,
    // DISPLAY_TEMP_450,
    DISPLAY_TEMP_500,
    // DISPLAY_TEMP_550,
    DISPLAY_TEMP_600,
    DISPLAY_TEMP_MAX // DISPLAY_TEMP_MAX 不能小于2
};

/****************************************************************/

// 配置文件中的参数
struct HP_UtilConfig
{
    // 三组预设温度
    int quickSetupTemp_0; // 快速设定温度0
    int quickSetupTemp_1; // 快速设定温度1
    int quickSetupTemp_2; // 快速设定温度2

    int16_t targetTemp;           // 设定的目标温度
    unsigned int workAirSpeed;    // 工作状态下的风速
    unsigned int coolingAirSpeed; // 冷却时的风速
    int16_t coolingFinishTemp;    // 冷却结束的温度
    int16_t alarmValue;           // 超温报警阈值
    uint16_t coreCnt;             // 发热芯数量
    uint16_t coreIDRecord;        // 使用位标志发热芯id编号 1~31位
    uint16_t curCoreID;           // 当前选择的发热芯ID
    ENABLE_STATE fastPID;         // 使能便捷PID
};

struct HP_CoreConfig
{
    unsigned int id; // 风枪的ID

    double kp;                        // PID参数
    double ki;                        // PID参数
    double kd;                        // PID参数
    unsigned int kt;                  // PID参数
    unsigned int pid_start_value_low; // PID调控的最低值
    unsigned int pid_end_value_high;  // PID调控的最高值

    int16_t useCalibrationParam;        // 是否使用校准参数
    int16_t calibrationTemp;            // 校准时的环境温度
    int16_t realTemp[DISPLAY_TEMP_MAX]; // 系统显示 DISPLAY_TEMP 度时，实际测量的温度
};

#define MAX_STAGE_NUM 8

struct HP_CurveConfig
{
    unsigned int id;                    // ID
    uint16_t stageNum;                  // 阶段数
    uint16_t temperatur[MAX_STAGE_NUM]; // 温度
    uint16_t slope[MAX_STAGE_NUM];      // 斜率
    uint16_t time[MAX_STAGE_NUM];       // 时间
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

/****************************************************************/

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

// 风枪通用设置参数
struct HA_UtilConfig
{
    // 三组预设温度
    int quickSetupTemp_0; // 快速设定温度0
    int quickSetupTemp_1; // 快速设定温度1
    int quickSetupTemp_2; // 快速设定温度2

    int16_t targetTemp;           // 设定的目标温度
    unsigned int workAirSpeed;    // 工作状态下的风速
    unsigned int coolingAirSpeed; // 冷却时的风速
    int16_t coolingFinishTemp;    // 冷却结束的温度
    int16_t alarmValue;           // 超温报警阈值
    uint16_t coreCnt;             // 发热芯数量
    uint16_t coreIDRecord;        // 使用位标志发热芯id编号 1~31位
    uint16_t curCoreID;           // 当前选择的发热芯ID
    ENABLE_STATE fastPID;         // 使能便捷PID
};

// 风枪手柄的参数
struct HA_CoreConfig
{
    unsigned int id; // 风枪的ID

    double kp;                        // PID参数
    double ki;                        // PID参数
    double kd;                        // PID参数
    unsigned int kt;                  // PID参数
    unsigned int pid_start_value_low; // PID调控的最低值
    unsigned int pid_end_value_high;  // PID调控的最高值

    int16_t useCalibrationParam;        // 是否使用校准参数
    int16_t calibrationTemp;            // 校准时的环境温度
    int16_t realTemp[DISPLAY_TEMP_MAX]; // 系统显示 DISPLAY_TEMP 度时，实际测量的温度
};

/****************************************************************/
#define ADJ_POWER_CUR_MAX 3500  // 可调电源最大电流ma
#define ADJ_POWER_VOL_MAX 24500 // 可调电源最大电压mv

enum ADJ_POWER_MODE
{
    ADJ_POWER_MODE_CV = 0, // 恒压模式
    ADJ_POWER_MODE_CC,     // 恒流模式

    ADJ_POWER_MODE_MAX_NUM
};

// 配置文件中的参数
struct AdjPowerConfig
{
    uint8_t autoOpen; // 开机自动开启输出
    uint8_t mode;     // 模式 （0恒压 1横流）
    // 三组预设电压
    uint32_t quickSetupVoltage_0; // 快速设定电压0
    uint32_t quickSetupVoltage_1; // 快速设定电压1
    uint32_t quickSetupVoltage_2; // 快速设定电压2

    uint16_t volDacValue;    // dac value 电源电压环的dac输出值
    uint16_t curDacValue;    // dac value 电源电流环的dac输出值
    uint32_t settingVoltage; // 设定的电压（mv）/ 占空比
    uint32_t settingCurrent; // 设定的电流（mA）
    uint16_t curZeroValue;   // 静态电流值（mA），用于去皮
    uint32_t stepByStep;     // 调节的不进（mV/mA）

    uint32_t volTheory_1; // 电压理论值1
    uint32_t volReally_1; // 电压实际值1
    uint32_t volTheory_2; // 电压理论值2
    uint32_t volReally_2; // 电压实际值2

    uint32_t curTheory_1; // 电流理论值1
    uint32_t curReally_1; // 电流实际值1
    uint32_t curTheory_2; // 电流理论值2
    uint32_t curReally_2; // 电流实际值2
    uint32_t curTheory_3; // 电流理论值3
    uint32_t curReally_3; // 电流实际值3
};

/****************************************************************/

const char solder_type_str[7][8] = {
    "UNHNOWN", "T12", "C210", "C245", "C470", "C115", "T20"};

#define SOLDER_SHAKE_TYPE_TEXT "None\nHigh\nLow\nChange"

enum SOLDER_SHAKE_TYPE
{
    SOLDER_SHAKE_TYPE_NONE = 0, // 无振动开关
    SOLDER_SHAKE_TYPE_HIGH,     // 高电平休眠
    SOLDER_SHAKE_TYPE_LOW,      // 低电平休眠
    SOLDER_SHAKE_TYPE_CHANGE    // 高低变化休眠
};

enum SOLDER_TYPE : unsigned char
{
    SOLDER_TYPE_UNHNOWN = 0,
    SOLDER_TYPE_T12,
    SOLDER_TYPE_JBC210,
    SOLDER_TYPE_JBC245,
    SOLDER_TYPE_JBC470,
    SOLDER_TYPE_JBC115,
    SOLDER_TYPE_T20,
    SOLDER_TYPE_MAX
};

#define SOLDER_PWM_FREQ_TEXT "64\n128\n256\n512\n1024"

enum SOLDER_PWM_FREQ : unsigned int
{
    SOLDER_PWM_FREQ_64 = 64,
    SOLDER_PWM_FREQ_128 = 128,
    SOLDER_PWM_FREQ_256 = 256,
    SOLDER_PWM_FREQ_512 = 512,
    SOLDER_PWM_FREQ_1024 = 1024
};

// 信息管理动作
enum INFO_MANAGE_ACTION : unsigned char
{
    INFO_MANAGE_ACTION_SOLDER_IDLE = 0,
    INFO_MANAGE_ACTION_MD_CUR_CORE_PID,
    INFO_MANAGE_ACTION_MD_CUR_CORE_PID_OK,

    INFO_MANAGE_ACTION_SOLDER_CREATE,
    INFO_MANAGE_ACTION_SOLDER_CREATE_OK,
    INFO_MANAGE_ACTION_SOLDER_READ,
    INFO_MANAGE_ACTION_SOLDER_READ_OK,
    INFO_MANAGE_ACTION_SOLDER_WRITE,
    INFO_MANAGE_ACTION_SOLDER_WRITE_OK,
    INFO_MANAGE_ACTION_SOLDER_DELETE,
    INFO_MANAGE_ACTION_SOLDER_DELETE_OK,

    INFO_MANAGE_ACTION_HA_IDLE,
    INFO_MANAGE_ACTION_HA_CREATE,
    INFO_MANAGE_ACTION_HA_CREATE_OK,
    INFO_MANAGE_ACTION_HA_READ,
    INFO_MANAGE_ACTION_HA_READ_OK,
    INFO_MANAGE_ACTION_HA_WRITE,
    INFO_MANAGE_ACTION_HA_WRITE_OK,
    INFO_MANAGE_ACTION_HA_DELETE,
    INFO_MANAGE_ACTION_HA_DELETE_OK,

    INFO_MANAGE_ACTION_HP_IDLE,
    INFO_MANAGE_ACTION_HP_CREATE,
    INFO_MANAGE_ACTION_HP_CREATE_OK,
    INFO_MANAGE_ACTION_HP_READ,
    INFO_MANAGE_ACTION_HP_READ_OK,
    INFO_MANAGE_ACTION_HP_WRITE,
    INFO_MANAGE_ACTION_HP_WRITE_OK,
    INFO_MANAGE_ACTION_HP_DELETE,
    INFO_MANAGE_ACTION_HP_DELETE_OK,

    INFO_MANAGE_ACTION_CURVE_IDLE,
    INFO_MANAGE_ACTION_CURVE_CREATE,
    INFO_MANAGE_ACTION_CURVE_CREATE_OK,
    INFO_MANAGE_ACTION_CURVE_READ,
    INFO_MANAGE_ACTION_CURVE_READ_OK,
    INFO_MANAGE_ACTION_CURVE_WRITE,
    INFO_MANAGE_ACTION_CURVE_WRITE_OK,

    INFO_MANAGE_ACTION_CONFIG_RESET_IDLE,
    INFO_MANAGE_ACTION_CONFIG_UTIL_RESET,
    INFO_MANAGE_ACTION_CONFIG_UTIL_RESET_OK,
    INFO_MANAGE_ACTION_CONFIG_CORE_RESET,
    INFO_MANAGE_ACTION_CONFIG_CORE_RESET_OK
};

enum SOLDER_STATE
{
    SOLDER_STATE_DISCONNECT = 0, // 断开连接
    SOLDER_STATE_CONNECT,        // 连接
    SOLDER_STATE_RECO_CORE,      // 烙铁芯（手柄）类型识别
    SOLDER_STATE_DEEP_SLEEP,     // 深度休眠
    SOLDER_STATE_SLEEP,          // 轻度休眠
    SOLDER_STATE_NORMAL          // 正常工作状态
};

enum HEATING_MODE
{
    HEATING_MODE_NONE = 0,
    HEATING_MODE_PWM,  // pwm加热
    HEATING_MODE_FULL, // 全速加热
};

// 烙铁配置文件中的参数
struct SolderUtilConfig
{
    // 三组预设温度
    int quickSetupTemp_0; // 快速设定温度0
    int quickSetupTemp_1; // 快速设定温度1
    int quickSetupTemp_2; // 快速设定温度2

    int16_t targetTemp;          // 设定的目标温度
    int16_t easySleepTemp;       // 浅度休眠的温度
    int32_t enterEasySleepTime;  // 进入浅休眠的时间
    int32_t enterDeepSleepTime;  // 进入深度休眠的时间
    int16_t alarmValue;          // 超温报警阈值
    uint16_t coreCnt;            // 发热芯数量
    uint16_t coreIDRecord;       // 使用位标志发热芯id编号 1~31位
    uint16_t curCoreID;          // 当前选择的发热芯ID
    ENABLE_STATE autoTypeSwitch; // 自动类型切换
    ENABLE_STATE fastPID;        // 使能便捷PID
};

// 发热芯配置文件中的参数
struct SolderCoreConfig
{
    unsigned int id;                     // 发热芯id 用户只读属性
    unsigned int wakeSensitivity;        // 唤醒灵敏度
    SOLDER_PWM_FREQ solderPwmFreq;       // 烙铁驱动频率
    SOLDER_TYPE solderType;              // 烙铁类型
    SOLDER_SHAKE_TYPE wakeSwitchType;    // 唤醒开关类型
    double kp;                           // PID参数
    double ki;                           // PID参数
    double kd;                           // PID参数
    unsigned int kt;                     // PID参数
    unsigned int pid_start_d_value_low;  // PID调控的差值
    unsigned int pid_start_d_value_high; // PID调控的差值

    int16_t useCalibrationParam;        // 是否使用校准参数
    int16_t calibrationTemp;            // 校准时的环境温度
    int16_t realTemp[DISPLAY_TEMP_MAX]; // 系统显示 DISPLAY_TEMP 度时，实际测量的温度
};

/****************************************************************/

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