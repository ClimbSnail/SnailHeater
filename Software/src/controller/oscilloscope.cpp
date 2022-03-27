#include "oscilloscope.h"

Oscilloscope::Oscilloscope(const char *name, uint8_t channelPin_0, uint8_t channelPin_1)
{
    strncpy(m_name, name, 16);
    // 引脚
    m_channelPin_0 = channelPin_0;
    m_channelPin_1 = channelPin_1;
}

Oscilloscope::~Oscilloscope()
{
}

bool Oscilloscope::start()
{
    pinMode(m_channelPin_0, INPUT);
    pinMode(m_channelPin_1, INPUT);
    return true;
}

bool Oscilloscope::process()
{
    Serial.printf(m_name);
    Serial.printf("\tchannel_0 ---> ");
    Serial.print(analogRead(m_channelPin_0));
    // Serial.println(analogRead(m_channelPin_0) / 4096.0 * 3.3 * 6);
    Serial.printf("\tchannel_1 ---> ");
    Serial.print(analogRead(m_channelPin_1));
    return true;
}

bool Oscilloscope::end()
{
    return true;
}