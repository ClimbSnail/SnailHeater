#include "ui.h"
#include "desktop_model.h"

#ifdef NEW_UI

static lv_timer_t *powerTimer;
static lv_obj_t *adjPowerPageUI = NULL;
static lv_obj_t *ui_voltageLabel;
static lv_obj_t *ui_voltSlider;
static lv_obj_t *ui_voltageUnitLabel;
static lv_obj_t *ui_currentLabel;
static lv_obj_t *ui_currentUnitLabel;
static lv_obj_t *ui_capacityLabel;
static lv_obj_t *ui_capacityUnitLabel;
static lv_obj_t *ui_enableSwitch;
static lv_obj_t *ui_modeButton;
static lv_obj_t *ui_modeLable;
static lv_obj_t *ui_backButton;
static lv_obj_t *ui_backButtonLabel;

static lv_group_t *btn_group;

static void ui_set_slider_changed(lv_event_t *e);
static void ui_back_btn_pressed(lv_event_t *e);
static void ui_enable_switch_pressed(lv_event_t *e);

static void adjPowerPageUI_focused()
{
    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_backButton);
    lv_group_add_obj(btn_group, ui_voltSlider);
    lv_group_add_obj(btn_group, ui_modeButton);
    lv_group_add_obj(btn_group, ui_enableSwitch);
    if (adjPowerModel.workState == 1)
    {
        lv_group_focus_obj(ui_enableSwitch);
    }
    else
    {
        lv_group_focus_obj(ui_backButton);
    }
    lv_indev_set_group(knobs_indev, btn_group);
}

static void powerTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    // 更新电压电流功率的函数
    lv_label_set_text_fmt(ui_voltageLabel, "%.2lf", adjPowerModel.voltage / 1000.0);
    lv_label_set_text_fmt(ui_currentLabel, "%.2lf", adjPowerModel.current / 1000.0);
    lv_label_set_text(ui_capacityUnitLabel, "W");
}

static bool adjPowerPageUI_init(lv_obj_t *father)
{
    if (NULL != adjPowerPageUI)
    {
        adjPowerPageUI = NULL;
    }
    top_layer_set_name();
    theme_color_init();

    adjPowerPageUI = lv_btn_create(father);
    adjPowerUIObj.mainButtonUI = adjPowerPageUI;

    lv_obj_remove_style_all(adjPowerPageUI);
    lv_obj_set_size(adjPowerPageUI, EACH_PAGE_SIZE_X, EACH_PAGE_SIZE_Y);
    lv_obj_center(adjPowerPageUI);
    lv_obj_add_flag(adjPowerPageUI, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(adjPowerPageUI, LV_OBJ_FLAG_SCROLLABLE);
    //lv_obj_set_style_bg_color(adjPowerPageUI, IS_WHITE_THEME ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(adjPowerPageUI, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_ButtonTmp = adjPowerPageUI;

    ui_voltageLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_voltageLabel, 150, 52);
    lv_obj_align(ui_voltageLabel, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_label_set_text_fmt(ui_voltageLabel, "%.2lf", adjPowerModel.voltage / 1000.0);
    lv_obj_set_style_text_align(ui_voltageLabel, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_voltageLabel, ADJ_POWER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_voltageLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_voltageLabel, &FontRoboto_52, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_voltageUnitLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_voltageUnitLabel, LV_ALIGN_TOP_LEFT, 152, 30);
    lv_label_set_text(ui_voltageUnitLabel, "V");
    lv_obj_set_style_text_color(ui_voltageUnitLabel, lv_color_hex(0x999798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_voltageUnitLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_voltageUnitLabel, &FontRoboto_36, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 电压调节滑杆
    ui_voltSlider = lv_slider_create(ui_ButtonTmp);
    lv_obj_set_size(ui_voltSlider, 10, 100);
    lv_obj_align(ui_voltSlider, LV_ALIGN_TOP_LEFT, 220, 20);
    lv_slider_set_range(ui_voltSlider, 0, DAC_DEFAULT_RESOLUTION);
    lv_obj_set_style_bg_color(ui_voltSlider, ADJ_POWER_THEME_COLOR1, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_voltSlider, lv_color_hex(0x999999), LV_PART_INDICATOR | LV_STATE_DEFAULT);

    if (ADJ_POWER_MODE_CV == adjPowerModel.mode)
    {
        lv_slider_set_value(ui_voltSlider,
                            DAC_DEFAULT_RESOLUTION - adjPowerModel.volDacValue,
                            LV_ANIM_OFF);
    }
    else if (ADJ_POWER_MODE_CC == adjPowerModel.mode)
    {
        lv_slider_set_value(ui_voltSlider,
                            DAC_DEFAULT_RESOLUTION - adjPowerModel.curDacValue,
                            LV_ANIM_OFF);
    }

    // lv_obj_add_flag(ui_voltSlider, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    // 电流
    ui_currentLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_currentLabel, 150, 52);
    lv_obj_align(ui_currentLabel, LV_ALIGN_TOP_LEFT, 0, 80);
    lv_label_set_text_fmt(ui_currentLabel, "%.2lf", adjPowerModel.current / 1000.0);
    lv_obj_set_style_text_align(ui_currentLabel, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_currentLabel, ADJ_POWER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_currentLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_currentLabel, &FontRoboto_52, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_currentUnitLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_currentUnitLabel, LV_ALIGN_TOP_LEFT, 152, 90);
    lv_label_set_text(ui_currentUnitLabel, "A");
    lv_obj_set_style_text_color(ui_currentUnitLabel, lv_color_hex(0x999798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_currentUnitLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_currentUnitLabel, &FontRoboto_36, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 功率
    ui_capacityLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_capacityLabel, 150, 52);
    lv_obj_align(ui_capacityLabel, LV_ALIGN_TOP_LEFT, 0, 140);
    lv_label_set_text_fmt(ui_capacityLabel, "%.2lf", adjPowerModel.capacity / 1000000.0);
    lv_obj_set_style_text_align(ui_capacityLabel, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_capacityLabel, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_capacityLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_capacityLabel, &FontRoboto_52, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_capacityUnitLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_capacityUnitLabel, LV_ALIGN_TOP_LEFT, 152, 150);
    lv_label_set_text(ui_capacityUnitLabel, "W");
    lv_obj_set_style_text_color(ui_capacityUnitLabel, lv_color_hex(0x999798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_capacityUnitLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_capacityUnitLabel, &FontRoboto_36, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 模式按钮
    ui_modeButton = lv_btn_create(ui_ButtonTmp); // 紫色
    lv_obj_set_size(ui_modeButton, 30, 25);
    lv_obj_align(ui_modeButton, LV_ALIGN_CENTER, 100, 90);
    lv_obj_set_style_bg_color(ui_modeButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_modeButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_add_style(ui_modeButton, &focused_style, LV_STATE_FOCUSED);

    // 模式标签
    ui_modeLable = lv_label_create(ui_modeButton);
    lv_obj_set_size(ui_modeLable, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_modeLable, 0, 0);
    lv_obj_set_align(ui_modeLable, LV_ALIGN_CENTER);
    if (ADJ_POWER_MODE_CV == adjPowerModel.mode)
    {
        lv_label_set_text_fmt(ui_modeLable, "CV");
    }
    else if (ADJ_POWER_MODE_CC == adjPowerModel.mode)
    {
        lv_label_set_text_fmt(ui_modeLable, "CC");
    }
    lv_obj_set_style_text_color(ui_modeLable, ADJ_POWER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_modeLable, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_modeLable, &FontJost, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 使能开关
    ui_enableSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_add_flag(ui_enableSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_enableSwitch, 40, 20);
    lv_obj_align(ui_enableSwitch, LV_ALIGN_CENTER, 100, 50);
    if (ENABLE_STATE_OPEN == adjPowerModel.workState)
    {
        lv_obj_add_state(ui_enableSwitch, LV_STATE_CHECKED); // 开
    }
    else
    {
        lv_obj_clear_state(ui_enableSwitch, LV_STATE_CHECKED); // 关
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_enableSwitch, HEAT_PLAT_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_enableSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_enableSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 返回图标
    ui_backButton = lv_btn_create(ui_PanelTop);
    lv_obj_remove_style_all(ui_backButton);
    lv_obj_align(ui_backButton, LV_ALIGN_TOP_RIGHT, -10, 0);
    lv_obj_add_flag(ui_backButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_backButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_backButton, &back_btn_style, LV_STATE_DEFAULT);
    lv_obj_add_style(ui_backButton, &back_btn_focused_style, LV_STATE_FOCUSED);


    ui_backButtonLabel = lv_label_create(ui_backButton);
    lv_obj_center(ui_backButtonLabel);
    lv_label_set_text(ui_backButtonLabel, LV_SYMBOL_LEFT);

    lv_obj_add_event_cb(ui_backButton, ui_back_btn_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_voltSlider, ui_set_slider_changed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_enableSwitch, ui_enable_switch_pressed, LV_EVENT_VALUE_CHANGED, NULL);

    powerTimer = lv_timer_create(powerTimer_timeout, 300, NULL);
    lv_timer_set_repeat_count(powerTimer, -1);
    return true;
}

static void adjPowerPageUI_release()
{
    if (NULL == adjPowerPageUI)
    {
        return;
    }
    if (NULL != btn_group)
    {
        lv_group_del(btn_group);
    }
    lv_timer_del(powerTimer);
    lv_obj_clean(adjPowerPageUI);
    adjPowerPageUI = NULL;
    adjPowerUIObj.mainButtonUI = NULL;
}

static void ui_back_btn_pressed(lv_event_t *e)
{
    // 返回项被按下
    show_menu();
}

static void ui_mode_bnt_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_PRESSED)
    {
        adjPowerModel.mode = (adjPowerModel.mode + 1) % ADJ_POWER_MODE_MAX_NUM;
        if (ADJ_POWER_MODE_CV == adjPowerModel.mode)
        {
            lv_label_set_text_fmt(ui_modeLable, "CV");
            lv_slider_set_value(ui_voltSlider,
                                DAC_DEFAULT_RESOLUTION - adjPowerModel.volDacValue,
                                LV_ANIM_OFF);
        }
        else if (ADJ_POWER_MODE_CC == adjPowerModel.mode)
        {
            lv_label_set_text_fmt(ui_modeLable, "CC");
            lv_slider_set_value(ui_voltSlider,
                                DAC_DEFAULT_RESOLUTION - adjPowerModel.curDacValue,
                                LV_ANIM_OFF);
        }
    }
}

static void ui_enable_switch_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        bool isEnable = lv_obj_has_state(target, LV_STATE_CHECKED); // 返回 bool 类型， 开-1 ； 关-2
        adjPowerModel.workState = isEnable == true ? ENABLE_STATE_OPEN : ENABLE_STATE_CLOSE;
    }
}

void ui_updateAdjPowerCurVoltage(void)
{
    if (NULL == adjPowerPageUI)
    {
        return;
    }
    lv_label_set_text_fmt(ui_voltageLabel, "%.2lf", adjPowerModel.voltage / 1000.0);
    lv_label_set_text_fmt(ui_currentLabel, "%.2lf", adjPowerModel.current / 1000.0);
    lv_label_set_text_fmt(ui_capacityLabel, "%.2lf", adjPowerModel.capacity / 1000000.0);
}

void ui_updateAdjPowerWorkState(void)
{
    if (NULL == adjPowerPageUI)
    {
        return;
    }

    if (ENABLE_STATE_CLOSE == adjPowerModel.workState)
    {
        lv_obj_clear_state(ui_enableSwitch, LV_STATE_CHECKED); // 关
    }
    else
    {
        lv_obj_add_state(ui_enableSwitch, LV_STATE_CHECKED); // 开
    }
}

static void ui_set_slider_changed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        if (ADJ_POWER_MODE_CV == adjPowerModel.mode)
        {
            adjPowerModel.volDacValue =
                DAC_DEFAULT_RESOLUTION - (int)lv_slider_get_value(ui_voltSlider);
        }
        else if (ADJ_POWER_MODE_CC == adjPowerModel.mode)
        {
            adjPowerModel.curDacValue =
                DAC_DEFAULT_RESOLUTION - (int)lv_slider_get_value(ui_voltSlider);
        }
    }
}

FE_FULL_UI_OBJ adjPowerUIObj = {adjPowerPageUI, adjPowerPageUI_init,
                                adjPowerPageUI_release, adjPowerPageUI_focused};

#endif