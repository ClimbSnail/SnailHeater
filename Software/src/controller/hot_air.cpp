#include "pid.h"
#include "common.h"
#include "hot_air.h"

#define AMPLIFY_MULTIPLE 101 // 放大倍数

// HotAir hotAir("HotAit", PWM_1_PIN, PWM_1_CHANNEL, FAN_1_PIN, FAN_1_CHANNEL, ADC0_PIN, SW_0_PIN);

HotAir::HotAir(const char *name, SuperManager *m_manager,
               uint8_t powerPin, uint8_t powerPinChannel,
               uint8_t airPin, uint8_t airPinChannel,
               uint8_t temperaturePin, uint8_t shakePin) : ControllerBase(name, m_manager)
{
    // 引脚
    m_powerPin = powerPin;
    m_powerPinChannel = powerPinChannel;
    m_airPin = airPin;
    m_airPinChannel = airPinChannel;
    m_temperaturePin = temperaturePin;
    m_shakePin = shakePin;

    //
    m_powerDuty = 100;
    m_airDuty = 0;

    m_targetTemp = 0;
    m_nowTemp = 0;
    m_swInterruptCounter = 0;
    m_swMux = portMUX_INITIALIZER_UNLOCKED;

    m_bufIndex = 0;
    m_tempBufCnt = 30 * TEMPERATURE_BUF_LEN;

    m_sleepFlag = false;
}

HotAir::~HotAir()
{
    this->end();
}

bool HotAir::start()
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

    // 风扇引脚
    // pinMode(m_airPin, OUTPUT);
    // digitalWrite(m_airPin, HIGH);
    ledcSetup(m_airPinChannel, 5000, 8);
    ledcAttachPin(m_airPin, m_airPinChannel);
    ledcWrite(m_airPinChannel, m_airDuty);

    // ADC引脚（读取热电偶）
    // adcAttachPin(m_temperaturePin); //将引脚映射到ADC
    // pinMode(m_temperaturePin, INPUT);
    pinMode(m_temperaturePin, INPUT_PULLUP);
    // analogSetPinAttenuation(m_temperaturePin, ADC_0db); // 设置精度

    // 振动开关
    pinMode(m_shakePin, INPUT_PULLUP);
    // attachInterrupt(digitalPinToInterrupt(m_shakePin), interruptCallBack, FALLING);
}

// static void interruptCallBack()
// {
//     hotAir.hotAirSwInterruptCallBack();
// }

void HotAir::hotAirSwInterruptCallBack()
{
    portENTER_CRITICAL_ISR(&m_swMux);
    m_swInterruptCounter++;
    Serial.print(m_name);
    Serial.println("-> SW");
    portEXIT_CRITICAL_ISR(&m_swMux);
}

bool HotAir::setTargetTemp(int16_t temperature)
{
    m_targetTemp = temperature;
    return 0;
}

int16_t HotAir::getTargetTemp()
{
    return m_targetTemp;
}

bool HotAir::setPowerDuty(uint8_t duty)
{
    // 由于本可控硅电路是低电平使能 故反向
    m_powerDuty = duty;
    uint8_t value = 100 - constrain(duty, 0, 100);
    ledcWrite(m_powerPinChannel, (int)(2.55 * value));
    return 0;
}

uint8_t HotAir::getPowerDuty()
{
    return m_powerDuty;
}

bool HotAir::disableAir(void)
{
    ledcWrite(m_airPinChannel, (int)(2.55 * 100));
    return 0;
}

bool HotAir::setAirDuty(uint8_t duty, bool flag)
{
    if (flag)
    {
        uint8_t value = 100 - duty;
        ledcWrite(m_airPinChannel, (int)(2.55 * value));
        return 0;
    }
    m_airDuty = constrain(duty, 0, 100);
    uint8_t value = 100 - m_airDuty;
    Serial.printf("\tm_airDuty ---> ");
    Serial.println(m_airDuty);
    ledcWrite(m_airPinChannel, (int)(2.55 * value));
    return 0;
}

uint8_t HotAir::getAirDuty()
{
    return m_airDuty;
}

double HotAir::getCurTemperature(bool flag)
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

bool HotAir::process()
{
    m_nowTemp = getCurTemperature(true);
    if (m_nowTemp <= 45)
    {
        this->disableAir();
    }

    Serial.print("\n[HotAir] ");
    Serial.print("\nnow_temp --> ");
    Serial.print(m_nowTemp);

    if (digitalRead(m_shakePin) == 0 && m_sleepFlag == false)
    {
        m_sleepFlag = true;
        Serial.println("\n-> [Air Sleep]");
        this->setPowerDuty(0);
        this->setAirDuty(100, true);
    }
    else if (digitalRead(m_shakePin) == 1)
    {
        m_sleepFlag = false;
        int pwm = m_pidContorller->get_output(m_targetTemp, m_nowTemp) / 4;
        Serial.print("\tPwm --> ");
        Serial.print(pwm);
        this->setAirDuty(m_airDuty);
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

uint8_t HotAir::getWorkState()
{
    // 状态机

    return 1;
}

bool HotAir::message_handle(const char *from, const char *to,
                            SUPER_MESSAGE_TYPE type, void *message,
                            void *ext_info)
{
    return true;
}

bool HotAir::end()
{
    if (NULL != m_pidContorller)
    {
        delete m_pidContorller;
    }
}