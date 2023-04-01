#include "./ui.h"
#include "../desktop_model.h"

#ifdef FULL_UI

static lv_obj_t *settingPageUI = NULL;

// 开关
static lv_obj_t *ui_knobsDirLabel;
static lv_obj_t *ui_knobsDirSwitch;
static lv_obj_t *ui_autoHeatLabel;
static lv_obj_t *ui_autoHeatSwitch;
static lv_obj_t *ui_whiteThemeLabel;
static lv_obj_t *ui_whiteThemeSwitch;
static lv_obj_t *ui_solderGridLabel;
static lv_obj_t *ui_solderGridSwitch;
static lv_obj_t *ui_airhotGridLabel;
static lv_obj_t *ui_airhotGridSwitch;
static lv_obj_t *ui_heatplatGridLabel;
static lv_obj_t *ui_heatplatGridSwitch;

// 下拉
static lv_obj_t *ui_hardVerLabel;
static lv_obj_t *ui_hardVerDropdown;
static lv_obj_t *ui_solderNameLabel;
static lv_obj_t *ui_solderNameDropdown;
static lv_obj_t *ui_solderWakeLabel;
static lv_obj_t *ui_solderWakeDropdown;

static lv_obj_t *ui_swVerLabel;

static lv_group_t *btn_group = NULL;

static void ui_knobs_dir_pressed(lv_event_t *e);
static void ui_hw_ver_pressed(lv_event_t *e);

static void settingPageUI_focused(lv_event_t *e)
{
    lv_group_focus_obj(ui_backBtn);
    lv_indev_set_group(knobs_indev, btn_group);
}

static void setSolderName(lv_obj_t *obj, const char *solderName)
{
    char name[16];
    int maxSize = lv_dropdown_get_option_cnt(obj);
    int cnt;
    for (cnt = 0; cnt < maxSize; cnt++)
    {
        lv_dropdown_set_selected(obj, cnt);
        lv_dropdown_get_selected_str(obj, name, 16);
        int ret = strcmp(solderName, name);
        if (ret == 0)
        {
            break;
        }
    }
}
static void setSolderSwitchType(lv_obj_t *obj, unsigned char type)
{
    uint8_t typeInd;
    switch (type)
    {
    case SOLDER_SHAKE_TYPE_NONE:
    {
        typeInd = 0;
    }
    break;
    case SOLDER_SHAKE_TYPE_HIGH:
    {
        typeInd = 1;
    }
    break;
    case SOLDER_SHAKE_TYPE_LOW:
    {
        typeInd = 2;
    }
    break;
    case SOLDER_SHAKE_TYPE_CHANGE:
    {
        typeInd = 3;
    }
    break;
    defualt:
        break;
    }
    lv_dropdown_set_selected(obj, typeInd);
}
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
    defualt:
        break;
    }
    lv_dropdown_set_selected(obj, verInd);
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

static void ui_auto_heat_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        if (lv_obj_has_state(target, LV_STATE_CHECKED))
            // SET_BIT(cfgKey1,CFG_KEY1_HP_AUTO_HEAT);
            sysInfoModel.uiGlobalParam.hpAutoHeatEnable = ENABLE_STATE_OPEN;
        else
            // CLR_BIT(cfgKey1,CFG_KEY1_HP_AUTO_HEAT);
            sysInfoModel.uiGlobalParam.hpAutoHeatEnable = ENABLE_STATE_CLOSE;
    }
}

static void ui_solder_grid_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        if (lv_obj_has_state(target, LV_STATE_CHECKED))
            // SET_BIT(cfgKey1,CFG_KEY1_SOLDER_GRID);
            sysInfoModel.uiGlobalParam.solderGridEnable = ENABLE_STATE_OPEN;
        else
            // CLR_BIT(cfgKey1,CFG_KEY1_SOLDER_GRID);
            sysInfoModel.uiGlobalParam.solderGridEnable = ENABLE_STATE_CLOSE;
    }
}

static void ui_air_hot_grid_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        if (lv_obj_has_state(target, LV_STATE_CHECKED))
            // SET_BIT(cfgKey1,CFG_KEY1_AIR_HOT_GRID);
            sysInfoModel.uiGlobalParam.airhotGridEnable = ENABLE_STATE_OPEN;
        else
            // CLR_BIT(cfgKey1,CFG_KEY1_AIR_HOT_GRID);
            sysInfoModel.uiGlobalParam.airhotGridEnable = ENABLE_STATE_CLOSE;
    }
}

static void ui_heat_plat_grid_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        if (lv_obj_has_state(target, LV_STATE_CHECKED))
            // SET_BIT(cfgKey1,CFG_KEY1_HEAT_PLAT_GRID);
            sysInfoModel.uiGlobalParam.heatplatGridEnable = ENABLE_STATE_OPEN;
        else
            // CLR_BIT(cfgKey1,CFG_KEY1_HEAT_PLAT_GRID);
            sysInfoModel.uiGlobalParam.heatplatGridEnable = ENABLE_STATE_CLOSE;
    }
}

static void ui_solder_name_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        // uint16_t index = lv_dropdown_get_selected(ui_solderNameDropdown); // 获取索引
        // solderModel.type = index;

        char id_text[16];
        char solderNameStr[16];
        // 烙铁名字的格式 id+'_'+type
        lv_dropdown_get_selected_str(ui_solderNameDropdown, solderNameStr, 16);
        int index;
        for (index = 0; solderNameStr[index] != '\0'; index++)
        {
            if (solderNameStr[index] == '_')
            {
                id_text[index] = '\0';
                break;
            }
            else
            {
                id_text[index] = solderNameStr[index];
            }
        }

        solderModel.curCoreID = 0;
        for (index = 0; id_text[index] != 0; index++)
        {
            solderModel.curCoreID = solderModel.curCoreID * 10 + id_text[index] - '0';
        }
        // solderModel.curCoreID = atoi(id_text);
    }
}

static void ui_solder_wake_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        uint16_t index = lv_dropdown_get_selected(ui_solderWakeDropdown); // 获取索引
        solderModel.wakeType = index;
    }
}

// 事件函数结束

static bool settingPageUI_init(lv_obj_t *father)
{
    bool isWhiteTheme = IS_WHITE_THEME;
    if (NULL != settingPageUI)
    {
        settingPageUI = NULL;
    }
    top_layer_set_name();
    theme_color_init();

    settingPageUI = lv_btn_create(father);
    settingUIObj.mainButtonUI = settingPageUI;

    lv_obj_remove_style_all(settingPageUI);
    lv_obj_set_size(settingPageUI, EACH_PAGE_SIZE_X, EACH_PAGE_SIZE_Y);
    lv_obj_center(settingPageUI);
    lv_obj_add_flag(settingPageUI, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(settingPageUI, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_style_bg_color(settingPageUI, isWhiteTheme ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(settingPageUI, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_ButtonTmp = settingPageUI;

    // 旋钮方向
    ui_knobsDirLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_knobsDirLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_label_set_text(ui_knobsDirLabel, SETTING_TEXT_KNOB_DIR);
    lv_obj_add_style(ui_knobsDirLabel, &label_text_style, 0);

    ui_knobsDirSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_add_flag(ui_knobsDirSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_knobsDirSwitch, 40, 20);
    lv_obj_align(ui_knobsDirSwitch, LV_ALIGN_TOP_LEFT, 105, 10);
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
    ui_whiteThemeLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_whiteThemeLabel, LV_ALIGN_TOP_LEFT, 10, 40);
    lv_label_set_text(ui_whiteThemeLabel, SETTING_TEXT_WHITE_THEME);
    lv_obj_add_style(ui_whiteThemeLabel, &label_text_style, 0);

    ui_whiteThemeSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_add_flag(ui_whiteThemeSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_whiteThemeSwitch, 40, 20);
    lv_obj_align(ui_whiteThemeSwitch, LV_ALIGN_TOP_LEFT, 105, 40);
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

    // 加热台自动加热
    ui_autoHeatLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_autoHeatLabel, LV_ALIGN_TOP_LEFT, 10, 70);
    lv_label_set_text(ui_autoHeatLabel, SETTING_TEXT_AUTO_HEAT);
    lv_obj_add_style(ui_autoHeatLabel, &label_text_style, 0);

    ui_autoHeatSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_add_flag(ui_autoHeatSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_autoHeatSwitch, 40, 20);
    lv_obj_align(ui_autoHeatSwitch, LV_ALIGN_TOP_LEFT, 105, 70);
    // if (GET_BIT(cfgKey1, CFG_KEY1_HP_AUTO_HEAT))
    if (ENABLE_STATE_OPEN == sysInfoModel.uiGlobalParam.hpAutoHeatEnable)
    {
        lv_obj_add_state(ui_autoHeatSwitch, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_clear_state(ui_autoHeatSwitch, LV_STATE_CHECKED);
    }
    lv_obj_set_style_outline_color(ui_autoHeatSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_autoHeatSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_autoHeatSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 烙铁曲线网格
    ui_solderGridLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_solderGridLabel, LV_ALIGN_TOP_LEFT, 10, 100);
    lv_label_set_text(ui_solderGridLabel, SETTING_TEXT_SOLDER_GRID);
    lv_obj_add_style(ui_solderGridLabel, &label_text_style, 0);

    ui_solderGridSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_add_flag(ui_solderGridSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_solderGridSwitch, 40, 20);
    lv_obj_align(ui_solderGridSwitch, LV_ALIGN_TOP_LEFT, 105, 100);
    // if (GET_BIT(cfgKey1, CFG_KEY1_SOLDER_GRID))
    if (ENABLE_STATE_OPEN == sysInfoModel.uiGlobalParam.solderGridEnable)
    {
        lv_obj_add_state(ui_solderGridSwitch, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_clear_state(ui_solderGridSwitch, LV_STATE_CHECKED);
    }
    lv_obj_set_style_outline_color(ui_solderGridSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_solderGridSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_solderGridSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 风枪曲线网格
    ui_airhotGridLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_airhotGridLabel, LV_ALIGN_TOP_LEFT, 10, 130);
    lv_label_set_text(ui_airhotGridLabel, SETTING_TEXT_AIR_HOT_GRID);
    lv_obj_add_style(ui_airhotGridLabel, &label_text_style, 0);

    ui_airhotGridSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_add_flag(ui_airhotGridSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_airhotGridSwitch, 40, 20);
    lv_obj_align(ui_airhotGridSwitch, LV_ALIGN_TOP_LEFT, 105, 130);
    // if (GET_BIT(cfgKey1, CFG_KEY1_AIR_HOT_GRID))
    if (ENABLE_STATE_OPEN == sysInfoModel.uiGlobalParam.airhotGridEnable)
    {
        lv_obj_add_state(ui_airhotGridSwitch, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_clear_state(ui_airhotGridSwitch, LV_STATE_CHECKED);
    }
    lv_obj_set_style_outline_color(ui_airhotGridSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_airhotGridSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_airhotGridSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 加热台曲线网格
    ui_heatplatGridLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_heatplatGridLabel, LV_ALIGN_TOP_LEFT, 10, 160);
    lv_label_set_text(ui_heatplatGridLabel, SETTING_TEXT_HEAT_PLAT_GRID);
    lv_obj_add_style(ui_heatplatGridLabel, &label_text_style, 0);

    ui_heatplatGridSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_add_flag(ui_heatplatGridSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_heatplatGridSwitch, 40, 20);
    lv_obj_align(ui_heatplatGridSwitch, LV_ALIGN_TOP_LEFT, 105, 160);
    // if (GET_BIT(cfgKey1, CFG_KEY1_HEAT_PLAT_GRID))
    if (ENABLE_STATE_OPEN == sysInfoModel.uiGlobalParam.heatplatGridEnable)
    {
        lv_obj_add_state(ui_heatplatGridSwitch, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_clear_state(ui_heatplatGridSwitch, LV_STATE_CHECKED);
    }
    lv_obj_set_style_outline_color(ui_heatplatGridSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_heatplatGridSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_heatplatGridSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 开始下拉菜单
    ui_solderNameLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_solderNameLabel, LV_ALIGN_TOP_LEFT, 180, 10);
    lv_label_set_text(ui_solderNameLabel, SETTING_TEXT_SOLDER_TYPE);
    lv_obj_add_style(ui_solderNameLabel, &label_text_style, 0);

    // static char solderName[] = "T12\nC210\nC245";
    ui_solderNameDropdown = lv_dropdown_create(ui_ButtonTmp);
    lv_dropdown_set_options(ui_solderNameDropdown, solderModel.coreNameList);
    // lv_dropdown_set_options_static(ui_solderNameDropdown, solderName);
    setSolderName(ui_solderNameDropdown, solderModel.curCoreName);
    lv_obj_set_size(ui_solderNameDropdown, 80, LV_SIZE_CONTENT);
    lv_obj_align(ui_solderNameDropdown, LV_ALIGN_TOP_LEFT, 180, 30);
    lv_obj_add_flag(ui_solderNameDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_text_font(ui_solderNameDropdown, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_solderNameDropdown, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_solderNameDropdown, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_solderNameDropdown, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    ui_solderWakeLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_solderWakeLabel, LV_ALIGN_TOP_LEFT, 180, 80);
    lv_label_set_text(ui_solderWakeLabel, SETTING_TEXT_SOLDER_WAKEUP);
    lv_obj_add_style(ui_solderWakeLabel, &label_text_style, 0);

    ui_solderWakeDropdown = lv_dropdown_create(ui_ButtonTmp);
    lv_dropdown_set_options(ui_solderWakeDropdown, "None\nHigh\nLow\nChange");
    setSolderSwitchType(ui_solderWakeDropdown, solderModel.wakeType);
    lv_obj_set_size(ui_solderWakeDropdown, 80, LV_SIZE_CONTENT);
    lv_obj_align(ui_solderWakeDropdown, LV_ALIGN_TOP_LEFT, 180, 100);
    lv_obj_add_flag(ui_solderWakeDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_text_font(ui_solderWakeDropdown, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_solderWakeDropdown, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_solderWakeDropdown, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_solderWakeDropdown, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    ui_hardVerLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_hardVerLabel, LV_ALIGN_TOP_LEFT, 180, 150);
    lv_label_set_text(ui_hardVerLabel, SETTING_TEXT_HW_VER);
    lv_obj_add_style(ui_hardVerLabel, &label_text_style, 0);

    static char hwVer[] = SHL_HW_VER;
    ui_hardVerDropdown = lv_dropdown_create(ui_ButtonTmp);
    // lv_dropdown_set_options(ui_hardVerDropdown, SHL_HW_VER);
    lv_dropdown_set_options_static(ui_hardVerDropdown, hwVer);
    setHwVer(ui_hardVerDropdown, sysInfoModel.outBoardVersion);
    lv_obj_set_size(ui_hardVerDropdown, 80, LV_SIZE_CONTENT);
    lv_obj_align(ui_hardVerDropdown, LV_ALIGN_TOP_LEFT, 180, 170);
    lv_obj_add_flag(ui_hardVerDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_text_font(ui_hardVerDropdown, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_hardVerDropdown, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_hardVerDropdown, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_hardVerDropdown, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 软件版本信息
    ui_swVerLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_swVerLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(ui_swVerLabel, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_set_style_text_color(ui_swVerLabel, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_swVerLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_swVerLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(ui_swVerLabel, "Ver %s", sysInfoModel.softwareVersion);

    lv_obj_add_event_cb(ui_knobsDirSwitch, ui_knobs_dir_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_whiteThemeSwitch, ui_white_theme_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_autoHeatSwitch, ui_auto_heat_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_solderGridSwitch, ui_solder_grid_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_airhotGridSwitch, ui_air_hot_grid_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_heatplatGridSwitch, ui_heat_plat_grid_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_solderNameDropdown, ui_solder_name_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_solderWakeDropdown, ui_solder_wake_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_hardVerDropdown, ui_hw_ver_pressed, LV_EVENT_VALUE_CHANGED, NULL);

    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_backBtn);
    lv_group_add_obj(btn_group, ui_knobsDirSwitch);
    lv_group_add_obj(btn_group, ui_whiteThemeSwitch);
    lv_group_add_obj(btn_group, ui_autoHeatSwitch);
    lv_group_add_obj(btn_group, ui_solderGridSwitch);
    lv_group_add_obj(btn_group, ui_airhotGridSwitch);
    lv_group_add_obj(btn_group, ui_heatplatGridSwitch);
    lv_group_add_obj(btn_group, ui_solderNameDropdown);
    lv_group_add_obj(btn_group, ui_solderWakeDropdown);
    lv_group_add_obj(btn_group, ui_hardVerDropdown);
    lv_group_focus_obj(ui_backBtn);
    lv_indev_set_group(knobs_indev, btn_group);

    return true;
}

void settingPageUI_release()
{
    if (NULL != settingPageUI)
    {
        lv_obj_del(settingPageUI);
        settingPageUI = NULL;
        settingUIObj.mainButtonUI = NULL;
    }

    if (NULL != btn_group)
    {
        lv_group_del(btn_group);
        btn_group = NULL;
    }
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
    }
}

FE_UI_OBJ settingUIObj = {settingPageUI, settingPageUI_init,
                          settingPageUI_release, settingPageUI_focused};

#endif