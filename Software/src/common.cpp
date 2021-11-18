#include "common.h"
// #include "rgb_led.h"
#include "config.h"
#include "network.h"
#include "knobs.h"
#include "buzzer.h"
#include <Preferences.h>

#include <TFT_eSPI.h>
/*
TFT pins should be set in path/to/Arduino/libraries/TFT_eSPI/User_Setups/Setup24_ST7789.h
*/
TFT_eSPI *tft = new TFT_eSPI(240, 240);

// Pixel rgb;
Config g_cfg;                                     // 全局配置文件
Network g_network;                                // 网络连接
Preferences prefs;                                // 声明Preferences对象
Display screen;                                   // 屏幕对象
Buzzer buzzer(BEEP_PIN, 10);                      // 蜂鸣器
Knobs knobs(EC11_A_PIN, EC11_B_PIN, EC11_SW_PIN); // EC11编码器

boolean doDelayMillisTime(unsigned long interval, unsigned long *previousMillis, boolean state)
{
    unsigned long currentMillis = millis();
    if (currentMillis - *previousMillis >= interval)
    {
        *previousMillis = currentMillis;
        state = !state;
    }
    return state;
}