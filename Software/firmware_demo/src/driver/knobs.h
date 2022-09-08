#ifndef __KNOBS_H
#define __KNOBS_H

#include <Arduino.h>

struct KeyInfo
{
    int16_t pulse_count;
    uint8_t switch_status;
    // 按钮按下的时间
    unsigned long switch_time;
};

class Knobs
{
public:
    static KeyInfo m_key_info;
    static uint8_t m_pinA_num;
    static uint8_t m_pinB_num;
    static uint8_t m_pinSw_num;
    static uint8_t m_pinA_Status;
    static uint8_t m_pinB_Status;
    static uint8_t flag;
    static unsigned long m_previousMillis;

public:
    Knobs(uint8_t pinA_num, uint8_t pinB_num, uint8_t pinSw_num);
    ~Knobs();
    KeyInfo get_data(void);
    uint16_t getDiff(void); // 获取旋转的数据
    uint8_t getState(void); // 0松开 1按下
    ICACHE_RAM_ATTR void static interruter_funcA(void);
    ICACHE_RAM_ATTR void static interruter_funcSW_ON(void);
};

#endif