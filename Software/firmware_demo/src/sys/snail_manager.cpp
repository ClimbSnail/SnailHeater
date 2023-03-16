#include "common.h"

#include "snail_manager.h"

#include "controller/heat_platform/heat_platform.h"
#include "controller/hot_air/hot_air.h"
#include "controller/osc/oscilloscope.h"
#include "controller/signal_generator/signal_generator.h"
#include "controller/power/adjustable_power.h"
#include "controller/pwm/pwm_controller.h"
#include "controller/solder/solder.h"

const char *ctrl_event_type_info[] = {"CTRL_MESSAGE_WIFI_CONN", "CTRL_MESSAGE_WIFI_AP",
                                      "CTRL_MESSAGE_WIFI_ALIVE", "CTRL_MESSAGE_WIFI_DISCONN",
                                      "CTRL_MESSAGE_UPDATE_TIME", "CTRL_MESSAGE_MQTT_DATA",
                                      "CTRL_MESSAGE_GET_PARAM", "CTRL_MESSAGE_SET_PARAM",
                                      "CTRL_MESSAGE_READ_CFG", "CTRL_MESSAGE_WRITE_CFG",
                                      "CTRL_MESSAGE_NONE"};

SnailManager::SnailManager(boolean flag)
{
    this->xTimer_background_process = NULL;

    this->ctrl_num = 0;
    ctrl_exit_flag = true;
    cur_ctrl_index = 0;
    pre_ctrl_index = 0;
    xTimerEventDeal = NULL;
}

SnailManager::~SnailManager()
{
}

bool SnailManager::initGPIO()
{
    // 烙铁MOS控制引脚
    pinMode(PWM_2_PIN, OUTPUT);
    digitalWrite(PWM_2_PIN, HIGH);

    Serial0.begin(115200);
    analogReadResolution(12); // ADC设置12位

    Serial0.println("\nSnailHeater Version: " SNAILHEATER_VERSION "\n");
    Serial0.println("LVGL Version: " VERSION_LVGL "\n");

    
    Serial0.printf("xTaskGetTickCount(): %llu\n", xTaskGetTickCount());


    // setCpuFrequencyMhz(240);
    Serial0.print(F("CpuFrequencyMhz: "));
    Serial0.println(getCpuFrequencyMhz());

    // flash运行模式
    Serial0.print(F("FlashChipMode: "));
    Serial0.println(ESP.getFlashChipMode());
    Serial0.println(F("FlashChipMode value: FM_QIO = 0, FM_QOUT = 1, FM_DIO = 2, FM_DOUT = 3, FM_FAST_READ = 4, FM_SLOW_READ = 5, FM_UNKNOWN = 255"));

    return 0;
}

bool SnailManager::initController(void)
{
    for (int cnt = 0; cnt < this->ctrl_num; ++cnt)
    {
        // 启动各个控制器
        ctrlList[cnt]->start();
    }

    ctrl_obj.hotAir_0 = (HotAir *)(this->getControllerByName("HotAit_0"));
    ctrl_obj.hotAir_1 = (HotAir *)(this->getControllerByName("HotAit_1"));
    ctrl_obj.heat_platform = (HeatPlatform *)(this->getControllerByName("HeatPlatform"));

    ctrl_obj.solder = (Solder *)(this->getControllerByName("T12"));
    if (NULL == ctrl_obj.solder)
    {
        ctrl_obj.solder = (Solder *)(this->getControllerByName("JBC245"));
    }
    if (NULL == ctrl_obj.solder)
    {
        ctrl_obj.solder = (Solder *)(this->getControllerByName("JBC210"));
    }

    ctrl_obj.oscilloscope = (Oscilloscope *)(this->getControllerByName("Oscilloscope"));
    ctrl_obj.signalGenerator = (SignalGenerator *)(this->getControllerByName("SignalGenerator"));
    ctrl_obj.pwm_controller = (PwmController *)(this->getControllerByName("PwmController"));
    ctrl_obj.adj_power = (AdjustablePower *)(this->getControllerByName("AdjPower"));

    // // 测试使用的初始化操作
    // ctrl_obj.pwm_controller->setDuty(0);

    if (NULL != ctrl_obj.adj_power)
    {
        ctrl_obj.adj_power->setVolt(3470);
    }

    // // ctrl_obj.heat_platform->setTargetTemp(200);
    ctrl_obj.solder->setTargetTemp(320);

    if (NULL != ctrl_obj.hotAir_0)
    {
        ctrl_obj.hotAir_0->setAirDuty(20);
        ctrl_obj.hotAir_0->setTargetTemp(240);
    }

    if (NULL != ctrl_obj.hotAir_1)
    {
        ctrl_obj.hotAir_1->setAirDuty(20);
        // ctrl_obj.hotAir_1->setTargetTemp(240);
    }

    // 使能定时器
    this->xTimer_background_process = xTimerCreate("Manager background_process",
                                                   50 / portTICK_PERIOD_MS,
                                                   pdTRUE, (void *)0, this->background_process);
    xTimerStart(this->xTimer_background_process, 0); //开启定时器

    return true;
}

bool SnailManager::loadWindows(void)
{
    tft->fillRect(0, 0, 280, 240, TFT_WHITE);

    tft->setTextColor(TFT_RED); //设置字体颜色
    tft->setTextSize(2);        //设置字体大小(默认1号)
    tft->setCursor(20, 10, 2);  //设置行号
    // tft->startWrite();
    // tft->drawString("SnailHeater", 80, 10); //显示USB
    // tft->endWrite();
    tft->setTextColor(TFT_WHITE); //设置字体颜色
    tft->setTextSize(1);          //设置字体大小(默认1号)

    return true;
}

bool SnailManager::restart(void)
{
    return true;
}

void SnailManager::background_process(TimerHandle_t xTimer)
{
    unsigned long currentMillis;
    currentMillis = xTaskGetTickCount();

    if (NULL != ctrl_obj.hotAir_0)
    {
        // currentMillis = xTaskGetTickCount();
        ctrl_obj.hotAir_0->process();
        // run_log(NULL, "hotAir_0->process() %lu", xTaskGetTickCount() - currentMillis);
    }

    if (NULL != ctrl_obj.hotAir_1)
    {
        // currentMillis = xTaskGetTickCount();
        ctrl_obj.hotAir_1->process();
        // run_log(NULL, "hotAir_1->process() %lu", xTaskGetTickCount() - currentMillis);
    }

    if (NULL != ctrl_obj.oscilloscope)
    {
        // currentMillis = xTaskGetTickCount();
        ctrl_obj.oscilloscope->process();
        // run_log(NULL, "oscilloscope->process() %lu", xTaskGetTickCount() - currentMillis);
    }

    // ctrl_obj.heat_platform->process();

    if (NULL != ctrl_obj.solder)
    {
        // currentMillis = xTaskGetTickCount();
        ctrl_obj.solder->process();
        // run_log(NULL, "solder->process() %lu", xTaskGetTickCount() - currentMillis);
    }

    if (NULL != ctrl_obj.adj_power)
    {
        // currentMillis = xTaskGetTickCount();
        ctrl_obj.adj_power->process();
    }

    if (NULL != ctrl_obj.signalGenerator)
    {
        currentMillis = xTaskGetTickCount();
        ctrl_obj.signalGenerator->process();
        run_log(NULL, "signalGenerator->process() %lu", xTaskGetTickCount() - currentMillis);
    }
}

int SnailManager::ui_process(KeyInfo *ret_info)
{
    if (NULL != ctrl_obj.solder && NULL != ctrl_obj.oscilloscope)
    {
        tft->setTextColor(TFT_BLACK); //设置字体颜色
        tft->fillRect(60, 20, 120, 20, TFT_WHITE);
        tft->drawString(String("Time: ") + ctrl_obj.solder->getInterval() / 1000 + "s", 60, 20);

        tft->setTextColor(TFT_WHITE); //设置字体颜色
        tft->fillRect(10, 40, 80, 160, TFT_DARKCYAN);
        tft->drawString(String(ctrl_obj.solder->m_name), 15, 40);
        tft->drawString(String("Set: ") + ctrl_obj.solder->getTargetTemp() + "C", 15, 60);
        tft->drawFloat(ctrl_obj.solder->getCurTemperature(false), 1, 15, 90);
        tft->drawString(String("AdjP: ") + ctrl_obj.adj_power->getVolt(), 15, 120);
        tft->drawString(String("Pwr: ") + ctrl_obj.solder->getPowerDuty() + "%", 15, 150);
        tft->drawString(String("Osc2: ") + ctrl_obj.oscilloscope->getChanne0(), 15, 180);
    }

    if (NULL != ctrl_obj.hotAir_1 && NULL != ctrl_obj.oscilloscope)
    {
        tft->setTextColor(TFT_WHITE); //设置字体颜色
        tft->fillRect(105, 40, 80, 160, TFT_BLUE);
        tft->drawString(String(ctrl_obj.hotAir_1->m_name), 110, 40);
        tft->drawString(String("Set: ") + ctrl_obj.hotAir_1->getTargetTemp() + "C", 110, 60);
        tft->drawFloat(ctrl_obj.hotAir_1->getCurTemperature(), 1, 110, 90);
        tft->drawString(String("Air: ") + ctrl_obj.hotAir_1->getAirDuty() + "%", 110, 120);
        tft->drawString(String("Pwr: ") + ctrl_obj.hotAir_1->getPowerDuty() + "%", 110, 150);
        tft->drawString(String("Osc1: ") + ctrl_obj.oscilloscope->getChanne1(), 110, 180);
    }

    if (NULL != ctrl_obj.hotAir_0 && NULL != ctrl_obj.oscilloscope)
    {
        tft->fillRect(200, 40, 80, 160, TFT_GOLD);
        tft->drawString(String(ctrl_obj.hotAir_0->m_name), 205, 40);
        tft->drawString(String("Set: ") + ctrl_obj.hotAir_0->getTargetTemp() + "C", 205, 60);
        tft->drawFloat(ctrl_obj.hotAir_0->getCurTemperature(), 1, 205, 90);
        tft->drawString(String("Air: ") + ctrl_obj.hotAir_0->getAirDuty() + "%", 205, 120);
        tft->drawString(String("Pwr: ") + ctrl_obj.hotAir_0->getPowerDuty() + "%", 205, 150);
        tft->drawString(String("Osc1: ") + ctrl_obj.oscilloscope->getChanne1(), 205, 180);
    }
    return 0;
}

int SnailManager::controller_is_legal(const ControllerBase *controller)
{
    // APP的合法性检测
    if (NULL == controller)
        return 1;
    if (CONTROLLER_MAX_NUM <= this->ctrl_num)
        return 2;
    return 0;
}

// 将控制器注册到 SnailManager 中
int SnailManager::controller_install(ControllerBase *controller,
                                     CTRL_RUN_TYPE run_type, boolean ctrlRunLogStatus)
{
    int ret_code = controller_is_legal(controller);
    if (0 != ret_code)
    {
        return ret_code;
    }

    ctrlList[this->ctrl_num] = controller;
    ctrlRunTypeList[this->ctrl_num] = run_type;
    ctrlRunLogStatusList[this->ctrl_num] = ctrlRunLogStatus;
    ++this->ctrl_num;
    return 0; // 安装成功
}

// 将 controller 从 SnailManager 中卸载（删除）
int SnailManager::controller_uninstall(const ControllerBase *controller)
{
    // todo
    return 0;
}

// 通信中心（消息转发）
int SnailManager::send_to(const char *from, const char *to,
                          CTRL_MESSAGE_TYPE type, void *message,
                          void *ext_info)
{
    ControllerBase *fromCtrl = getControllerByName(from); // 来自谁 有可能为空
    ControllerBase *toCtrl = getControllerByName(to);     // 发送给谁 有可能为空
    if (type <= CTRL_MESSAGE_MQTT_DATA)
    {
        // 更新事件的请求者
        if (eventList.size() > EVENT_LIST_MAX_LENGTH)
        {
            return 1;
        }
        // 发给控制器的消息(目前都是wifi事件)
        EVENT_OBJ new_event = {fromCtrl, type, message, 3, 0, 0};
        eventList.push_back(new_event);
        Serial0.print("[EVENT]\tAdd -> " + String(ctrl_event_type_info[type]));
        Serial0.print(F("\tEventList Size: "));
        Serial0.println(eventList.size());
    }
    else
    {
        // 各个APP之间通信的消息
        if (NULL != toCtrl)
        {
            Serial0.print("[Massage]\tFrom " + String(fromCtrl->m_name) + "\tTo " + String(toCtrl->m_name) + "\n");
            // if (NULL != toCtrl->message_handle)
            {
                toCtrl->message_handle(from, to, type, message, ext_info);
            }
        }
        else if (!strcmp(to, MANAGER_NAME))
        {
            Serial0.print("[Massage]\tFrom " + String(fromCtrl->m_name) + "\tTo " + MANAGER_NAME + "\n");
            // deal_config(type, (const char *)message, (char *)ext_info);
        }
    }
    return 0;
}

void SnailManager::controller_exit()
{
    ctrl_exit_flag = 0; // 退出 ctrl

    // 清空该对象的所有请求
    for (std::list<EVENT_OBJ>::iterator event = eventList.begin(); event != eventList.end();)
    {
        if (ctrlList[cur_ctrl_index] == (*event).from)
        {
            event = eventList.erase(event); // 删除该响应事件
        }
        else
        {
            ++event;
        }
    }
}

ControllerBase *SnailManager::getControllerByName(const char *name)
{
    for (int ind = 0; ind < this->ctrl_num; ++ind)
    {
        if (!strcmp(name, ctrlList[ind]->m_name))
        {
            return ctrlList[ind];
        }
    }

    return NULL;
}

int SnailManager::getControllerIdxByName(const char *name)
{
    int totol = ctrl_num;
    for (int ind = 0; ind < totol; ++ind)
    {
        if (NULL == ctrlList[ind])
        {
            continue;
        }
        if (!strcmp(name, ctrlList[ind]->m_name))
        {
            return ind;
        }
    }

    return -1;
}

int SnailManager::addEvent(CTRL_EVENT event)
{
    switch (event)
    {
    case CTRL_EVENT_SOLDER_SLEEP:
    {
        this->run_log(NULL, "Solder Wake");
    }
    case CTRL_EVENT_AIRHOT_WAKE:
    {
        this->run_log(NULL, "AirHot Wake");
    }
    break;
    defualt:
        break;
    }

    buzzer.set_beep_time(300);
    return 0;
}

size_t SnailManager::run_log(const ControllerBase *ctrller, const char *in_format, ...)
{
    return 0;
#define MIDDLE_FORMAT 128
    char format[MIDDLE_FORMAT] = {0};

    if (NULL == ctrller)
    {
        snprintf(format, MIDDLE_FORMAT, "[SnailManager] %s\n", in_format);
    }
    else
    {
        switch (ctrller->m_type)
        {
        // case CTRL_TYPE_HOTAIR:
        // case CTRL_TYPE_HEATPLATFORM:
        case CTRL_TYPE_SOLDER:
        // case CTRL_TYPE_OSCILLOSCOPE:
        // case CTRL_TYPE_SIGNALGENERATOR:
        // case CTRL_TYPE_PWMCONTROLLER:
        // case CTRL_TYPE_ADJUSTABLEPOWER:
        case CTRL_TYPE_NONE:
        {
            snprintf(format, MIDDLE_FORMAT, "[%s] %s\n", ctrller->m_name, in_format);
        }
        break;
        default:
        {
            return 0;
        }
        break;
        }

        // int index = this->getControllerIdxByName("AdjPower");
        // Serial0.printf("\nindex = %d\n", index);
        // // Serial0.printf("\nctrller->m_name = %s\n", ctrller->m_name);
        // if (index != -1 && CTRL_LOG_STATUS_OPEN == ctrlRunLogStatusList[index])
        // {
        //     snprintf(format, MIDDLE_FORMAT, "[%s] %s\n", ctrller->m_name, in_format);
        // }
        // else
        // {
        //     return 0;
        // }
    }

    char loc_buf[64];
    char *temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
    va_end(copy);
    if (len < 0)
    {
        va_end(arg);
        return 0;
    };
    if (len >= sizeof(loc_buf))
    {
        temp = (char *)malloc(len + 1);
        if (temp == NULL)
        {
            va_end(arg);
            return 0;
        }
        len = vsnprintf(temp, len + 1, format, arg);
    }
    va_end(arg);
    len = Serial0.write((uint8_t *)temp, len);
    if (temp != loc_buf)
    {
        free(temp);
    }
    return len;
}