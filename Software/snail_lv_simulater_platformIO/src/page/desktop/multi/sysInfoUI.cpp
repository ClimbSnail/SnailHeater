#include "./ui.h"
#include "../desktop_model.h"

#ifdef MULTI_UI

static lv_obj_t *sysInfoPageUI = NULL;
static lv_obj_t *ui_settingImage;
static lv_obj_t *ui_sysInfoLabel;
static lv_obj_t *ui_knobsDirLabel;
static lv_obj_t *ui_knobsImage;
static lv_obj_t *ui_knobsDirDropdown;

static lv_obj_t *ui_hardVerImage;
static lv_obj_t *ui_hardVerDropdown;

static lv_obj_t *ui_swVerLabel; // 软件版本

static lv_obj_t *ui_moreButton;
static lv_obj_t *ui_moreButtonLabel;
static lv_obj_t *ui_backButton;
static lv_obj_t *ui_backImage;
static lv_obj_t *ui_backButtonLabel;
static lv_obj_t *ui_powerBar;
static lv_style_t chFontStyle;

static lv_group_t *btn_group = NULL;

static void ui_knobs_dir_pressed(lv_event_t *e);
static void ui_hw_ver_pressed(lv_event_t *e);
static void ui_back_btn_pressed(lv_event_t *e);

static void setknobsDir(lv_obj_t *obj, KNOBS_DIR dir)
{
    uint8_t dirInd;
    switch (dir)
    {
    case KNOBS_DIR_POS:
    {
        dirInd = 0;
    }
    break;
    case KNOBS_DIR_NEG:
    {
        dirInd = 1;
    }
    break;
    defualt:
        break;
    }
    lv_dropdown_set_selected(obj, dirInd);
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

static bool sysInfoPageUI_init(lv_obj_t *father)
{
    if (NULL != sysInfoPageUI)
    {
        return false;
    }

    sysInfoPageUI = lv_btn_create(father); // 黄色
    sysInfoUIObj.mainButtonUI = sysInfoPageUI;

    lv_obj_set_size(sysInfoPageUI, EACH_PAGE_SIZE_X, EACH_PAGE_SIZE_Y);
#ifdef NEW_UI
    lv_obj_set_align(sysInfoPageUI, LV_ALIGN_LEFT_MID);
#else
    lv_obj_set_pos(sysInfoPageUI, START_UI_OBJ_X, 0);
    lv_obj_set_align(sysInfoPageUI, LV_ALIGN_CENTER);
#endif
    lv_obj_add_flag(sysInfoPageUI, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(sysInfoPageUI, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_bg_color(sysInfoPageUI, lv_color_hex(0xB8860B), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(sysInfoPageUI, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(sysInfoPageUI, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_PRESSED);
    lv_obj_set_style_border_opa(sysInfoPageUI, 255, LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_PRESSED);

    lv_obj_t *ui_ButtonTmp = sysInfoPageUI;

    // 中文字体
    lv_style_init(&chFontStyle);
    lv_style_set_text_opa(&chFontStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_style_set_text_color(&chFontStyle, lv_color_hex(0xFFFFFF));
    lv_style_set_text_font(&chFontStyle, &sh_ch_font_14);

    // 系统设置图标
    ui_settingImage = lv_img_create(ui_ButtonTmp);
    lv_img_set_src(ui_settingImage, &setting_ico_32);
    // lv_obj_set_size(ui_settingImage, 16, 16);
    lv_obj_set_pos(ui_settingImage, -35, -75);
    lv_obj_set_align(ui_settingImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_settingImage, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_settingImage, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_sysInfoLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_sysInfoLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_sysInfoLabel, 15, -80);
    lv_obj_set_align(ui_sysInfoLabel, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_sysInfoLabel, SHL_SYSINFO);
    lv_obj_set_style_text_color(ui_sysInfoLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_sysInfoLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_sysInfoLabel, &sh_ch_font_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 旋钮图标
    ui_knobsImage = lv_img_create(ui_ButtonTmp);
    lv_img_set_src(ui_knobsImage, &knobs_16);
    // lv_obj_set_size(ui_knobsImage, 16, 16);
    lv_obj_set_pos(ui_knobsImage, -30, -23);
    lv_obj_set_align(ui_knobsImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_knobsImage, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_knobsImage, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    static char knobsDir[] = SHL_KNOBS_DIR;
    ui_knobsDirDropdown = lv_dropdown_create(ui_ButtonTmp);
    // lv_dropdown_set_options(ui_knobsDirDropdown, SHL_KNOBS_DIR);
    lv_dropdown_set_options_static(ui_knobsDirDropdown, knobsDir);
    setknobsDir(ui_knobsDirDropdown, sysInfoModel.knobDir);
    lv_obj_set_size(ui_knobsDirDropdown, 55, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_knobsDirDropdown, 20, -25);
    lv_obj_set_align(ui_knobsDirDropdown, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_knobsDirDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    // lv_obj_set_style_text_font(ui_knobsDirDropdown, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_knobsDirDropdown, &sh_ch_font_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_dropdown_set_dir(ui_knobsDirDropdown, LV_DROPDOWN_DIR_DOWN);
    // 注意：如果使用自定义的字库，并且你的字库中没有这些symbol符号，那么下拉列表的符号就不会显示了
    // lv_dropdown_set_symbol(ui_knobsDirDropdown, LV_SYMBOL_LEFT);
    lv_dropdown_set_symbol(ui_knobsDirDropdown, "V");
    lv_obj_add_style(ui_knobsDirDropdown, &focused_style, LV_STATE_FOCUSED);
    // 下拉框重新定义字体（中文字体必须此操作）
    lv_obj_t *ui_knobsDirDropdown_list = lv_dropdown_get_list(ui_knobsDirDropdown);
    lv_obj_set_style_text_font(ui_knobsDirDropdown_list, &sh_ch_font_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 硬件版本图标
    ui_hardVerImage = lv_img_create(ui_ButtonTmp);
    lv_img_set_src(ui_hardVerImage, &hardware_16);
    lv_obj_set_pos(ui_hardVerImage, -30, 17);
    lv_obj_set_align(ui_hardVerImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_hardVerImage, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_hardVerImage, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    static char hwVer[] = SHL_HW_VER;
    ui_hardVerDropdown = lv_dropdown_create(ui_ButtonTmp);
    // lv_dropdown_set_options(ui_hardVerDropdown, SHL_HW_VER);
    lv_dropdown_set_options_static(ui_hardVerDropdown, hwVer);
    setHwVer(ui_hardVerDropdown, sysInfoModel.outBoardVersion);
    lv_obj_set_size(ui_hardVerDropdown, 55, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_hardVerDropdown, 20, 15);
    lv_obj_set_align(ui_hardVerDropdown, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_hardVerDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    // lv_obj_set_style_text_font(ui_hardVerDropdown, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_hardVerDropdown, &sh_ch_font_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_dropdown_set_dir(ui_hardVerDropdown, LV_DROPDOWN_DIR_DOWN);
    // 注意：如果使用自定义的字库，并且你的字库中没有这些symbol符号，那么下拉列表的符号就不会显示了
    // lv_dropdown_set_symbol(ui_hardVerDropdown, LV_SYMBOL_LEFT);
    lv_dropdown_set_symbol(ui_hardVerDropdown, "V");
    lv_obj_add_style(ui_hardVerDropdown, &focused_style, LV_STATE_FOCUSED);
    // 下拉框重新定义字体（中文字体必须此操作）
    lv_obj_t *ui_hardVerDropdown_list = lv_dropdown_get_list(ui_hardVerDropdown);
    lv_obj_set_style_text_font(ui_hardVerDropdown_list, &sh_ch_font_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 软件版本信息
    ui_swVerLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_swVerLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_swVerLabel, 0, 50);
    lv_obj_set_align(ui_swVerLabel, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(ui_swVerLabel, lv_color_hex(0x1D1517), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_swVerLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_swVerLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(ui_swVerLabel, "Ver %s", sysInfoModel.softwareVersion);

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
    lv_obj_add_event_cb(ui_knobsDirDropdown, ui_knobs_dir_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_hardVerDropdown, ui_hw_ver_pressed, LV_EVENT_VALUE_CHANGED, NULL);

    return true;
}

void sysInfoPageUI_release()
{
    if (NULL != sysInfoPageUI)
    {
        lv_obj_del(sysInfoPageUI);
        sysInfoPageUI = NULL;
        sysInfoUIObj.mainButtonUI = NULL;
    }
}

static void ui_back_btn_pressed(lv_event_t *e)
{
    // 返回项被按下
    if (NULL != btn_group)
    {
        lv_group_del(btn_group);
        btn_group = NULL;
    }
    ui_main_pressed(e);
}

static void sysInfoPageUI_pressed(lv_event_t *e)
{
    // 系统设置项被按下

    // 创建操作的组
    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_knobsDirDropdown);
    lv_group_add_obj(btn_group, ui_hardVerDropdown);
    lv_group_add_obj(btn_group, ui_moreButton);
    lv_group_add_obj(btn_group, ui_backButton);
    lv_group_focus_obj(ui_backButton); // 聚焦到退出按键

    lv_indev_set_group(knobs_indev, btn_group);
}

static void ui_knobs_dir_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        uint16_t index = lv_dropdown_get_selected(ui_knobsDirDropdown); // 获取索引
        sysInfoModel.knobDir = (KNOBS_DIR)index;
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

FE_UI_OBJ sysInfoUIObj = {sysInfoPageUI, sysInfoPageUI_init,
                          sysInfoPageUI_release, sysInfoPageUI_pressed};

#endif