#ifndef __BUZZER_H
#define __BUZZER_H

#include <Arduino.h>

/**********************************
 * 蜂鸣器控制器
 */
class Buzzer
{
public:
    static uint16_t m_count;
    static uint16_t m_cycle_time;
    static uint8_t m_pin_num;

public:
    Buzzer(uint8_t buzzer_num, uint16_t cycle_time = 10);
    ~Buzzer();
    void static timer_handler(TimerHandle_t xTimer);
    void set_beep_time(uint16_t time);
};

#endif