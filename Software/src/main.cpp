#include <Arduino.h>
#include "driver/lv_port_indev.h"
#include "driver/lv_port_fatfs.h"
#include "common.h"
#include "config.h"
#include "controller/hot_air.h"
#include "controller/oscilloscope.h"
#include "controller/signal_generator.h"
#include "controller/power.h"
#include "UI/settings_gui.h"

// 创建热风枪
HotAir hotAir_1("HotAit_1", PWM_1_PIN, PWM_1_CHANNEL, FAN_1_PIN, FAN_1_CHANNEL, ADC0_PIN, SW_0_PIN);
HotAir hotAir_2("HotAit_2", PWM_2_PIN, PWM_2_CHANNEL, FAN_2_PIN, FAN_2_CHANNEL, ADC1_PIN, SW_1_PIN);
// 创建示波器
Oscilloscope oscilloscope("Oscilloscope", ADC2_PIN, ADC3_PIN);
// 创建函数发生器
SignalGenerator signalGenerator("SignalGenerator", DAC_1);
// 可调电源
// Power power("Power", POWER_PIN, POWER_PWM_PIN, POWER_PWM_CHANNEL);

TimerHandle_t xTimer_beep = NULL;

TimerHandle_t xTimer_hotair = NULL;

KeyInfo ret_info;

uint32_t updateTime = 0; // time for next update

TFT_eSprite clk = TFT_eSprite(tft);

void gotTouch()
{
    Serial.println("ESP32 Touch Interrupt");
    // buzzer.set_beep_time(1000);
}

void hot_air(TimerHandle_t xTimer)
{
    hotAir_1.process();
    hotAir_2.process();
    oscilloscope.process();
    signalGenerator.process();
}

void setup()
{
    Serial.begin(115200);
    Serial.println("\nSuperHeat Version: " SuperHeat_VERSION "\n");

    config_read(&g_cfg);

    ledcSetup(HIGH_CHANNEL, 5, 8);
    ledcAttachPin(PWM_3_PIN, HIGH_CHANNEL);
    ledcWrite(HIGH_CHANNEL, 180);

    hotAir_1.start();
    hotAir_2.start();
    oscilloscope.start();
    signalGenerator.start();
    // power.start();
    // power.setVolt(100);

    touchSetCycles(0x3000, 0x1000); // measure值表示量程范围 0x1000为100 0x3000为300
    // 其中40为阈值，当通道T0上的值<40时，会触发中断
    // 在日常使用中，可以先使用 touchRead(TOUCH_PIN) 来确定 touchAttachInterrupt() 的阈值
    touchAttachInterrupt(TOUCH_PIN, gotTouch, 40);

    screen.init();
    // 因为tft需要使用spi，在tft->init()的时候会置位GPIO19，进而影响BEEP，故需要初始化一次引脚
    buzzer.start();

    /*** Init IMU as input device ***/
    lv_port_indev_init();

    tft->fillRect(0, 0, 280, 240, TFT_GOLD);
    tft->fillRect(40, 50, 80, 140, TFT_BLUE);
    tft->fillRect(160, 50, 80, 140, TFT_DARKCYAN);

    tft->setTextColor(TFT_RED); //设置字体颜色
    tft->setTextSize(2);        //设置字体大小(默认1号)
    tft->setCursor(20, 10, 2);  //设置行号
    tft->startWrite();
    // tft->print("USB");         //显示USB
    tft->drawString("SuperHeat", 80, 10); //显示USB
    tft->endWrite();
    tft->setTextColor(TFT_WHITE); //设置字体颜色
    tft->setTextSize(1);          //设置字体大小(默认1号)

    // lv_example_style_8();

    xTimer_beep = xTimerCreate("BEEP contorller",
                               buzzer.m_cycle_time / portTICK_PERIOD_MS,
                               pdTRUE, (void *)0, buzzer.timer_handler);
    xTimerStart(xTimer_beep, 0); //开启定时器

    hotAir_1.setAirDuty(70);
    hotAir_1.setTargetTemp(250);
    hotAir_2.setAirDuty(70);
    hotAir_2.setTargetTemp(200);
    xTimer_hotair = xTimerCreate("HotAit contorller",
                                 100 / portTICK_PERIOD_MS,
                                 pdTRUE, (void *)0, hot_air);
    xTimerStart(xTimer_hotair, 0); //开启定时器
}

void loop()
{
    // screen.routine();
    delay(500);
    // delay(1000);
    // power.start();
    // delay(1000);
    // power.end();
    tft->fillRect(40, 50, 80, 150, TFT_BLUE);
    tft->drawString(String("Set: ") + hotAir_1.getTargetTemp() + "C", 45, 60);
    tft->drawFloat(hotAir_1.getTemperature(), 1, 45, 90);
    tft->drawString(String("Air: ") + hotAir_1.getAirDuty() + "%", 45, 120);
    tft->drawString(String("Pwr: ") + hotAir_1.getPowerDuty() + "%", 45, 150);
    tft->drawString(String("Osc1: ") + oscilloscope.getChanne0(), 45, 180);
    
    tft->fillRect(160, 50, 80, 150, TFT_DARKCYAN);
    tft->drawString(String("Set: ") + hotAir_2.getTargetTemp() + "C", 165, 60);
    tft->drawFloat(hotAir_2.getTemperature(), 1, 165, 90);
    tft->drawString(String("Air: ") + hotAir_2.getAirDuty() + "%", 165, 120);
    tft->drawString(String("Pwr: ") + hotAir_2.getPowerDuty() + "%", 165, 150);
    tft->drawString(String("Osc2: ") + oscilloscope.getChanne1(), 165, 180);

    // ret_info = knobs.get_data();
    // Serial.printf("\n【Knobs】 count---> ");
    // Serial.print(ret_info.pulse_count);
    // Serial.printf("\tstatus---> ");
    // Serial.print(ret_info.switch_status);
    // Serial.printf("\ttime---> ");
    // Serial.print(ret_info.switch_time);
    // Serial.printf("\ttouchRead---> ");
    // Serial.println(touchRead(TOUCH_PIN));
}