#include "max6675.h"

// #include <SPI.h>

MAX6675::MAX6675(uint8_t sck, uint8_t miso, uint8_t cs)
{
    m_sck = sck;
    m_miso = miso;
    m_cs = cs;

    pinMode(m_sck, OUTPUT);
    pinMode(m_miso, INPUT_PULLUP);
    pinMode(m_cs, OUTPUT);

    // SPI.pins(sck, miso, mosi, cs);
    // SPI.begin();
    digitalWrite(m_cs, HIGH);
}

uint16_t MAX6675::read_temperature(void)
{
    digitalWrite(m_cs, LOW);
    // SPI.transfer(READ_DATA);
    // SPI.transfer(0);
    digitalWrite(m_cs, HIGH);
    return 0;
}

uint16_t MAX6675::read_temperature_mock(void)
{
    char i = 0;
    uint16_t dat = 0;
    uint16_t temperature = 0;

    digitalWrite(m_cs, LOW);
    digitalWrite(m_sck, LOW);
    for (i = 0; i < 16; i++)
    {
        digitalWrite(m_sck, HIGH);
        dat = dat << 1;
        if (digitalRead(m_miso))
            dat = dat | 0x01;
        digitalWrite(m_sck, LOW);
    }
    digitalWrite(m_cs, HIGH);

    if ((dat & MAX6675_CONNECT) != 0)
    {
        // 没检测到数据
        return 4095;
    }

    //读出来的数据的D3~D14是温度值
    temperature = ((dat & 0x7FFF) >> 3) >> 2;
    return temperature;
}

MAX6675::~MAX6675()
{
}