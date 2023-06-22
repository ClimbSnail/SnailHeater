#include "./ui.h"
#include "../desktop_model.h"

#ifdef FULL_UI

static lv_timer_t *adjPowerTimer = NULL;
static lv_obj_t *adjPowerPageUI = NULL;
static lv_obj_t *ui_voltageLabel;
static lv_obj_t *ui_voltSlider;
static lv_obj_t *ui_voltageUnitLabel;
static lv_obj_t *ui_currentLabel;
static lv_obj_t *ui_currentUnitLabel;
static lv_obj_t *ui_settingCurrentButton;
static lv_obj_t *ui_settingCurrentLabel;
static lv_obj_t *ui_settingCurrentUnitLabel;
static lv_obj_t *ui_settingCurrentArc = NULL;
static lv_group_t *ui_setArcGroup = NULL;
static lv_obj_t *ui_capacityLabel;
static lv_obj_t *ui_capacityUnitLabel;
static lv_obj_t *ui_enableSwitch;
static lv_obj_t *ui_modeButton;
static lv_obj_t *ui_modeLable;

static lv_group_t *btn_group = NULL;

static void ui_set_slider_changed(lv_event_t *e);
static void ui_set_cur_pressed(lv_event_t *e);
static void ui_enable_switch_pressed(lv_event_t *e);
static void ui_mode_bnt_pressed(lv_event_t *e);

static void adjPowerPageUI_focused(lv_event_t *e)
{
    if (adjPowerModel.workState == 1)
    {
        lv_group_focus_obj(ui_enableSwitch);
    }
    else
    {
        lv_group_focus_obj(ui_backBtn);
    }
    lv_indev_set_group(knobs_indev, btn_group);
}

static void adjPowerTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    // 更新电压电流功率的函数
    lv_label_set_text_fmt(ui_voltageLabel, "%.2lf", adjPowerModel.voltage / 1000.0);
    lv_label_set_text_fmt(ui_currentLabel, "%.2lf", adjPowerModel.current / 1000.0);
    lv_label_set_text_fmt(ui_capacityLabel, "%.2lf", adjPowerModel.capacity / 1000000.0);
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
    // lv_obj_set_style_bg_color(adjPowerPageUI, IS_WHITE_THEME ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(adjPowerPageUI, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_ButtonTmp = adjPowerPageUI;

    ui_voltageLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_voltageLabel, 140, 52);
    lv_obj_align(ui_voltageLabel, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_label_set_text_fmt(ui_voltageLabel, "%.2lf", adjPowerModel.voltage / 1000.0);
    lv_obj_set_style_text_align(ui_voltageLabel, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_voltageLabel, ADJ_POWER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_voltageLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_voltageLabel, &FontRoboto_52, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_voltageUnitLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_voltageUnitLabel, LV_ALIGN_TOP_LEFT, 142, 30);
    lv_label_set_text(ui_voltageUnitLabel, "V");
    lv_obj_set_style_text_color(ui_voltageUnitLabel, lv_color_hex(0x999798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_voltageUnitLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_voltageUnitLabel, &FontRoboto_36, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 电压调节滑杆
    ui_voltSlider = lv_slider_create(ui_ButtonTmp);
    lv_obj_set_size(ui_voltSlider, 70, 10);
    lv_obj_align(ui_voltSlider, LV_ALIGN_TOP_LEFT, 190, 40);
    lv_slider_set_range(ui_voltSlider, 0, DAC_DEFAULT_RESOLUTION);
    lv_obj_set_style_bg_color(ui_voltSlider, ADJ_POWER_THEME_COLOR1, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_voltSlider, lv_color_hex(0x999999), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_slider_set_value(ui_voltSlider,
                        DAC_DEFAULT_RESOLUTION - adjPowerModel.utilConfig.volDacValue,
                        LV_ANIM_OFF);

    // lv_obj_add_flag(ui_voltSlider, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    // 实际电流
    ui_currentLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_currentLabel, 140, 52);
    lv_obj_align(ui_currentLabel, LV_ALIGN_TOP_LEFT, 0, 80);
    lv_label_set_text_fmt(ui_currentLabel, "%.2lf", adjPowerModel.current / 1000.0);
    lv_obj_set_style_text_align(ui_currentLabel, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_currentLabel, ADJ_POWER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_currentLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_currentLabel, &FontRoboto_52, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_currentUnitLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_currentUnitLabel, LV_ALIGN_TOP_LEFT, 142, 90);
    lv_label_set_text(ui_currentUnitLabel, "A");
    lv_obj_set_style_text_color(ui_currentUnitLabel, lv_color_hex(0x999798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_currentUnitLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_currentUnitLabel, &FontRoboto_36, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 设定的电流
    ui_settingCurrentButton = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_settingCurrentButton);
    lv_obj_set_size(ui_settingCurrentButton, 100, 30);
    lv_obj_align(ui_settingCurrentButton, LV_ALIGN_CENTER, 80, -5);
    lv_obj_add_flag(ui_settingCurrentButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_settingCurrentButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_settingCurrentButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_settingCurrentButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_settingCurrentButton, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_settingCurrentButton, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_settingCurrentButton, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_settingCurrentButton, &btn_type1_pressed_style, LV_STATE_EDITED);

    ui_settingCurrentLabel = lv_label_create(ui_settingCurrentButton);
    // lv_obj_set_size(ui_settingCurrentLabel, -40, 0);
    lv_obj_align(ui_settingCurrentLabel, LV_ALIGN_CENTER, -5, 0);
    lv_label_set_text_fmt(ui_settingCurrentLabel, "%.2lf", adjPowerModel.utilConfig.settingCurrent / 1000.0);
    lv_obj_set_style_text_align(ui_settingCurrentLabel, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_settingCurrentLabel, ADJ_POWER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_settingCurrentLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_settingCurrentLabel, &FontRoboto_36, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_settingCurrentUnitLabel = lv_label_create(ui_settingCurrentButton);
    lv_obj_align(ui_settingCurrentUnitLabel, LV_ALIGN_CENTER, 40, 3);
    lv_label_set_text(ui_settingCurrentUnitLabel, "A");
    lv_obj_set_style_text_color(ui_settingCurrentUnitLabel, lv_color_hex(0x999798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_settingCurrentUnitLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_settingCurrentUnitLabel, &FontRoboto_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 功率
    ui_capacityLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_capacityLabel, 140, 52);
    lv_obj_align(ui_capacityLabel, LV_ALIGN_TOP_LEFT, 0, 140);
    lv_label_set_text_fmt(ui_capacityLabel, "%.2lf", adjPowerModel.capacity / 1000000.0);
    lv_obj_set_style_text_align(ui_capacityLabel, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_capacityLabel, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_capacityLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_capacityLabel, &FontRoboto_52, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_capacityUnitLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_capacityUnitLabel, LV_ALIGN_TOP_LEFT, 142, 150);
    lv_label_set_text(ui_capacityUnitLabel, "W");
    lv_obj_set_style_text_color(ui_capacityUnitLabel, lv_color_hex(0x999798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_capacityUnitLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_capacityUnitLabel, &FontRoboto_36, LV_PART_MAIN | LV_STATE_DEFAULT);

    // // 模式按钮
    // ui_modeButton = lv_btn_create(ui_ButtonTmp);
    // lv_obj_set_size(ui_modeButton, 30, 25);
    // lv_obj_align(ui_modeButton, LV_ALIGN_CENTER, 100, 90);
    // lv_obj_set_style_bg_color(ui_modeButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_modeButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // // 模式标签
    // ui_modeLable = lv_label_create(ui_modeButton);
    // lv_obj_set_size(ui_modeLable, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    // lv_obj_set_pos(ui_modeLable, 0, 0);
    // lv_obj_set_align(ui_modeLable, LV_ALIGN_CENTER);
    // if (ADJ_POWER_MODE_CV == adjPowerModel.utilConfig.mode)
    // {
    //     lv_label_set_text_fmt(ui_modeLable, "CV");
    // }
    // else if (ADJ_POWER_MODE_CC == adjPowerModel.utilConfig.mode)
    // {
    //     lv_label_set_text_fmt(ui_modeLable, "CC");
    // }
    // lv_obj_set_style_text_color(ui_modeLable, ADJ_POWER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_opa(ui_modeLable, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(ui_modeLable, &FontJost, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 使能开关
    ui_enableSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_add_flag(ui_enableSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_enableSwitch, 60, 30);
    lv_obj_align(ui_enableSwitch, LV_ALIGN_CENTER, 90, 50);
    if (ENABLE_STATE_OPEN == adjPowerModel.workState)
    {
        lv_obj_add_state(ui_enableSwitch, LV_STATE_CHECKED); // 开
    }
    else
    {
        lv_obj_clear_state(ui_enableSwitch, LV_STATE_CHECKED); // 关
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_enableSwitch, ADJ_POWER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_bg_color(ui_enableSwitch, ADJ_POWER_THEME_COLOR1, LV_PART_INDICATOR | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_enableSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_enableSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    lv_obj_add_event_cb(ui_voltSlider, ui_set_slider_changed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_settingCurrentButton, ui_set_cur_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_enableSwitch, ui_enable_switch_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    // lv_obj_add_event_cb(ui_modeButton, ui_mode_bnt_pressed, LV_EVENT_PRESSED, NULL);

    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_backBtn);
    lv_group_add_obj(btn_group, ui_voltSlider);
    lv_group_add_obj(btn_group, ui_settingCurrentButton);
    lv_group_add_obj(btn_group, ui_enableSwitch);
    // lv_group_add_obj(btn_group, ui_modeButton);
    if (adjPowerModel.workState == 1)
    {
        lv_group_focus_obj(ui_enableSwitch);
    }
    else
    {
        lv_group_focus_obj(ui_backBtn);
    }
    lv_indev_set_group(knobs_indev, btn_group);

    ui_setArcGroup = lv_group_create();

    adjPowerTimer = lv_timer_create(adjPowerTimer_timeout, DATA_REFRESH_MS, NULL);
    lv_timer_set_repeat_count(adjPowerTimer, -1);
    return true;
}

static void adjPowerPageUI_release()
{
    if (NULL != adjPowerPageUI)
    {
        lv_obj_del(adjPowerPageUI);
        adjPowerPageUI = NULL;
        adjPowerUIObj.mainButtonUI = NULL;
    }

    if (NULL != btn_group)
    {
        lv_group_del(btn_group);
        btn_group = NULL;
    }

    if (NULL != adjPowerTimer)
    {
        lv_timer_del(adjPowerTimer);
        adjPowerTimer = NULL;
    }
}

static void ui_mode_bnt_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_PRESSED)
    {
        // adjPowerModel.utilConfig.mode = (adjPowerModel.utilConfig.mode + 1) % ADJ_POWER_MODE_MAX_NUM;
        // if (ADJ_POWER_MODE_CV == adjPowerModel.utilConfig.mode)
        // {
        //     lv_label_set_text_fmt(ui_modeLable, "CV");
        //     lv_slider_set_value(ui_voltSlider,
        //                         DAC_DEFAULT_RESOLUTION - adjPowerModel.utilConfig.volDacValue,
        //                         LV_ANIM_OFF);
        // }
        // else if (ADJ_POWER_MODE_CC == adjPowerModel.utilConfig.mode)
        // {
        //     lv_label_set_text_fmt(ui_modeLable, "CC");
        //     lv_slider_set_value(ui_voltSlider,
        //                         adjPowerModel.utilConfig.settingCurrent,
        //                         LV_ANIM_OFF);
        // }
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
        adjPowerModel.utilConfig.volDacValue =
            DAC_DEFAULT_RESOLUTION - (int)lv_slider_get_value(ui_voltSlider);
    }
}

static void ui_set_curArc_changed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        lv_label_set_text_fmt(ui_settingCurrentLabel, "%.2lf", lv_arc_get_value(ui_settingCurrentArc) / 100.0);
    }
}

static void ui_set_curArc_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        adjPowerModel.utilConfig.settingCurrent = lv_arc_get_value(ui_settingCurrentArc) * 10;

        lv_group_focus_freeze(ui_setArcGroup, false);
        lv_indev_set_group(knobs_indev, btn_group);

        LV_LOG_USER("set_tempArc LV_EVENT_PRESSED % u", event_code);
        if (NULL != ui_settingCurrentArc)
        {
            lv_group_remove_obj(ui_settingCurrentArc);
            lv_obj_del(ui_settingCurrentArc);
            ui_settingCurrentArc = NULL;
        }
        lv_obj_clear_state(ui_settingCurrentButton, LV_STATE_EDITED);
    }
}

static void ui_set_cur_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        LV_LOG_USER("button_pressed LV_EVENT_PRESSED % u", event_code);

        // 给它增加一个状态
        lv_obj_add_state(ui_settingCurrentButton, LV_STATE_EDITED);

        if (NULL != ui_settingCurrentArc)
        {
            lv_group_remove_obj(ui_settingCurrentArc);
            lv_obj_del(ui_settingCurrentArc);
            ui_settingCurrentArc = NULL;
        }

        ui_settingCurrentArc = lv_arc_create(ui_settingCurrentButton);
        lv_obj_set_size(ui_settingCurrentArc, 30, 30);
        lv_obj_center(ui_settingCurrentArc);
        lv_arc_set_range(ui_settingCurrentArc, 0, 500); // 电流峰值5000ma 这里使用10ma为步进
        lv_arc_set_value(ui_settingCurrentArc, adjPowerModel.utilConfig.settingCurrent / 10);
        // 设置隐藏
        lv_obj_set_style_opa(ui_settingCurrentArc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_group_add_obj(ui_setArcGroup, ui_settingCurrentArc);
        lv_indev_set_group(knobs_indev, ui_setArcGroup);
        lv_group_focus_obj(ui_settingCurrentArc);

        lv_obj_add_event_cb(ui_settingCurrentArc, ui_set_curArc_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_settingCurrentArc, ui_set_curArc_changed, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

FE_UI_OBJ adjPowerUIObj = {adjPowerPageUI, adjPowerPageUI_init,
                           adjPowerPageUI_release, adjPowerPageUI_focused};

#endif