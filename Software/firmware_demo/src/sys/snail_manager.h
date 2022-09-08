#ifndef SNAIL_MANAGER_H
#define SNAIL_MANAGER_H

#define MANAGER_NAME "SnailManager"
#define CONTROLLER_NAME_LEN 16   // 控制器的名字长度
#define CONTROLLER_MAX_NUM 10    // 最大的可运行的控制器数量
#define EVENT_LIST_MAX_LENGTH 10 // 消息队列的容量

// 日志打印开关
#define CTRL_LOG_STATUS_OPEN true
#define CTRL_LOG_STATUS_CLOSE false

#include "sys/interface.h"
#include "controller/base/controller_base.h"

#include "controller/heat_platform/heat_platform.h"
#include "controller/hot_air/hot_air.h"
#include "controller/osc/oscilloscope.h"
#include "controller/signal_generator/signal_generator.h"
#include "controller/power/adjustable_power.h"
#include "controller/pwm/pwm_controller.h"
#include "controller/solder/solder.h"

#include <list>

struct CTRL_OBJ
{
    HotAir *hotAir_0;
    HotAir *hotAir_1;
    HeatPlatform *heat_platform;
    Solder *solder;
    Oscilloscope *oscilloscope;
    SignalGenerator *signalGenerator;
    PwmController *pwm_controller;
    AdjustablePower *adj_power;
};

struct EVENT_OBJ
{
    const ControllerBase *from; // 发送请求服务的APP
    CTRL_MESSAGE_TYPE type;     // app的事件类型
    void *info;                 // 请求携带的信息
    uint8_t retryMaxNum;        // 重试次数
    uint8_t retryCount;         // 重试计数
    unsigned long nextRunTime;  // 下次运行的时间戳
};

// 系统配置文件中的参数
struct SysConfig
{
    String sn;
    String srceenVersion;    // 屏幕版版本号
    String coreVersion;      // 核心板版本号
    String outBoardVersion;  // 功率板版本号
    uint8_t backLight;       // 屏幕背光亮度
    uint8_t pilotLampLight;  // 指示灯亮度
    uint8_t pilotLampSpeed;  // 指示灯渐变速度
    uint8_t knobOrder;       // 旋钮方向 0顺 1逆
    uint8_t isStartupAnim;   //是否启动开机动画 0不启动 1启动
    uint8_t isStartupBell;   // 是否开启开机铃声
    uint16_t lockScreenTime; // 锁屏时间(分钟)
    uint16_t adcVrefVoltage; // ADC的参考电压（mv）
    uint16_t sysVoltage;     // 系统的供电电压（mv）
};

class SnailManager
{
public:
    char name[CONTROLLER_NAME_LEN];                    // app控制器的名字
    SysConfig m_sysConfig;                             // 烙铁的设置信息
    ControllerBase *ctrlList[CONTROLLER_MAX_NUM];      // 预留 CONTROLLER_MAX_NUM 个APP注册位
    CTRL_RUN_TYPE ctrlRunTypeList[CONTROLLER_MAX_NUM]; // 对应控制器的运行类型
    boolean ctrlRunLogStatusList[CONTROLLER_MAX_NUM];  // 对应控制器的运行日志打印的开关
    // std::list<const APP_OBJ *> app_list; // APP注册位(为了C语言可移植，放弃使用链表)
    std::list<EVENT_OBJ> eventList;   // 用来储存事件
    boolean m_wifi_status;            // 表示是wifi状态 true开启 false关闭
    unsigned long m_preWifiReqMillis; // 保存上一回请求的时间戳
    int ctrl_num;
    boolean ctrl_exit_flag; // 表示是否退出APP应用
    int cur_ctrl_index;     // 当前运行的APP下标
    int pre_ctrl_index;     // 上一次运行的APP下标

    static CTRL_OBJ ctrl_obj;      // 控制器便捷操作对象
    TimerHandle_t xTimerEventDeal; // 事件处理定时器

public:
    SnailManager(boolean flag = true);
    ~SnailManager();
    bool initGPIO();
    bool initController();
    bool loadWindows();
    bool restart();
    static void background_process(TimerHandle_t xTimer);
    int ui_process(KeyInfo *ret_info);
    // 将控制器注册到 SnailManager 中
    int controller_install(ControllerBase *controller,
                           CTRL_RUN_TYPE run_type = CTRL_RUN_TYPE_REAL_TIME,
                           boolean ctrlRunLogStatus = CTRL_LOG_STATUS_OPEN);
    // 将 ctrl 从 SnailManager 中卸载（删除）
    int controller_uninstall(const ControllerBase *controller);
    void controller_exit(void); // 提供给 controller 退出的系统调用
    // 消息发送
    int send_to(const char *from, const char *to,
                CTRL_MESSAGE_TYPE type, void *message,
                void *ext_info);
    // 添加事件
    int addEvent(CTRL_EVENT event);
    // 包装日志打印类
    static size_t run_log(const ControllerBase *ctrller, const char *in_format, ...);

    void write_config(SysConfig *cfg);
    void read_config(SysConfig *cfg);

private:
    TimerHandle_t xTimer_background_process; // 后台进程定时器
    ControllerBase *getControllerByName(const char *name);
    int getControllerIdxByName(const char *name);
    int controller_is_legal(const ControllerBase *ctrl_obj);
};

#endif