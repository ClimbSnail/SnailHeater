#include "pwm_controller.h"

PwmController::PwmController(const char *name, SuperManager *m_manager,
                             uint8_t pwmPin, uint8_t pwmChannel,
                             double freq, double duty, uint8_t bits) : ControllerBase(name, m_manager)
{
    // 引脚
    m_pwmPin = pwmPin;
    m_pwmChannel = pwmChannel;
    // 精度（多少位）
    m_bits = bits;
    m_duty = duty;
    m_freq = freq;
}

PwmController::~PwmController()
{
    this->end();
}

bool PwmController::start()
{
    pinMode(m_pwmPin, OUTPUT);
    ledcSetup(m_pwmChannel, m_freq, m_bits);
    ledcAttachPin(m_pwmPin, m_pwmChannel);

    // uint32_t duty_value = m_duty * (1 << m_bits);
    // ledcWrite(m_pwmChannel, duty_value);
    return true;
}

bool PwmController::process()
{
    return true;
}

bool PwmController::setDuty(double duty)
{
    // duty 0.0~100.0
    Serial.printf(m_name);
    m_duty = duty;

    uint32_t duty_value = m_duty / 100 * (1 << m_bits);
    ledcWrite(m_pwmChannel, duty_value);

    return true;
}

bool PwmController::end()
{
    ledcWrite(m_pwmChannel, 0);
    return true;
}

bool PwmController::message_handle(const char *from, const char *to,
                                   SUPER_MESSAGE_TYPE type, void *message,
                                   void *ext_info)
{
    return true;
}