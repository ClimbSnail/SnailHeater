#include "signal_generator.h"

SignalGenerator::SignalGenerator(const char *name, uint8_t channelPin)
{
    strncpy(m_name, name, 16);
    // 引脚
    m_channelPin = channelPin;
}

SignalGenerator::~SignalGenerator()
{
}

bool SignalGenerator::start()
{
    pinMode(m_channelPin, OUTPUT);
    return true;
}

bool SignalGenerator::process()
{
    Serial.printf(m_name);
    Serial.printf("\tchannel_0 ---> ");
    dacWrite(m_channelPin, 100); // 输出DAC
    return true;
}

bool SignalGenerator::end()
{
    return true;
}