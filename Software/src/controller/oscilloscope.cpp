#include "oscilloscope.h"

Oscilloscope::Oscilloscope(const char *name, uint8_t channelPin_0, uint8_t channelPin_1)
{
    strncpy(m_name, name, 16);
    // 引脚
    m_channelPin_0 = channelPin_0;
    m_channelPin_1 = channelPin_1;
    m_channel0_val = 0;
    m_channel1_val = 0;
}

Oscilloscope::~Oscilloscope()
{
}

bool Oscilloscope::start()
{
    pinMode(m_channelPin_0, INPUT);
    pinMode(m_channelPin_1, INPUT);
    analogReadResolution(12);
    analogSetWidth(12);
    analogSetAttenuation(ADC_11db);
    adcAttachPin(m_channelPin_0);
    adcAttachPin(m_channelPin_1);
    return true;
}

bool Oscilloscope::process()
{
    Serial.printf("\n");
    Serial.printf(m_name);
    Serial.printf("\tchannel_0 ---> ");
    m_channel0_val = analogRead(m_channelPin_0);
    Serial.print(m_channel0_val);
    // Serial.println(analogRead(m_channelPin_0) / 4096.0 * 3.3 * 6);
    Serial.printf("\tchannel_1 ---> ");
    m_channel1_val = analogRead(m_channelPin_1);
    Serial.print(m_channel1_val);
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