#include "./ui.h"
#include "../desktop_model.h"

#ifdef FULL_UI

static lv_timer_t *spotWelderTimer = NULL;
static lv_obj_t *spotWelderPageUI = NULL;
static lv_obj_t *ui_enableSwitch;
static lv_obj_t *ui_modeSwitch;
static lv_obj_t *pulseWidthLabel_0;
static lv_obj_t *ui_pulseWidth_0;
static lv_obj_t *ui_pulseWidth_1;
static lv_obj_t *ui_triggerDeyal;
static lv_obj_t *ui_voltage;
static lv_obj_t *ui_count;
static lv_obj_t *ui_manualTrigger;

static lv_group_t *btn_group = NULL;

static void ui_set_obj_change(lv_event_t *e);
static void ui_enable_switch_pressed(lv_event_t *e);
static void ui_bnt_obj_pressed(lv_event_t *e);
static void ui_updateSignalData(void);
static void ui_updateSignalWorkState(void);
static void spotWelderPageUI_release();

static void spotWelderPageUI_focused(lv_event_t *e)
{
    lv_indev_set_group(knobs_indev, btn_group);
    if (stopWelderModel.workState == 1)
    {
        lv_group_focus_obj(ui_enableSwitch);
    }
    else
    {
        lv_group_focus_obj(ui_backBtn);
    }
}

static void spotWelderTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    ui_updateSignalData();
    ui_updateSignalWorkState();
}

static void setMode()
{
    if (SPOTWELDER_MODE::SPOTWELDER_MODE_DOUBLE == stopWelderModel.utilConfig.mode)
    {
        // 双脉冲模式
        lv_group_remove_obj(ui_pulseWidth_0);
        lv_group_remove_obj(ui_pulseWidth_1);
        lv_group_remove_obj(ui_triggerDeyal);
        lv_group_remove_obj(ui_enableSwitch);
        lv_group_remove_obj(ui_manualTrigger);
        lv_group_add_obj(btn_group, ui_pulseWidth_0);
        lv_group_add_obj(btn_group, ui_pulseWidth_1);
        lv_group_add_obj(btn_group, ui_triggerDeyal);
        lv_group_add_obj(btn_group, ui_enableSwitch);
        lv_group_add_obj(btn_group, ui_manualTrigger);
        // 显示控件
        lv_obj_set_style_text_opa(pulseWidthLabel_0, 255, LV_PART_MAIN);
        lv_obj_set_style_text_opa(ui_pulseWidth_0, 255, LV_PART_MAIN);
    }
    else
    {
        // 单脉冲模式
        lv_group_remove_obj(ui_pulseWidth_0);
        // 隐藏控件
        lv_obj_set_style_text_opa(pulseWidthLabel_0, 150, LV_PART_MAIN);
        lv_obj_set_style_text_opa(ui_pulseWidth_0, 150, LV_PART_MAIN);
    }
}

static bool spotWelderPageUI_init(lv_obj_t *father)
{
    spotWelderPageUI_release();

    top_layer_set_name();
    theme_color_init();

    spotWelderPageUI = lv_btn_create(father);
    spotWelderUIObj.mainButtonUI = spotWelderPageUI;
    lv_obj_t *ui_ButtonTmp = spotWelderPageUI;

    lv_obj_remove_style_all(ui_ButtonTmp);
    lv_obj_set_size(ui_ButtonTmp, EACH_PAGE_SIZE_X, EACH_PAGE_SIZE_Y);
    lv_obj_center(ui_ButtonTmp);
    lv_obj_add_flag(ui_ButtonTmp, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_ButtonTmp, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_style_bg_color(spotWelderPageUI, IS_WHITE_THEME ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_ButtonTmp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 双脉冲模式
    lv_obj_t *modeLabel = lv_label_create(ui_ButtonTmp);
    // lv_obj_align_to(modeLabel, ui_freqSetStepLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    lv_obj_align(modeLabel, LV_ALIGN_TOP_LEFT, 25, 20);
    lv_label_set_text_fmt(modeLabel, "%s", TEXT_SPOTWELDER_DOUBLE_PULSE);
    lv_obj_add_style(modeLabel, &label_text_style, 0);

    ui_modeSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_add_flag(ui_modeSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_modeSwitch, 30, 15);
    lv_obj_align_to(ui_modeSwitch, modeLabel, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
    if (SPOTWELDER_MODE::SPOTWELDER_MODE_DOUBLE == stopWelderModel.utilConfig.mode)
    {
        lv_obj_add_state(ui_modeSwitch, LV_STATE_CHECKED); // 开
    }
    else
    {
        lv_obj_clear_state(ui_modeSwitch, LV_STATE_CHECKED); // 关
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_modeSwitch, SPOT_WELDER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_bg_color(ui_modeSwitch, SPOT_WELDER_THEME_COLOR1, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_outline_opa(ui_modeSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_modeSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 脉宽0
    pulseWidthLabel_0 = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(pulseWidthLabel_0, modeLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_label_set_text_fmt(pulseWidthLabel_0, "%s", TEXT_SPOTWELDER_PULSE_WIDTH "1");
    lv_obj_add_style(pulseWidthLabel_0, &label_text_style, 0);

    ui_pulseWidth_0 = lv_numberbtn_create(ui_ButtonTmp);
    lv_obj_t *pulseWidthBtnLabel_0 = lv_label_create(ui_pulseWidth_0);
    lv_numberbtn_set_label_and_format(ui_pulseWidth_0,
                                      pulseWidthBtnLabel_0, "%.1fms", 0.1);
    lv_numberbtn_set_range(ui_pulseWidth_0, 0.1, 100);
    lv_numberbtn_set_value(ui_pulseWidth_0, stopWelderModel.utilConfig.pulseWidth_0 / 10.0);
    lv_obj_remove_style_all(ui_pulseWidth_0);
    lv_obj_set_size(ui_pulseWidth_0, 70, 20);
    lv_obj_align_to(ui_pulseWidth_0, pulseWidthLabel_0, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_add_flag(ui_pulseWidth_0, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_pulseWidth_0, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_pulseWidth_0, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_pulseWidth_0, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_pulseWidth_0, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_pulseWidth_0, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_pulseWidth_0, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_pulseWidth_0, &btn_type1_pressed_style, LV_STATE_EDITED);

    // 脉宽1
    lv_obj_t *pulseWidthLabel_1 = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(pulseWidthLabel_1, pulseWidthLabel_0, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_label_set_text_fmt(pulseWidthLabel_1, "%s", TEXT_SPOTWELDER_PULSE_WIDTH "2");
    lv_obj_add_style(pulseWidthLabel_1, &label_text_style, 0);

    ui_pulseWidth_1 = lv_numberbtn_create(ui_ButtonTmp);
    lv_obj_t *pulseWidthBtnLabel_1 = lv_label_create(ui_pulseWidth_1);
    lv_numberbtn_set_label_and_format(ui_pulseWidth_1,
                                      pulseWidthBtnLabel_1, "%.1fms", 0.1);
    lv_numberbtn_set_range(ui_pulseWidth_1, 0.1, 100);
    lv_numberbtn_set_value(ui_pulseWidth_1, stopWelderModel.utilConfig.pulseWidth_1 / 10.0);
    lv_obj_remove_style_all(ui_pulseWidth_1);
    lv_obj_set_size(ui_pulseWidth_1, 70, 20);
    // lv_obj_align_to(ui_pulseWidth_1, pulseWidthLabel_1, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_align_to(ui_pulseWidth_1, ui_pulseWidth_0, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_add_flag(ui_pulseWidth_1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_pulseWidth_1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_pulseWidth_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_pulseWidth_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_pulseWidth_1, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_pulseWidth_1, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_pulseWidth_1, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_pulseWidth_1, &btn_type1_pressed_style, LV_STATE_EDITED);

    // 延时
    lv_obj_t *triggerDeyalLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(triggerDeyalLabel, pulseWidthLabel_1, LV_ALIGN_OUT_BOTTOM_MID, -22, 10);
    lv_label_set_text_fmt(triggerDeyalLabel, "%s", TEXT_SPOTWELDER_TRIGGER_DELAY);
    lv_obj_add_style(triggerDeyalLabel, &label_text_style, 0);

    ui_triggerDeyal = lv_numberbtn_create(ui_ButtonTmp);
    lv_obj_t *ui_triggerDeyalBtnLabel = lv_label_create(ui_triggerDeyal);
    lv_numberbtn_set_label_and_format(ui_triggerDeyal,
                                      ui_triggerDeyalBtnLabel, "%dms", 1);
    lv_numberbtn_set_range(ui_triggerDeyal, 100, 1000);
    lv_numberbtn_set_value(ui_triggerDeyal, stopWelderModel.utilConfig.interval);
    lv_obj_remove_style_all(ui_triggerDeyal);
    lv_obj_set_size(ui_triggerDeyal, 70, 20);
    // lv_obj_align_to(ui_triggerDeyal, triggerDeyalLabel, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_align_to(ui_triggerDeyal, ui_pulseWidth_1, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_add_flag(ui_triggerDeyal, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_triggerDeyal, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_triggerDeyal, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_triggerDeyal, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_triggerDeyal, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_triggerDeyal, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_triggerDeyal, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_triggerDeyal, &btn_type1_pressed_style, LV_STATE_EDITED);

    // 电容电压
    lv_obj_t *voltageLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(voltageLabel, triggerDeyalLabel, LV_ALIGN_OUT_BOTTOM_MID, -12, 10);
    lv_label_set_text_fmt(voltageLabel, "%s", TEXT_SPOTWELDER_VOLTAGE);
    lv_obj_add_style(voltageLabel, &label_text_style, 0);

    ui_voltage = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(ui_voltage, ui_triggerDeyal, LV_ALIGN_OUT_BOTTOM_MID, -10, 10);
    lv_label_set_text_fmt(ui_voltage, "%.2fV", stopWelderModel.voltage / 1000.0);
    lv_obj_add_style(ui_voltage, &label_text_style, 0);
    lv_obj_set_style_text_color(ui_voltage, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_voltage, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_voltage, 150, LV_PART_MAIN);

    // 点焊计数
    lv_obj_t *countLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(countLabel, voltageLabel, LV_ALIGN_OUT_BOTTOM_MID, -12, 10);
    lv_label_set_text_fmt(countLabel, "%s", TEXT_SPOTWELDER_SPOT_COUNT);
    lv_obj_add_style(countLabel, &label_text_style, 0);

    ui_count = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(ui_count, ui_voltage, LV_ALIGN_OUT_BOTTOM_MID, -10, 10);
    lv_label_set_text_fmt(ui_count, "%d", stopWelderModel.count);
    lv_obj_add_style(ui_count, &label_text_style, 0);
    lv_obj_set_style_text_color(ui_count, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_count, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_count, 150, LV_PART_MAIN);

    // 使能开关
    ui_enableSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_add_flag(ui_enableSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_enableSwitch, 60, 30);
    // lv_obj_align_to(ui_enableSwitch, ui_count, LV_ALIGN_OUT_BOTTOM_MID, -5, 10);
    lv_obj_align(ui_enableSwitch, LV_ALIGN_TOP_RIGHT, -20, 15);
    if (ENABLE_STATE_OPEN == stopWelderModel.workState)
    {
        lv_obj_add_state(ui_enableSwitch, LV_STATE_CHECKED); // 开
    }
    else
    {
        lv_obj_clear_state(ui_enableSwitch, LV_STATE_CHECKED); // 关
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_enableSwitch, SPOT_WELDER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_bg_color(ui_enableSwitch, SPOT_WELDER_THEME_COLOR1, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_outline_opa(ui_enableSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_enableSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 手动触发
    ui_manualTrigger = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_manualTrigger);
    lv_obj_set_size(ui_manualTrigger, 65, 25);
    lv_obj_align_to(ui_manualTrigger, ui_enableSwitch, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
    lv_obj_add_flag(ui_manualTrigger, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_manualTrigger, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_manualTrigger, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_manualTrigger, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_manualTrigger, &btn_type1_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_manualTrigger, &btn_type1_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *manualTriggerLabel = lv_label_create(ui_manualTrigger);
    lv_obj_set_align(manualTriggerLabel, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(manualTriggerLabel, "%s", TEXT_SPOTWELDER_MANUAL_TRIGGER);
    lv_obj_add_style(manualTriggerLabel, &label_text_style, 0);
    

    lv_obj_add_event_cb(ui_modeSwitch, ui_set_obj_change, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_pulseWidth_0, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_pulseWidth_1, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_triggerDeyal, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_enableSwitch, ui_set_obj_change, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_manualTrigger, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);

    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_backBtn);
    lv_group_add_obj(btn_group, ui_modeSwitch);
    lv_group_add_obj(btn_group, ui_pulseWidth_0);
    lv_group_add_obj(btn_group, ui_pulseWidth_1);
    lv_group_add_obj(btn_group, ui_triggerDeyal);
    lv_group_add_obj(btn_group, ui_enableSwitch);
    lv_group_add_obj(btn_group, ui_manualTrigger);
    lv_indev_set_group(knobs_indev, btn_group);

    setMode();

    spotWelderTimer = lv_timer_create(spotWelderTimer_timeout, DATA_REFRESH_MS, NULL);
    lv_timer_set_repeat_count(spotWelderTimer, -1);
    return true;
}

static void spotWelderPageUI_release()
{
    if (NULL != spotWelderTimer)
    {
        lv_timer_del(spotWelderTimer);
        spotWelderTimer = NULL;
    }

    if (NULL != btn_group)
    {
        lv_group_del(btn_group);
        btn_group = NULL;
    }

    if (NULL != spotWelderPageUI)
    {
        lv_obj_del(spotWelderPageUI);
        spotWelderPageUI = NULL;
        spotWelderUIObj.mainButtonUI = NULL;
    }
}

static void ui_enable_switch_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        bool isEnable = lv_obj_has_state(target, LV_STATE_CHECKED); // 返回 bool 类型， 开-1 ； 关-2
        stopWelderModel.workState = isEnable == true ? ENABLE_STATE_OPEN : ENABLE_STATE_CLOSE;
    }
}

void ui_updateSignalData(void)
{
    if (NULL == spotWelderPageUI)
    {
        return;
    }
    lv_label_set_text_fmt(ui_voltage, "%.2fV", stopWelderModel.voltage / 1000.0);
    lv_label_set_text_fmt(ui_count, "%d", stopWelderModel.count);
}

void ui_updateSignalWorkState(void)
{
    if (NULL == spotWelderPageUI)
    {
        return;
    }

    if (ENABLE_STATE_CLOSE == stopWelderModel.workState)
    {
        lv_obj_clear_state(ui_enableSwitch, LV_STATE_CHECKED); // 关
    }
    else
    {
        lv_obj_add_state(ui_enableSwitch, LV_STATE_CHECKED); // 开
    }
}

static void ui_bnt_obj_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_PRESSED)
    {
        if (target == ui_pulseWidth_0)
        {
            stopWelderModel.utilConfig.pulseWidth_0 = lv_numberbtn_get_value(ui_pulseWidth_0) * 10;
        }
        else if (target == ui_pulseWidth_1)
        {
            stopWelderModel.utilConfig.pulseWidth_1 = lv_numberbtn_get_value(ui_pulseWidth_1) * 10;
        }
        else if (target == ui_triggerDeyal)
        {
            stopWelderModel.utilConfig.interval = lv_numberbtn_get_value(ui_triggerDeyal);
        }
        else if (target == ui_manualTrigger)
        {
            stopWelderModel.manualTriggerFlag = 1;
        }
    }
}

static void ui_set_obj_change(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        if (target == ui_enableSwitch)
        {
            bool isEnable = lv_obj_has_state(target, LV_STATE_CHECKED); // 返回 bool 类型， 开-1 ； 关-2
            stopWelderModel.workState = isEnable == true ? ENABLE_STATE_OPEN : ENABLE_STATE_CLOSE;
        }
        else if (target == ui_modeSwitch)
        {

            bool isEnable = lv_obj_has_state(target, LV_STATE_CHECKED); // 返回 bool 类型， 开-1 ； 关-2
            stopWelderModel.utilConfig.mode = isEnable == true ? SPOTWELDER_MODE_DOUBLE : SPOTWELDER_MODE_SINGLE;
            setMode();
        }
    }
}

FE_UI_OBJ spotWelderUIObj = {spotWelderPageUI, spotWelderPageUI_init,
                             spotWelderPageUI_release, spotWelderPageUI_focused};

#endif