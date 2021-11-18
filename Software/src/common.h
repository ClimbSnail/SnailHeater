#ifndef COMMON_H
#define COMMON_H

#define SuperHeat_VERSION "0.6.0"

#include <Arduino.h>
// #include "rgb_led.h"
#include "config.h"
#include "display.h"
#include "network.h"
#include "knobs.h"
#include "buzzer.h"
#include <Preferences.h>

// define pins
#define ADC0_PIN 36
#define ADC1_PIN 39
#define ADC2_PIN 34
#define ADC3_PIN 35
#define BL_PWM_PIN 12
#define PWM_1_PIN 33
#define PWM_2_PIN 32
#define PWM_3_PIN 15
#define FAN_1_PIN 2
#define FAN_2_PIN 13
#define EC11_A_PIN 26
#define EC11_B_PIN 27
#define EC11_SW_PIN 14
#define BEEP_PIN 19
#define TOUCH_PIN 4
#define SW_1_PIN 22
#define SW_2_PIN 21
#define DAC_1 25
// #define TX_PIN 0
// #define RX_PIN 0

// pwm通道
#define PWM_1_CHANNEL 8
#define PWM_2_CHANNEL 9
#define FAN_1_CHANNEL 10
#define FAN_2_CHANNEL 11
#define HIGH_CHANNEL 0

// #define TFT_RES_PIN 16
// #define TFT_DC_PIN 17
// #define TFT_SCK_PIN 18
// #define TFT_MOSI_PIN 23

#define USER_SET_PARAM_NUM 10 // 用户的设置项保存参数的数量（UserData）

#include <TFT_eSPI.h>
/*
TFT pins should be set in path/to/Arduino/libraries/TFT_eSPI/User_Setups/Setup24_ST7789.h
*/
extern TFT_eSPI *tft;

extern Config g_cfg; // 全局配置文件
// extern Pixel rgb;
extern Network g_network; // 网络连接
extern Preferences prefs; // 声明Preferences对象
extern Display screen;    // 屏幕对象
extern Knobs knobs;       // EC11编码器
extern Buzzer buzzer;     // 蜂鸣器

boolean doDelayMillisTime(unsigned long interval,
                          unsigned long *previousMillis,
                          boolean state);

#endif