#include <Arduino.h>
#include "driver/lv_port_indev.h"
#include "driver/lv_port_fatfs.h"
#include "driver/rgb_led.h"
#include "common.h"
#include "config.h"

#include "sys/snail_manager.h"

#include "controller/heat_platform/heat_platform.h"
#include "controller/hot_air/hot_air.h"
#include "controller/osc/oscilloscope.h"
#include "controller/signal_generator/signal_generator.h"
#include "controller/power/adjustable_power.h"
#include "controller/pwm/pwm_controller.h"
#include "controller/solder/solder.h"

#include "lvgl.h"
#include "demos/lv_demos.h"

KeyInfo ret_info;

TaskHandle_t handleTaskLvgl;
void TaskLvglUpdate(void *parameter)
{
    // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    for (;;)
    {
        lv_task_handler();
        vTaskDelay(5);
    }
}

TaskHandle_t handleUpdataADC;
void TaskUpdateData(void *parameter)
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    for (;;)
    {
        updateADC(false);
        // updateADC(true);
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

// 创建热风枪
unsigned long HotAir::s_wakeMillis_0 = xTaskGetTickCount() / portTICK_PERIOD_MS; // 获取Tick值
unsigned long HotAir::s_wakeMillis_1 = xTaskGetTickCount() / portTICK_PERIOD_MS; // 获取Tick值
uint8_t HotAir::s_hotAirObjNum = 0;                                              // 风枪对象的数量
HotAir *hotAir_0 = new HotAir("HotAit_0", NULL, PWM_0_PIN, PWM_0_CHANNEL,
                              FAN_0_PIN, FAN_0_CHANNEL, ADC0_PIN, SW_0_PIN);

HotAir *hotAir_1 = new HotAir("HotAit_1", NULL, PWM_1_PIN, PWM_1_CHANNEL,
                              FAN_1_PIN, FAN_1_CHANNEL, ADC1_PIN, SW_1_PIN);

// 加热台
unsigned long HeatPlatform::s_wakeMillis = xTaskGetTickCount() / portTICK_PERIOD_MS; // 获取Tick值
HeatPlatform *heat_platform = new HeatPlatform("HeatPlatform", NULL, PWM_1_PIN, PWM_1_CHANNEL, ADC1_PIN);

// 电烙铁
unsigned long Solder::s_wakeMillis = xTaskGetTickCount() / portTICK_PERIOD_MS; // 获取Tick值
// Solder *solder = new Solder("T12", NULL, SOLDER_TYPE_T12, PWM_2_PIN, PWM_2_CHANNEL, ADC2_PIN, SW_2_PIN);
Solder *solder = new Solder("JBC245", NULL, SOLDER_TYPE_JBC245, PWM_2_PIN, PWM_2_CHANNEL, ADC2_PIN, SW_2_PIN);
// Solder *solder = new Solder("JBC210", NULL, SOLDER_TYPE_JBC210, PWM_2_PIN, PWM_2_CHANNEL, ADC2_PIN, SW_2_PIN);

// 创建示波器
Oscilloscope *oscilloscope = new Oscilloscope("Oscilloscope", NULL, ADC3_PIN, ADC4_PIN);

// 创建函数发生器
SignalGenerator *signalGenerator = new SignalGenerator("SignalGenerator", NULL, DAC_2);

// 创建高精度PWM
PwmController *pwm_controller = new PwmController("PwmController", NULL, PWM_3_PIN, HIGH_CHANNEL);

// 可调电源
AdjustablePower *adj_power = new AdjustablePower("AdjPower", NULL, POWER_EN_PIN, POWER_DAC_PIN, PWM_DAC_CHANNEL,
                                                 POWER_CUR_ADC_PIN, POWER_OV_ADC_PIN);

TimerHandle_t xTimer_beep = NULL;

TFT_eSprite clk = TFT_eSprite(tft);

CTRL_OBJ SnailManager::ctrl_obj = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
SnailManager *manager = new SnailManager(true);

void gotTouch()
{
    manager->run_log(NULL, "ESP32 Touch Interrupt");
}

void myPrintf(const char *str)
{
    manager->run_log(NULL, "[LVGL] %s", str);
}

void setup()
{
    manager->initGPIO();

    // 需要放在Setup里初始化
    if (!SPIFFS.begin(true))
    {
        Serial0.println("SPIFFS Mount Failed");
        return;
    }

    /*** Init on-board RGB ***/
    rgb.init();
    rgb.setBrightness(0.5).setRGB(64, 64, 64);

    manager->controller_install(hotAir_0, CTRL_RUN_TYPE_BACKGROUND, CTRL_LOG_STATUS_OPEN);
    manager->controller_install(hotAir_1, CTRL_RUN_TYPE_BACKGROUND, CTRL_LOG_STATUS_OPEN);
    // manager->controller_install(&heat_platform);
    manager->controller_install(solder, CTRL_RUN_TYPE_BACKGROUND, CTRL_LOG_STATUS_OPEN);
    manager->controller_install(oscilloscope, CTRL_RUN_TYPE_BACKGROUND, CTRL_LOG_STATUS_OPEN);
    // manager->controller_install(signalGenerator), CTRL_RUN_TYPE_BACKGROUND, CTRL_LOG_STATUS_OPEN;
    // manager->controller_install(pwm_controller, CTRL_RUN_TYPE_BACKGROUND, CTRL_LOG_STATUS_OPEN);
    manager->controller_install(adj_power, CTRL_RUN_TYPE_BACKGROUND, CTRL_LOG_STATUS_OPEN);

    manager->initController();

    // BaseType_t taskUpdateDataReturned;
    // taskUpdateDataReturned = xTaskCreate(
    //     TaskUpdateData,
    //     "TaskUpdateData",
    //     2000,
    //     nullptr,
    //     // configMAX_PRIORITIES - 1,
    //     tskIDLE_PRIORITY,
    //     &handleUpdataADC);
    // if (taskUpdateDataReturned != pdPASS)
    // {
    //     manager->run_log(NULL, "Fail Create TaskUpdateData %d!!!", 1);
    // }

    touchSetCycles(0x1000, 0x5); // measure值表示量程范围 0x1000为100 0x3000为300
    // 其中40为阈值，当通道T0上的值<40时，会触发中断
    // 在日常使用中，可以先使用 touchRead(TOUCH_PIN) 来确定 touchAttachInterrupt() 的阈值
    touch_value_t threshold = touchRead(TOUCH_PIN) + 100000;
    manager->run_log(NULL, "%u", threshold);
    // touch_value_t threshold = 1000000;
    touchAttachInterrupt(TOUCH_PIN, gotTouch, threshold);
    threshold = touchRead(TOUCH_PIN) + 100000;
    manager->run_log(NULL, "%u", threshold);

    /*** Init screen ***/
    screen.init();

    // 因为tft需要使用spi，在tft->init()的时候会置位GPIO19，进而影响BEEP，故需要初始化一次引脚
    buzzer.start();

    /*** Init IMU as input device ***/
    // lv_port_indev_init();

    // lv_log_register_print_cb(myPrintf);

    // Update display in parallel thread.
    // BaseType_t taskLvglUpdateReturned;
    // taskLvglUpdateReturned = xTaskCreate(
    //     TaskLvglUpdate,
    //     "LvglThread",
    //     20000,
    //     nullptr,
    //     configMAX_PRIORITIES - 1,
    //     &handleTaskLvgl);
    // if (taskLvglUpdateReturned != pdPASS)
    // {
    //     manager->run_log(NULL, "Fail Create taskLvglUpdateReturned!!!");
    // }

    manager->loadWindows();

    xTimer_beep = xTimerCreate("BEEP contorller",
                               buzzer.m_cycle_time / portTICK_PERIOD_MS,
                               pdTRUE, (void *)0, buzzer.timer_handler);
    xTimerStart(xTimer_beep, 0); //开启定时器

    // lv_example_super_heat();
}

void loop()
{
    // screen.routine();

    // ret_info = knobs.get_data();
    manager->ui_process(&ret_info);
    vTaskDelay(300 / portTICK_PERIOD_MS);
}