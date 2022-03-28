#include "pid.h"
#include "common.h"
#include "hot_air.h"

#define AMPLIFY_MULTIPLE 101 // 放大倍数

// HotAir hotAir("HotAit", PWM_1_PIN, PWM_1_CHANNEL, FAN_1_PIN, FAN_1_CHANNEL, ADC0_PIN, SW_0_PIN);

HotAir::HotAir(const char *name, uint8_t powerPin, uint8_t powerPinChannel,
               uint8_t airPin, uint8_t airPinChannel,
               uint8_t temperaturePin, uint8_t shakePin)
{
    strncpy(m_name, name, 16);
    m_pidContorller = new PID(4.5, 0.15, 0.2, 0.2);
    // 引脚
    m_powerPin = powerPin;
    m_powerPinChannel = powerPinChannel;
    m_airPin = airPin;
    m_airPinChannel = airPinChannel;
    m_temperaturePin = temperaturePin;
    m_shakePin = shakePin;

    //
    m_powerDuty = (int)(1.0 * 255);
    m_airDuty = 0;

    m_targetTemp = 0;
    m_swInterruptCounter = 0;
    m_swMux = portMUX_INITIALIZER_UNLOCKED;

    m_bufIndex = 0;
    m_m_tempBufCnt = 0;
}

HotAir::~HotAir()
{
}

bool HotAir::start()
{
    // 初始化PID控制对象

    // 使能对应引脚

    // 可控硅控制引脚
    // pinMode(m_powerPin, OUTPUT);
    // digitalWrite(m_powerPin, HIGH);
    ledcSetup(m_powerPinChannel, 5, 8);
    ledcAttachPin(m_powerPin, m_powerPinChannel);
    ledcWrite(m_powerPinChannel, m_powerDuty);

    // 风扇引脚
    // pinMode(m_airPin, OUTPUT);
    // digitalWrite(m_airPin, HIGH);
    ledcSetup(m_airPinChannel, 5000, 8);
    ledcAttachPin(m_airPin, m_airPinChannel);
    ledcWrite(m_airPinChannel, m_airDuty);

    // ADC引脚（读取热电偶）
    // adcAttachPin(m_temperaturePin); //将引脚映射到ADC
    pinMode(m_temperaturePin, INPUT);
    // analogSetPinAttenuation(m_temperaturePin, ADC_0db); // 设置精度

    // 振动开关
    pinMode(m_shakePin, INPUT_PULLUP);
    // attachInterrupt(digitalPinToInterrupt(m_shakePin), interruptCallBack, FALLING);
}

static void interruptCallBack()
{
    hotAir.swInterruptCallBack();
}

void HotAir::swInterruptCallBack()
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
    m_powerDuty = constrain(duty, 0, 100);
    ledcWrite(m_powerPinChannel, (int)(2.55 * m_powerDuty));
    return 0;
}

uint8_t HotAir::getPowerDuty()
{
    return m_powerDuty;
}

bool HotAir::setAirDuty(uint8_t duty)
{
    m_airDuty = constrain(duty, 0, 100);
    Serial.printf("\tm_airDuty ---> ");
    Serial.println(m_airDuty);
    ledcWrite(m_airPinChannel, (int)(2.55 * m_airDuty));
    return 0;
}

uint8_t HotAir::getAirDuty()
{
    return m_airDuty;
}

double HotAir::getTemperature()
{
    Serial.printf("\tADC ---> ");
    uint16_t votl = analogRead(m_temperaturePin) + analogRead(m_temperaturePin) + analogRead(m_temperaturePin);
    double votl_avg = 3300.0 / 4096 * votl / 3 / AMPLIFY_MULTIPLE;
    Serial.print(votl_avg);

    // 将本次的温度存入缓冲区 并计算Buffer中的总大小
    m_m_tempBufCnt -= m_tempBuf[m_bufIndex];
    m_tempBuf[m_bufIndex] = votl_avg / 0.04096;
    m_m_tempBufCnt += m_tempBuf[m_bufIndex];
    // 下标推进
    m_bufIndex = (m_bufIndex + 1) % TEMPERATURE_BUF_LEN;

    return m_m_tempBufCnt / TEMPERATURE_BUF_LEN;
}

bool HotAir::process()
{

    double now_temp = getTemperature();
    int pwm = 100 - m_pidContorller->get_output(m_targetTemp, now_temp) / 4;
    Serial.print("\tnow_temp --> ");
    Serial.print(now_temp);
    Serial.print("\tPwm --> ");
    Serial.println(pwm);
    if (pwm > 0)
    {
        this->setPowerDuty(pwm);
    }
    if (digitalRead(m_shakePin) == 0)
    {
        Serial.println("-> SW");
    }
}

bool HotAir::end()
{
}