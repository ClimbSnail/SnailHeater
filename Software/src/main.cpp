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

volatile int swInterruptCounter_1 = 0;
volatile int swInterruptCounter_2 = 0;

// 同步标志
portMUX_TYPE mux_1 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE mux_2 = portMUX_INITIALIZER_UNLOCKED;

void gotTouch()
{
    Serial.println("ESP32 Touch Interrupt");
    buzzer.set_beep_time(1000);
}

void sw_1()
{
    portENTER_CRITICAL_ISR(&mux_1);
    swInterruptCounter_1++;
    Serial.println("sw_1");
    portEXIT_CRITICAL_ISR(&mux_1);
}

void sw_2()
{
    portENTER_CRITICAL_ISR(&mux_2);
    swInterruptCounter_2++;
    Serial.println("sw_2");
    portEXIT_CRITICAL_ISR(&mux_2);
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

    ledcSetup(HIGH_CHANNEL, 5000, 8);
    ledcAttachPin(PWM_3_PIN, HIGH_CHANNEL);
    ledcWrite(HIGH_CHANNEL, (int)(0.8 * 255));

    ledcSetup(FAN_1_CHANNEL, 5000, 8);
    ledcAttachPin(FAN_1_PIN, FAN_1_CHANNEL);
    ledcWrite(FAN_1_CHANNEL, (int)(0.8 * 255));

    ledcSetup(FAN_2_CHANNEL, 5000, 8);
    ledcAttachPin(FAN_2_PIN, FAN_2_CHANNEL);
    ledcWrite(FAN_2_CHANNEL, (int)(0.01 * 255));

    // ADC引脚
    pinMode(ADC0_PIN, INPUT);
    pinMode(ADC1_PIN, INPUT);
    pinMode(ADC2_PIN, INPUT);
    pinMode(ADC3_PIN, INPUT);

    // 开关
    // 按下与松开
    pinMode(SW_1_PIN, INPUT_PULLUP);
    pinMode(SW_2_PIN, INPUT_PULLUP);
    // attachInterrupt(digitalPinToInterrupt(SW_1_PIN), sw_1, FALLING);
    // attachInterrupt(digitalPinToInterrupt(SW_2_PIN), sw_2, FALLING);

    dacWrite(DAC_1, 100); // 输出DAC
    delay(5);

    tft->fillRect(0, 0, 50, 50, TFT_RED);

    tft->setTextColor(TFT_WHITE); //设置字体颜色
    tft->setTextSize(1);          //设置字体大小(默认1号)
    tft->setCursor(20, 10, 2);    //设置行号
    tft->startWrite();
    // tft->print("USB");         //显示USB
    tft->drawString("USB", 30, 30); //显示USB
    tft->endWrite();
}

void loop()
{
    portENTER_CRITICAL(&mux_1);

    ret_info = knobs.get_data();
    Serial.printf("count---> ");
    Serial.print(ret_info.pulse_count);
    Serial.printf("\tstatus---> ");
    Serial.print(ret_info.switch_status);
    Serial.printf("\ttime---> ");
    Serial.println(ret_info.switch_time);

    Serial.printf("\ttouchRead---> ");
    Serial.print(touchRead(TOUCH_PIN));

    Serial.printf("\tADC0 ---> ");
    Serial.print(analogRead(ADC0_PIN));
    Serial.printf("\tADC1 ---> ");
    Serial.println(analogRead(ADC1_PIN));

    Serial.printf("\tADC2 ---> ");
    Serial.print(analogRead(ADC2_PIN)/4096*3300);
    Serial.printf("\tADC3 ---> ");
    Serial.println(analogRead(ADC3_PIN)/4096*3300);

    portEXIT_CRITICAL(&mux_1);

    delay(1000);
    digitalWrite(BEEP_PIN, HIGH);
    delay(1000);
    digitalWrite(BEEP_PIN, LOW);

    ledcWrite(PWM_1_CHANNEL, (int)(0.9 * 255));
    ledcWrite(PWM_2_CHANNEL, (int)(0.9 * 255));

    // while (1)
    // {
    //     for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle = dutyCycle + 5)
    //     {
    //         ledcWrite(FAN_2_CHANNEL, dutyCycle);
    //         delay(200);
    //     }
    // }

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