#include "./settingUIFull.h"

#ifdef FULL_UI

#include "../ui.h"
#include "../../desktop_model.h"

static lv_obj_t *ui_father = NULL;

static lv_obj_t *ui_touchSwitch;
static lv_obj_t *ui_beepVolumeBtn;
static lv_obj_t *ui_sysToneSwitch;
static lv_obj_t *ui_knobsToneSwitch;
static lv_obj_t *ui_knobsDirSwitch;
static lv_obj_t *ui_invertColorSwitch;
static lv_obj_t *ui_backgroudSwitch;
static lv_obj_t *ui_wallpaperSwitch;
static lv_obj_t *ui_wallpaperDelayBtn;
static lv_obj_t *ui_wallpaperSwTimeBtn;
static lv_obj_t *ui_pilotLampLightBtn;
static lv_obj_t *ui_pilotLampTimeBtn;

static lv_obj_t *ui_whiteThemeSwitch;
static lv_obj_t *ui_hardVerDropdown;
static lv_obj_t *ui_swVer;
static lv_obj_t *ui_userDataBnt;

static void setHwVer(lv_obj_t *obj, VERSION_INFO ver)
{
    uint16_t verInd = 0;
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
    default:
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
        sysInfoModel.coreConfig.outBoardVersion = (VERSION_INFO)(index * 3 + 2);
        // 或者
        // switch (index)
        // {
        // case 0:
        // {
        //     sysInfoModel.coreConfig.outBoardVersion = VERSION_INFO_OUT_BOARD_V20;
        // }
        // break;
        // case 1:
        // {
        //     sysInfoModel.coreConfig.outBoardVersion = VERSION_INFO_OUT_BOARD_V21;
        // }
        // break;
        // case 2:
        // {
        //     sysInfoModel.coreConfig.outBoardVersion = VERSION_INFO_OUT_BOARD_V25;
        // }
        // break;
        // default:
        //     break;
        // }
        if (VERSION_INFO_OUT_BOARD_V25 > sysInfoModel.coreConfig.outBoardVersion)
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
            sysInfoModel.utilConfig.touchFlag = (ENABLE_STATE)flag;
        }
        else if (target == ui_sysToneSwitch)
        {
            sysInfoModel.utilConfig.sysTone = (ENABLE_STATE)flag;
        }
        else if (target == ui_knobsToneSwitch)
        {
            sysInfoModel.utilConfig.knobTone = (ENABLE_STATE)flag;
        }
        else if (target == ui_knobsDirSwitch)
        {
            sysInfoModel.utilConfig.knobDir = (KNOBS_DIR)flag;
        }
        else if (target == ui_invertColorSwitch)
        {
            sysInfoModel.utilConfig.invertColor = (bool)flag;
        }
        else if (target == ui_backgroudSwitch)
        {
            sysInfoModel.utilConfig.enableBackgroud = (ENABLE_STATE)flag;
        }
        else if (target == ui_wallpaperSwitch)
        {
            sysInfoModel.utilConfig.enableWallpaper = (ENABLE_STATE)flag;
        }
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
            sysInfoModel.utilConfig.uiParam.uiGlobalParam.whiteThemeEnable =
                ENABLE_STATE::ENABLE_STATE_OPEN;
            lv_style_set_bg_color(&black_white_theme_style, lv_color_white());
            // lv_img_set_src(ui_PanelTopBgImg, &img_top_bar_white);
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
            sysInfoModel.utilConfig.uiParam.uiGlobalParam.whiteThemeEnable = ENABLE_STATE_CLOSE;
            lv_style_set_bg_color(&black_white_theme_style, lv_color_black());
            // lv_img_set_src(ui_PanelTopBgImg, &img_top_bar_black);
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
    lv_obj_t *ui_touchLabel = lv_label_create(father);
    lv_obj_align(ui_touchLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_label_set_text(ui_touchLabel, SETTING_TEXT_TOUCH);
    lv_obj_add_style(ui_touchLabel, &label_text_style, 0);

    ui_touchSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_touchSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_touchSwitch, 40, 20);
    lv_obj_align_to(ui_touchSwitch, ui_touchLabel,
                    LV_ALIGN_OUT_LEFT_MID, 140, 0);
    // lv_obj_set_style_bg_color(ui_touchSwitch, lv_color_hex(0xF6B428),
    //                           LV_PART_INDICATOR | LV_STATE_CHECKED);
    if (ENABLE_STATE::ENABLE_STATE_OPEN == sysInfoModel.utilConfig.touchFlag)
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

    // 提示音音量
    lv_obj_t *ui_beepVolumeLabel = lv_label_create(father);
    lv_obj_align_to(ui_beepVolumeLabel, ui_touchLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_label_set_text(ui_beepVolumeLabel, SETTING_TEXT_BEEP_VOLUME);
    lv_obj_add_style(ui_beepVolumeLabel, &label_text_style, 0);

    ui_beepVolumeBtn = lv_numberbtn_create(father);
    lv_obj_t *ui_beepVolumeBtnLabel = lv_label_create(ui_beepVolumeBtn);
    lv_numberbtn_set_label_and_format(ui_beepVolumeBtn,
                                      ui_beepVolumeBtnLabel, "%d%%", 1);
    lv_numberbtn_set_range(ui_beepVolumeBtn, 0, 100);
    lv_numberbtn_set_value(ui_beepVolumeBtn, sysInfoModel.utilConfig.beepVolume);
    lv_obj_set_size(ui_beepVolumeBtn, 50, 20);
    lv_obj_align_to(ui_beepVolumeBtn, ui_beepVolumeLabel,
                    LV_ALIGN_OUT_LEFT_MID, 145, 0);
    // lv_obj_remove_style_all(ui_beepVolumeBtn);
    lv_obj_add_flag(ui_beepVolumeBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_beepVolumeBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_beepVolumeBtn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_beepVolumeBtn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_beepVolumeBtn, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_beepVolumeBtn, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_beepVolumeBtn, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_beepVolumeBtn, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 系统提示音
    lv_obj_t *ui_sysToneLabel = lv_label_create(father);
    lv_obj_align_to(ui_sysToneLabel, ui_beepVolumeLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    lv_label_set_text(ui_sysToneLabel, SETTING_TEXT_SYS_TONE);
    lv_obj_add_style(ui_sysToneLabel, &label_text_style, 0);

    ui_sysToneSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_sysToneSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_sysToneSwitch, 40, 20);
    lv_obj_align_to(ui_sysToneSwitch, ui_sysToneLabel,
                    LV_ALIGN_OUT_LEFT_MID, 140, 0);
    if (sysInfoModel.utilConfig.sysTone)
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
    lv_obj_t *ui_knobsToneLabel = lv_label_create(father);
    lv_obj_align_to(ui_knobsToneLabel, ui_sysToneLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    lv_label_set_text(ui_knobsToneLabel, SETTING_TEXT_KNOB_TONE);
    lv_obj_add_style(ui_knobsToneLabel, &label_text_style, 0);

    ui_knobsToneSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_knobsToneSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_knobsToneSwitch, 40, 20);
    lv_obj_align_to(ui_knobsToneSwitch, ui_knobsToneLabel,
                    LV_ALIGN_OUT_LEFT_MID, 140, 0);
    if (sysInfoModel.utilConfig.knobTone)
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
    lv_obj_t *ui_knobsDirLabel = lv_label_create(father);
    lv_obj_align_to(ui_knobsDirLabel, ui_knobsToneLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    lv_label_set_text(ui_knobsDirLabel, SETTING_TEXT_KNOB_DIR);
    lv_obj_add_style(ui_knobsDirLabel, &label_text_style, 0);

    ui_knobsDirSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_knobsDirSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_knobsDirSwitch, 40, 20);
    lv_obj_align_to(ui_knobsDirSwitch, ui_knobsDirLabel,
                    LV_ALIGN_OUT_LEFT_MID, 140, 0);
    if (sysInfoModel.utilConfig.knobDir)
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

    // 屏幕反色
    lv_obj_t *ui_invertColorLabel = lv_label_create(father);
    lv_obj_align_to(ui_invertColorLabel, ui_knobsDirLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    lv_label_set_text(ui_invertColorLabel, SETTING_TEXT_INVERT_COLOR);
    lv_obj_add_style(ui_invertColorLabel, &label_text_style, 0);

    ui_invertColorSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_invertColorSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_invertColorSwitch, 40, 20);
    lv_obj_align_to(ui_invertColorSwitch, ui_invertColorLabel,
                    LV_ALIGN_OUT_LEFT_MID, 140, 0);
    if (sysInfoModel.utilConfig.invertColor)
    {
        lv_obj_add_state(ui_invertColorSwitch, LV_STATE_CHECKED); // 反向
    }
    else
    {
        lv_obj_clear_state(ui_invertColorSwitch, LV_STATE_CHECKED);
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_invertColorSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_invertColorSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_invertColorSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 黑白主题
    lv_obj_t *ui_whiteThemeLabel = lv_label_create(father);
    lv_obj_align_to(ui_whiteThemeLabel, ui_invertColorLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    lv_label_set_text(ui_whiteThemeLabel, SETTING_TEXT_WHITE_THEME);
    lv_obj_add_style(ui_whiteThemeLabel, &label_text_style, 0);

    ui_whiteThemeSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_whiteThemeSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_whiteThemeSwitch, 40, 20);
    lv_obj_align_to(ui_whiteThemeSwitch, ui_whiteThemeLabel,
                    LV_ALIGN_OUT_LEFT_MID, 140, 0);
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

    // 背景
    lv_obj_t *ui_backgroudLabel = lv_label_create(father);
    lv_obj_align_to(ui_backgroudLabel, ui_whiteThemeLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_label_set_text(ui_backgroudLabel, SETTING_TEXT_BACKGROUD);
    lv_obj_add_style(ui_backgroudLabel, &label_text_style, 0);

    ui_backgroudSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_backgroudSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_backgroudSwitch, 40, 20);
    lv_obj_align_to(ui_backgroudSwitch, ui_backgroudLabel,
                    LV_ALIGN_OUT_LEFT_MID, 140, 0);
    if (sysInfoModel.utilConfig.enableBackgroud)
    {
        lv_obj_add_state(ui_backgroudSwitch, LV_STATE_CHECKED); // 反向
    }
    else
    {
        lv_obj_clear_state(ui_backgroudSwitch, LV_STATE_CHECKED);
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_backgroudSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_backgroudSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_backgroudSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 锁屏壁纸
    lv_obj_t *ui_wallpaperLabel = lv_label_create(father);
    lv_obj_align_to(ui_wallpaperLabel, ui_backgroudLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_label_set_text(ui_wallpaperLabel, SETTING_TEXT_WALLPAPER);
    lv_obj_add_style(ui_wallpaperLabel, &label_text_style, 0);

    ui_wallpaperSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_wallpaperSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_wallpaperSwitch, 40, 20);
    lv_obj_align_to(ui_wallpaperSwitch, ui_wallpaperLabel,
                    LV_ALIGN_OUT_LEFT_MID, 140, 0);
    if (sysInfoModel.utilConfig.enableWallpaper)
    {
        lv_obj_add_state(ui_wallpaperSwitch, LV_STATE_CHECKED); // 反向
    }
    else
    {
        lv_obj_clear_state(ui_wallpaperSwitch, LV_STATE_CHECKED);
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_wallpaperSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_wallpaperSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_wallpaperSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 锁屏壁纸延迟时间
    lv_obj_t *ui_wallpaperDelayLabel = lv_label_create(father);
    lv_obj_align_to(ui_wallpaperDelayLabel, ui_wallpaperLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_label_set_text(ui_wallpaperDelayLabel, SETTING_TEXT_WALLPAPER_DEYLAY);
    lv_obj_add_style(ui_wallpaperDelayLabel, &label_text_style, 0);

    ui_wallpaperDelayBtn = lv_numberbtn_create(father);
    lv_obj_t *ui_wallpaperDelayBtnLabel = lv_label_create(ui_wallpaperDelayBtn);
    lv_numberbtn_set_label_and_format(ui_wallpaperDelayBtn,
                                      ui_wallpaperDelayBtnLabel, "%d", 1);
    lv_numberbtn_set_range(ui_wallpaperDelayBtn, 3, 1800);
    lv_numberbtn_set_value(ui_wallpaperDelayBtn, sysInfoModel.utilConfig.lockScreenDelayTime);
    lv_obj_set_size(ui_wallpaperDelayBtn, 50, 20);
    lv_obj_align_to(ui_wallpaperDelayBtn, ui_wallpaperDelayLabel,
                    LV_ALIGN_OUT_LEFT_MID, 145, 0);
    // lv_obj_remove_style_all(ui_wallpaperDelayBtn);
    lv_obj_add_flag(ui_wallpaperDelayBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_wallpaperDelayBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_wallpaperDelayBtn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_wallpaperDelayBtn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_wallpaperDelayBtn, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_wallpaperDelayBtn, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_wallpaperDelayBtn, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_wallpaperDelayBtn, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 锁屏壁纸切换时间
    lv_obj_t *ui_wallpaperSwTimeLabel = lv_label_create(father);
    lv_obj_align_to(ui_wallpaperSwTimeLabel, ui_wallpaperDelayLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_label_set_text(ui_wallpaperSwTimeLabel, SETTING_TEXT_WALLPAPER_SW_TIME);
    lv_obj_add_style(ui_wallpaperSwTimeLabel, &label_text_style, 0);

    ui_wallpaperSwTimeBtn = lv_numberbtn_create(father);
    lv_obj_t *ui_wallpaperSwTimeBtnLabel = lv_label_create(ui_wallpaperSwTimeBtn);
    lv_numberbtn_set_label_and_format(ui_wallpaperSwTimeBtn,
                                      ui_wallpaperSwTimeBtnLabel, "%d", 1);
    lv_numberbtn_set_range(ui_wallpaperSwTimeBtn, 3, 1800);
    lv_numberbtn_set_value(ui_wallpaperSwTimeBtn, sysInfoModel.utilConfig.wallpaperSwitchTime);
    lv_obj_set_size(ui_wallpaperSwTimeBtn, 50, 20);
    lv_obj_align_to(ui_wallpaperSwTimeBtn, ui_wallpaperSwTimeLabel,
                    LV_ALIGN_OUT_LEFT_MID, 145, 0);
    // lv_obj_remove_style_all(ui_wallpaperSwTimeBtn);
    lv_obj_add_flag(ui_wallpaperSwTimeBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_wallpaperSwTimeBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_wallpaperSwTimeBtn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_wallpaperSwTimeBtn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_wallpaperSwTimeBtn, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_wallpaperSwTimeBtn, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_wallpaperSwTimeBtn, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_wallpaperSwTimeBtn, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 呼吸灯亮度
    lv_obj_t *ui_pilotLampLightLabel = lv_label_create(father);
    lv_obj_align_to(ui_pilotLampLightLabel, ui_wallpaperSwTimeLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_label_set_text(ui_pilotLampLightLabel, SETTING_TEXT_PILOTLAMP_LIGHT);
    lv_obj_add_style(ui_pilotLampLightLabel, &label_text_style, 0);

    ui_pilotLampLightBtn = lv_numberbtn_create(father);
    lv_obj_t *ui_pilotLampLightBtnLabel = lv_label_create(ui_pilotLampLightBtn);
    lv_numberbtn_set_label_and_format(ui_pilotLampLightBtn,
                                      ui_pilotLampLightBtnLabel, "%d", 1);
    lv_numberbtn_set_range(ui_pilotLampLightBtn, 0, 255);
    lv_numberbtn_set_value(ui_pilotLampLightBtn, sysInfoModel.utilConfig.pilotLampLight);
    lv_obj_set_size(ui_pilotLampLightBtn, 50, 20);
    lv_obj_align_to(ui_pilotLampLightBtn, ui_pilotLampLightLabel,
                    LV_ALIGN_OUT_LEFT_MID, 145, 0);
    // lv_obj_remove_style_all(ui_pilotLampLightBtn);
    lv_obj_add_flag(ui_pilotLampLightBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_pilotLampLightBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_pilotLampLightBtn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_pilotLampLightBtn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_pilotLampLightBtn, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_pilotLampLightBtn, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_pilotLampLightBtn, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_pilotLampLightBtn, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 呼吸灯时间
    lv_obj_t *ui_pilotLampTimeLabel = lv_label_create(father);
    lv_obj_align_to(ui_pilotLampTimeLabel, ui_pilotLampLightLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_label_set_text(ui_pilotLampTimeLabel, SETTING_TEXT_PILOTLAMP_TIME);
    lv_obj_add_style(ui_pilotLampTimeLabel, &label_text_style, 0);

    ui_pilotLampTimeBtn = lv_numberbtn_create(father);
    lv_obj_t *ui_pilotLampTimeBtnLabel = lv_label_create(ui_pilotLampTimeBtn);
    lv_numberbtn_set_label_and_format(ui_pilotLampTimeBtn,
                                      ui_pilotLampTimeBtnLabel, "%d", 1);
    lv_numberbtn_set_range(ui_pilotLampTimeBtn, 50, 255);
    lv_numberbtn_set_value(ui_pilotLampTimeBtn, sysInfoModel.utilConfig.pilotLampSpeed);
    lv_obj_set_size(ui_pilotLampTimeBtn, 50, 20);
    lv_obj_align_to(ui_pilotLampTimeBtn, ui_pilotLampTimeLabel,
                    LV_ALIGN_OUT_LEFT_MID, 145, 0);
    // lv_obj_remove_style_all(ui_pilotLampTimeBtn);
    lv_obj_add_flag(ui_pilotLampTimeBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_pilotLampTimeBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_pilotLampTimeBtn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_pilotLampTimeBtn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_pilotLampTimeBtn, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_pilotLampTimeBtn, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_pilotLampTimeBtn, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_pilotLampTimeBtn, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 硬件版本
    lv_obj_t *ui_hardVerLabel = lv_label_create(father);
    lv_obj_align_to(ui_hardVerLabel, ui_pilotLampTimeLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_label_set_text(ui_hardVerLabel, SETTING_TEXT_HW_VER);
    lv_obj_add_style(ui_hardVerLabel, &label_text_style, 0);

    static char hwVer[] = SHL_HW_VER;
    ui_hardVerDropdown = lv_dropdown_create(father);
    // lv_dropdown_set_options(ui_hardVerDropdown, SHL_HW_VER);
    lv_dropdown_set_options_static(ui_hardVerDropdown, hwVer);
    setHwVer(ui_hardVerDropdown, sysInfoModel.coreConfig.outBoardVersion);
    lv_obj_set_size(ui_hardVerDropdown, 80, 30);
    lv_obj_align_to(ui_hardVerDropdown, ui_hardVerLabel,
                    LV_ALIGN_OUT_LEFT_MID, 155, 0);
    lv_obj_add_flag(ui_hardVerDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_text_font(ui_hardVerDropdown, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_hardVerDropdown, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_hardVerDropdown, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_hardVerDropdown, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 软件版本信息
    lv_obj_t *ui_swVerLabel = lv_label_create(father);
    lv_obj_align_to(ui_swVerLabel, ui_hardVerLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 25);
    lv_label_set_text(ui_swVerLabel, SETTING_TEXT_SW_VER);
    lv_obj_add_style(ui_swVerLabel, &label_text_style, 0);

    ui_swVer = lv_label_create(father);
    lv_obj_align_to(ui_swVer, ui_swVerLabel,
                    LV_ALIGN_OUT_LEFT_MID, 110, 0);
    lv_obj_set_style_text_color(ui_swVer, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_swVer, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_swVer, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(ui_swVer, "Ver %s", sysInfoModel.softwareVersion);

    // 用户数据管理
    lv_obj_t *ui_userDataLabel = lv_label_create(father);
    lv_obj_align_to(ui_userDataLabel, ui_swVerLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    lv_label_set_text(ui_userDataLabel, SETTING_TEXT_USERDATA_MANAGE);
    lv_obj_add_style(ui_userDataLabel, &label_text_style, 0);

    ui_userDataBnt = lv_btn_create(father);
    // lv_obj_remove_style_all(ui_userDataBnt);
    lv_obj_set_size(ui_userDataBnt, 40, 20);
    lv_obj_align_to(ui_userDataBnt, ui_userDataLabel,
                    LV_ALIGN_OUT_LEFT_MID, 140, 0);
    lv_obj_add_flag(ui_userDataBnt, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_userDataBnt, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_userDataBnt, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_userDataBnt, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_userDataBnt, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_userDataBnt, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    lv_obj_t *ui_userDataBntLabel = lv_label_create(ui_userDataBnt);
    lv_obj_align(ui_userDataBntLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui_userDataBntLabel, SETTING_TEXT_ENTER);
    lv_obj_add_style(ui_userDataBntLabel, &label_text_style, 0);

    // 返回
    ui_subBackBtn = lv_btn_create(father);
    lv_obj_remove_style_all(ui_subBackBtn);
    lv_obj_align_to(ui_subBackBtn, ui_userDataLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_obj_add_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_subBackBtn, &back_btn_style, LV_STATE_DEFAULT);
    lv_obj_add_style(ui_subBackBtn, &back_btn_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_subBackBtnLabel = lv_label_create(ui_subBackBtn);
    lv_obj_center(ui_subBackBtnLabel);
    lv_obj_add_style(ui_subBackBtnLabel, &label_text_style, 0);
    lv_label_set_text(ui_subBackBtnLabel, SETTING_TEXT_BACK);
}

void userdata_verification_cb(lv_obj_t *obj, bool isLegal)
{
    if (isLegal)
    {
        LV_LOG_USER("userdata_verification_cb True");
        lv_obj_t *father = ui_father;
        ui_sys_setting_release();
        ui_father = father;
        ui_userdata_init(ui_father);
    }
    else
    {
        LV_LOG_USER("userdata_verification_cb False");
        lv_indev_set_group(knobs_indev, sub_btn_group);
    }
    return;
}

static void ui_sys_param_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == ui_beepVolumeBtn)
        {
            sysInfoModel.utilConfig.beepVolume = lv_numberbtn_get_value(ui_beepVolumeBtn);
        }
        else if (target == ui_wallpaperDelayBtn)
        {
            sysInfoModel.utilConfig.lockScreenDelayTime = lv_numberbtn_get_value(ui_wallpaperDelayBtn);
        }
        else if (target == ui_wallpaperSwTimeBtn)
        {
            sysInfoModel.utilConfig.wallpaperSwitchTime = lv_numberbtn_get_value(ui_wallpaperSwTimeBtn);
        }
        else if (target == ui_pilotLampLightBtn)
        {
            sysInfoModel.utilConfig.pilotLampLight = lv_numberbtn_get_value(ui_pilotLampLightBtn);
        }
        else if (target == ui_pilotLampTimeBtn)
        {
            sysInfoModel.utilConfig.pilotLampSpeed = lv_numberbtn_get_value(ui_pilotLampTimeBtn);
        }
        else if (target == ui_userDataBnt)
        {
            userdata_verification_init(ui_father, ui_userDataBnt,
                                       knobs_indev, userdata_verification_cb);
            // sysInfoModel.manageConfigAction =
            //     INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_UTIL_RESET;
        }
    }
}

void ui_sys_setting_init_group(lv_obj_t *father)
{
    lv_obj_add_event_cb(ui_touchSwitch, ui_sys_setting_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_beepVolumeBtn, ui_sys_param_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_sysToneSwitch, ui_sys_setting_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_knobsToneSwitch, ui_sys_setting_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_knobsDirSwitch, ui_sys_setting_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_invertColorSwitch, ui_sys_setting_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_whiteThemeSwitch, ui_white_theme_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_backgroudSwitch, ui_sys_setting_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_wallpaperSwitch, ui_sys_setting_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_wallpaperDelayBtn, ui_sys_param_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_wallpaperSwTimeBtn, ui_sys_param_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_pilotLampLightBtn, ui_sys_param_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_pilotLampTimeBtn, ui_sys_param_pressed, LV_EVENT_PRESSED, NULL);
    // lv_obj_add_event_cb(ui_hardVerDropdown, ui_hw_ver_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_userDataBnt, ui_sys_param_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_subBackBtn, ui_tab_back_btn_pressed, LV_EVENT_PRESSED, 0);

    if (NULL != sub_btn_group)
    {
        lv_group_del(sub_btn_group);
        sub_btn_group = NULL;
    }
    sub_btn_group = lv_group_create();
    lv_group_add_obj(sub_btn_group, ui_subBackBtn);
    lv_group_add_obj(sub_btn_group, ui_touchSwitch);
    lv_group_add_obj(sub_btn_group, ui_beepVolumeBtn);
    lv_group_add_obj(sub_btn_group, ui_sysToneSwitch);
    lv_group_add_obj(sub_btn_group, ui_knobsToneSwitch);
    lv_group_add_obj(sub_btn_group, ui_knobsDirSwitch);
    lv_group_add_obj(sub_btn_group, ui_invertColorSwitch);
    lv_group_add_obj(sub_btn_group, ui_whiteThemeSwitch);
    lv_group_add_obj(sub_btn_group, ui_backgroudSwitch);
    lv_group_add_obj(sub_btn_group, ui_wallpaperSwitch);
    lv_group_add_obj(sub_btn_group, ui_wallpaperDelayBtn);
    lv_group_add_obj(sub_btn_group, ui_wallpaperSwTimeBtn);
    lv_group_add_obj(sub_btn_group, ui_pilotLampLightBtn);
    lv_group_add_obj(sub_btn_group, ui_pilotLampTimeBtn);
    lv_group_add_obj(sub_btn_group, ui_userDataBnt);
    // 取消版本的设置
    // lv_group_add_obj(sub_btn_group, ui_hardVerDropdown);
    lv_indev_set_group(knobs_indev, sub_btn_group);
}

void ui_sys_setting_release(void *param)
{
    if (NULL != sub_btn_group)
    {
        lv_group_del(sub_btn_group);
        sub_btn_group = NULL;
    }

    if (NULL != ui_father)
    {
        userdata_verification_release();
        lv_obj_clean(ui_father);
        ui_father = NULL;
    }
}

#endif