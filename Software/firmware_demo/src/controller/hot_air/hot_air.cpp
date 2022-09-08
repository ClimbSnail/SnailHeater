#include "pid.h"
#include "common.h"
#include "hot_air.h"

#include "sys/snail_manager.h"

#define SECTION_CTRL // 使用分段控制（加速优化）

#define AMPLIFY_MULTIPLE 101          // 放大倍数
#define HOT_AIR_ADC_RESOLUTION 4096.0 // 分辨率
// #define HOT_AIR_ADC_RESOLUTION 8192.0 // 分辨率

#define HOT_FREQ PWM_GROUP_0_FREQ
#define HOT_BIT_WIDTH PWM_GROUP_0_BIT_WIDTH
#define HOT_FULL_DUTY PWM_GROUP_0_DUTY

// #define AIR_FREQ 512
// #define AIR_FREQ 256
// #define AIR_BIT_WIDTH 8
// #define AIR_FULL_DUTY 2.55

#define AIR_FREQ PWM_GROUP_1_FREQ
#define AIR_BIT_WIDTH PWM_GROUP_1_BIT_WIDTH
#define AIR_FULL_DUTY PWM_GROUP_1_DUTY

#define PID_START_D_VALUE 50

static int count_display = 0;

HotAir::HotAir(const char *name, SnailManager *m_manager,
               uint8_t powerPin, uint8_t powerPinChannel,
               uint8_t airPin, uint8_t airPinChannel,
               uint8_t temperaturePin, uint8_t shakePin) : ControllerBase(name, CTRL_TYPE_HOTAIR, m_manager)
{
    m_pidContorller = NULL;

    // 引脚
    m_powerPin = powerPin;
    m_powerPinChannel = powerPinChannel;
    m_airPin = airPin;
    m_airPinChannel = airPinChannel;
    m_temperaturePin = temperaturePin;
    m_shakePin = shakePin;

    //
    m_powerDuty = 0;
    m_airDuty = 10;

    m_targetTemp = 0;
    m_curTemp = 0;

    curHotAirObjInd = s_hotAirObjNum;
    ++s_hotAirObjNum;

    // 温度缓冲区初始化赋值
    m_bufIndex = 0;
    m_tempBufSum = 30 * TEMPERATURE_BUF_LEN;
    for (int cnt = 0; cnt < TEMPERATURE_BUF_LEN; ++cnt)
    {
        m_tempBuf[cnt] = 30;
    }
    m_workState = HOTAIR_STATE_SLEEP;
    m_heatingMode = HOT_HEATING_MODE_NONE;
    m_airMode = AIR_HEATING_MODE_NONE;
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
        m_pidContorller = new PID(0.15, 0.5, 2.0, 0.05);
    }

    // 使能对应引脚

    // 可控硅控制引脚
    pinMode(m_powerPin, OUTPUT);
    ledcSetup(m_powerPinChannel, HOT_FREQ, HOT_BIT_WIDTH);
#ifndef SECTION_CTRL
    ledcAttachPin(m_powerPin, m_powerPinChannel);
#endif
    // 关闭可控硅的输出
    this->setHeatingMode(HOT_HEATING_MODE_FULL, true);
    this->setPowerDuty(m_powerDuty);

    // 风扇引脚
    pinMode(m_airPin, OUTPUT);
    ledcSetup(m_airPinChannel, AIR_FREQ, AIR_BIT_WIDTH);
#ifndef SECTION_CTRL
    ledcAttachPin(m_airPin, m_airPinChannel);
#endif
    // 关闭风扇
    this->setAirgMode(AIR_HEATING_MODE_FULL, true);
    this->enbleAirDuty(0);

    // ADC引脚（读取热电偶）
    // adcAttachPin(m_temperaturePin); //将引脚映射到ADC
    // pinMode(m_temperaturePin, INPUT);
    pinMode(m_temperaturePin, INPUT_PULLUP);
    analogSetPinAttenuation(m_temperaturePin, ADC_11db); // 设置精度
    // registerMyADC(m_temperaturePin);                     // 注册ADC检测服务 todo

    // 振动开关
    pinMode(m_shakePin, INPUT_PULLUP);
    if (0 == curHotAirObjInd)
    {
        attachInterrupt(digitalPinToInterrupt(m_shakePin), HotAir::hotAirSwInterruptCallBack_0, FALLING);
    }
    else
    {
        attachInterrupt(digitalPinToInterrupt(m_shakePin), HotAir::hotAirSwInterruptCallBack_1, FALLING);
    }

    // 读取配置文件
    this->read_config(&m_config, 0);

    this->m_targetTemp = m_config.settingTemp;
    this->m_airDuty = m_config.workAirSpeed;

    return true;
}

void HotAir::hotAirSwInterruptCallBack_0(void)
{
    // 记录唤醒时的时间
    s_wakeMillis_0 = xTaskGetTickCount() / portTICK_PERIOD_MS;
}

void HotAir::hotAirSwInterruptCallBack_1(void)
{
    // 记录唤醒时的时间
    s_wakeMillis_1 = xTaskGetTickCount() / portTICK_PERIOD_MS;
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

bool HotAir::setHeatingMode(HOT_HEATING_MODE mode, bool force)
{
    // // 连接pwm控制器
    // ledcAttachPin(m_powerPin, m_powerPinChannel);
    // return 0;
    // mode = HOT_HEATING_MODE_FULL; // todo

#ifdef SECTION_CTRL
    switch (mode)
    {
    case HOT_HEATING_MODE_NONE:
    {
    }
    break;
    case HOT_HEATING_MODE_PWM:
    {
        if (force || this->m_heatingMode != mode)
        {
            // 连接pwm控制器
            ledcAttachPin(m_powerPin, m_powerPinChannel);
        }
    }
    break;
    case HOT_HEATING_MODE_FULL:
    {
        if (force || this->m_heatingMode != mode)
        {
            // 全速装懂 断开PWM控制
            ledcDetachPin(m_powerPin); // 断开PWM控制器
            pinMode(m_powerPin, OUTPUT);
        }
    }
    break;
    defualt:
        break;
    }
    // 更新状态
    this->m_heatingMode = mode;
#endif
    return 0;
}

bool HotAir::setPowerDuty(uint16_t duty)
{
    // duty = 50;
    // // 由于本可控硅电路是低电平使能 故反向
    // m_powerDuty = duty;
    // uint8_t value = 100 - constrain(duty, 0, 100);
    // ledcWrite(m_powerPinChannel, (int)(HOT_FULL_DUTY * value));
    // return 0;

#ifndef SECTION_CTRL
    // 由于本可控硅电路是低电平使能 故反向
    m_powerDuty = duty;
    uint8_t value = 100 - constrain(duty, 0, 100);
    ledcWrite(m_powerPinChannel, (int)(HOT_FULL_DUTY * value));

#else

    switch (this->m_heatingMode)
    {
    case HEATING_MODE_NONE:
    {
    }
    break;
    case HEATING_MODE_PWM:
    {
        // 由于本可控硅电路是低电平使能 故反向
        m_powerDuty = duty;
        uint8_t value = 100 - constrain(duty, 0, 100);
        ledcWrite(m_powerPinChannel, (int)(HOT_FULL_DUTY * value));
    }
    break;
    case HEATING_MODE_FULL:
    {
        m_powerDuty = duty;
        if (duty == 0)
        {
            digitalWrite(m_powerPin, HIGH);
        }
        else
        {
            digitalWrite(m_powerPin, LOW);
        }
    }
    break;
    defualt:
        break;
    }
#endif
    return 0;
}

uint8_t HotAir::getPowerDuty()
{
    return m_powerDuty;
}

bool HotAir::setAirgMode(AIR_HEATING_MODE mode, bool force)
{
#ifdef SECTION_CTRL
    switch (mode)
    {
    case AIR_HEATING_MODE_NONE:
    {
    }
    break;
    case AIR_HEATING_MODE_PWM:
    {
        if (force || this->m_airMode != mode)
        {
            // 连接pwm控制器
            ledcAttachPin(m_airPin, m_airPinChannel);
            m_manager->run_log(this, "-----------> [ledcAttachPin]");
        }
    }
    break;
    case AIR_HEATING_MODE_FULL:
    {
        if (force || this->m_airMode != mode)
        {
            // 全速装懂 断开PWM控制
            ledcDetachPin(m_airPin); // 断开PWM控制器
            pinMode(m_airPin, OUTPUT);
            m_manager->run_log(this, "-----------> [ledcDetachPin]");
        }
    }
    break;
    defualt:
        break;
    }
    // 更新状态
    this->m_airMode = mode;
#endif
    return 0;
}

bool HotAir::enbleAirDuty(uint8_t duty)
{
#define EACH_VOLTAGE 5.76 // 24*24/100 24V风扇功率100等分

#ifndef SECTION_CTRL
    // 由于本可控硅电路是低电平使能 故反向
    int vol = (int)(sqrt(EACH_VOLTAGE * duty) / 24.0 * 100);
    uint8_t value = 100 - constrain(vol, 0, 100);
    m_manager->run_log(this, "-----------> duty = %u", (int)(AIR_FULL_DUTY * value));
    ledcWrite(m_airPinChannel, (int)(AIR_FULL_DUTY * value));

#else

    switch (this->m_airMode)
    {
    case AIR_HEATING_MODE_NONE:
    {
    }
    break;
    case AIR_HEATING_MODE_PWM:
    {
        // 由于本可控硅电路是低电平使能 故反向
        int vol = (int)(sqrt(EACH_VOLTAGE * duty) / 24.0 * 100);
        uint8_t value = 100 - constrain(vol, 0, 100);
        m_manager->run_log(this, "-----------> duty = %u", (int)(AIR_FULL_DUTY * value));
        ledcWrite(m_airPinChannel, (int)(AIR_FULL_DUTY * value));
        m_manager->run_log(this, "-----------> airVol = %u", (int)(AIR_FULL_DUTY * value) * (3300 / 256));
    }
    break;
    case AIR_HEATING_MODE_FULL:
    {
        if (duty == 0)
        {
            digitalWrite(m_airPin, HIGH);
        }
        else
        {
            digitalWrite(m_airPin, LOW);
        }
    }
    break;
    default:
        break;
    }
#endif
    return 0;
}

bool HotAir::setAirDuty(uint8_t duty)
{
    this->m_airDuty = duty;
    this->setAirgMode(AIR_HEATING_MODE_PWM, true);
    this->enbleAirDuty(this->m_airDuty);
    return true;
}

uint8_t HotAir::getAirDuty()
{
    return m_airDuty;
}

double HotAir::getCurTemperature(bool isReal)
{
    if (false == isReal)
    {
        return m_tempBufSum / TEMPERATURE_BUF_LEN;
    }

    // uint16_t adcVal = getMyADC(m_temperaturePin);
    uint16_t adcVal = getMyADC(m_temperaturePin, 10, false);
    double votl_avg = ADC_11DB_VREF / HOT_AIR_ADC_RESOLUTION * adcVal / AMPLIFY_MULTIPLE;
    double realTemp = votl_avg / 0.04096;

    if (m_workState >= HOTAIR_STATE_SLEEP && count_display % 9 == 0)
    {
        m_manager->run_log(this, "\nADC ---> %u", adcVal);
        m_manager->run_log(this, "realTemp --> %llf", realTemp);
        m_manager->run_log(this, "%llf mV", ADC_11DB_VREF / HOT_AIR_ADC_RESOLUTION * adcVal);
        m_manager->run_log(this, "AirDuty = %d %%", this->m_airDuty);
        m_manager->run_log(this, "m_tempBufSum = %llf", m_tempBufSum);
        // for (int ind = 0; ind < TEMPERATURE_BUF_LEN; ind++)
        // {
        //     m_manager->run_log(this, "m_tempBuf[%d] = %llf", ind, m_tempBuf[ind]);
        // }
    }

    // 将本次的温度存入缓冲区 并计算Buffer中的总大小
    m_tempBufSum -= m_tempBuf[m_bufIndex];
    m_tempBuf[m_bufIndex] = realTemp;
    m_tempBufSum += m_tempBuf[m_bufIndex];
    // 下标推进
    m_bufIndex = (m_bufIndex + 1) % TEMPERATURE_BUF_LEN;

    return realTemp;
}

bool HotAir::process()
{
    int heat_pwm = 0;

    m_curTemp = getCurTemperature(true);

    count_display = (count_display + 1) % 10000;

    switch (m_workState)
    {
    case HOTAIR_STATE_SLEEP:
    {
        if (digitalRead(m_shakePin) == 1)
        {
            // 唤醒
            if (m_targetTemp > m_curTemp + m_config.pid_start_low_value)
            {
                // 温差太大时 全速加热
                this->setHeatingMode(HOT_HEATING_MODE_FULL); // 设置加热模式
            }
            else
            {
                this->setHeatingMode(HOT_HEATING_MODE_PWM); // 设置加热模式
                m_pidContorller->reset();                   // 准备复位pid数据 重新计算
            }

            m_workState = HOTAIR_STATE_NORMAL;
            this->setAirgMode(AIR_HEATING_MODE_PWM);
            this->enbleAirDuty(this->m_airDuty);
            heat_pwm = 0; // 或者100也行

            m_manager->addEvent(CTRL_EVENT_AIRHOT_WAKE);
            m_manager->run_log(this, "-> [Wake]");
        }
    }
    break;
    case HOTAIR_STATE_INTER_SLEEP:
    {
        if (digitalRead(m_shakePin) == 1)
        {
            // 唤醒
            if (m_targetTemp > m_curTemp + m_config.pid_start_low_value)
            {
                // 温差太大时 全速加热
                this->setHeatingMode(HOT_HEATING_MODE_FULL); // 设置加热模式
            }
            else
            {
                this->setHeatingMode(HOT_HEATING_MODE_PWM); // 设置加热模式
                m_pidContorller->reset();                   // 准备复位pid数据 重新计算
            }

            m_workState = HOTAIR_STATE_NORMAL;
            this->setAirgMode(AIR_HEATING_MODE_PWM);
            this->enbleAirDuty(this->m_airDuty);
            heat_pwm = 0; // 或者100也行

            m_manager->addEvent(CTRL_EVENT_AIRHOT_WAKE);
            m_manager->run_log(this, "-> [Wake]");
        }
        else if (m_curTemp <= m_config.coolingFinishTemp)
        {
            // 进入休眠状态
            m_workState = HOTAIR_STATE_SLEEP;
            this->setHeatingMode(HOT_HEATING_MODE_FULL); // 设置加热模式
            this->setAirgMode(AIR_HEATING_MODE_FULL);
            this->enbleAirDuty(0);
            heat_pwm = 0;

            m_manager->addEvent(CTRL_EVENT_AIRHOT_SLEEP);
            m_manager->run_log(this, "-> [Air Sleep]");
        }
    }
    break;
    case HOTAIR_STATE_NORMAL:
    {
        if (digitalRead(m_shakePin) == 0)
        {
            // 进入预休眠状态
            m_workState = HOTAIR_STATE_INTER_SLEEP;
            this->setHeatingMode(HOT_HEATING_MODE_FULL); // 设置加热模式
            this->setAirgMode(AIR_HEATING_MODE_FULL);
            this->enbleAirDuty(100); // 全速降温

            m_manager->run_log(this, "-> [Air Inter Sleep]");
            heat_pwm = 0;
        }
        else if (m_targetTemp > m_curTemp + m_config.pid_start_low_value)
        {
            // 全速加热
            this->setHeatingMode(HOT_HEATING_MODE_FULL); // 设置加热模式
            heat_pwm = 100;                              // todo
        }
        else if (m_curTemp > m_targetTemp - m_config.pid_start_low_value && (m_curTemp < m_targetTemp + m_config.pid_start_high_value))
        {
            // 进入pid
            this->setHeatingMode(HOT_HEATING_MODE_PWM);
            heat_pwm = m_pidContorller->get_output(m_targetTemp, m_curTemp);
        }
        else
        {
            // 超温
            this->setHeatingMode(HOT_HEATING_MODE_PWM);
            // m_pidContorller->get_output(m_targetTemp, m_curTemp);
            m_pidContorller->reset(); // 准备复位pid数据 重新计算
            heat_pwm = 0;
        }
    }
    break;
    default:
        break;
    }

    if (heat_pwm < 0)
    {
        heat_pwm = 0;
    }
    this->setPowerDuty(heat_pwm);

    if (m_workState >= HOTAIR_STATE_SLEEP && count_display % 20 == 0)
    {
        m_manager->run_log(this, "heat_pwm = %d", heat_pwm);
    }

    return true;
}

uint8_t HotAir::getWorkState()
{
    // 状态机

    return 1;
}

bool HotAir::message_handle(const char *from, const char *to,
                            CTRL_MESSAGE_TYPE type, void *message,
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
    return true;
}