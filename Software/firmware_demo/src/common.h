#ifndef COMMON_H
#define COMMON_H

#define SNAILHEATER_VERSION "1.0.0"

#define ESP32S2_12K

/* Number to string macro */
#define _VERSION_NUM_TO_STR_(n) #n
#define VERSION_NUM_TO_STR(n) _VERSION_NUM_TO_STR_(n)

/* LVGL Version */
#include "lvgl.h"
#define VERSION_LVGL "v" VERSION_NUM_TO_STR(LVGL_VERSION_MAJOR) "." VERSION_NUM_TO_STR(LVGL_VERSION_MINOR) "." VERSION_NUM_TO_STR(LVGL_VERSION_PATCH) " " LVGL_VERSION_INFO

#include <Arduino.h>
#include "network.h"
// #include <Preferences.h>

#include "driver/display.h"
#include "driver/flash_fs.h"
#include "driver/knobs.h"
#include "driver/buzzer.h"
#include "driver/rgb_led.h"

// #define ADC_11DB_VREF 2680 // ADC 11db的基准电压
#define ADC_11DB_VREF 2516 // ADC 11db的基准电压
// #define ADC_11DB_VREF 1350 // ADC 11db的基准电压
#define EMPTY_PIN 255 // 定义空引脚

// define pins
#if defined(ESP32S2_12K)
#define ADC0_PIN 1
#define ADC1_PIN 2
#define ADC2_PIN 3
#define ADC3_PIN 4
#define ADC4_PIN 14

#define POWER_EN_PIN 45
#define POWER_DAC_PIN 17
#define POWER_CUR_ADC_PIN 6
#define POWER_OV_ADC_PIN 7

#define BL_PWM_PIN 38
#define RGB_LED_PIN 20

#define PWM_0_PIN 21
#define PWM_1_PIN 26
#define PWM_2_PIN 33
#define PWM_3_PIN 34
#define FAN_0_PIN 5
#define FAN_1_PIN 15
#define FAN_2_PIN 16
#define EC11_A_PIN 39
#define EC11_B_PIN 40
#define EC11_SW_PIN 41
// #define EC11_SW_PIN 46

#define BEEP_PIN 42
#define TOUCH_PIN 8
#define SW_0_PIN 35
#define SW_1_PIN 36
#define SW_2_PIN 37
#define DAC_2 18
// #define TX_PIN 43
// #define RX_PIN 44

// pwm通道
#define PWM_0_CHANNEL 0
#define PWM_1_CHANNEL 1
#define FAN_0_CHANNEL 2
#define FAN_1_CHANNEL 3
#define PWM_2_CHANNEL 4
#define LCD_BL_PWM_CHANNEL 5
#define PWM_DAC_CHANNEL 6
#define HIGH_CHANNEL 7

// pwm通道组的频率 两个通道将会使用同一组配置
#define PWM_GROUP_0_FREQ 4
#define PWM_GROUP_0_BIT_WIDTH 14
#define PWM_GROUP_0_DUTY 163.83

#define PWM_GROUP_1_FREQ 128
#define PWM_GROUP_1_BIT_WIDTH 10
#define PWM_GROUP_1_DUTY 10.23

#define PWM_GROUP_2_FREQ 128     // 2441
#define PWM_GROUP_2_BIT_WIDTH 12 // 10
#define PWM_GROUP_2_DUTY 40.95   // 10.23

// #define PWM_GROUP_3_FREQ 1024
// #define PWM_GROUP_3_BIT_WIDTH 8
// #define PWM_GROUP_3_DUTY 2.55
#define PWM_GROUP_3_FREQ 2441
#define PWM_GROUP_3_BIT_WIDTH 12
#define PWM_GROUP_3_DUTY 40.95

// #define USER_PWM_CTRL_SRCEEN // 定时器不够 不使用

// #define TFT_RES_PIN 9
// #define TFT_DC_PIN 13
// #define TFT_SCK_PIN 12
// #define TFT_MOSI_PIN 11
// #define TFT_CS_PIN 10
#else
#define ADC0_PIN 36
#define ADC1_PIN 39
#define ADC2_PIN 39 // 与ADC1_PIN同一个脚
#define ADC3_PIN 34
#define ADC4_PIN 35

#define POWER_EN_PIN 5
#define POWER_DAC_PIN 25
#define POWER_CUR_ADC_PIN -1
#define POWER_OV_ADC_PIN -1

#define BL_PWM_PIN 12
#define RGB_LED_PIN -1

#define PWM_0_PIN 33
#define PWM_1_PIN 32
#define PWM_2_PIN 15
#define FAN_0_PIN 2
#define FAN_1_PIN 13
#define EC11_A_PIN 26
#define EC11_B_PIN 27
#define EC11_SW_PIN 14

#define BEEP_PIN 19
#define TOUCH_PIN 4
#define SW_0_PIN 22
#define SW_1_PIN 21
#define SW_2_PIN 21 // 与 SW_1_PIN 同一个脚
#define DAC_2 25    // 与 POWER_DAC_PIN 同一个脚
// #define TX_PIN 0
// #define RX_PIN 0

// pwm通道
#define PWM_0_CHANNEL 8
#define PWM_1_CHANNEL 9
#define POWER_PWM_CHANNEL 7
#define FAN_0_CHANNEL 10
#define FAN_1_CHANNEL 11
#define BEEP_CHANNEL 12
#define HIGH_CHANNEL 0
#define LCD_BL_PWM_CHANNEL 1

// #define TFT_RES_PIN 16
// #define TFT_DC_PIN 17
// #define TFT_SCK_PIN 18
// #define TFT_MOSI_PIN 23
// #define TFT_CS_PIN -1
#endif

#include <TFT_eSPI.h>
#include "esp_adc_cal.h"
/*
TFT pins should be set in path/to/Arduino/libraries/TFT_eSPI/User_Setups/Setup24_ST7789.h
*/
extern TFT_eSPI *tft;

// extern Config g_cfg; // 全局配置文件
extern Pixel rgb;
extern Network g_network;  // 网络连接
extern FlashFS g_flashCfg; // flash中的文件系统（替代原先的Preferences）
// extern Preferences prefs;  // 声明Preferences对象
extern Display screen; // 屏幕对象
extern Knobs knobs;    // EC11编码器
extern Buzzer buzzer;  // 蜂鸣器

boolean doDelayMillisTime(unsigned long interval,
                          unsigned long *previousMillis,
                          boolean state);

uint16_t getMyADC(uint8_t pin, int num, bool doDelay = false); // 获取ADC的值
bool registerMyADC(uint8_t pin);                               // 注册ADC检测引脚
void updateADC(bool doDelay = true);                           // ADC更新服务
uint16_t getMyADC(uint8_t pin);                                // 获取ADC的值
extern bool adc_calibration_init(void);
extern void print_char_val_type(esp_adc_cal_value_t val_type);

#endif