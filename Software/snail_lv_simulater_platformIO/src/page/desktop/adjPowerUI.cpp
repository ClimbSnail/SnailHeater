#include "ui.h"
#include "desktop_model.h"

#define FONT_DEBUG 1

static lv_obj_t *adjPowerPageUI = NULL;
static lv_obj_t *ui_voltageLabel;
static lv_obj_t *ui_voltSlider;
static lv_obj_t *ui_currentLabel;
static lv_obj_t *ui_capacityLabel;
static lv_obj_t *ui_enableSwitch;
static lv_obj_t *ui_heatplatformTypeDropdown;
static lv_obj_t *ui_heatplatformNumLabel;
static lv_obj_t *ui_heatplatformDropdown;
static lv_obj_t *ui_moreButton;
static lv_obj_t *ui_moreButtonLabel;
static lv_obj_t *ui_backButton;
static lv_obj_t *ui_backImage;
static lv_obj_t *ui_backButtonLabel;
static lv_obj_t *ui_powerBar;
static lv_style_t chFontStyle;

static lv_group_t *btn_group;
static lv_group_t *setTempArcGroup;

static void ui_set_slider_changed(lv_event_t *e);
static void ui_back_btn_pressed(lv_event_t *e);
static void ui_enable_switch_pressed(lv_event_t *e);

static bool adjPowerPageUI_init(lv_obj_t *father)
{
    if (NULL != adjPowerPageUI)
    {
        return false;
    }

    adjPowerPageUI = lv_btn_create(father); // 紫色
    adjPowerUIObj.mainButtonUI = adjPowerPageUI;

    lv_obj_set_size(adjPowerPageUI, 110, 200);
    // lv_obj_set_pos(adjPowerPageUI, btnPosXY[2][0], btnPosXY[2][1]);
    lv_obj_set_pos(adjPowerPageUI, START_UI_OBJ_X, 0);
    lv_obj_set_align(adjPowerPageUI, LV_ALIGN_CENTER);
    lv_obj_add_flag(adjPowerPageUI, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(adjPowerPageUI, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_bg_color(adjPowerPageUI, lv_color_hex(0x959595), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(adjPowerPageUI, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(adjPowerPageUI, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_PRESSED);
    lv_obj_set_style_border_opa(adjPowerPageUI, 255, LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_PRESSED);

    lv_obj_t *ui_ButtonTmp = adjPowerPageUI;

    // 中文字体
    lv_style_init(&chFontStyle);
    lv_style_set_text_opa(&chFontStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_style_set_text_color(&chFontStyle, lv_color_hex(0x1D1517));
    lv_style_set_text_font(&chFontStyle, &sh_ch_font_14);

    ui_voltageLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_voltageLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_voltageLabel, 0, -70);
    lv_obj_set_align(ui_voltageLabel, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_voltageLabel, "%.2lf",
                          adjPowerModel.voltage / 1000.0);
    lv_obj_set_style_text_color(ui_voltageLabel, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_voltageLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_voltageLabel, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 电压调节滑杆
    ui_voltSlider = lv_slider_create(ui_ButtonTmp);
    lv_obj_set_size(ui_voltSlider, 80, 6);
    lv_obj_set_pos(ui_voltSlider, 0, -30);
    lv_slider_set_range(ui_voltSlider, 0, DAC_DEFAULT_RESOLUTION);
    lv_slider_set_value(ui_voltSlider,
                        DAC_DEFAULT_RESOLUTION - adjPowerModel.adcValue,
                        LV_ANIM_OFF);
    lv_obj_set_align(ui_voltSlider, LV_ALIGN_CENTER);
    // lv_obj_add_flag(ui_voltSlider, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    // lv_obj_clear_flag(ui_voltSlider, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_bg_color(ui_voltSlider, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_voltSlider, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_voltSlider, &focused_style, LV_STATE_FOCUSED);

    // 电流
    ui_currentLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_currentLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_currentLabel, 0, 0);
    lv_obj_set_align(ui_currentLabel, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_currentLabel, "%dmA", adjPowerModel.current);
    lv_obj_set_style_text_color(ui_currentLabel, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_currentLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_currentLabel, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 功率
    ui_capacityLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_capacityLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_capacityLabel, 0, 25);
    lv_obj_set_align(ui_capacityLabel, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_capacityLabel, "%.2lfW", adjPowerModel.capacity / 1000000.0);
    lv_obj_set_style_text_color(ui_capacityLabel, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_capacityLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_capacityLabel, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 使能开关
    ui_enableSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_set_size(ui_enableSwitch, 40, 20);
    lv_obj_set_pos(ui_enableSwitch, 0, 50);
    lv_obj_set_align(ui_enableSwitch, LV_ALIGN_CENTER);
    if (adjPowerModel.workState == ADJ_POWER_OPEN_STATE_OPEN)
    {
        lv_obj_add_state(ui_enableSwitch, LV_STATE_CHECKED); // 开
    }
    else
    {
        lv_obj_clear_state(ui_enableSwitch, LV_STATE_CHECKED); // 关
    }
    lv_obj_add_style(ui_enableSwitch, &focused_style, LV_STATE_FOCUSED);

    ui_moreButton = lv_btn_create(ui_ButtonTmp);
    lv_obj_set_size(ui_moreButton, 32, 18);
    lv_obj_set_pos(ui_moreButton, -35, 78);
    lv_obj_set_align(ui_moreButton, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_moreButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(ui_moreButton, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_bg_color(ui_moreButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_moreButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_moreButton, &focused_style, LV_STATE_FOCUSED);

    ui_moreButtonLabel = lv_label_create(ui_moreButton);
    lv_obj_set_size(ui_moreButtonLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_moreButtonLabel, 0, -5);
    lv_obj_set_align(ui_moreButtonLabel, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_moreButtonLabel, "...");
    lv_obj_set_style_text_color(ui_moreButtonLabel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_moreButtonLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_moreButtonLabel, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_backButton = lv_btn_create(ui_ButtonTmp);
    lv_obj_set_size(ui_backButton, 32, 18);
    lv_obj_set_pos(ui_backButton, 35, 78);
    lv_obj_set_align(ui_backButton, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_backButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(ui_backButton, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_bg_color(ui_backButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_backButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_backButton, &focused_style, LV_STATE_FOCUSED);

    // 返回图标
    ui_backImage = lv_img_create(ui_backButton);
    lv_img_set_src(ui_backImage, &back_16);
    lv_obj_set_align(ui_backImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_backImage, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_backImage, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    // ui_backButtonLabel = lv_label_create(ui_backButton);
    // lv_obj_set_size(ui_backButtonLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    // lv_obj_set_pos(ui_backButtonLabel, 0, 0);
    // lv_obj_set_align(ui_backButtonLabel, LV_ALIGN_CENTER);
    // lv_label_set_text_fmt(ui_backButtonLabel, "Back");
    // lv_obj_set_style_text_color(ui_backButtonLabel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_opa(ui_backButtonLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(ui_backButtonLabel, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_powerBar = lv_bar_create(ui_ButtonTmp);
    lv_obj_set_size(ui_powerBar, 105, 8);
    lv_obj_set_pos(ui_powerBar, 0, 97);
    lv_obj_set_align(ui_powerBar, LV_ALIGN_CENTER);
    lv_bar_set_range(ui_powerBar, 0, 100);
    lv_bar_set_value(ui_powerBar, 0, LV_ANIM_ON);
    lv_obj_set_style_bg_color(ui_powerBar, lv_color_hex(0xFF0000), LV_PART_INDICATOR);

    lv_obj_add_event_cb(ui_backButton, ui_back_btn_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_voltSlider, ui_set_slider_changed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_enableSwitch, ui_enable_switch_pressed, LV_EVENT_VALUE_CHANGED, NULL);

    setTempArcGroup = lv_group_create();
    return true;
}

static void adjPowerPageUI_release()
{
    if (NULL == adjPowerPageUI)
    {
        return;
    }
    lv_obj_clean(adjPowerPageUI);
    adjPowerPageUI = NULL;
    adjPowerUIObj.mainButtonUI = NULL;
}

static void ui_back_btn_pressed(lv_event_t *e)
{
    // 返回项被按下
    lv_group_del(btn_group);
    ui_main_pressed(e);
}

static void adjPowerPageUI_pressed(lv_event_t *e)
{
    // 创建操作的组
    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_voltSlider);
    lv_group_add_obj(btn_group, ui_enableSwitch);
    lv_group_add_obj(btn_group, ui_moreButton);
    lv_group_add_obj(btn_group, ui_backButton);
    lv_group_focus_obj(ui_backButton); // 聚焦到退出按键

    lv_indev_set_group(knobs_indev, btn_group);
}

static void ui_enable_switch_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {

        bool isEnable = lv_obj_has_state(target, LV_STATE_CHECKED); // 返回 bool 类型， 开-1 ； 关-2
        adjPowerModel.workState = isEnable == true ? ADJ_POWER_OPEN_STATE_OPEN : ADJ_POWER_OPEN_STATE_CLOSE;
    }
}

void ui_updateAdjPowerCurVoltage(void)
{
    if (NULL == adjPowerPageUI)
    {
        return;
    }
    lv_label_set_text_fmt(ui_voltageLabel, "%.2lf",
                          adjPowerModel.voltage / 1000.0);
    lv_label_set_text_fmt(ui_currentLabel, "%dmA", adjPowerModel.current);
    lv_label_set_text_fmt(ui_capacityLabel, "%.2lfW", adjPowerModel.capacity / 1000000.0);
}

void ui_updateAdjPowerWorkState(void)
{
    if (NULL == adjPowerPageUI)
    {
        return;
    }

    if (ADJ_POWER_OPEN_STATE_CLOSE == adjPowerModel.workState)
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
        adjPowerModel.adcValue = DAC_DEFAULT_RESOLUTION - (int)lv_slider_get_value(ui_voltSlider);
    }
}

FE_UI_OBJ adjPowerUIObj = {adjPowerPageUI, adjPowerPageUI_init,
                           adjPowerPageUI_release, adjPowerPageUI_pressed};