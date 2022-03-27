#ifndef __MAX6675_H
#define __MAX6675_H

#include <Arduino.h>

#define uint8_t unsigned char
#define MAX6675_CONNECT 0x04

class MAX6675
{
public:
    MAX6675(uint8_t sck, uint8_t miso, uint8_t cs);
    ~MAX6675();
    uint16_t read_temperature(void);
    uint16_t read_temperature_mock(void);

private:
    uint8_t m_sck;
    uint8_t m_mosi;
    uint8_t m_miso;
    uint8_t m_cs;
};

#endif