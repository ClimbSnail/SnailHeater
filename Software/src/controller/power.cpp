#include "power.h"

Power::Power(const char *name, uint8_t enPin,
             uint8_t pwmPin, uint8_t pwmChannel)
{
    strncpy(m_name, name, 16);
    // 引脚
    m_enPin = enPin;
    m_pwmPin = pwmPin;
    m_pwmChannel = pwmChannel;
    // 电源的值
    m_volt = 0;
    m_volt = (int)(0.5 * 255);
}

Power::~Power()
{
}

bool Power::start()
{
    pinMode(m_enPin, OUTPUT);
    digitalWrite(m_enPin, HIGH);

    ledcSetup(m_pwmChannel, 5, 8);
    ledcAttachPin(m_pwmPin, m_pwmChannel);
    ledcWrite(m_pwmChannel, m_volt);
    return true;
}

bool Power::setVolt(uint8_t volt)
{
    Serial.printf(m_name);
    m_volt = volt;
    ledcWrite(m_pwmChannel, m_volt);
    return true;
}

bool Power::end()
{
    digitalWrite(m_enPin, LOW);
    return true;
}