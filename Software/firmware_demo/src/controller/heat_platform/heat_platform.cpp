#include "pid.h"
#include "common.h"
#include "heat_platform.h"

#include "sys/snail_manager.h"

#define AMPLIFY_MULTIPLE 101 // 放大倍数

#define HEAT_PLATFORM_HOT_FREQ 5
#define HEAT_PLATFORM_HOT_BIT_WIDTH 14
#define HEAT_PLATFORM_HOT_FULL_DUTY 163.83

#define HP_ADC_RESOLUTION 8191.0 // 分辨率

HeatPlatform::HeatPlatform(const char *name, SnailManager *m_manager,
                           uint8_t powerPin, uint8_t powerPinChannel,
                           uint8_t temperaturePin, uint8_t shakePin) : ControllerBase(name, CTRL_TYPE_HEATPLATFORM, m_manager)
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

    // 温度缓冲区初始化赋值
    m_bufIndex = 0;
    m_tempBufSum = 30 * TEMPERATURE_BUF_LEN;
    for (int cnt = 0; cnt < TEMPERATURE_BUF_LEN; ++cnt)
    {
        m_tempBuf[cnt] = 30;
    }

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
    ledcSetup(m_powerPinChannel, HEAT_PLATFORM_HOT_FREQ, HEAT_PLATFORM_HOT_BIT_WIDTH);
    ledcAttachPin(m_powerPin, m_powerPinChannel);
    this->setPowerDuty(m_powerDuty);

    // ADC引脚（读取热电偶）
    // adcAttachPin(m_temperaturePin); //将引脚映射到ADC
    // pinMode(m_temperaturePin, INPUT);
    pinMode(m_temperaturePin, INPUT_PULLUP);
    analogSetPinAttenuation(m_temperaturePin, ADC_11db); // 设置精度
    registerMyADC(m_temperaturePin);                     // 注册ADC检测服务

    if (255 != m_shakePin)
    {
        // 振动开关
        pinMode(m_shakePin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(m_shakePin), HeatPlatform::swInterruptCallBack, FALLING);
    }
    return true;
}

void HeatPlatform::swInterruptCallBack(void)
{
    // 记录唤醒时的时间
    s_wakeMillis = xTaskGetTickCount() / portTICK_PERIOD_MS;
}

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
    m_powerDuty = duty = 50;
    uint8_t value = 100 - constrain(duty, 0, 100);
    ledcWrite(m_powerPinChannel, (int)(HEAT_PLATFORM_HOT_FULL_DUTY * value));
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
    m_manager->run_log(this, "\tADC ---> ");
    // uint16_t votl = analogRead(m_temperaturePin) + analogRead(m_temperaturePin) + analogRead(m_temperaturePin);
    uint16_t votl = getMyADC(m_temperaturePin);
    m_manager->run_log(this, "%llf", votl);
    double votl_avg = ADC_11DB_VREF / HP_ADC_RESOLUTION * votl / AMPLIFY_MULTIPLE;
    m_manager->run_log(this, "%llf mA", ADC_11DB_VREF / HP_ADC_RESOLUTION * votl);
    // m_manager->run_log(this, votl_avg);

    // 将本次的温度存入缓冲区 并计算Buffer中的总大小
    m_tempBufSum -= m_tempBuf[m_bufIndex];
    m_tempBuf[m_bufIndex] = votl_avg / 0.04096;
    m_tempBufSum += m_tempBuf[m_bufIndex];
    // 下标推进
    m_bufIndex = (m_bufIndex + 1) % TEMPERATURE_BUF_LEN;

    return m_tempBufSum / TEMPERATURE_BUF_LEN;
}

bool HeatPlatform::process()
{
    m_nowTemp = getCurTemperature(true);

    m_manager->run_log(this, "\n[HeatPlatform] ");
    m_manager->run_log(this, "now_temp --> ");
    m_manager->run_log(this, "%llf", m_nowTemp);

    if (-1 == m_shakePin)
    {
        int pwm = m_pidContorller->get_output(m_targetTemp, m_nowTemp) / 4;
        m_manager->run_log(this, "\tPwm --> ");
        m_manager->run_log(this, "%d", pwm);
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
            m_manager->run_log(this, "\tPwm --> ");
            m_manager->run_log(this, "%d", pwm);
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
    return true;
}

uint8_t HeatPlatform::getWorkState()
{
    // 状态机

    return 1;
}

bool HeatPlatform::message_handle(const char *from, const char *to,
                                  CTRL_MESSAGE_TYPE type, void *message,
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