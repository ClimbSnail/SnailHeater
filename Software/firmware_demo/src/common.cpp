#include "common.h"
// #include "rgb_led.h"
// #include "config.h"
#include "network.h"
#include "driver/knobs.h"
#include "driver/buzzer.h"
// #include <Preferences.h>

#include <TFT_eSPI.h>
/*
TFT pins should be set in path/to/Arduino/libraries/TFT_eSPI/User_Setups/Setup24_ST7789.h
*/
TFT_eSPI *tft = new TFT_eSPI(240, 280);

Pixel rgb;
// Config g_cfg;                                     // 全局配置文件
Network g_network;  // 网络连接
FlashFS g_flashCfg; // flash中的文件系统（替代原先的Preferences）
// Preferences prefs;                                // 声明Preferences对象
Display screen;                                   // 屏幕对象
Buzzer buzzer(BEEP_PIN, 10);                      // 蜂鸣器
Knobs knobs(EC11_A_PIN, EC11_B_PIN, EC11_SW_PIN); // EC11编码器

boolean doDelayMillisTime(unsigned long interval, unsigned long *previousMillis, boolean state)
{
    // unsigned long currentMillis = millis();
    unsigned long currentMillis = xTaskGetTickCount();
    if (currentMillis - *previousMillis >= interval)
    {
        *previousMillis = currentMillis;
        state = !state;
    }
    return state;
}

// 映射ADC引脚与储存区的位置
uint16_t adcPinValue[64] = {0};

#define ADCReadNUm 10
// 存放ADC的信息
struct ADCInfo
{
    uint8_t pinNum;                       // ADC的引脚
    uint16_t adcValue;                    // ADC的采集信息
    uint16_t adcPinValueTemp[ADCReadNUm]; // ADC临时存放的区域
};
ADCInfo registAdcInfo[16];
int adcRegisterCnt = 0;
// 储存区的

bool registerMyADC(uint8_t pin)
{
    // 注册ADC
    adcPinValue[pin] = adcRegisterCnt;
    registAdcInfo[adcRegisterCnt].pinNum = pin;
    registAdcInfo[adcRegisterCnt].adcValue = 0;

    ++adcRegisterCnt;
    return true;
}

void updateADC(bool doDelay)
{
    // 采集 ADCReadNUm 次
    for (int i = 0; i < ADCReadNUm; i++)
    {
        for (int pos = 0; pos < adcRegisterCnt; pos++)
        {
            registAdcInfo[pos].adcPinValueTemp[i] = analogRead(registAdcInfo[pos].pinNum);
        }
        if (doDelay)
        {
            // vTaskDelay(1 / portTICK_PERIOD_MS)
        }
    }

    // 滤波
    for (int num = 0; num < adcRegisterCnt; num++)
    {

        // 排序
        uint16_t swap_tmp = 0;
        uint16_t *data = registAdcInfo[num].adcPinValueTemp;
        for (int i = 0; i < ADCReadNUm - 1; ++i)
        {
            for (int j = i + 1; j < ADCReadNUm; ++j)
            {
                if (data[i] < data[j])
                {
                    swap_tmp = data[i];
                    data[i] = data[j];
                    data[j] = swap_tmp;
                }
            }
        }

        // 删除偏大偏小值
        uint32_t sum = 0;
        for (int ind = 2; ind < ADCReadNUm - 2; ++ind)
        {
            sum += data[ind];
        }
        registAdcInfo[num].adcValue = sum / (ADCReadNUm - 2 * 2);
    }
}

uint16_t getMyADC(uint8_t pin)
{
    uint16_t result;

    result = registAdcInfo[adcPinValue[pin]].adcValue;

    return result;
}

#define DATA_BUF_LEN 20
uint16_t adcData[DATA_BUF_LEN];

// uint16_t getMyADC(uint8_t pin, int num, bool doDelay)
// {
//     // 采集
//     // unsigned long currentMillis = xTaskGetTickCount();
//     uint16_t adcData[DATA_BUF_LEN] = {0};
//     for (int pos = 0; pos < num * 2; pos += 2)
//     {
//         adcData[pos] = analogRead(pin);
//         adcData[pos + 1] = analogRead(pin);
//         if (doDelay)
//         {
//             vTaskDelay(1 / portTICK_PERIOD_MS);
//         }
//     }
//     // Serial0.print("getMyADC(uint8_t pin, int num, bool doDelay) ---> ");
//     // Serial0.println(xTaskGetTickCount() - currentMillis);

//     // 排序
//     uint16_t swap_tmp = 0;
//     for (int i = 0; i < num * 2 - 1; ++i)
//     {
//         for (int j = i + 1; j < num * 2; ++j)
//         {
//             if (adcData[i] < adcData[j])
//             {
//                 swap_tmp = adcData[i];
//                 adcData[i] = adcData[j];
//                 adcData[j] = swap_tmp;
//             }
//         }
//     }

//     uint32_t sum = 0;
//     for (int ind = 4; ind < num * 2 - 4; ++ind)
//     {
//         sum += adcData[ind];
//     }
//     return sum / (num * 2 - 4 * 2);
// }

uint16_t getMyADC(uint8_t pin, int num, bool doDelay)
{
    // 采集
    for (int pos = 0; pos < num; pos += 1)
    {
        adcData[pos] = analogRead(pin);
        if (doDelay)
        {
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
    }

    // 排序
    uint16_t swap_tmp = 0;
    for (int i = 0; i < num - 1; ++i)
    {
        for (int j = i + 1; j < num; ++j)
        {
            if (adcData[i] < adcData[j])
            {
                swap_tmp = adcData[i];
                adcData[i] = adcData[j];
                adcData[j] = swap_tmp;
            }
        }
    }

    uint32_t sum = 0;
    for (int ind = 2; ind < num - 2; ++ind)
    {
        sum += adcData[ind];
    }
    return sum / (num - 2 * 2);
}