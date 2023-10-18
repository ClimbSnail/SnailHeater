#include "./settingUIFull.h"

#ifdef FULL_UI

#include "../ui.h"
#include "../../desktop_model.h"

static lv_obj_t *ui_father = NULL;

static lv_obj_t *ui_touchLabel;
static lv_obj_t *ui_touchSwitch;
static lv_obj_t *ui_sysToneLabel;
static lv_obj_t *ui_sysToneSwitch;
static lv_obj_t *ui_knobsToneLabel;
static lv_obj_t *ui_knobsToneSwitch;
static lv_obj_t *ui_knobsDirLabel;
static lv_obj_t *ui_knobsDirSwitch;

static lv_obj_t *ui_whiteThemeLabel;
static lv_obj_t *ui_whiteThemeSwitch;
static lv_obj_t *ui_hardVerLabel;
static lv_obj_t *ui_hardVerDropdown;
static lv_obj_t *ui_swVerLabel;

static void setHwVer(lv_obj_t *obj, VERSION_INFO ver)
{
    uint8_t verInd;
    switch (ver)
    {
    case VERSION_INFO_OUT_BOARD_V20:
    {
        verInd = 0;
    }
    break;
    case VERSION_INFO_OUT_BOARD_V21:
    {
        verInd = 1;
    }
    break;
    case VERSION_INFO_OUT_BOARD_V25:
    {
        verInd = 2;
    }
    break;
    case VERSION_INFO_OUT_BOARD_V26:
    {
        verInd = 3;
    }
    break;
    case VERSION_INFO_OUT_BOARD_V27:
    {
        verInd = 4;
    }
    break;
    defualt:
        break;
    }
    lv_dropdown_set_selected(obj, verInd);
}

static void ui_hw_ver_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        uint16_t index = lv_dropdown_get_selected(ui_hardVerDropdown); // 获取索引
        // 计算方法由 VERSION_INFO 功率板的顺序规则得到
        sysInfoModel.outBoardVersion = (VERSION_INFO)(index * 3 + 2);
        // 或者
        // switch (index)
        // {
        // case 0:
        // {
        //     sysInfoModel.outBoardVersion = VERSION_INFO_OUT_BOARD_V20;
        // }
        // break;
        // case 1:
        // {
        //     sysInfoModel.outBoardVersion = VERSION_INFO_OUT_BOARD_V21;
        // }
        // break;
        // case 2:
        // {
        //     sysInfoModel.outBoardVersion = VERSION_INFO_OUT_BOARD_V25;
        // }
        // break;
        // defualt:
        //     break;
        // }
        if (VERSION_INFO_OUT_BOARD_V25 > sysInfoModel.outBoardVersion)
        {
            // V2.0板子不支持
            solderModel.utilConfig.autoTypeSwitch = ENABLE_STATE_CLOSE;
        }
    }
}

static void ui_sys_setting_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        bool flag = lv_obj_has_state(target, LV_STATE_CHECKED);
        if (target == ui_touchSwitch)
        {
            sysInfoModel.touchFlag = (ENABLE_STATE)flag;
        }
        else if (target == ui_sysToneSwitch)
        {
            sysInfoModel.sysTone = (ENABLE_STATE)flag;
        }
        else if (target == ui_knobsToneSwitch)
        {
            sysInfoModel.knobTone = (ENABLE_STATE)flag;
        }
        else if (target == ui_knobsDirSwitch)
        {
            sysInfoModel.knobDir = (KNOBS_DIR)flag;
        }
    }
}

// 按键事件
static void ui_knobs_dir_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        bool isAntiDir = lv_obj_has_state(target, LV_STATE_CHECKED);
        sysInfoModel.knobDir = (KNOBS_DIR)isAntiDir;
    }
}

static void ui_white_theme_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        if (lv_obj_has_state(target, LV_STATE_CHECKED))
        // SET_BIT(cfgKey1,CFG_KEY1_WHITE_THEME);
        {
            sysInfoModel.uiGlobalParam.whiteThemeEnable = ENABLE_STATE_OPEN;
            lv_style_set_bg_color(&black_white_theme_style, lv_color_white());
            lv_img_set_src(ui_PanelTopBgImg, &img_top_bar_white);
            lv_style_set_text_color(&label_text_style, WHITE_THEME_LABEL_TEXT_COLOR);
#if USE_MENU_STYLE == 3
            lv_style_set_text_color(&menu_button_style, WHITE_THEME_ARCMENU_TEXT_COLOR);
            lv_style_set_arc_color(&menu_arc_style, WHITE_THEME_ARCMENU_BG_COLOR);
#endif
#if USE_MENU_STYLE == 2
            lv_obj_set_style_bg_color(rollerMenu, WHITE_THEME_ROLLERMENU_BG_COLOR, LV_PART_MAIN);
            lv_obj_set_style_text_color(rollerMenu, WHITE_THEME_ROLLERMENU_TEXT_COLOR, LV_PART_MAIN);
#endif
        }

        else
        {
            sysInfoModel.uiGlobalParam.whiteThemeEnable = ENABLE_STATE_CLOSE;
            lv_style_set_bg_color(&black_white_theme_style, lv_color_black());
            lv_img_set_src(ui_PanelTopBgImg, &img_top_bar_black);
            lv_style_set_text_color(&label_text_style, BLACK_THEME_LABEL_TEXT_COLOR);
#if USE_MENU_STYLE == 3
            lv_style_set_text_color(&menu_button_style, BLACK_THEME_ARCMENU_TEXT_COLOR);
            lv_style_set_arc_color(&menu_arc_style, BLACK_THEME_ARCMENU_BG_COLOR);
#endif
#if USE_MENU_STYLE == 2
            lv_obj_set_style_bg_color(rollerMenu, BLACK_THEME_ROLLERMENU_BG_COLOR, LV_PART_MAIN);
            lv_obj_set_style_text_color(rollerMenu, BLACK_THEME_ROLLERMENU_TEXT_COLOR, LV_PART_MAIN);
#endif
        }
        // CLR_BIT(cfgKey1,CFG_KEY1_WHITE_THEME);
        // 直接在这里切换
        lv_obj_invalidate(lv_scr_act());
    }
}

void ui_sys_setting_init(lv_obj_t *father)
{
    ui_father = father;
    // 触摸开关
    ui_touchLabel = lv_label_create(father);
    lv_obj_align(ui_touchLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_label_set_text(ui_touchLabel, SETTING_TEXT_TOUCH);
    lv_obj_add_style(ui_touchLabel, &label_text_style, 0);

    ui_touchSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_touchSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_touchSwitch, 40, 20);
    lv_obj_align(ui_touchSwitch, LV_ALIGN_TOP_LEFT, 105, 10);
    if (sysInfoModel.touchFlag)
    {
        lv_obj_add_state(ui_touchSwitch, LV_STATE_CHECKED); // 反向
    }
    else
    {
        lv_obj_clear_state(ui_touchSwitch, LV_STATE_CHECKED);
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_touchSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_touchSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_touchSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 系统提示音
    ui_sysToneLabel = lv_label_create(father);
    lv_obj_align(ui_sysToneLabel, LV_ALIGN_TOP_LEFT, 10, 45);
    lv_label_set_text(ui_sysToneLabel, SETTING_TEXT_SYS_TONE);
    lv_obj_add_style(ui_sysToneLabel, &label_text_style, 0);

    ui_sysToneSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_sysToneSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_sysToneSwitch, 40, 20);
    lv_obj_align(ui_sysToneSwitch, LV_ALIGN_TOP_LEFT, 105, 45);
    if (sysInfoModel.sysTone)
    {
        lv_obj_add_state(ui_sysToneSwitch, LV_STATE_CHECKED); // 反向
    }
    else
    {
        lv_obj_clear_state(ui_sysToneSwitch, LV_STATE_CHECKED);
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_sysToneSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_sysToneSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_sysToneSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 旋钮提示音
    ui_knobsToneLabel = lv_label_create(father);
    lv_obj_align(ui_knobsToneLabel, LV_ALIGN_TOP_LEFT, 10, 80);
    lv_label_set_text(ui_knobsToneLabel, SETTING_TEXT_KNOB_TONE);
    lv_obj_add_style(ui_knobsToneLabel, &label_text_style, 0);

    ui_knobsToneSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_knobsToneSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_knobsToneSwitch, 40, 20);
    lv_obj_align(ui_knobsToneSwitch, LV_ALIGN_TOP_LEFT, 105, 80);
    if (sysInfoModel.knobTone)
    {
        lv_obj_add_state(ui_knobsToneSwitch, LV_STATE_CHECKED); // 反向
    }
    else
    {
        lv_obj_clear_state(ui_knobsToneSwitch, LV_STATE_CHECKED);
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_knobsToneSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_knobsToneSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_knobsToneSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 旋钮方向
    ui_knobsDirLabel = lv_label_create(father);
    lv_obj_align(ui_knobsDirLabel, LV_ALIGN_TOP_LEFT, 10, 115);
    lv_label_set_text(ui_knobsDirLabel, SETTING_TEXT_KNOB_DIR);
    lv_obj_add_style(ui_knobsDirLabel, &label_text_style, 0);

    ui_knobsDirSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_knobsDirSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_knobsDirSwitch, 40, 20);
    lv_obj_align(ui_knobsDirSwitch, LV_ALIGN_TOP_LEFT, 105, 115);
    if (sysInfoModel.knobDir)
    {
        lv_obj_add_state(ui_knobsDirSwitch, LV_STATE_CHECKED); // 反向
    }
    else
    {
        lv_obj_clear_state(ui_knobsDirSwitch, LV_STATE_CHECKED);
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_knobsDirSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_knobsDirSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_knobsDirSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 黑白主题
    ui_whiteThemeLabel = lv_label_create(father);
    lv_obj_align(ui_whiteThemeLabel, LV_ALIGN_TOP_LEFT, 10, 150);
    lv_label_set_text(ui_whiteThemeLabel, SETTING_TEXT_WHITE_THEME);
    lv_obj_add_style(ui_whiteThemeLabel, &label_text_style, 0);

    ui_whiteThemeSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_whiteThemeSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_whiteThemeSwitch, 40, 20);
    lv_obj_align(ui_whiteThemeSwitch, LV_ALIGN_TOP_LEFT, 105, 150);
    bool isWhiteTheme = IS_WHITE_THEME;
    if (isWhiteTheme)
    {
        lv_obj_add_state(ui_whiteThemeSwitch, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_clear_state(ui_whiteThemeSwitch, LV_STATE_CHECKED);
    }
    lv_obj_set_style_outline_color(ui_whiteThemeSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_whiteThemeSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_whiteThemeSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 硬件版本
    ui_hardVerLabel = lv_label_create(father);
    lv_obj_align(ui_hardVerLabel, LV_ALIGN_TOP_LEFT, 10, 185);
    lv_label_set_text(ui_hardVerLabel, SETTING_TEXT_HW_VER);
    lv_obj_add_style(ui_hardVerLabel, &label_text_style, 0);

    static char hwVer[] = SHL_HW_VER;
    ui_hardVerDropdown = lv_dropdown_create(father);
    // lv_dropdown_set_options(ui_hardVerDropdown, SHL_HW_VER);
    lv_dropdown_set_options_static(ui_hardVerDropdown, hwVer);
    setHwVer(ui_hardVerDropdown, sysInfoModel.outBoardVersion);
    lv_obj_set_size(ui_hardVerDropdown, 80, 30);
    lv_obj_align(ui_hardVerDropdown, LV_ALIGN_TOP_LEFT, 105, 180);
    lv_obj_add_flag(ui_hardVerDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_text_font(ui_hardVerDropdown, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_hardVerDropdown, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_hardVerDropdown, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_hardVerDropdown, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 软件版本信息
    ui_swVerLabel = lv_label_create(father);
    lv_obj_set_size(ui_swVerLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(ui_swVerLabel, LV_ALIGN_TOP_LEFT, 10, 220);
    lv_obj_set_style_text_color(ui_swVerLabel, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_swVerLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_swVerLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(ui_swVerLabel, "Ver %s", sysInfoModel.softwareVersion);

    ui_subBackBtn = lv_btn_create(father);
    lv_obj_remove_style_all(ui_subBackBtn);
    lv_obj_align(ui_subBackBtn, LV_ALIGN_TOP_LEFT, 105, 215);
    lv_obj_add_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_subBackBtn, &back_btn_style, LV_STATE_DEFAULT);
    lv_obj_add_style(ui_subBackBtn, &back_btn_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_subBackBtnLabel = lv_label_create(ui_subBackBtn);
    lv_obj_center(ui_subBackBtnLabel);
    lv_obj_add_style(ui_subBackBtnLabel, &label_text_style, 0);
    lv_label_set_text(ui_subBackBtnLabel, "返回");
}

void ui_sys_setting_init_group(lv_obj_t *father)
{
    lv_obj_add_event_cb(ui_touchSwitch, ui_sys_setting_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_sysToneSwitch, ui_sys_setting_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_knobsToneSwitch, ui_sys_setting_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_knobsDirSwitch, ui_sys_setting_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_whiteThemeSwitch, ui_white_theme_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    // lv_obj_add_event_cb(ui_hardVerDropdown, ui_hw_ver_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_subBackBtn, ui_tab_back_btn_pressed, LV_EVENT_PRESSED, 0);

    sub_btn_group = lv_group_create();
    lv_group_add_obj(sub_btn_group, ui_subBackBtn);
    lv_group_add_obj(sub_btn_group, ui_touchSwitch);
    lv_group_add_obj(sub_btn_group, ui_sysToneSwitch);
    lv_group_add_obj(sub_btn_group, ui_sysToneSwitch);
    lv_group_add_obj(sub_btn_group, ui_knobsToneSwitch);
    lv_group_add_obj(sub_btn_group, ui_knobsDirSwitch);
    lv_group_add_obj(sub_btn_group, ui_whiteThemeSwitch);
    // 取消版本的设置
    // lv_group_add_obj(sub_btn_group, ui_hardVerDropdown);
    lv_indev_set_group(knobs_indev, sub_btn_group);
}

void ui_sys_setting_release(void *param)
{
    if (NULL != ui_father)
    {
        lv_obj_clean(ui_father);
        ui_father = NULL;
    }
}

#endif