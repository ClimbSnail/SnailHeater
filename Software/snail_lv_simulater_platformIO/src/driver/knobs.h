#ifndef __KNOBS_H
#define __KNOBS_H

// #include <Arduino.h>
#include <stdint.h>

enum KNOBS_DIR
{
    KNOBS_DIR_POS = 0, // 正向
    KNOBS_DIR_NEG      // 反向
};

// struct KeyInfo
// {
//     int16_t pulse_count;
//     // 按钮按下的时间
//     uint16_t switch_time;
//     uint8_t switch_status;
// };

// class Knobs
// {
// public:
//     static KeyInfo m_key_info;
//     KNOBS_DIR m_direction; // 旋钮的方向
//     static uint8_t m_pinA_num;
//     static uint8_t m_pinB_num;
//     static uint8_t m_pinSw_num;
//     static uint8_t m_pinA_Status;
//     static uint8_t m_pinB_Status;
//     static uint8_t flag;
//     static uint32_t m_previousMillis;

// public:
//     Knobs(uint8_t pinA_num, uint8_t pinB_num, uint8_t pinSw_num);
//     ~Knobs();
//     KeyInfo get_data(void);
//     int16_t getDiff(void);            // 获取旋转的数据
//     bool setDirection(KNOBS_DIR dir); // 设置方向
//     bool getState(void);              // false松开 true按下
//     ICACHE_RAM_ATTR void static interruter_funcA(void);
//     ICACHE_RAM_ATTR void static interruter_funcSW_ON(void);
// };

#endif