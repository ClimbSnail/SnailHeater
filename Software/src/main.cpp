#include <Arduino.h>
#include "lv_port_indev.h"
#include "lv_port_fatfs.h"
#include "common.h"
#include "config.h"

TimerHandle_t xTimer_beep = NULL;

KeyInfo ret_info;

uint32_t updateTime = 0; // time for next update
int oldi = 0;
int i = 0;

void gotTouch()
{
    Serial.println("ESP32 Touch Interrupt");
    buzzer.set_beep_time(1000);
}

void sw_1()
{
    Serial.println("sw_1");
}

void sw_2()
{
    Serial.println("sw_2");
}

void setup()
{
    Serial.begin(115200);
    Serial.println("SuperHeat Version: " SuperHeat_VERSION "\n");

    config_read(&g_cfg);

    touchSetCycles(0x3000, 0x1000); // measure值表示量程范围 0x1000为100 0x3000为300
    // 其中40为阈值，当通道T0上的值<40时，会触发中断
    // 在日常使用中，可以先使用 touchRead(TOUCH_PIN) 来确定 touchAttachInterrupt() 的阈值
    touchAttachInterrupt(TOUCH_PIN, gotTouch, 40);

    screen.init();

    /*** Init IMU as input device ***/
    lv_port_indev_init();

    // 蜂鸣器
    buzzer.set_beep_time(1000);

    xTimer_beep = xTimerCreate("BEEP contorller",
                               buzzer.m_cycle_time / portTICK_PERIOD_MS,
                               pdTRUE, (void *)0, buzzer.timer_handler);
    xTimerStart(xTimer_beep, 0); //开启定时器

    ledcSetup(PWM_1_CHANNEL, 5000, 8);
    ledcAttachPin(PWM_1_PIN, PWM_1_CHANNEL);
    ledcWrite(PWM_1_CHANNEL, (int)(0.4 * 255));

    ledcSetup(PWM_2_CHANNEL, 5000, 8);
    ledcAttachPin(PWM_2_PIN, PWM_2_CHANNEL);
    ledcWrite(PWM_2_CHANNEL, (int)(0.4 * 255));

    ledcSetup(FAN_1_CHANNEL, 5000, 8);
    ledcAttachPin(5, FAN_1_CHANNEL);
    ledcWrite(FAN_1_CHANNEL, (int)(0.4 * 255));

    ledcSetup(FAN_2_CHANNEL, 5000, 8);
    ledcAttachPin(2, FAN_2_CHANNEL);
    ledcWrite(FAN_2_CHANNEL, (int)(0.4 * 255));

    ledcSetup(HIGH_CHANNEL, 5000, 8);
    ledcAttachPin(15, HIGH_CHANNEL);
    ledcWrite(HIGH_CHANNEL, (int)(0.4 * 255));

    // ADC引脚
    pinMode(ADC0_PIN, INPUT);
    pinMode(ADC1_PIN, INPUT);

    // 开关
    // 按下与松开
    attachInterrupt(digitalPinToInterrupt(SW_1_PIN), sw_1, CHANGE);
    attachInterrupt(digitalPinToInterrupt(SW_2_PIN), sw_2, CHANGE);

    dacWrite(DAC_1, 100);       // 输出DAC
    dacWrite(DAC_2, 255 - 100); // 输出DAC
    delay(5);

    // tft->fillRect(0, 0, 50, 50, TFT_RED);

    tft->setTextColor(TFT_WHITE); //设置字体颜色
    tft->setTextSize(1);          //设置字体大小(默认1号)
    tft->setCursor(20, 10, 2);    //设置行号
    // tft->startWrite();
    // tft->print("USB");         //显示USB
    tft->drawString("USB", 1, 1); //显示USB
    // tft->endWrite();
}

void loop()
{
    // // ret_info = knobs.get_data();
    // Serial.print(ret_info.pulse_count);
    // Serial.printf(" ---> ");
    // Serial.print(ret_info.switch_status);
    // Serial.printf(" ---> ");
    // Serial.println(ret_info.switch_time);

    // // Serial.printf(" ---> ");
    // // Serial.println(touchRead(TOUCH_PIN));

    // Serial.printf("ADC0 ---> ");
    // Serial.println(analogRead(ADC0_PIN));
    // Serial.printf("ADC1 ---> ");
    // Serial.println(analogRead(ADC1_PIN));

    // ledcWrite(PWM_1_CHANNEL, (int)(0.9 * 255));
    // ledcWrite(PWM_2_CHANNEL, (int)(0.9 * 255));
    // delay(3000);
    // ledcWrite(PWM_1_CHANNEL, (int)(0.1 * 255));
    // ledcWrite(PWM_2_CHANNEL, (int)(0.1 * 255));
    // delay(3000);

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