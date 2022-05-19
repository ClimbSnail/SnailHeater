#ifndef __CONTROLLER_BASE_H
#define __CONTROLLER_BASE_H

#include "sys/interface.h"

class SuperManager;

class ControllerBase
{
protected:
    char m_name[16];      // 控制器名字
    SuperManager *m_manager;   // 
public:
    ControllerBase(SuperManager *manager)
    {
        m_manager = manager;
    }
    // 控制器启动初始化
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