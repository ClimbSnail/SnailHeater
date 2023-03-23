

#include "ui.h"
#include "desktop_model.h"

#ifndef NEW_UI

#define FONT_DEBUG 1

static lv_obj_t *solderPageUI = NULL;
static lv_obj_t *ui_curTempLabel;
static lv_obj_t *ui_curTempCLabel;
static lv_obj_t *ui_setTempButton;
static lv_obj_t *ui_setTempLabel;
static lv_obj_t *ui_setTempArc;
static lv_obj_t *ui_solderSetTempTextArea;
static lv_obj_t *ui_solderSetTempCLabel;
static lv_obj_t *ui_typeLabel;
static lv_obj_t *ui_solderImage;
static lv_obj_t *ui_solderTypeDropdown;
static lv_obj_t *ui_solderWakeLabel;
static lv_obj_t *ui_wakeImage;
static lv_obj_t *ui_solderWakeDropdown;
static lv_obj_t *ui_solderNumLabel;
static lv_obj_t *ui_solderDropdown;
static lv_obj_t *ui_moreButton;
static lv_obj_t *ui_moreButtonLabel;
static lv_obj_t *ui_backButton;
static lv_obj_t *ui_backImage;
static lv_obj_t *ui_backButtonLabel;
static lv_obj_t *ui_powerBar;
static lv_style_t chFontStyle;

static lv_group_t *btn_group;
static lv_group_t *setTempArcGroup;

static void ui_type_pressed(lv_event_t *e);
static void ui_wake_type_changed(lv_event_t *e);
static void ui_set_temp_btn_pressed(lv_event_t *e);
static void ui_back_btn_pressed(lv_event_t *e);

static void setSolderType(lv_obj_t *obj, unsigned char type)
{
    uint8_t typeInd;
    switch (type)
    {
    case SOLDER_TYPE_T12:
    {
        typeInd = 0;
    }
    break;
    case SOLDER_TYPE_JBC210:
    {
        typeInd = 1;
    }
    break;
    case SOLDER_TYPE_JBC245:
    {
        typeInd = 2;
    }
    break;
    defualt:
        break;
    }
    lv_dropdown_set_selected(obj, typeInd);
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

static bool solderPageUI_init(lv_obj_t *father)
{
    if (NULL != solderPageUI)
    {
        return false;
    }

    solderPageUI = lv_btn_create(father); // 黄色
    solderUIObj.mainButtonUI = solderPageUI;

    lv_obj_set_size(solderPageUI, EACH_PAGE_SIZE_X, EACH_PAGE_SIZE_Y);
    

#ifdef NEW_UI
    lv_obj_set_align(solderPageUI, LV_ALIGN_LEFT_MID);
#else
    lv_obj_set_pos(solderPageUI, START_UI_OBJ_X, 0);
    lv_obj_set_align(solderPageUI, LV_ALIGN_CENTER);
#endif



    lv_obj_add_flag(solderPageUI, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(solderPageUI, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_bg_color(solderPageUI, lv_color_hex(0xDFD338), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(solderPageUI, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    //lv_obj_set_style_border_color(solderPageUI, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_PRESSED);
    //lv_obj_set_style_border_opa(solderPageUI, 255, LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_PRESSED);

    lv_obj_t *ui_ButtonTmp = solderPageUI;

    // 中文字体
    lv_style_init(&chFontStyle);
    lv_style_set_text_opa(&chFontStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_style_set_text_color(&chFontStyle, lv_color_hex(0x1D1517));
    lv_style_set_text_font(&chFontStyle, &sh_ch_font_14);

    ui_curTempLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_curTempLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_curTempLabel, 0, -70);
    lv_obj_set_align(ui_curTempLabel, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_curTempLabel, "%d", solderModel.curTemp);
    lv_obj_set_style_text_color(ui_curTempLabel, lv_color_hex(0x8168F7), LV_PART_MAIN | LV_STATE_DEFAULT);
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
    lv_label_set_text_fmt(ui_setTempLabel, "%d°C", solderModel.targetTemp);
    lv_obj_set_style_text_color(ui_setTempLabel, lv_color_hex(0xF76889), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_setTempLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setTempLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ui_solderSetTempTextArea = lv_textarea_create(ui_ButtonTmp);
    // lv_obj_set_size(ui_solderSetTempTextArea, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    // lv_obj_set_pos(ui_solderSetTempTextArea, 0, -25);
    // lv_obj_set_align(ui_solderSetTempTextArea, LV_ALIGN_CENTER);
    // char setTemp[8];
    // snprintf(setTemp, 8, "%d°C", solderModel.targetTemp);
    // lv_textarea_set_text(ui_solderSetTempTextArea, setTemp);
    // lv_textarea_set_one_line(ui_solderSetTempTextArea, true);
    // lv_obj_set_style_border_color(ui_solderSetTempTextArea, lv_color_hex(0xF76889), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_border_opa(ui_solderSetTempTextArea, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_blend_mode(ui_solderSetTempTextArea, LV_BLEND_MODE_MULTIPLY, LV_PART_MAIN | LV_STATE_DEFAULT);

    //     // 烙铁芯类型选择
    //     ui_typeLabel = lv_label_create(ui_ButtonTmp);
    //     lv_obj_set_size(ui_typeLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    //     lv_obj_set_pos(ui_typeLabel, -30, 12);
    //     lv_obj_set_align(ui_typeLabel, LV_ALIGN_CENTER);
    // #if FONT_DEBUG == 1
    //     lv_obj_set_style_text_color(ui_typeLabel, lv_color_hex(0x1D1517), LV_PART_MAIN | LV_STATE_DEFAULT);
    //     lv_obj_set_style_text_opa(ui_typeLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    //     lv_obj_set_style_text_font(ui_typeLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    //     // lv_obj_set_style_text_font(ui_typeLabel, &sh_ch_font_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    // #else
    //     lv_obj_add_style(ui_typeLabel, &chFontStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    // #endif
    //     lv_label_set_text(ui_typeLabel, "Type");
    //     // lv_label_set_text(ui_typeLabel, "类型");

    // 烙铁芯图标
    ui_solderImage = lv_img_create(ui_ButtonTmp);
    lv_img_set_src(ui_solderImage, &solder_16);
    // lv_obj_set_size(ui_settingImage, 16, 16);
    lv_obj_set_pos(ui_solderImage, -30, 12);
    lv_obj_set_align(ui_solderImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_solderImage, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_solderImage, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    static char solderType[] = "T12\nC210\nC245";
    ui_solderTypeDropdown = lv_dropdown_create(ui_ButtonTmp);
    // lv_dropdown_set_options(ui_solderTypeDropdown, "T12\nC210\nC245");
    lv_dropdown_set_options_static(ui_solderTypeDropdown, solderType);
    setSolderType(ui_solderTypeDropdown, solderModel.type);
    lv_obj_set_size(ui_solderTypeDropdown, 55, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_solderTypeDropdown, 20, 10);
    lv_obj_set_align(ui_solderTypeDropdown, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_solderTypeDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_set_style_text_font(ui_solderTypeDropdown, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_solderTypeDropdown, &focused_style, LV_STATE_FOCUSED);

    //     ui_solderWakeLabel = lv_label_create(ui_ButtonTmp);
    //     lv_obj_set_size(ui_solderWakeLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    //     lv_obj_set_pos(ui_solderWakeLabel, -30, 47);
    //     lv_obj_set_align(ui_solderWakeLabel, LV_ALIGN_CENTER);
    // #if FONT_DEBUG == 1
    //     lv_obj_set_style_text_color(ui_solderWakeLabel, lv_color_hex(0x1D1517), LV_PART_MAIN | LV_STATE_DEFAULT);
    //     lv_obj_set_style_text_opa(ui_solderWakeLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    //     lv_obj_set_style_text_font(ui_solderWakeLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    //     // lv_obj_set_style_text_font(ui_solderWakeLabel, &sh_ch_font_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    // #else
    //     lv_obj_add_style(ui_solderWakeLabel, &chFontStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    // #endif
    //     lv_label_set_text(ui_solderWakeLabel, "Wake");
    //     // lv_label_set_text(ui_solderWakeLabel, "烙铁芯");

    // 唤醒图标
    ui_wakeImage = lv_img_create(ui_ButtonTmp);
    lv_img_set_src(ui_wakeImage, &wake_16);
    // lv_obj_set_size(ui_settingImage, 16, 16);
    lv_obj_set_pos(ui_wakeImage, -30, 47);
    lv_obj_set_align(ui_wakeImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_wakeImage, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_wakeImage, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_solderWakeDropdown = lv_dropdown_create(ui_ButtonTmp);
    lv_dropdown_set_options(ui_solderWakeDropdown, "None\nHigh\nLow\nChange");
    setSolderSwitchType(ui_solderWakeDropdown, solderModel.wakeType);
    lv_obj_set_size(ui_solderWakeDropdown, 55, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_solderWakeDropdown, 20, 45);
    lv_obj_set_align(ui_solderWakeDropdown, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_solderWakeDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_set_style_text_font(ui_solderWakeDropdown, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_solderWakeDropdown, &focused_style, LV_STATE_FOCUSED);

    //     ui_solderNumLabel = lv_label_create(ui_ButtonTmp);
    //     lv_obj_set_size(ui_solderNumLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    //     lv_obj_set_pos(ui_solderNumLabel, -30, 47);
    //     lv_obj_set_align(ui_solderNumLabel, LV_ALIGN_CENTER);
    // #if FONT_DEBUG == 1
    //     lv_obj_set_style_text_color(ui_solderNumLabel, lv_color_hex(0x1D1517), LV_PART_MAIN | LV_STATE_DEFAULT);
    //     lv_obj_set_style_text_opa(ui_solderNumLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    //     lv_obj_set_style_text_font(ui_solderNumLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    //     // lv_obj_set_style_text_font(ui_solderNumLabel, &sh_ch_font_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    // #else
    //     lv_obj_add_style(ui_solderNumLabel, &chFontStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    // #endif
    //     lv_label_set_text(ui_solderNumLabel, "Num");
    //     // lv_label_set_text(ui_solderNumLabel, "烙铁芯");

    // ui_solderDropdown = lv_dropdown_create(ui_ButtonTmp);
    // lv_dropdown_set_options(ui_solderDropdown, "#1\n#2");
    // lv_obj_set_size(ui_solderDropdown, 55, LV_SIZE_CONTENT);
    // lv_obj_set_pos(ui_solderDropdown, 20, 45);
    // lv_obj_set_align(ui_solderDropdown, LV_ALIGN_CENTER);
    // lv_obj_add_flag(ui_solderDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    // lv_obj_set_style_text_font(ui_solderDropdown, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_add_style(ui_solderDropdown, &focused_style, LV_STATE_FOCUSED);

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
    // ui_backButton = lv_imgbtn_create(ui_ButtonTmp);
    // lv_imgbtn_set_src(ui_backButton, LV_IMGBTN_STATE_RELEASED, &back_16, NULL, NULL);
    // lv_imgbtn_set_src(ui_backButton, LV_IMGBTN_STATE_PRESSED, &back_16, NULL, NULL);
    // lv_obj_set_size(ui_backButton, back_16.header.w, back_16.header.h);
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
    lv_obj_add_event_cb(ui_solderTypeDropdown, ui_type_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_solderWakeDropdown, ui_wake_type_changed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_setTempButton, ui_set_temp_btn_pressed, LV_EVENT_PRESSED, NULL);

    setTempArcGroup = lv_group_create();
    return true;
}

void solderPageUI_release()
{
    if (NULL == solderPageUI)
    {
        return;
    }
    lv_obj_clean(solderPageUI);
    solderPageUI = NULL;
    solderUIObj.mainButtonUI = NULL;
}

void ui_updateSolderCurTempAndPowerDuty(void)
{
    if (NULL == solderPageUI)
    {
        return;
    }

    if (solderModel.curTemp >= DISCONNCT_TEMP)
    {
        // 未连接
        lv_label_set_text_fmt(ui_curTempLabel, "xxx");
    }
    else
    {
        lv_label_set_text_fmt(ui_curTempLabel, "%d", solderModel.curTemp);
    }

    lv_bar_set_value(ui_powerBar, (int32_t)solderModel.powerRatio, LV_ANIM_ON);
}

static void ui_updateSolderWarkState(void)
{
}

static void ui_back_btn_pressed(lv_event_t *e)
{
    // 返回项被按下
    lv_group_del(btn_group);
    ui_main_pressed(e);
}

static void solderPageUI_pressed(lv_event_t *e)
{
    // 烙铁项被按下
    
    // 创建操作的组
    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_setTempButton);
    // lv_group_add_obj(btn_group, ui_solderSetTempTextArea);
    lv_group_add_obj(btn_group, ui_solderTypeDropdown);
    lv_group_add_obj(btn_group, ui_solderWakeDropdown);
    // lv_group_add_obj(btn_group, ui_solderDropdown);
    lv_group_add_obj(btn_group, ui_moreButton);
    lv_group_add_obj(btn_group, ui_backButton);
    lv_group_focus_obj(ui_backButton); // 聚焦到退出按键

    lv_indev_set_group(knobs_indev, btn_group);
}

static void ui_type_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        uint16_t index = lv_dropdown_get_selected(ui_solderTypeDropdown); // 获取索引
        solderModel.type = index;
    }
}

static void ui_wake_type_changed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        uint16_t index = lv_dropdown_get_selected(ui_solderWakeDropdown); // 获取索引
        solderModel.wakeType = index;
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

static void ui_solder_set_tempArc_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        solderModel.targetTemp = lv_arc_get_value(ui_setTempArc);

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
        lv_arc_set_value(ui_setTempArc, solderModel.targetTemp);
        // 设置隐藏
        lv_obj_set_style_opa(ui_setTempArc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_group_add_obj(setTempArcGroup, ui_setTempArc);
        lv_indev_set_group(knobs_indev, setTempArcGroup);
        lv_group_focus_obj(ui_setTempArc);

        lv_obj_add_event_cb(ui_setTempArc, ui_solder_set_tempArc_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_setTempArc, ui_set_tempArc_changed, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

FE_UI_OBJ solderUIObj = {solderPageUI, solderPageUI_init,
                         solderPageUI_release, solderPageUI_pressed};

#endif