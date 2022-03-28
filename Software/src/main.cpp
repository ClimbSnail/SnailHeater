#include <Arduino.h>
#include "driver/lv_port_indev.h"
#include "driver/lv_port_fatfs.h"
#include "common.h"
#include "config.h"
#include "controller/hot_air.h"
#include "controller/oscilloscope.h"
#include "controller/signal_generator.h"
#include "controller/power.h"

// 创建热风枪
HotAir hotAir("HotAit", PWM_1_PIN, PWM_1_CHANNEL, FAN_1_PIN, FAN_1_CHANNEL, ADC0_PIN, SW_0_PIN);
// HotAir hotAir("HotAit", PWM_2_PIN, PWM_2_CHANNEL, FAN_2_PIN, FAN_2_CHANNEL, ADC1_PIN, SW_1_PIN);
// 创建示波器
Oscilloscope oscilloscope("Oscilloscope", ADC2_PIN, ADC3_PIN);
// 创建函数发生器
SignalGenerator signalGenerator("SignalGenerator", DAC_1);
// 可调电源
Power power("Power", POWER_PIN, POWER_PWM_PIN, POWER_PWM_CHANNEL);

TimerHandle_t xTimer_beep = NULL;

TimerHandle_t xTimer_hotair = NULL;

KeyInfo ret_info;

uint32_t updateTime = 0; // time for next update

void gotTouch()
{
    Serial.println("ESP32 Touch Interrupt");
    // buzzer.set_beep_time(1000);
}

void hot_air(TimerHandle_t xTimer)
{
    hotAir.process();
}

void setup()
{
    Serial.begin(115200);
    Serial.println("\nSuperHeat Version: " SuperHeat_VERSION "\n");

    config_read(&g_cfg);

    hotAir.start();
    oscilloscope.start();
    signalGenerator.start();
    power.start();

    touchSetCycles(0x3000, 0x1000); // measure值表示量程范围 0x1000为100 0x3000为300
    // 其中40为阈值，当通道T0上的值<40时，会触发中断
    // 在日常使用中，可以先使用 touchRead(TOUCH_PIN) 来确定 touchAttachInterrupt() 的阈值
    touchAttachInterrupt(TOUCH_PIN, gotTouch, 40);

    screen.init();

    /*** Init IMU as input device ***/
    lv_port_indev_init();

    tft->fillRect(0, 0, 240, 140, TFT_GOLD);
    tft->fillRect(0, 140, 240, 140, TFT_DARKGREY);

    // tft->setTextColor(TFT_WHITE); //设置字体颜色
    // tft->setTextSize(1);          //设置字体大小(默认1号)
    // tft->setCursor(20, 10, 2);    //设置行号
    // tft->startWrite();
    // // tft->print("USB");         //显示USB
    // tft->drawString("USB", 30, 30); //显示USB
    // tft->endWrite();

    // xTimer_beep = xTimerCreate("BEEP contorller",
    //                            buzzer.m_cycle_time / portTICK_PERIOD_MS,
    //                            pdTRUE, (void *)0, buzzer.timer_handler);
    // xTimerStart(xTimer_beep, 0); //开启定时器
    // // 蜂鸣器
    // buzzer.set_beep_time(1000);
    hotAir.setAirDuty(40);
    hotAir.setTargetTemp(80);
    xTimer_hotair = xTimerCreate("HotAit contorller",
                                 200 / portTICK_PERIOD_MS,
                                 pdTRUE, (void *)0, hot_air);
    xTimerStart(xTimer_hotair, 0); //开启定时器
}

void loop()
{
    // while(1);
    ret_info = knobs.get_data();
    Serial.printf("【Knobs】 count---> ");
    Serial.print(ret_info.pulse_count);
    Serial.printf("\tstatus---> ");
    Serial.print(ret_info.switch_status);
    Serial.printf("\ttime---> ");
    Serial.print(ret_info.switch_time);
    Serial.printf("\ttouchRead---> ");
    Serial.println(touchRead(TOUCH_PIN));

    delay(2000);
    // buzzer.set_beep_time(1000);

    // int oldi = 0;
    // int i = 0;
    // updateTime = millis(); // Next update time
    // if (updateTime <= millis())
    // {
    //     updateTime = millis() + 30; //每30ms更新一次
    //     if (i < 350)
    //         i += 5;
    //     else
    //         oldi = i = -50;
    //     while (i != oldi)
    //     {
    //         tft->drawFastHLine(0, oldi, 50, TFT_BLACK);
    //         oldi++;
    //         tft->drawFastHLine(0, oldi + 50, 50, TFT_RED);
    //     }
    // }
}