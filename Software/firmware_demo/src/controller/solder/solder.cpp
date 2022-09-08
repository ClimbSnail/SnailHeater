#include "pid.h"
#include "common.h"
#include "solder.h"

#include "sys/snail_manager.h"

#define SOLDER_AMPLIFY_MULTIPLE 201  // 放大倍数
#define SOLDER_ADC_RESOLUTION 4096.0 // 分辨率

#define N_MOSFET

#if defined(P_MOSFET) // P-Channel MOSFET
#define HEATER_ON 1023
#define HEATER_OFF 0
#define HEATER_PWM 1023 - Output
#elif defined(N_MOSFET) // N-Channel MOSFET
#define HEATER_ON 0
#define HEATER_OFF 1023
#define HEATER_PWM Output
#endif

#define SOLDER_FREQ PWM_GROUP_2_FREQ // 开关频率低，断开的时候可能会有残留的高电平影响测量
// 开关频率高，可能烙铁芯会有杂音并且振动进而和振动开关共振，导致无法休眠
#define SOLDER_BIT_WIDTH PWM_GROUP_2_BIT_WIDTH
#define SOLDER_FULL_DUTY PWM_GROUP_2_DUTY

#define STANDARD_TEMP_NUM 12
// double StandardTemp[STANDARD_TEMP_NUM][2] = {{0, 0.0},
//                                              {25, 0.135108},
//                                              {50, 4.041835},
//                                              {100, 11.6},
//                                              {150, 19.17275},
//                                              {200, 26.7455},
//                                              {250, 33},
//                                              {300, 40},
//                                              {350, 47},
//                                              {400, 54},
//                                              {450, 61},
//                                              {500, 68}};

double StandardTemp[STANDARD_TEMP_NUM][2] = {{0, 0.0},
                                             {25, 1.024},
                                             {50, 2.048},
                                             {100, 4.096},
                                             {150, 6.144},
                                             {200, 8.192},
                                             {250, 10.24},
                                             {300, 12.288},
                                             {350, 14.336},
                                             {400, 16.384},
                                             {450, 61},
                                             {500, 68}};

#define SOLDER_START_PID_D_VALUE_LOW 20
#define SOLDER_START_PID_D_VALUE_HIGH 5
#define DEEP_SLEEP_TEMP 0                   // 深度休眠温度
#define SLEEP_TEMP (m_config.easySleepTemp) // 休眠温度

#define SOLDER_DEBUG

// 设定休眠的时间 毫秒数 120000(120秒)
#define SLEEP_MILLIS (m_config.enterEasySleepTime)      // 进入轻度休眠时间
#define DEEP_SLEEP_MILLIS (m_config.enterDeepSleepTime) // 进入深度休眠时间

static int count_display = 0;

Solder::Solder(const char *name, SnailManager *m_manager,
               SOLDER_TYPE type, uint8_t powerPin, uint8_t powerPinChannel,
               uint8_t temperaturePin, uint8_t shakePin)
    : ControllerBase(name, CTRL_TYPE_SOLDER, m_manager)
{
    m_pidContorller = NULL;
    // 烙铁的类型
    m_type = type;
    // 引脚
    m_powerPin = powerPin;
    m_powerPinChannel = powerPinChannel;
    m_temperaturePin = temperaturePin;
    m_shakePin = shakePin;

    //
    m_powerDuty = 0;

    m_targetTemp = 0;
    m_curTemp = 0;

    // 温度缓冲区初始化赋值
    m_bufIndex = 0;
    m_tempBufSum = 30 * TEMPERATURE_BUF_LEN;
    for (int cnt = 0; cnt < TEMPERATURE_BUF_LEN; ++cnt)
    {
        m_tempBuf[cnt] = 30;
    }

    m_sleepFlag = false;
    m_workState = SOLDER_STATE_SLEEP;
    m_heatingMode = HEATING_MODE_NONE;

    // 间接使烙铁开机即进入休眠状态
    s_wakeSignalCnt = 0;
    s_wakeMillis = xTaskGetTickCount() / portTICK_PERIOD_MS - SLEEP_MILLIS;

    volCoefficient = ADC_11DB_VREF / SOLDER_ADC_RESOLUTION / SOLDER_AMPLIFY_MULTIPLE;
    tempCoefficient = 1;
}

Solder::~Solder()
{
    this->end();
}

bool Solder::start()
{
    // 初始化PID控制对象
    if (NULL == m_pidContorller)
    {
        // m_pidContorller = new PID(4.5, 0.005, 2.0, 0.1);
        m_pidContorller = new PID(1.5, 1.0, 2.0, 0.05);
    }

    // 使能对应引脚

    // MOS控制引脚
    pinMode(m_powerPin, OUTPUT);
    ledcSetup(m_powerPinChannel, SOLDER_FREQ, SOLDER_BIT_WIDTH);
    this->setHeatingMode(HEATING_MODE_FULL, true);
    this->setPowerDuty(m_powerDuty);

    // ADC引脚（读取热电偶）
    // adcAttachPin(m_temperaturePin); //将引脚映射到ADC
    // pinMode(m_temperaturePin, INPUT);
    pinMode(m_temperaturePin, INPUT_PULLUP);
    analogSetPinAttenuation(m_temperaturePin, ADC_11db); // 设置精度
    analogReadResolution(12);

    // 振动开关
    pinMode(m_shakePin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(m_shakePin), Solder::solderSwInterruptCallBack, CHANGE);

    // 读取配置文件
    this->read_config(&m_config);
    // 参数赋值
    this->m_targetTemp = m_config.settingTemp;

    return true;
}

void Solder::solderSwInterruptCallBack(void)
{
    // 记录唤醒时的时间
    s_wakeMillis = xTaskGetTickCount() / portTICK_PERIOD_MS;
}

bool Solder::setTargetTemp(int16_t temperature)
{
    m_targetTemp = temperature;
    return 0;
}

int16_t Solder::getTargetTemp()
{
    return m_targetTemp;
}

bool Solder::setPowerDuty(uint16_t duty)
{
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
        ledcWrite(m_powerPinChannel, (int)(SOLDER_FULL_DUTY * value));
    }
    break;
    case HEATING_MODE_FULL:
    {
        m_powerDuty = duty;
        if (duty == 0)
        {
            digitalWrite(m_powerPin, HIGH);
        }
        else if (duty == 100)
        {
            digitalWrite(m_powerPin, LOW);
        }
    }
    break;
    defualt:
        break;
    }

    return 0;
}

bool Solder::setHeatingMode(HEATING_MODE mode, bool force)
{
    // mode = HEATING_MODE_PWM; // todo

    switch (mode)
    {
    case HEATING_MODE_NONE:
    {
    }
    break;
    case HEATING_MODE_PWM:
    {
        if (force || this->m_heatingMode != mode)
        {
            // 连接pwm控制器
            ledcAttachPin(m_powerPin, m_powerPinChannel);
        }
    }
    break;
    case HEATING_MODE_FULL:
    {
        if (force || this->m_heatingMode != mode)
        {
            // 全速加热 断开PWM控制
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
    return 0;
}

// 临时关闭输出
bool Solder::tmpCloseOutPut(void)
{
    // if (SOLDER_TYPE_JBC245 == m_type)
    // {
    //     // JBC245不需要断电测温
    //     return 0;
    // }

    switch (this->m_heatingMode)
    {
    case HEATING_MODE_NONE:
    case HEATING_MODE_PWM:
    {
        {
            // 全速加热 则需要断开PWM控制
            ledcDetachPin(m_powerPin); // 断开PWM控制器
            pinMode(m_powerPin, OUTPUT);
        }
        digitalWrite(m_powerPin, HIGH);
        // ledcWrite(m_powerPinChannel, HEATER_OFF);
    }
    case HEATING_MODE_FULL:
    {
        digitalWrite(m_powerPin, HIGH);
    }
    break;
    defualt:
        break;
    }

    vTaskDelay(1 / portTICK_PERIOD_MS);
    // this->sleepCheck(); // todo

    return 0;
}

uint8_t Solder::getPowerDuty()
{
    return m_powerDuty;
}

double Solder::getCurTemperature(bool flag)
{
    if (false == flag)
    {
        // 排序
        double swap_tmp = 0;
        for (int i = 0; i < TEMPERATURE_BUF_LEN - 1; ++i)
        {
            for (int j = i + 1; j < TEMPERATURE_BUF_LEN; ++j)
            {
                if (m_tempBuf[i] < m_tempBuf[j])
                {
                    swap_tmp = m_tempBuf[i];
                    m_tempBuf[i] = m_tempBuf[j];
                    m_tempBuf[j] = swap_tmp;
                }
            }
        }

        double sum = 0;
        for (int ind = 5; ind < TEMPERATURE_BUF_LEN - 0; ++ind)
        {
            sum += m_tempBuf[ind];
        }
        return sum / (TEMPERATURE_BUF_LEN - 5);
        // return m_tempBufSum / TEMPERATURE_BUF_LEN;
    }

#ifdef SOLDER_DEBUG
    // 暂停加热 准备测温
    this->tmpCloseOutPut();
#endif

    // uint16_t adcVal = getMyADC(m_temperaturePin, 10, true);
    uint16_t adcVal = getMyADC(m_temperaturePin, 10, false); // / 9.77;

#ifdef SOLDER_DEBUG
    // if (SOLDER_TYPE_T12 == m_type)
    {
        // 恢复工作 继续加热
        this->setHeatingMode(this->m_heatingMode, true); // 强制设置状态
        this->setPowerDuty(this->m_powerDuty);
    }
#endif

    if (adcVal > 8000)
    {
        m_manager->run_log(this, "adcVal > 8000");
        m_manager->run_log(this, "%u\n", adcVal);
        return m_tempBufSum / TEMPERATURE_BUF_LEN;
    }

    // 计算得到热电偶电压 ADC_11DB_VREF / SOLDER_ADC_RESOLUTION * adcVal / SOLDER_AMPLIFY_MULTIPLE
    double thermocoupleVol = this->volCoefficient * adcVal;

    double nowTemp = 0;
    // int ntcTemp = 0;
    if (SOLDER_TYPE_T12 == m_type)
    {
        nowTemp = thermocoupleVol / 0.022;
        // int found_high_ind = 0;
        // for (int pos = 1; pos < STANDARD_TEMP_NUM; pos++)
        // {
        //     if (StandardTemp[pos][1] > thermocoupleVol)
        //     {
        //         found_high_ind = pos;
        //         break;
        //     }
        // }
        // double tempStep = (StandardTemp[found_high_ind][1] - StandardTemp[found_high_ind - 1][1]) / (StandardTemp[found_high_ind][0] - StandardTemp[found_high_ind - 1][0]);
        // nowTemp = StandardTemp[found_high_ind - 1][0] + (thermocoupleVol - StandardTemp[found_high_ind - 1][1]) / tempStep;
        // m_manager->run_log(this, "tempStep = %llf, found_high_ind = %d", tempStep, found_high_ind);
    }
    else if (SOLDER_TYPE_JBC245 == m_type)
    {
        nowTemp = thermocoupleVol / 0.025;
    }
    else if (SOLDER_TYPE_JBC210 == m_type)
    {
        nowTemp = thermocoupleVol / 0.007;
    }

    // 将本次的温度存入缓冲区 并计算Buffer中的总大小
    m_tempBufSum -= m_tempBuf[m_bufIndex];
    m_tempBuf[m_bufIndex] = nowTemp;
    m_tempBufSum += m_tempBuf[m_bufIndex];
    // 下标推进
    m_bufIndex = (m_bufIndex + 1) % TEMPERATURE_BUF_LEN;

    if (count_display % 19 == 0)
    {
        // m_manager->run_log(this, "ADC ---> %llfmv", ADC_11DB_VREF / SOLDER_ADC_RESOLUTION * adcVal);
        // m_manager->run_log(this, "thermocoupleVol = %llf", thermocoupleVol);
        // m_manager->run_log(this, "TempAvg = %llf", m_tempBufSum / TEMPERATURE_BUF_LEN);
        // m_manager->run_log(this, "m_tempBufSum = %llf", m_tempBufSum);
    }

    // return m_tempBufSum / TEMPERATURE_BUF_LEN;
    return nowTemp;
}

bool Solder::process()
{
    int pwm;
    m_curTemp = getCurTemperature(true);

    count_display = (count_display + 1) % 10000;

    switch (m_workState)
    {
    case SOLDER_STATE_DEEP_SLEEP: // 深度休眠状态
    {
        // 检测状态
        if (this->nextState() == SOLDER_STATE_NORMAL)
        {
            // 唤醒
            m_workState = SOLDER_STATE_NORMAL;
            pwm = 100;
            this->setHeatingMode(HEATING_MODE_FULL); // 设置全速加热模式

            m_manager->addEvent(CTRL_EVENT_SOLDER_WAKE);
            m_pidContorller->reset();
        }
    }
    break;
    case SOLDER_STATE_SLEEP: // 休眠状态
    {
        // 休眠状态下的温度维持
        if (m_curTemp - SLEEP_TEMP > SOLDER_START_PID_D_VALUE_HIGH)
        {
            // 过冲
            pwm = 0;
            m_pidContorller->get_output(SLEEP_TEMP, m_curTemp);
            this->setHeatingMode(HEATING_MODE_FULL); // 设置加热模式
        }
        else if (SLEEP_TEMP - m_curTemp > SOLDER_START_PID_D_VALUE_LOW)
        {
            // 当前温度远低于设定的温度
            pwm = 100;
            m_pidContorller->get_output(SLEEP_TEMP, m_curTemp);
            this->setHeatingMode(HEATING_MODE_FULL); // 设置加热模式
        }
        else
        {
            // 控温区间
            pwm = m_pidContorller->get_output(SLEEP_TEMP, m_curTemp);
            this->setHeatingMode(HEATING_MODE_PWM); // 设置加热模式
        }

        // 检测状态
        if (this->nextState() == SOLDER_STATE_NORMAL)
        {
            // 唤醒
            m_workState = SOLDER_STATE_NORMAL;
            pwm = 100;
            this->setHeatingMode(HEATING_MODE_FULL); // 设置加热模式

            m_manager->addEvent(CTRL_EVENT_SOLDER_WAKE);
            m_pidContorller->reset();
        }
        else if (this->nextState() == SOLDER_STATE_DEEP_SLEEP)
        {
            // 唤醒
            m_workState = SOLDER_STATE_DEEP_SLEEP;
            pwm = 0;
            this->setHeatingMode(HEATING_MODE_FULL); // 设置加热模式

            m_manager->addEvent(CTRL_EVENT_SOLDER_DEEP_SLEEP);
        }
    }
    break;
    case SOLDER_STATE_NORMAL: // 正常工作状态
    {
        if (m_curTemp - m_targetTemp > SOLDER_START_PID_D_VALUE_HIGH)
        {
            // 过冲
            pwm = 0;
            m_pidContorller->get_output(m_targetTemp, m_curTemp);
            m_pidContorller->reset();
            this->setHeatingMode(HEATING_MODE_FULL); // 设置加热模式
        }
        else if (m_targetTemp - m_curTemp > SOLDER_START_PID_D_VALUE_LOW)
        {
            // 当前温度远低于设定的温度
            pwm = 100;
            m_pidContorller->get_output(m_targetTemp, m_curTemp);
            this->setHeatingMode(HEATING_MODE_FULL); // 设置加热模式
        }
        else
        {
            // 控温区间
            pwm = m_pidContorller->get_output(m_targetTemp, m_curTemp);
            this->setHeatingMode(HEATING_MODE_PWM); // 设置加热模式
        }

        if (this->nextState() == SOLDER_STATE_SLEEP)
        {
            // 休眠状态
            m_workState = SOLDER_STATE_SLEEP;
            pwm = 0;
            this->setHeatingMode(HEATING_MODE_FULL); // 设置加热模式

            m_manager->addEvent(CTRL_EVENT_SOLDER_SLEEP);
            m_pidContorller->reset();
        }
    }
    break;
    default:
        break;
    }

    if (pwm < 0)
    {
        pwm = 0;
    }
    this->setPowerDuty(pwm);

    if (m_workState >= SOLDER_STATE_DEEP_SLEEP && count_display % 19 == 0)
    {
        m_manager->run_log(this, "Pwm = %d", pwm);
    }

    return true;
}

unsigned long Solder::getInterval(void)
{
    return xTaskGetTickCount() / portTICK_PERIOD_MS - s_wakeMillis;
}

SOLDER_STATE Solder::nextState()
{
    if (count_display % 10 == 0)
    {
        m_manager->run_log(this, "\nisWake() interval = %lu\n",
                           xTaskGetTickCount() / portTICK_PERIOD_MS - s_wakeMillis);
    }

    // 求与上回唤醒的时间间隔
    unsigned long interval = xTaskGetTickCount() / portTICK_PERIOD_MS - s_wakeMillis;
    if (interval > DEEP_SLEEP_MILLIS)
    {
        return SOLDER_STATE_DEEP_SLEEP;
    }
    else if (interval > SLEEP_MILLIS)
    {
        return SOLDER_STATE_SLEEP;
    }
    else
    {
        return SOLDER_STATE_NORMAL;
    }

    return SOLDER_STATE_NORMAL;
}

void Solder::sleepCheck(void)
{
    // 暂停加热 准备检测按键，本函数放tmpCloseOutPut()里调用
    // this->tmpCloseOutPut();
    if (digitalRead(m_shakePin) == 0)
    {
        // 检测到振动 更新值
        s_wakeMillis = xTaskGetTickCount() / portTICK_PERIOD_MS;
    }
    // 继续加热
    // this->setPowerDuty(this->m_powerDuty);
}

bool Solder::message_handle(const char *from, const char *to,
                            CTRL_MESSAGE_TYPE type, void *message,
                            void *ext_info)
{
    return true;
}

bool Solder::end()
{
    if (NULL != m_pidContorller)
    {
        delete m_pidContorller;
    }
    return true;
}