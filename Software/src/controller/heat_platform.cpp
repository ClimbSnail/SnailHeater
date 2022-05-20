#include "pid.h"
#include "common.h"
#include "heat_platform.h"

#define AMPLIFY_MULTIPLE 101 // 放大倍数

HeatPlatform::HeatPlatform(const char *name, SuperManager *m_manager,
                           uint8_t powerPin, uint8_t powerPinChannel,
                           uint8_t temperaturePin, uint8_t shakePin) : ControllerBase(name, m_manager)
{
    // m_pidContorller = new PID(4.5, 0.005, 2.0, 0.1);
    m_pidContorller = new PID(4.5, 0.5, 2.0, 0.1);
    // 引脚
    m_powerPin = powerPin;
    m_powerPinChannel = powerPinChannel;
    m_temperaturePin = temperaturePin;
    m_shakePin = shakePin;

    //
    m_powerDuty = 100;

    m_targetTemp = 0;
    m_nowTemp = 0;

    m_bufIndex = 0;
    m_tempBufCnt = 30 * TEMPERATURE_BUF_LEN;

    m_sleepFlag = false;
}

HeatPlatform::~HeatPlatform()
{
    this->end();
}

bool HeatPlatform::start()
{
    // 初始化PID控制对象
    if (NULL == m_pidContorller)
    {
        // m_pidContorller = new PID(4.5, 0.005, 2.0, 0.1);
        m_pidContorller = new PID(4.5, 0.5, 2.0, 0.1);
    }

    // 使能对应引脚

    // 可控硅控制引脚
    // pinMode(m_powerPin, OUTPUT);
    // digitalWrite(m_powerPin, HIGH);
    ledcSetup(m_powerPinChannel, 5, 8);
    ledcAttachPin(m_powerPin, m_powerPinChannel);
    this->setPowerDuty(m_powerDuty);

    // ADC引脚（读取热电偶）
    // adcAttachPin(m_temperaturePin); //将引脚映射到ADC
    // pinMode(m_temperaturePin, INPUT);
    pinMode(m_temperaturePin, INPUT_PULLUP);
    // analogSetPinAttenuation(m_temperaturePin, ADC_0db); // 设置精度

    // 振动开关
    pinMode(m_shakePin, INPUT_PULLUP);
    // attachInterrupt(digitalPinToInterrupt(m_shakePin), interruptCallBack, FALLING);
}

// void HeatPlatform::swInterruptCallBack()
// {
//     portENTER_CRITICAL_ISR(&m_swMux);
//     m_swInterruptCounter++;
//     Serial.print(m_name);
//     Serial.println("-> SW");
//     portEXIT_CRITICAL_ISR(&m_swMux);
// }

bool HeatPlatform::setTargetTemp(int16_t temperature)
{
    m_targetTemp = temperature;
    return 0;
}

int16_t HeatPlatform::getTargetTemp()
{
    return m_targetTemp;
}

bool HeatPlatform::setPowerDuty(uint8_t duty)
{
    // 由于本可控硅电路是低电平使能 故反向
    m_powerDuty = duty;
    uint8_t value = 100 - constrain(duty, 0, 100);
    ledcWrite(m_powerPinChannel, (int)(2.55 * value));
    return 0;
}

uint8_t HeatPlatform::getPowerDuty()
{
    return m_powerDuty;
}

double HeatPlatform::getCurTemperature(bool flag)
{
    if (false == flag)
    {
        return m_nowTemp;
    }
    Serial.printf("\tADC ---> ");
    uint16_t votl = analogRead(m_temperaturePin) + analogRead(m_temperaturePin) + analogRead(m_temperaturePin);
    Serial.print(votl / 3.0);
    double votl_avg = 3300.0 / 4096 * votl / 3 / AMPLIFY_MULTIPLE;
    // Serial.print(votl_avg);

    // 将本次的温度存入缓冲区 并计算Buffer中的总大小
    m_tempBufCnt -= m_tempBuf[m_bufIndex];
    m_tempBuf[m_bufIndex] = votl_avg / 0.04096;
    m_tempBufCnt += m_tempBuf[m_bufIndex];
    // 下标推进
    m_bufIndex = (m_bufIndex + 1) % TEMPERATURE_BUF_LEN;

    return m_tempBufCnt / TEMPERATURE_BUF_LEN;
}

bool HeatPlatform::process()
{
    m_nowTemp = getCurTemperature(true);

    Serial.print("\n[HeatPlatform] ");
    Serial.print("now_temp --> ");
    Serial.print(m_nowTemp);

    if (-1 == m_shakePin)
    {
        int pwm = m_pidContorller->get_output(m_targetTemp, m_nowTemp) / 4;
        Serial.print("\tPwm --> ");
        Serial.print(pwm);
    }
    else
    {
        if (digitalRead(m_shakePin) == 0 && m_sleepFlag == false)
        {
            m_sleepFlag = true;
            this->setPowerDuty(0);
        }
        else if (digitalRead(m_shakePin) == 1)
        {
            m_sleepFlag = false;
            int pwm = m_pidContorller->get_output(m_targetTemp, m_nowTemp) / 4;
            Serial.print("\tPwm --> ");
            Serial.print(pwm);
            if (pwm > 0)
            {
                this->setPowerDuty(pwm);
            }
            else
            {
                this->setPowerDuty(0);
            }
        }
    }
}

uint8_t HeatPlatform::getWorkState()
{
    // 状态机

    return 1;
}

bool HeatPlatform::message_handle(const char *from, const char *to,
                                  SUPER_MESSAGE_TYPE type, void *message,
                                  void *ext_info)
{
    return true;
}

bool HeatPlatform::end()
{
    if (NULL != m_pidContorller)
    {
        delete m_pidContorller;
    }
}