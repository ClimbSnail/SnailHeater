#include "knobs.h"

KeyInfo Knobs::m_key_info = {0, 0};
uint8_t Knobs::m_pinA_num = 0;
uint8_t Knobs::m_pinB_num = 0;
uint8_t Knobs::m_pinSw_num = 0;
uint8_t Knobs::m_pinA_Status = 0;
uint8_t Knobs::m_pinB_Status = 0;
unsigned long Knobs::m_previousMillis = 0;
uint8_t Knobs::flag = 0;

/***********************************************************
 * 编码器类的初始化   注：在大多数情况下，
 * 只能使用引脚0、2、4、5、12、13、14、15和16
 * 
 */
Knobs::Knobs(uint8_t pinA_num, uint8_t pinB_num, uint8_t pinSw_num)
{
    m_pinA_num = pinA_num;
    m_pinB_num = pinB_num;
    m_pinSw_num = pinSw_num;
    m_pinA_Status = 0;
    m_pinB_Status = 0;
    m_key_info.pulse_count = 0;
    m_key_info.switch_status = 0;
    flag = 0;

    pinMode(m_pinA_num, INPUT_PULLUP);
    pinMode(m_pinB_num, INPUT_PULLUP);
    pinMode(m_pinSw_num, INPUT_PULLUP);

    // 下降沿触发，触发中断0，调用blink函数
    attachInterrupt(digitalPinToInterrupt(m_pinA_num), interruter_funcA, CHANGE);
    // 按下与松开
    attachInterrupt(digitalPinToInterrupt(m_pinSw_num), interruter_funcSW_ON, CHANGE);
}

void Knobs::interruter_funcA(void)
{
    // 发生外部中断后执行的函数
    if (m_pinA_Status == 0 && digitalRead(m_pinA_num) == 0) //第一次中断，并且A相是下降沿
    {
        flag = 0;
        if (digitalRead(m_pinB_num))
        {
            flag = 1; //根据B相，设定标志
        }
        m_pinA_Status = 1; //中断计数
    }
    if (m_pinA_Status && digitalRead(m_pinA_num)) //第二次中断，并且A相是上升沿
    {
        m_pinB_Status = digitalRead(m_pinB_num);
        if (m_pinB_Status == 0 && flag == 1)
        {
            ++(m_key_info.pulse_count);
        }
        if (m_pinB_Status && flag == 0)
        {
            --(m_key_info.pulse_count);
        }
        m_pinA_Status = 0; //中断计数复位，准备下一次
    }
}

void Knobs::interruter_funcSW_ON(void)
{
    // 发生外部中断后执行的函数
    int status = digitalRead(m_pinSw_num);
    if (0 == status)    // 按下
    {
        // delay(10);   // 不能加延时，会发生终端异常
        status = digitalRead(m_pinSw_num);
        if (0 == status)
        {
            m_previousMillis = millis();
        }
    }
    else{   // 松开
        
        status = digitalRead(m_pinSw_num);
        if (false || 1 == status)
        {
            m_key_info.switch_status = 1;
            m_key_info.switch_time = millis() - m_previousMillis;
        }
    }
}

KeyInfo Knobs::get_data(void)
{
    KeyInfo ret_info = m_key_info;
    m_key_info.pulse_count = 0;   // clear
    m_key_info.switch_status = 0; // clear
    m_key_info.switch_time = 0;
    return ret_info;
}

Knobs::~Knobs()
{
}