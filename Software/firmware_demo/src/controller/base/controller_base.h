#ifndef __CONTROLLER_BASE_H
#define __CONTROLLER_BASE_H

#include "sys/interface.h"

class SnailManager;

enum CTRL_TYPE : unsigned char
{
    CTRL_TYPE_HOTAIR = 0,
    CTRL_TYPE_HEATPLATFORM,
    CTRL_TYPE_SOLDER,
    CTRL_TYPE_OSCILLOSCOPE,
    CTRL_TYPE_SIGNALGENERATOR,
    CTRL_TYPE_PWMCONTROLLER,
    CTRL_TYPE_ADJUSTABLEPOWER,
    CTRL_TYPE_NONE
};

class ControllerBase
{
public:
    char m_name[32];         // 控制器名字
    CTRL_TYPE m_type;        // 控制器类型
    SnailManager *m_manager; //
public:
    ControllerBase(const char *name, CTRL_TYPE type, SnailManager *manager)
    {
        memset(m_name, 0, 32);
        snprintf(m_name, 32, "%s", name);
        m_type = type;
        m_manager = manager;
    }
    // 析构函数
    virtual ~ControllerBase(){};
    // 控制器启动初始化 ps:只初始化赋值必要的变量，不进行任何引脚的初始化
    virtual bool start() = 0;
    // 主处理程序
    virtual bool process() = 0;
    // 控制器关闭
    virtual bool end() = 0;
    // 消息处理
    virtual bool message_handle(const char *from, const char *to,
                                CTRL_MESSAGE_TYPE type, void *message,
                                void *ext_info) = 0;
};

#endif