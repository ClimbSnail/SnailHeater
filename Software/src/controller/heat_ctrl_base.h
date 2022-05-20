#ifndef __HEAT_CONTROLLER_BASE_H
#define __HEAT_CONTROLLER_BASE_H

#include "sys/interface.h"
#include "controller_base.h"

#define TEMPERATURE_BUF_LEN 10

enum SHAKE_STATE : uint8_t
{
    SHAKE_STATE_UNKNOW = 0,
    SHAKE_STATE_WORK,
    SHAKE_STATE_WORK_TO_SLEEP,
    SHAKE_STATE_SLEEP,
    SHAKE_STATE_WAKE
};

class HeatControllerBase
{
protected:
    char m_name[16];         // 控制器名字
    SuperManager *m_manager; //
public:
    HeatControllerBase(const char *name, SuperManager *m_manager);
    // 析构函数
    virtual ~HeatControllerBase(){};
    // 控制器启动初始化 ps:只初始化赋值必要的变量，不进行任何引脚的初始化
    virtual bool start() = 0;
    // 主处理程序
    virtual bool process() = 0;
    // 控制器关闭
    virtual bool end() = 0;
    // 消息处理
    virtual bool message_handle(const char *from, const char *to,
                                SUPER_MESSAGE_TYPE type, void *message,
                                void *ext_info) = 0;
};

#endif