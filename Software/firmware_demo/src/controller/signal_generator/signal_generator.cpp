#include "signal_generator.h"

#include "sys/snail_manager.h"

SignalGenerator::SignalGenerator(const char *name, SnailManager *m_manager,
                                 uint8_t channelPin) : ControllerBase(name, CTRL_TYPE_SIGNALGENERATOR, m_manager)
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
    m_manager->run_log(this, "\n");
    m_manager->run_log(this, m_name);
    m_manager->run_log(this, "\tchannel_0 ---> ");
    dacWrite(m_channelPin, 128); // 输出DAC
    return true;
}

bool SignalGenerator::end()
{
    return true;
}

bool SignalGenerator::message_handle(const char *from, const char *to,
                                     CTRL_MESSAGE_TYPE type, void *message,
                                     void *ext_info)
{
    return true;
}