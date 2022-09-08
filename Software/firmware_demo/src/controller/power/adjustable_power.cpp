#include "adjustable_power.h"
#include "common.h"

#include "sys/snail_manager.h"

#define POWER_ON HIGH
#define POWER_OFF LOW

// #define PWM_DAC_FREQ 2441（推荐）
#define PWM_DAC_FREQ PWM_GROUP_3_FREQ
#define PWM_DAC_BIT_WIDTH PWM_GROUP_3_BIT_WIDTH
#define PWM_DAC_FULL_DUTY 1

// 电流检测计算
#define VALUE 2.083
#define POWER_ADC_RESOLUTION 4096.0 // 分辨率
#define POWER_AMPLIFY_MULTIPLE 11   // 放大倍数

AdjustablePower::AdjustablePower(const char *name, SnailManager *m_manager,
                                 uint8_t enPin, uint8_t dacPin, uint8_t pwmPinChannel,
                                 uint8_t curADCPin, uint8_t oVADCPin) : ControllerBase(name, CTRL_TYPE_ADJUSTABLEPOWER, m_manager)
{
    // 引脚
    m_enPin = enPin;
    m_dacPin = dacPin;
    m_pwmPinChannel = pwmPinChannel;
    m_curADCPin = curADCPin;
    m_oVADCPin = oVADCPin;
    // 电源的值
    m_volt = 0;
    m_volt = (int)(0.5 * 3099);

    volCoefficient = ADC_11DB_VREF / POWER_ADC_RESOLUTION * POWER_AMPLIFY_MULTIPLE;
    currentCoefficient = ADC_11DB_VREF / POWER_ADC_RESOLUTION * VALUE;
}

AdjustablePower::~AdjustablePower()
{
    this->end();
}

bool AdjustablePower::start()
{
    pinMode(m_enPin, OUTPUT);
    digitalWrite(m_enPin, POWER_ON);

    pinMode(m_dacPin, OUTPUT);
    ledcSetup(m_pwmPinChannel, PWM_DAC_FREQ, PWM_DAC_BIT_WIDTH);
    ledcAttachPin(m_dacPin, m_pwmPinChannel);

    if (EMPTY_PIN != m_curADCPin)
    {
        pinMode(m_curADCPin, INPUT_PULLUP);
        analogSetPinAttenuation(m_curADCPin, ADC_11db);
        registerMyADC(m_curADCPin); // 注册ADC检测服务
    }
    if (EMPTY_PIN != m_oVADCPin)
    {
        pinMode(m_oVADCPin, INPUT_PULLUP);
        analogSetPinAttenuation(m_oVADCPin, ADC_11db);
        registerMyADC(m_oVADCPin); // 注册ADC检测服务
    }
    return true;
}

bool AdjustablePower::process()
{
    uint16_t cur_votl = 0;
    uint16_t oV_votl = 0;
    if (EMPTY_PIN != m_curADCPin)
    {
        cur_votl = getMyADC(m_curADCPin, 10, false);
        // cur_votl = getMyADC(m_curADCPin);
        // vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    if (EMPTY_PIN != m_oVADCPin)
    {
        oV_votl = getMyADC(m_oVADCPin, 10, false);
        // oV_votl = getMyADC(m_oVADCPin);
        // vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    m_manager->run_log(this, "\n");

    // ADC_11DB_VREF / POWER_ADC_RESOLUTION * cur_votl * VALUE
    double cur_votl_v = currentCoefficient * cur_votl;
    m_manager->run_log(this, "cur_val = %u, cur_votl = %llfmA", cur_votl, cur_votl_v);

    // ADC_11DB_VREF / POWER_ADC_RESOLUTION * oV_votl * POWER_AMPLIFY_MULTIPLE
    m_volt = volCoefficient * oV_votl;
    m_manager->run_log(this, "oV_val = %u, oV_votl = %umV", oV_votl, m_volt);
    return true;
}

bool AdjustablePower::setVolt(uint16_t volt)
{
    // dacWrite(m_dacPin, volt/16); // 输出DAC
    ledcWrite(m_pwmPinChannel, (int)(PWM_DAC_FULL_DUTY * volt));

    return true;
}

uint16_t AdjustablePower::getVolt()
{
    return m_volt;
}

bool AdjustablePower::message_handle(const char *from, const char *to,
                                     CTRL_MESSAGE_TYPE type, void *message,
                                     void *ext_info)
{
    return true;
}

bool AdjustablePower::end()
{
    digitalWrite(m_enPin, POWER_OFF);
    dacWrite(m_dacPin, 0); // 输出DAC
    return true;
}