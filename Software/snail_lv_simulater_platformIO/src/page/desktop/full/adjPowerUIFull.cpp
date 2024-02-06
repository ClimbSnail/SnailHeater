#include "./ui.h"
#include "../desktop_model.h"

#ifdef FULL_UI

static lv_timer_t *adjPowerTimer = NULL;
static lv_obj_t *adjPowerPageUI = NULL;
static lv_obj_t *ui_voltageLabel;
static lv_obj_t *ui_voltSlider;
static lv_obj_t *ui_currentLabel;
static lv_obj_t *ui_settingVoltageButton;
static lv_obj_t *ui_settingCurrentButton;
static lv_obj_t *ui_capacityLabel;
static lv_obj_t *ui_enableSwitch;
static lv_obj_t *ui_modeButton;
static lv_obj_t *ui_modeLable;

static lv_obj_t *ui_quickSetupVolButton0;
static lv_obj_t *ui_quickSetupVolButton1;
static lv_obj_t *ui_quickSetupVolButton2;

static lv_group_t *btn_group = NULL;

static void ui_set_slider_changed(lv_event_t *e);
static void ui_set_vol_cur_change(lv_event_t *e);
static void ui_enable_switch_pressed(lv_event_t *e);
static void ui_mode_bnt_pressed(lv_event_t *e);
static void ui_bnt_obj_pressed(lv_event_t *e);
void ui_updateAdjPowerData(void);
static void adjPowerPageUI_release();

static void adjPowerPageUI_focused(lv_event_t *e)
{
    lv_indev_set_group(knobs_indev, btn_group);
    if (adjPowerModel.workState == 1)
    {
        lv_group_focus_obj(ui_enableSwitch);
    }
    else
    {
        lv_group_focus_obj(ui_backBtn);
    }
}

static void adjPowerTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    ui_updateAdjPowerData();
}

static bool adjPowerPageUI_init(lv_obj_t *father)
{
    adjPowerPageUI_release();

    top_layer_set_name();
    theme_color_init();

    adjPowerPageUI = lv_btn_create(father);
    adjPowerUIObj.mainButtonUI = adjPowerPageUI;
    lv_obj_t *ui_ButtonTmp = adjPowerPageUI;

    lv_obj_remove_style_all(ui_ButtonTmp);
    lv_obj_set_size(ui_ButtonTmp, EACH_PAGE_SIZE_X, EACH_PAGE_SIZE_Y);
    lv_obj_center(ui_ButtonTmp);
    lv_obj_add_flag(ui_ButtonTmp, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_ButtonTmp, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_style_bg_color(adjPowerPageUI, IS_WHITE_THEME ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_ButtonTmp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *lb1 = lv_label_create(ui_ButtonTmp);
    lv_label_set_text(lb1, TEXT_TEMP_POWER_VOL_CHANNEL_MD);
    lv_obj_align(lb1, LV_ALIGN_TOP_LEFT, 10, 12);
    lv_obj_add_style(lb1, &label_text_style, 0);

    ui_quickSetupVolButton0 = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_quickSetupVolButton0);
    lv_obj_set_size(ui_quickSetupVolButton0, 38, 20);
    // lv_obj_align(ui_quickSetupVolButton0, LV_ALIGN_TOP_LEFT, 74, 10);
    lv_obj_align_to(ui_quickSetupVolButton0, lb1,
                    LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_add_flag(ui_quickSetupVolButton0, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_quickSetupVolButton0, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_quickSetupVolButton0, &btn_type1_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_quickSetupVolButton0, &btn_type1_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_quickSetupVolLabel1 = lv_label_create(ui_quickSetupVolButton0);
    lv_obj_set_align(ui_quickSetupVolLabel1, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_quickSetupVolLabel1, "%.1lf",
                          adjPowerModel.utilConfig.quickSetupVoltage_0 / 1000.0);

    ui_quickSetupVolButton1 = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_quickSetupVolButton1);
    lv_obj_set_size(ui_quickSetupVolButton1, 38, 20);
    // lv_obj_align(ui_quickSetupVolButton1, LV_ALIGN_TOP_LEFT, 122, 10);
    lv_obj_align_to(ui_quickSetupVolButton1, ui_quickSetupVolButton0,
                    LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_add_flag(ui_quickSetupVolButton1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_quickSetupVolButton1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_quickSetupVolButton1, &btn_type1_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_quickSetupVolButton1, &btn_type1_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_quickSetupVolLabel2 = lv_label_create(ui_quickSetupVolButton1);
    lv_obj_set_align(ui_quickSetupVolLabel2, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_quickSetupVolLabel2, "%.1lf",
                          adjPowerModel.utilConfig.quickSetupVoltage_1 / 1000.0);

    ui_quickSetupVolButton2 = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_quickSetupVolButton2);
    lv_obj_set_size(ui_quickSetupVolButton2, 38, 20);
    // lv_obj_align(ui_quickSetupVolButton2, LV_ALIGN_TOP_LEFT, 170, 10);
    lv_obj_align_to(ui_quickSetupVolButton2, ui_quickSetupVolButton1,
                    LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_add_flag(ui_quickSetupVolButton2, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_quickSetupVolButton2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_quickSetupVolButton2, &btn_type1_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_quickSetupVolButton2, &btn_type1_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_quickSetupVolLabel3 = lv_label_create(ui_quickSetupVolButton2);
    lv_obj_set_align(ui_quickSetupVolLabel3, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_quickSetupVolLabel3, "%.1lf",
                          adjPowerModel.utilConfig.quickSetupVoltage_2 / 1000.0);

    ui_voltageLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_voltageLabel, 140, 52);
    lv_obj_align(ui_voltageLabel, LV_ALIGN_TOP_LEFT, 5, 45);
    lv_label_set_text_fmt(ui_voltageLabel, "%.2lf", adjPowerModel.voltage / 1000.0);
    lv_obj_set_style_text_align(ui_voltageLabel, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_voltageLabel, ADJ_POWER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_voltageLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_voltageLabel, &FontRoboto_52, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_voltageUnitLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(ui_voltageUnitLabel, ui_voltageLabel,
                    LV_ALIGN_OUT_RIGHT_MID, 0, -5);
    lv_label_set_text(ui_voltageUnitLabel, "V");
    lv_obj_set_style_text_color(ui_voltageUnitLabel, lv_color_hex(0x999798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_voltageUnitLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_voltageUnitLabel, &FontRoboto_36, LV_PART_MAIN | LV_STATE_DEFAULT);

    //     // 电压调节滑杆
    //     ui_voltSlider = lv_slider_create(ui_ButtonTmp);
    //     lv_obj_set_size(ui_voltSlider, 70, 10);
    //     lv_obj_align_to(ui_voltSlider, ui_voltageUnitLabel,
    //                     LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    // #if SH_HARDWARE_VER == SH_ESP32S2_WROOM_V26
    //     // 一二车250mv 77 三车100mv 31
    //     lv_slider_set_range(ui_voltSlider, 0, DAC_DEFAULT_RESOLUTION - 31);
    // #elif SH_HARDWARE_VER < SH_ESP32S2_WROOM_V26
    //     lv_slider_set_range(ui_voltSlider, 0, DAC_DEFAULT_RESOLUTION - 77);
    // #endif
    //     lv_obj_set_style_bg_color(ui_voltSlider, ADJ_POWER_THEME_COLOR1, LV_PART_KNOB | LV_STATE_DEFAULT);
    //     lv_obj_set_style_bg_color(ui_voltSlider, lv_color_hex(0x999999), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    //     lv_slider_set_value(ui_voltSlider,
    //                         DAC_DEFAULT_RESOLUTION - adjPowerModel.utilConfig.volDacValue,
    //                         LV_ANIM_OFF);

    // 设定的电压
    lv_obj_t *ui_settingVoltageUnitLabel = lv_label_create(ui_ButtonTmp);
#if SH_HARDWARE_VER == SH_ESP32S2_WROOM_V26
    lv_obj_align(ui_settingVoltageUnitLabel, LV_ALIGN_RIGHT_MID, -30, -30);
#elif SH_HARDWARE_VER == SH_ESP32S3_FN8_V27
    lv_obj_align(ui_settingVoltageUnitLabel, LV_ALIGN_RIGHT_MID, -30, -30);
#else
    lv_obj_align(ui_settingVoltageUnitLabel, LV_ALIGN_RIGHT_MID, -10, -30);
#endif
    lv_label_set_text(ui_settingVoltageUnitLabel, "V");
    lv_obj_set_style_text_color(ui_settingVoltageUnitLabel, lv_color_hex(0x999798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_settingVoltageUnitLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_settingVoltageUnitLabel, &FontRoboto_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_settingVoltageButton = lv_numberbtn_create(ui_ButtonTmp);

    lv_obj_t *settingVoltageLabel = lv_label_create(ui_settingVoltageButton);
    lv_numberbtn_set_label_and_format(ui_settingVoltageButton,
                                      settingVoltageLabel, "%.2lf", 0.05);
    lv_numberbtn_set_range(ui_settingVoltageButton, 0.6, ADJ_POWER_VOL_MAX / 1000.0);
    lv_numberbtn_set_value(ui_settingVoltageButton, adjPowerModel.utilConfig.settingVoltage / 1000.0);
    lv_numberbtn_set_align(ui_settingVoltageButton, LV_ALIGN_RIGHT_MID);
    lv_obj_remove_style_all(ui_settingVoltageButton);
    lv_obj_set_size(ui_settingVoltageButton, 95, 30);
    lv_obj_align_to(ui_settingVoltageButton, ui_settingVoltageUnitLabel,
                    LV_ALIGN_OUT_LEFT_BOTTOM, 0, 0);
    lv_obj_add_flag(ui_settingVoltageButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_settingVoltageButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_settingVoltageButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_settingVoltageButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_settingVoltageButton, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_settingVoltageButton, &FontRoboto_36, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_settingVoltageButton, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_settingVoltageButton, &btn_type1_pressed_style, LV_STATE_EDITED);

    // 实际电流
    ui_currentLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_currentLabel, 140, 55);
    lv_obj_align_to(ui_currentLabel, ui_voltageLabel,
                    LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_label_set_text_fmt(ui_currentLabel, "%.2lf", adjPowerModel.current / 1000.0);
    lv_obj_set_style_text_align(ui_currentLabel, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_currentLabel, ADJ_POWER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_currentLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_currentLabel, &FontRoboto_52, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_currentUnitLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(ui_currentUnitLabel, ui_currentLabel,
                    LV_ALIGN_OUT_RIGHT_MID, 0, -5);
    lv_label_set_text(ui_currentUnitLabel, "A");
    lv_obj_set_style_text_color(ui_currentUnitLabel, lv_color_hex(0x999798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_currentUnitLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_currentUnitLabel, &FontRoboto_36, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 设定的电流
    lv_obj_t *ui_settingCurrentUnitLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(ui_settingCurrentUnitLabel, ui_settingVoltageUnitLabel,
                    LV_ALIGN_OUT_BOTTOM_MID, 5, 40);
    lv_label_set_text(ui_settingCurrentUnitLabel, "A");
    lv_obj_set_style_text_color(ui_settingCurrentUnitLabel, lv_color_hex(0x999798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_settingCurrentUnitLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_settingCurrentUnitLabel, &FontRoboto_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_settingCurrentButton = lv_numberbtn_create(ui_ButtonTmp);

    lv_obj_t *settingCurrentLabel = lv_label_create(ui_settingCurrentButton);
    lv_numberbtn_set_label_and_format(ui_settingCurrentButton,
                                      settingCurrentLabel, "%.2lf", 0.01);
    lv_numberbtn_set_range(ui_settingCurrentButton, 0, ADJ_POWER_CUR_MAX / 1000.0);
    lv_numberbtn_set_value(ui_settingCurrentButton, adjPowerModel.utilConfig.settingCurrent / 1000.0);
    lv_numberbtn_set_align(ui_settingCurrentButton, LV_ALIGN_RIGHT_MID);
    lv_obj_remove_style_all(ui_settingCurrentButton);
    lv_obj_set_size(ui_settingCurrentButton, 80, 30);
    lv_obj_align_to(ui_settingCurrentButton, ui_settingCurrentUnitLabel,
                    LV_ALIGN_OUT_LEFT_BOTTOM, 0, 0);
    lv_obj_add_flag(ui_settingCurrentButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_settingCurrentButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_settingCurrentButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_settingCurrentButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_settingCurrentButton, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_settingCurrentButton, &FontRoboto_36, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_settingCurrentButton, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_settingCurrentButton, &btn_type1_pressed_style, LV_STATE_EDITED);

    // 功率
    ui_capacityLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_capacityLabel, 140, 52);
    lv_obj_align_to(ui_capacityLabel, ui_currentLabel,
                    LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_label_set_text_fmt(ui_capacityLabel, "%.2lf", adjPowerModel.capacity / 1000000.0);
    lv_obj_set_style_text_align(ui_capacityLabel, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_capacityLabel, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_capacityLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_capacityLabel, &FontRoboto_52, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_capacityUnitLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(ui_capacityUnitLabel, ui_capacityLabel,
                    LV_ALIGN_OUT_RIGHT_MID, 0, -5);
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
    lv_obj_align_to(ui_enableSwitch, ui_settingCurrentUnitLabel,
                    LV_ALIGN_OUT_BOTTOM_MID, -35, 25);
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
    lv_obj_set_style_bg_color(ui_enableSwitch, ADJ_POWER_THEME_COLOR1, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_outline_opa(ui_enableSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_enableSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    lv_obj_add_event_cb(ui_quickSetupVolButton0, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_quickSetupVolButton1, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_quickSetupVolButton2, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    // lv_obj_add_event_cb(ui_voltSlider, ui_set_slider_changed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_settingVoltageButton, ui_set_vol_cur_change, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_settingCurrentButton, ui_set_vol_cur_change, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_enableSwitch, ui_enable_switch_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    // lv_obj_add_event_cb(ui_modeButton, ui_mode_bnt_pressed, LV_EVENT_PRESSED, NULL);

    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_backBtn);
    lv_group_add_obj(btn_group, ui_quickSetupVolButton0);
    lv_group_add_obj(btn_group, ui_quickSetupVolButton1);
    lv_group_add_obj(btn_group, ui_quickSetupVolButton2);
    // lv_group_add_obj(btn_group, ui_voltSlider);
    lv_group_add_obj(btn_group, ui_settingVoltageButton);
    lv_group_add_obj(btn_group, ui_settingCurrentButton);
    lv_group_add_obj(btn_group, ui_enableSwitch);
    // lv_group_add_obj(btn_group, ui_modeButton);
    lv_indev_set_group(knobs_indev, btn_group);
    if (adjPowerModel.workState == 1)
    {
        lv_group_focus_obj(ui_enableSwitch);
    }
    else
    {
        lv_group_focus_obj(ui_backBtn);
    }

    adjPowerTimer = lv_timer_create(adjPowerTimer_timeout, DATA_REFRESH_MS, NULL);
    lv_timer_set_repeat_count(adjPowerTimer, -1);
    return true;
}

static void adjPowerPageUI_release()
{
    if (NULL != adjPowerTimer)
    {
        lv_timer_del(adjPowerTimer);
        adjPowerTimer = NULL;
    }

    if (NULL != btn_group)
    {
        lv_group_del(btn_group);
        btn_group = NULL;
    }

    if (NULL != adjPowerPageUI)
    {
        lv_obj_del(adjPowerPageUI);
        adjPowerPageUI = NULL;
        adjPowerUIObj.mainButtonUI = NULL;
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

void ui_updateAdjPowerData(void)
{
    if (NULL == adjPowerPageUI)
    {
        return;
    }
    lv_label_set_text_fmt(ui_voltageLabel, "%.2lf", adjPowerModel.voltage / 1000.0);
    lv_label_set_text_fmt(ui_currentLabel, "%.2lf", adjPowerModel.current / 1000.0);
    if (adjPowerModel.capacity > 100000000)
    {
        lv_label_set_text_fmt(ui_capacityLabel, "%.1lf", adjPowerModel.capacity / 1000000.0);
    }
    else
    {
        lv_label_set_text_fmt(ui_capacityLabel, "%.2lf", adjPowerModel.capacity / 1000000.0);
    }
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

static void ui_bnt_obj_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_PRESSED)
    {
        if (target == ui_quickSetupVolButton0)
        {
            adjPowerModel.utilConfig.settingVoltage =
                adjPowerModel.utilConfig.quickSetupVoltage_0;
            lv_numberbtn_set_value(ui_settingVoltageButton,
                                   adjPowerModel.utilConfig.settingVoltage / 1000.0);
        }
        else if (target == ui_quickSetupVolButton1)
        {
            adjPowerModel.utilConfig.settingVoltage =
                adjPowerModel.utilConfig.quickSetupVoltage_1;
            lv_numberbtn_set_value(ui_settingVoltageButton,
                                   adjPowerModel.utilConfig.settingVoltage / 1000.0);
        }
        else if (target == ui_quickSetupVolButton2)
        {
            adjPowerModel.utilConfig.settingVoltage =
                adjPowerModel.utilConfig.quickSetupVoltage_2;
            lv_numberbtn_set_value(ui_settingVoltageButton,
                                   adjPowerModel.utilConfig.settingVoltage / 1000.0);
        }
    }
}

static void ui_set_vol_cur_change(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        if (target == ui_settingVoltageButton)
        {
            adjPowerModel.utilConfig.settingVoltage =
                lv_numberbtn_get_value(ui_settingVoltageButton) * 1000;
        }
        else if (target == ui_settingCurrentButton)
        {
            adjPowerModel.utilConfig.settingCurrent =
                lv_numberbtn_get_value(ui_settingCurrentButton) * 1000;
        }
    }
}

FE_UI_OBJ adjPowerUIObj = {adjPowerPageUI, adjPowerPageUI_init,
                           adjPowerPageUI_release, adjPowerPageUI_focused};

#endif