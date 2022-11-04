

#include "ui.h"
#include "desktop_model.h"

#define FONT_DEBUG 1

static lv_obj_t *hpPageUI = NULL;
static lv_obj_t *ui_curTempLabel;
static lv_obj_t *ui_heatplatformCurTempCLabel;
static lv_obj_t *ui_setTempButton;
static lv_obj_t *ui_setTempLabel;
static lv_obj_t *ui_enableSwitch;
static lv_obj_t *ui_setTempArc;
static lv_obj_t *ui_nameLabel;
static lv_obj_t *ui_platformImage;
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

static void ui_set_temp_btn_pressed(lv_event_t *e);
static void ui_back_btn_pressed(lv_event_t *e);
static void ui_enable_switch_pressed(lv_event_t *e);

static bool hpPageUI_init(lv_obj_t *father)
{
    if (NULL != hpPageUI)
    {
        return false;
    }

    hpPageUI = lv_btn_create(father); // 紫色
    hpUIObj.mainButtonUI = hpPageUI;

    lv_obj_set_size(hpPageUI, 110, 200);
    // lv_obj_set_pos(hpPageUI, btnPosXY[2][0], btnPosXY[2][1]);
    lv_obj_set_pos(hpPageUI, START_UI_OBJ_X, 0);
    lv_obj_set_align(hpPageUI, LV_ALIGN_CENTER);
    lv_obj_add_flag(hpPageUI, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(hpPageUI, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    // 0x935EFF
    lv_obj_set_style_bg_color(hpPageUI, lv_color_hex(0x935EDF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(hpPageUI, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(hpPageUI, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_PRESSED);
    lv_obj_set_style_border_opa(hpPageUI, 255, LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_PRESSED);

    lv_obj_t *ui_ButtonTmp = hpPageUI;

    // 中文字体
    lv_style_init(&chFontStyle);
    lv_style_set_text_opa(&chFontStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_style_set_text_color(&chFontStyle, lv_color_hex(0x1D1517));
    lv_style_set_text_font(&chFontStyle, &sh_ch_font_14);

    ui_curTempLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_curTempLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_curTempLabel, 0, -70);
    lv_obj_set_align(ui_curTempLabel, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_curTempLabel, "%d", heatplatformModel.curTemp);
    lv_obj_set_style_text_color(ui_curTempLabel, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_curTempLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
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
    lv_label_set_text_fmt(ui_setTempLabel, "%d°C", heatplatformModel.targetTemp);
    lv_obj_set_style_text_color(ui_setTempLabel, lv_color_hex(0xF76889), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_setTempLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setTempLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 使能开关
    ui_enableSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_set_size(ui_enableSwitch, 40, 20);
    lv_obj_set_pos(ui_enableSwitch, 0, 5);
    lv_obj_set_align(ui_enableSwitch, LV_ALIGN_CENTER);
    if (heatplatformModel.enable == 1)
    {
        lv_obj_add_state(ui_enableSwitch, LV_STATE_CHECKED); // 开
    }
    else
    {
        lv_obj_clear_state(ui_enableSwitch, LV_STATE_CHECKED); // 关
    }
    lv_obj_add_style(ui_enableSwitch, &focused_style, LV_STATE_FOCUSED);

    //     ui_nameLabel = lv_label_create(ui_ButtonTmp);
    //     lv_obj_set_size(ui_nameLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    //     lv_obj_set_pos(ui_nameLabel, 0, 40);
    //     lv_obj_set_align(ui_nameLabel, LV_ALIGN_CENTER);
    //     // lv_label_set_text(ui_nameLabel, "类型");
    // #if FONT_DEBUG == 1
    //     lv_obj_set_style_text_color(ui_nameLabel, lv_color_hex(0x1D1517), LV_PART_MAIN | LV_STATE_DEFAULT);
    //     lv_obj_set_style_text_opa(ui_nameLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    //     lv_obj_set_style_text_font(ui_nameLabel, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    //     // lv_obj_set_style_text_font(ui_nameLabel, &sh_ch_font_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    // #else
    //     lv_obj_add_style(ui_nameLabel, &chFontStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    // #endif
    //     // lv_label_set_text(ui_nameLabel, "HeatPlatform");
    //     lv_label_set_text_fmt(ui_nameLabel, "Heat\nPlatform");

    // 加热台图标
    ui_platformImage = lv_img_create(ui_ButtonTmp);
    lv_img_set_src(ui_platformImage, &heat_platform_48);
    // lv_obj_set_size(ui_settingImage, 16, 16);
    lv_obj_set_pos(ui_platformImage, 0, 40);
    lv_obj_set_align(ui_platformImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_platformImage, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_platformImage, LV_OBJ_FLAG_SCROLLABLE); /// Flags

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
    lv_obj_add_event_cb(ui_setTempButton, ui_set_temp_btn_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_enableSwitch, ui_enable_switch_pressed, LV_EVENT_VALUE_CHANGED, NULL);

    setTempArcGroup = lv_group_create();
    return true;
}

static void hpPageUI_release()
{
    if (NULL == hpPageUI)
    {
        return;
    }
    lv_obj_clean(hpPageUI);
    hpPageUI = NULL;
    hpUIObj.mainButtonUI = NULL;
}

static void ui_back_btn_pressed(lv_event_t *e)
{
    // 返回项被按下
    lv_group_del(btn_group);
    ui_main_pressed(e);
}

static void ui_enable_switch_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {

        bool isEnable = lv_obj_has_state(target, LV_STATE_CHECKED); // 返回 bool 类型， 开-1 ； 关-2
        heatplatformModel.enable = isEnable == true ? 1 : 0;
        // if (heatplatformModel.enable == 1)
        // {
        //     lv_obj_add_state(ui_enableSwitch, LV_STATE_CHECKED); // 开
        // }
        // else
        // {
        //     lv_obj_clear_state(ui_enableSwitch, LV_STATE_CHECKED); // 关
        // }
    }
}

static void hpPageUI_pressed(lv_event_t *e)
{
    // 加热台项被按下

    // 创建操作的组
    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_setTempButton);
    lv_group_add_obj(btn_group, ui_enableSwitch);
    lv_group_add_obj(btn_group, ui_moreButton);
    lv_group_add_obj(btn_group, ui_backButton);
    lv_group_focus_obj(ui_backButton); // 聚焦到退出按键

    lv_indev_set_group(knobs_indev, btn_group);
}

void ui_updateHeatplatformCurTempAndPowerDuty(void)
{
    if (NULL == hpPageUI)
    {
        return;
    }
    lv_label_set_text_fmt(ui_curTempLabel, "%d", heatplatformModel.curTemp);
    lv_bar_set_value(ui_powerBar, heatplatformModel.powerRatio, LV_ANIM_ON);
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
        heatplatformModel.targetTemp = lv_arc_get_value(ui_setTempArc);

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
        lv_arc_set_value(ui_setTempArc, heatplatformModel.targetTemp);
        // 设置隐藏
        lv_obj_set_style_opa(ui_setTempArc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_group_add_obj(setTempArcGroup, ui_setTempArc);
        lv_indev_set_group(knobs_indev, setTempArcGroup);
        lv_group_focus_obj(ui_setTempArc);

        lv_obj_add_event_cb(ui_setTempArc, ui_set_tempArc_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_setTempArc, ui_set_tempArc_changed, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

FE_UI_OBJ hpUIObj = {hpPageUI, hpPageUI_init,
                     hpPageUI_release, hpPageUI_pressed};