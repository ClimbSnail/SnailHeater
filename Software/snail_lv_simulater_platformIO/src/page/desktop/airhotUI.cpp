

#include "ui.h"
#include "desktop_model.h"

#define FONT_DEBUG 1

static lv_obj_t *airhotPageUI = NULL;
static lv_obj_t *ui_curTempLabel;
static lv_obj_t *ui_curTempCLabel;
static lv_obj_t *ui_setTempButton;
static lv_obj_t *ui_setTempLabel;
static lv_obj_t *ui_setTempArc;
static lv_obj_t *ui_airVoltageLabel;
static lv_obj_t *ui_airVoltageImage;
static lv_obj_t *ui_airVoltageDropdown;
static lv_obj_t *ui_airFanImage;
static lv_obj_t *ui_airDutyTextArea;
static lv_obj_t *ui_setAirDutyButton;
static lv_obj_t *ui_setAirDutyLabel;
static lv_obj_t *ui_setAirArc;
static lv_obj_t *ui_moreButton;
static lv_obj_t *ui_moreButtonLabel;
static lv_obj_t *ui_backButton;
static lv_obj_t *ui_backImage;
static lv_obj_t *ui_backButtonLabel;
static lv_obj_t *ui_powerBar;
static lv_style_t chFontStyle;

static lv_group_t *btn_group;
static lv_group_t *setTempArcGroup;
static lv_group_t *setAirArcGroup;

static void ui_vol_pressed(lv_event_t *e);
static void ui_set_temp_btn_pressed(lv_event_t *e);
static void ui_set_air_btn_pressed(lv_event_t *e);
static void ui_back_btn_pressed(lv_event_t *e);

static bool airhotPageUI_init(lv_obj_t *father)
{
    if (NULL != airhotPageUI)
    {
        return false;
    }

    airhotPageUI = lv_btn_create(father); // 绿色
    airhotUIObj.mainButtonUI = airhotPageUI;

    lv_obj_set_size(airhotPageUI, 110, 200);
    // lv_obj_set_pos(airhotPageUI, btnPosXY[1][0], btnPosXY[1][1]);
    lv_obj_set_pos(airhotPageUI, START_UI_OBJ_X, 0);
    lv_obj_set_align(airhotPageUI, LV_ALIGN_CENTER);
    lv_obj_add_flag(airhotPageUI, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(airhotPageUI, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_bg_color(airhotPageUI, lv_color_hex(0x9FDC2D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(airhotPageUI, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(airhotPageUI, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_PRESSED);
    lv_obj_set_style_border_opa(airhotPageUI, 255, LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_PRESSED);

    lv_obj_t *ui_ButtonTmp = airhotPageUI;

    // 中文字体
    lv_style_init(&chFontStyle);
    lv_style_set_text_opa(&chFontStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_style_set_text_color(&chFontStyle, lv_color_hex(0x1D1517));
    lv_style_set_text_font(&chFontStyle, &sh_ch_font_14);

    ui_curTempLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_curTempLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_curTempLabel, 0, -70);
    lv_obj_set_align(ui_curTempLabel, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_curTempLabel, "%d", airhotModel.curTemp);
    lv_obj_set_style_text_color(ui_curTempLabel, lv_color_hex(0x8168F7), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_curTempLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(ui_curTempLabel, &lv_font_montserrat_36, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_curTempLabel, &sh_number_50, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 温度调节
    ui_setTempButton = lv_btn_create(ui_ButtonTmp);
    lv_obj_set_size(ui_setTempButton, 65, 20);
    lv_obj_set_pos(ui_setTempButton, 0, -25);
    lv_obj_set_align(ui_setTempButton, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_setTempButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(ui_setTempButton, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_bg_color(ui_setTempButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_setTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_setTempButton, &focused_style, LV_STATE_FOCUSED);

    ui_setTempLabel = lv_label_create(ui_setTempButton);
    lv_obj_set_size(ui_setTempLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_setTempLabel, 0, 0);
    lv_obj_set_align(ui_setTempLabel, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_setTempLabel, "%d°C", airhotModel.targetTemp);
    lv_obj_set_style_text_color(ui_setTempLabel, lv_color_hex(0xF76889), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_setTempLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setTempLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    //     // 风枪风机电压调节
    //     ui_airVoltageLabel = lv_label_create(ui_ButtonTmp);
    //     lv_obj_set_size(ui_airVoltageLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    //     lv_obj_set_pos(ui_airVoltageLabel, -30, 12);
    //     lv_obj_set_align(ui_airVoltageLabel, LV_ALIGN_CENTER);
    //     // lv_label_set_text(ui_airVoltageLabel, "电压");
    // #if FONT_DEBUG == 1
    //     lv_obj_set_style_text_color(ui_airVoltageLabel, lv_color_hex(0x1D1517), LV_PART_MAIN | LV_STATE_DEFAULT);
    //     lv_obj_set_style_text_opa(ui_airVoltageLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    //     lv_obj_set_style_text_font(ui_airVoltageLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    //     // lv_obj_set_style_text_font(ui_airVoltageLabel, &sh_ch_font_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    // #else
    //     lv_obj_add_style(ui_airVoltageLabel, &chFontStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    // #endif
    //     lv_label_set_text(ui_airVoltageLabel, "Vol");
    //     // lv_label_set_text(ui_airVoltageLabel, "电压");

    // 风枪风机电压调节
    ui_airVoltageImage = lv_img_create(ui_ButtonTmp);
    lv_img_set_src(ui_airVoltageImage, &fan);
    // lv_obj_set_size(ui_airFanImage, 16, 16);
    lv_obj_set_pos(ui_airVoltageImage, -30, 12);
    lv_obj_set_align(ui_airVoltageImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_airVoltageImage, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_airVoltageImage, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_airVoltageDropdown = lv_dropdown_create(ui_ButtonTmp);
    lv_dropdown_set_options(ui_airVoltageDropdown, "5V\n9V\n12V\n24V");
    lv_dropdown_set_selected(ui_airVoltageDropdown, 3);
    lv_obj_set_size(ui_airVoltageDropdown, 53, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_airVoltageDropdown, 20, 10);
    lv_obj_set_align(ui_airVoltageDropdown, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_airVoltageDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_set_style_text_font(ui_airVoltageDropdown, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_airVoltageDropdown, &focused_style, LV_STATE_FOCUSED);

    // 风枪风力
    ui_airFanImage = lv_img_create(ui_ButtonTmp);
    lv_img_set_src(ui_airFanImage, &wind_16);
    // lv_obj_set_size(ui_airFanImage, 16, 16);
    lv_obj_set_pos(ui_airFanImage, -30, 47);
    lv_obj_set_align(ui_airFanImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_airFanImage, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_airFanImage, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    // ui_airDutyTextArea = lv_textarea_create(ui_ButtonTmp);
    // lv_obj_set_size(ui_airDutyTextArea, 53, LV_SIZE_CONTENT);
    // lv_obj_set_pos(ui_airDutyTextArea, 20, 45);
    // lv_obj_set_align(ui_airDutyTextArea, LV_ALIGN_CENTER);
    // lv_textarea_set_one_line(ui_airDutyTextArea, true);
    // char speed[8] = {0};
    // snprintf(speed, 8, "%d%%", airhotModel.workAirSpeed);
    // lv_textarea_set_text(ui_airDutyTextArea, speed);

    ui_setAirDutyButton = lv_btn_create(ui_ButtonTmp);
    lv_obj_set_size(ui_setAirDutyButton, 53, 30);
    lv_obj_set_pos(ui_setAirDutyButton, 20, 45);
    lv_obj_set_align(ui_setAirDutyButton, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_setAirDutyButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(ui_setAirDutyButton, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_bg_color(ui_setAirDutyButton, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_setAirDutyButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_setAirDutyButton, &focused_style, LV_STATE_FOCUSED);

    ui_setAirDutyLabel = lv_label_create(ui_setAirDutyButton);
    lv_obj_set_size(ui_setAirDutyLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_setAirDutyLabel, 0, 0);
    lv_obj_set_align(ui_setAirDutyLabel, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_setAirDutyLabel, "%d%%", airhotModel.workAirSpeed);
    lv_obj_set_style_text_color(ui_setAirDutyLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_setAirDutyLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setAirDutyLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

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
    lv_bar_set_range(ui_powerBar, 0, 1000);
    lv_bar_set_value(ui_powerBar, 0, LV_ANIM_ON);
    lv_obj_set_style_bg_color(ui_powerBar, lv_color_hex(0xFF0000), LV_PART_INDICATOR);

    lv_obj_add_event_cb(ui_backButton, ui_back_btn_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_airVoltageDropdown, ui_vol_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_setTempButton, ui_set_temp_btn_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_setAirDutyButton, ui_set_air_btn_pressed, LV_EVENT_PRESSED, NULL);

    setTempArcGroup = lv_group_create();
    setAirArcGroup = lv_group_create();

    return true;
}

static void airhotPageUI_release()
{
    if (NULL == airhotPageUI)
    {
        return;
    }

    lv_obj_clean(airhotPageUI);
    airhotPageUI = NULL;
    airhotUIObj.mainButtonUI = NULL;
}

void ui_updateAirhotCurTempAndPowerDuty(void)
{
    if (NULL == airhotPageUI)
    {
        return;
    }

    if (airhotModel.curTemp > DISCONNCT_TEMP)
    {
        // 未连接
        lv_label_set_text_fmt(ui_curTempLabel, "xxx");
    }
    else
    {
        lv_label_set_text_fmt(ui_curTempLabel, "%d", airhotModel.curTemp);
    }
    lv_bar_set_value(ui_powerBar, airhotModel.powerRatio, LV_ANIM_ON);
}

void ui_updateAirhotWarkState(void)
{
}

static void ui_back_btn_pressed(lv_event_t *e)
{
    // 返回项被按下
    lv_group_del(btn_group);
    ui_main_pressed(e);
}

static void airhotPageUI_pressed(lv_event_t *e)
{
    // 风枪项被按下

    // 创建操作的组
    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_setTempButton);
    lv_group_add_obj(btn_group, ui_airVoltageDropdown);
    lv_group_add_obj(btn_group, ui_setAirDutyButton);
    lv_group_add_obj(btn_group, ui_moreButton);
    lv_group_add_obj(btn_group, ui_backButton);
    lv_group_focus_obj(ui_backButton); // 聚焦到退出按键

    lv_indev_set_group(knobs_indev, btn_group);
}

static void ui_vol_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        uint16_t index = lv_dropdown_get_selected(ui_airVoltageDropdown); // 获取索引
    }
}

static void ui_set_tempArc_changed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        lv_label_set_text_fmt(ui_setTempLabel, "%d°C", lv_arc_get_value(ui_setTempArc));
    }
}

static void ui_set_tempArc_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        airhotModel.targetTemp = lv_arc_get_value(ui_setTempArc);

        lv_group_focus_freeze(setTempArcGroup, false);
        lv_indev_set_group(knobs_indev, btn_group);

        LV_LOG_USER("set_tempArc LV_EVENT_PRESSED % u", event_code);
        if (NULL != ui_setTempArc)
        {
            lv_group_remove_obj(ui_setTempArc);
            lv_obj_clean(ui_setTempArc);
        }
    }
}

static void ui_set_temp_btn_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        LV_LOG_USER("button_pressed LV_EVENT_PRESSED % u", event_code);

        if (NULL != ui_setTempArc)
        {
            lv_group_remove_obj(ui_setTempArc);
            lv_obj_clean(ui_setTempArc);
        }
        ui_setTempArc = lv_arc_create(ui_setTempButton);
        lv_obj_set_size(ui_setTempArc, 30, 30);
        lv_obj_set_pos(ui_setTempArc, 0, 0);
        lv_obj_set_align(ui_setTempArc, LV_ALIGN_CENTER);
        lv_arc_set_range(ui_setTempArc, 0, 500);
        lv_arc_set_value(ui_setTempArc, airhotModel.targetTemp);
        // 设置隐藏
        lv_obj_set_style_opa(ui_setTempArc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_group_add_obj(setTempArcGroup, ui_setTempArc);
        lv_indev_set_group(knobs_indev, setTempArcGroup);
        lv_group_focus_obj(ui_setTempArc);

        lv_obj_add_event_cb(ui_setTempArc, ui_set_tempArc_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_setTempArc, ui_set_tempArc_changed, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

static void ui_set_airArc_changed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        lv_label_set_text_fmt(ui_setAirDutyLabel, "%d%%", lv_arc_get_value(ui_setAirArc));
    }
}

static void ui_set_airArc_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        airhotModel.workAirSpeed = lv_arc_get_value(ui_setAirArc);

        lv_group_focus_freeze(setAirArcGroup, false);
        lv_indev_set_group(knobs_indev, btn_group);

        LV_LOG_USER("set_tempArc LV_EVENT_PRESSED % u", event_code);
        if (NULL != ui_setAirArc)
        {
            lv_group_remove_obj(ui_setAirArc);
            lv_obj_clean(ui_setAirArc);
        }
    }
}

static void ui_set_air_btn_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        LV_LOG_USER("button_pressed LV_EVENT_PRESSED % u", event_code);

        if (NULL != ui_setAirArc)
        {
            lv_group_remove_obj(ui_setAirArc);
            lv_obj_clean(ui_setAirArc);
        }
        ui_setAirArc = lv_arc_create(ui_setAirDutyButton);
        lv_obj_set_size(ui_setAirArc, 30, 30);
        lv_obj_set_pos(ui_setAirArc, 0, 0);
        lv_obj_set_align(ui_setAirArc, LV_ALIGN_CENTER);
        lv_arc_set_range(ui_setAirArc, 5, 100);
        lv_arc_set_value(ui_setAirArc, airhotModel.workAirSpeed);
        // 设置隐藏
        lv_obj_set_style_opa(ui_setAirArc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_group_add_obj(setAirArcGroup, ui_setAirArc);
        lv_indev_set_group(knobs_indev, setAirArcGroup);
        lv_group_focus_obj(ui_setAirArc);

        lv_obj_add_event_cb(ui_setAirArc, ui_set_airArc_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_setAirArc, ui_set_airArc_changed, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

FE_UI_OBJ airhotUIObj = {airhotPageUI, airhotPageUI_init,
                         airhotPageUI_release, airhotPageUI_pressed};