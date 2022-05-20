#include "signal_generator.h"

SignalGenerator::SignalGenerator(const char *name, SuperManager *m_manager,
                                 uint8_t channelPin) : ControllerBase(name, m_manager)
{
    // 引脚
    m_channelPin = channelPin;
}

SignalGenerator::~SignalGenerator()
{
    this->end();
}

bool SignalGenerator::start()
{
    pinMode(m_channelPin, OUTPUT);
    return true;
}

bool SignalGenerator::process()
{
    Serial.printf("\n");
    Serial.printf(m_name);
    Serial.printf("\tchannel_0 ---> ");
    dacWrite(m_channelPin, 100); // 输出DAC
    return true;
}

bool SignalGenerator::end()
{
    return true;
}

bool SignalGenerator::message_handle(const char *from, const char *to,
                                     SUPER_MESSAGE_TYPE type, void *message,
                                     void *ext_info)
{
    return true;
}