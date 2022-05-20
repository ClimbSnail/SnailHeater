#include "adjustable_power.h"

AdjustablePower::AdjustablePower(const char *name, SuperManager *m_manager,
                                 uint8_t enPin, uint8_t pwmPin, uint8_t pwmChannel) : ControllerBase(name, m_manager)
{
    // 引脚
    m_enPin = enPin;
    m_pwmPin = pwmPin;
    m_pwmChannel = pwmChannel;
    // 电源的值
    m_volt = 0;
    m_volt = (int)(0.5 * 255);
}

AdjustablePower::~AdjustablePower()
{
    this->end();
}

bool AdjustablePower::start()
{
    pinMode(m_enPin, OUTPUT);
    digitalWrite(m_enPin, HIGH);

    pinMode(m_pwmPin, OUTPUT);
    // ledcSetup(m_pwmChannel, 10000, 10);
    // ledcAttachPin(m_pwmPin, m_pwmChannel);
    // ledcWrite(m_pwmChannel, m_volt);
    return true;
}

bool AdjustablePower::process()
{
    return true;
}

bool AdjustablePower::setVolt(uint16_t volt)
{
    Serial.printf(m_name);
    m_volt = volt;
    dacWrite(m_pwmPin, volt); // 输出DAC
    // ledcWrite(m_pwmChannel, m_volt);

    return true;
}

bool AdjustablePower::message_handle(const char *from, const char *to,
                                     SUPER_MESSAGE_TYPE type, void *message,
                                     void *ext_info)
{
    return true;
}

bool AdjustablePower::end()
{
    digitalWrite(m_enPin, LOW);
    dacWrite(m_pwmPin, 0); // 输出DAC
    return true;
}