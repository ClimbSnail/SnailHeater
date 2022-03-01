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
    // buzzer.set_beep_time(1000);
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
    Serial.println("\nSuperHeat Version: " SuperHeat_VERSION "\n");

    config_read(&g_cfg);

    touchSetCycles(0x3000, 0x1000); // measure值表示量程范围 0x1000为100 0x3000为300
    // 其中40为阈值，当通道T0上的值<40时，会触发中断
    // 在日常使用中，可以先使用 touchRead(TOUCH_PIN) 来确定 touchAttachInterrupt() 的阈值
    touchAttachInterrupt(TOUCH_PIN, gotTouch, 40);

    screen.init();

    /*** Init IMU as input device ***/
    lv_port_indev_init();

    // xTimer_beep = xTimerCreate("BEEP contorller",
    //                            buzzer.m_cycle_time / portTICK_PERIOD_MS,
    //                            pdTRUE, (void *)0, buzzer.timer_handler);
    // xTimerStart(xTimer_beep, 0); //开启定时器
    // // 蜂鸣器
    // buzzer.set_beep_time(1000);

    ledcSetup(PWM_1_CHANNEL, 5, 8);
    ledcAttachPin(PWM_1_PIN, PWM_1_CHANNEL);
    ledcWrite(PWM_1_CHANNEL, (int)(0.2 * 255));
    // pinMode(PWM_1_PIN, OUTPUT);
    // digitalWrite(PWM_1_PIN, HIGH);

    ledcSetup(PWM_2_CHANNEL, 5, 8);
    ledcAttachPin(PWM_2_PIN, PWM_2_CHANNEL);
    ledcWrite(PWM_2_CHANNEL, (int)(0.2 * 255));

    ledcSetup(HIGH_CHANNEL, 5000, 8);
    ledcAttachPin(PWM_3_PIN, HIGH_CHANNEL);
    ledcWrite(HIGH_CHANNEL, (int)(0.2 * 255));

    // ledcSetup(FAN_1_CHANNEL, 5000, 8);
    // ledcAttachPin(FAN_1_PIN, FAN_1_CHANNEL);
    // ledcWrite(FAN_1_CHANNEL, (int)(0.2 * 255));

    // ledcSetup(BEEP_CHANNEL, 5000, 8);
    // ledcAttachPin(BEEP_PIN, BEEP_CHANNEL);
    // ledcWrite(BEEP_CHANNEL, (int)(0.8 * 255));

    // ledcSetup(FAN_2_CHANNEL, 5000, 8);
    // ledcAttachPin(FAN_2_PIN, FAN_2_CHANNEL);
    // ledcWrite(FAN_2_CHANNEL, (int)(0.02 * 255));

    // ADC引脚
    // adcAttachPin(ADC3_PIN); //将引脚连接到ADC
    pinMode(ADC0_PIN, INPUT);
    pinMode(ADC1_PIN, INPUT);
    pinMode(ADC2_PIN, INPUT);
    pinMode(ADC3_PIN, INPUT);
    // analogSetPinAttenuation(ADC2_PIN, ADC_0db);

    pinMode(BEEP_PIN, OUTPUT);

    // 开关
    // 按下与松开
    pinMode(SW_1_PIN, INPUT_PULLUP);
    pinMode(SW_2_PIN, INPUT_PULLUP);
    pinMode(FAN_1_PIN, OUTPUT);
    pinMode(FAN_2_PIN, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(SW_1_PIN), sw_1, FALLING);
    attachInterrupt(digitalPinToInterrupt(SW_2_PIN), sw_2, RISING);

    dacWrite(DAC_1, 100); // 输出DAC
    delay(5);

    tft->fillRect(0, 0, 240, 140, TFT_GOLD);
    tft->fillRect(0, 140, 240, 140, TFT_DARKGREY);

    // tft->setTextColor(TFT_WHITE); //设置字体颜色
    // tft->setTextSize(1);          //设置字体大小(默认1号)
    // tft->setCursor(20, 10, 2);    //设置行号
    // tft->startWrite();
    // // tft->print("USB");         //显示USB
    // tft->drawString("USB", 30, 30); //显示USB
    // tft->endWrite();
}

void loop()
{
    // while(1);

    // portENTER_CRITICAL(&mux_1);

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
    Serial.print(analogRead(ADC2_PIN) / 4096.0 * 3.3 * 21);
    Serial.printf("\tADC3 ---> ");
    Serial.println(analogRead(ADC3_PIN) / 4096.0 * 3.3 * 6);

    // digitalWrite(PWM_1_PIN, HIGH);
    // delay(500);
    // digitalWrite(PWM_1_PIN, LOW);
    // delay(500);
    ledcWrite(PWM_1_CHANNEL, (int)(0.01 * 255));
    ledcWrite(PWM_2_CHANNEL, (int)(0.01 * 255));
    // ledcWrite(FAN_1_CHANNEL, (int)(0.01 * 255));
    // ledcWrite(FAN_2_CHANNEL, (int)(0.01 * 255));
    digitalWrite(FAN_1_PIN, HIGH);
    digitalWrite(FAN_2_PIN, HIGH);
    delay(1500);
    ledcWrite(PWM_1_CHANNEL, (int)(1 * 255));
    ledcWrite(PWM_2_CHANNEL, (int)(1 * 255));
    // ledcWrite(FAN_1_CHANNEL, (int)(0.8 * 255));
    // ledcWrite(FAN_2_CHANNEL, (int)(0.8 * 255));
    digitalWrite(FAN_1_PIN, LOW);
    digitalWrite(FAN_2_PIN, LOW);
    delay(1500);

    // portEXIT_CRITICAL(&mux_1);

    // buzzer.set_beep_time(1000);
    // digitalWrite(BEEP_PIN, LOW);
    // delay(1000);
    // digitalWrite(BEEP_PIN, HIGH);
    // delay(1000);
    // digitalWrite(BEEP_PIN, LOW);
    // delay(500);

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

    // // while (1)
    // {
    //     int dutyCycle;
    //     dutyCycle = 0;
    //     ledcWrite(PWM_1_CHANNEL, dutyCycle);
    //     ledcWrite(PWM_2_CHANNEL, dutyCycle);
    //     ledcWrite(HIGH_CHANNEL, dutyCycle);
    //     ledcWrite(FAN_1_CHANNEL, dutyCycle);
    //     ledcWrite(FAN_2_CHANNEL, dutyCycle);
    //     // ledcWrite(BEEP_CHANNEL, dutyCycle);
    //     delay(1000);
    //     dutyCycle = 128;
    //     ledcWrite(PWM_1_CHANNEL, dutyCycle);
    //     ledcWrite(PWM_2_CHANNEL, dutyCycle);
    //     ledcWrite(HIGH_CHANNEL, dutyCycle);
    //     ledcWrite(FAN_1_CHANNEL, dutyCycle);
    //     ledcWrite(FAN_2_CHANNEL, dutyCycle);
    //     // ledcWrite(BEEP_CHANNEL, dutyCycle);
    //     delay(1000);
    //     dutyCycle = 255;
    //     ledcWrite(PWM_1_CHANNEL, dutyCycle);
    //     ledcWrite(PWM_2_CHANNEL, dutyCycle);
    //     ledcWrite(HIGH_CHANNEL, dutyCycle);
    //     ledcWrite(FAN_1_CHANNEL, dutyCycle);
    //     ledcWrite(FAN_2_CHANNEL, dutyCycle);
    //     // ledcWrite(BEEP_CHANNEL, dutyCycle);
    //     delay(1000);

    //     // for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle = dutyCycle + 5)
    //     // {
    //     //     ledcWrite(PWM_1_CHANNEL, dutyCycle);
    //     //     ledcWrite(PWM_2_CHANNEL, dutyCycle);
    //     //     ledcWrite(HIGH_CHANNEL, dutyCycle);
    //     //     ledcWrite(FAN_1_CHANNEL, dutyCycle);
    //     //     ledcWrite(FAN_2_CHANNEL, dutyCycle);
    //     //     delay(500);
    //     // }
    //     // for (int dutyCycle = 255; dutyCycle >=0; dutyCycle = dutyCycle - 5)
    //     // {
    //     //     ledcWrite(PWM_1_CHANNEL, dutyCycle);
    //     //     ledcWrite(PWM_2_CHANNEL, dutyCycle);
    //     //     ledcWrite(HIGH_CHANNEL, dutyCycle);
    //     //     ledcWrite(FAN_1_CHANNEL, dutyCycle);
    //     //     ledcWrite(FAN_2_CHANNEL, dutyCycle);
    //     //     delay(500);
    //     // }
    // }
}