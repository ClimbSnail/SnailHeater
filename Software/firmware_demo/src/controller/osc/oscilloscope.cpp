#include "oscilloscope.h"

#include "sys/snail_manager.h"

#define OSC_AMPLIFY_MULTIPLE (3.0 / 11) // 放大倍数
#define OSC_AMPLIFY_MULTIPLE_1 (3.0 / 11)
#define OSC_ADC_RESOLUTION 4096.0 // 分辨率

Oscilloscope::Oscilloscope(const char *name, SnailManager *m_manager,
                           uint8_t channelPin_0, uint8_t channelPin_1) : ControllerBase(name, CTRL_TYPE_OSCILLOSCOPE, m_manager)
{
    // 引脚
    m_channelPin_0 = channelPin_0;
    m_channelPin_1 = channelPin_1;
    m_channel0_val = 0;
    m_channel1_val = 0;

    channel0Coefficient = ADC_11DB_VREF / OSC_ADC_RESOLUTION / OSC_AMPLIFY_MULTIPLE;
    channel1Coefficient = ADC_11DB_VREF / OSC_ADC_RESOLUTION / OSC_AMPLIFY_MULTIPLE_1;
}

Oscilloscope::~Oscilloscope()
{
    this->end();
}

bool Oscilloscope::start()
{
    pinMode(m_channelPin_0, INPUT);
    pinMode(m_channelPin_1, INPUT);

    adcAttachPin(m_channelPin_0);
    analogSetPinAttenuation(m_channelPin_0, ADC_11db);
    registerMyADC(m_channelPin_0); // 注册ADC检测服务
    adcAttachPin(m_channelPin_1);
    analogSetPinAttenuation(m_channelPin_1, ADC_11db);
    registerMyADC(m_channelPin_1); // 注册ADC检测服务
    return true;
}

bool Oscilloscope::process()
{
    uint16_t channelPinAdc_0 = getMyADC(m_channelPin_0, 6, false);
    // uint16_t channelPinAdc_0 = getMyADC(m_channelPin_0);
    // ADC_11DB_VREF / OSC_ADC_RESOLUTION * channelPinAdc_0 / OSC_AMPLIFY_MULTIPLE
    m_channel0_val = channel0Coefficient * channelPinAdc_0;
    m_manager->run_log(this, "m_channel0_adc = %u, m_channel0_val = %u", channelPinAdc_0, m_channel0_val);

    uint16_t channelPinAdc_1 = getMyADC(m_channelPin_1, 6, false);
    // uint16_t channelPinAdc_1 = getMyADC(m_channelPin_1);
    // ADC_11DB_VREF / OSC_ADC_RESOLUTION * channelPinAdc_1 / OSC_AMPLIFY_MULTIPLE_1
    m_channel1_val = channel1Coefficient * channelPinAdc_1;
    m_manager->run_log(this, "m_channel1_adc = %u, m_channel1_val = %u", channelPinAdc_1, m_channel1_val);

    return true;
}

uint16_t Oscilloscope::getChanne0()
{
    return m_channel0_val;
}

uint16_t Oscilloscope::getChanne1()
{
    return m_channel1_val;
}

bool Oscilloscope::end()
{
    return true;
}

bool Oscilloscope::message_handle(const char *from, const char *to,
                                  CTRL_MESSAGE_TYPE type, void *message,
                                  void *ext_info)
{
    return true;
}