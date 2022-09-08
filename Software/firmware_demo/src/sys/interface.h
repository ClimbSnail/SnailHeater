#ifndef INTERFACE_H
#define INTERFACE_H

enum CTRL_MESSAGE_TYPE : unsigned char
{
    CTRL_MESSAGE_WIFI_CONN = 0, // 开启连接
    CTRL_MESSAGE_WIFI_AP,       // 开启AP事件
    CTRL_MESSAGE_WIFI_ALIVE,    // wifi开关的心跳维持
    CTRL_MESSAGE_WIFI_DISCONN,  // 连接断开
    CTRL_MESSAGE_UPDATE_TIME,
    CTRL_MESSAGE_GET_PARAM, // 获取参数
    CTRL_MESSAGE_SET_PARAM, // 设置参数
    CTRL_MESSAGE_READ_CFG,  // 向磁盘读取参数
    CTRL_MESSAGE_WRITE_CFG, // 向磁盘写入参数
    CTRL_MESSAGE_MQTT_DATA, // MQTT客户端收到消息

    CTRL_MESSAGE_NONE
};

enum CTRL_RUN_TYPE : unsigned char
{
    CTRL_RUN_TYPE_REAL_TIME = 0, // 实时
    CTRL_RUN_TYPE_BACKGROUND,    // 后台

    CTRL_RUN_TYPE_NONE
};

enum CTRL_EVENT : unsigned char
{
    CTRL_EVENT_AIRHOT_SLEEP = 0, // 风枪休眠
    CTRL_EVENT_AIRHOT_WAKE,      // 风枪唤醒

    CTRL_EVENT_SOLDER_DEEP_SLEEP, // 烙铁深度休眠
    CTRL_EVENT_SOLDER_SLEEP,      // 烙铁休眠
    CTRL_EVENT_SOLDER_WAKE,       // 烙铁唤醒
    
    CTRL_EVENT_NONE
};

#endif