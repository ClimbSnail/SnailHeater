#include "./settingUIFull.h"

#ifdef FULL_UI

#include "../ui.h"
#include "../../desktop_model.h"

static lv_timer_t *userdataTimer = NULL;

static lv_obj_t *ui_father = NULL;
static lv_obj_t *ui_resetSysUtilBnt;
static lv_obj_t *ui_resetSolderUtilBnt;
static lv_obj_t *ui_resetSolderCoreBnt;
static lv_obj_t *ui_resetAHUtilBnt;
static lv_obj_t *ui_resetAHCoreBnt;
static lv_obj_t *ui_resetHPUtilBnt;
static lv_obj_t *ui_resetHPCoreBnt;
static lv_obj_t *ui_resetHPCurveBnt;
static lv_obj_t *ui_resetPowerUtilBnt;
static lv_obj_t *ui_resetAllBnt;

static lv_obj_t *ui_passwd_obj = NULL;
static lv_obj_t *ui_Keyboard = NULL;
static lv_obj_t *ui_keyLabel;
static lv_obj_t *ui_keyTextArea;
lv_style_t keyboard_style;
lv_obj_t *ui_triggerObj; // 保存触发对象
// 密码输入完毕的回调函数
void (*ui_verification_cb)(lv_obj_t *obj, bool);

static lv_group_t *key_group = NULL;

static void userdataTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    userdata_verification_release();
    ui_verification_cb(ui_triggerObj, true);
}

static void textarea_event_handler(lv_event_t *e)
{
    lv_obj_t *ta = lv_event_get_target(e);
    LV_LOG_USER("Enter was pressed. The current text is: %s", lv_textarea_get_text(ta));
}

static void userdata_verification_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_user_data(e);

    // const char *txt = lv_btnmatrix_get_btn_text(obj, lv_btnmatrix_get_selected_btn(obj));

    // if (strcmp(txt, "x") == 0)
    //     lv_textarea_del_char(ta);
    // else if (strcmp(txt, "退出") == 0)
    //     lv_event_send(ta, LV_EVENT_READY, NULL);
    // else
    //     lv_textarea_add_text(ta, txt);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        uint16_t key_code = lv_keyboard_get_selected_btn(ui_Keyboard);
        LV_LOG_USER("key_code = %u", key_code);
        if (7 == key_code)
        {
            const char *text = lv_textarea_get_text(ui_keyTextArea);
            if (strcmp(text, "8888") == 0)
            {
                lv_obj_add_flag(ui_passwd_obj, LV_OBJ_FLAG_HIDDEN);
                userdataTimer = lv_timer_create(userdataTimer_timeout, 800, NULL);
                lv_timer_set_repeat_count(userdataTimer, 1);
                return;
            }

            // userdataTimer = lv_timer_create(userdataTimer_timeout, 800, NULL);
            // lv_timer_set_repeat_count(userdataTimer, 1);
            // return;
        }
        else if (3 == key_code)
        {
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(ui_passwd_obj, LV_OBJ_FLAG_HIDDEN);

            ui_verification_cb(ui_triggerObj, false);
            userdata_verification_release();
            return;
        }
    }

    // if (event_code == LV_EVENT_FOCUSED)
    // {
    //     lv_keyboard_set_textarea(obj, ta);
    //     lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    // }

    // if (event_code == LV_EVENT_DEFOCUSED)
    // {
    //     lv_keyboard_set_textarea(obj, NULL);
    //     lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    // }
}

void userdata_verification_init(lv_obj_t *father, lv_obj_t *triggerObj,
                                lv_indev_t *indev,
                                void (*verification_cb)(lv_obj_t *obj, bool))
{
    userdata_verification_release();

    ui_triggerObj = triggerObj;
    ui_verification_cb = verification_cb;

    ui_passwd_obj = lv_obj_create(father);
    lv_obj_set_size(ui_passwd_obj, 180, 150);
    lv_obj_set_pos(ui_passwd_obj, 0, lv_obj_get_y(ui_triggerObj) - 150);
    lv_obj_add_style(ui_passwd_obj, &black_white_theme_style,
                     LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_color(ui_passwd_obj, lv_color_white(),
    //                  LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_keyLabel = lv_label_create(ui_passwd_obj);
    lv_obj_set_size(ui_keyLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(ui_keyLabel, LV_ALIGN_TOP_LEFT, 0, 5);
    lv_label_set_text(ui_keyLabel, "超级密码");

    ui_keyTextArea = lv_textarea_create(ui_passwd_obj);
    lv_obj_set_size(ui_keyTextArea, 80, LV_SIZE_CONTENT);
    lv_obj_align(ui_keyTextArea, LV_ALIGN_TOP_RIGHT, 0, -5);
    lv_textarea_set_one_line(ui_keyTextArea, true);
    lv_obj_add_event_cb(ui_keyTextArea, textarea_event_handler, LV_EVENT_READY, ui_keyTextArea);
    lv_obj_add_state(ui_keyTextArea, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/

    lv_style_init(&keyboard_style);
    lv_style_set_radius(&keyboard_style, 4);
    lv_style_set_border_width(&keyboard_style, 0);
    lv_style_set_text_opa(&keyboard_style, 255);
    // lv_style_set_text_font(&keyboard_style, &FontDeyi_16);
    lv_style_set_text_font(&keyboard_style, &lv_font_montserrat_14);
    lv_style_set_border_color(&keyboard_style, BTN_TYPE1_BORDER_COLOR);
    lv_style_set_text_color(&keyboard_style, BTN_TYPE1_TEXT_COLOR);

    // static const char *btnm_map[] = {"1", "2", "3", "\n",
    //                                  "4", "5", "6", "\n",
    //                                  "7", "8", "9", "\n",
    //                                  "0", "x", "退出", ""};
    // //    LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_NEW_LINE, ""};

    // ui_Keyboard = lv_btnmatrix_create(ui_obj);
    // lv_obj_remove_style_all(ui_Keyboard);
    // lv_obj_set_size(ui_Keyboard, 150, 80);
    // lv_obj_align(ui_Keyboard, LV_ALIGN_CENTER, 0, 20);
    // lv_obj_clear_flag(ui_Keyboard, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    // lv_btnmatrix_set_map(ui_Keyboard, btnm_map);
    // lv_obj_add_style(ui_Keyboard, &keyboard_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_add_event_cb(ui_Keyboard, userdata_verification_cb,
    //                     LV_EVENT_VALUE_CHANGED, ui_keyTextArea);

    ui_Keyboard = lv_keyboard_create(ui_passwd_obj);
    // lv_obj_remove_style_all(ui_Keyboard);
    lv_obj_set_size(ui_Keyboard, 150, 80);
    lv_obj_align(ui_Keyboard, LV_ALIGN_CENTER, 0, 20);
    lv_keyboard_set_mode(ui_Keyboard, LV_KEYBOARD_MODE_NUMBER);
    lv_keyboard_set_textarea(ui_Keyboard, ui_keyTextArea);
    lv_obj_add_style(ui_Keyboard, &keyboard_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_Keyboard, userdata_verification_cb, LV_EVENT_ALL, ui_Keyboard);

    if (NULL != key_group)
    {
        lv_group_del(key_group);
        key_group = NULL;
    }
    key_group = lv_group_create();
    lv_group_add_obj(key_group, ui_Keyboard);
    lv_indev_set_group(indev, key_group);
    // lv_group_focus_obj(ui_Keyboard);
}

void userdata_verification_release(void)
{
    if (NULL != key_group)
    {
        lv_group_del(key_group);
        key_group = NULL;
    }

    if (NULL != ui_passwd_obj)
    {
        lv_obj_del(ui_passwd_obj);
        ui_passwd_obj = NULL;
        ui_Keyboard = NULL;
        ui_keyLabel = NULL;
        ui_keyTextArea = NULL;
    }
    return;
}

void ui_userdata_group(lv_obj_t *father);

static void ui_bnt_obj_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_PRESSED)
    {
        if (target == ui_resetSysUtilBnt)
        {
            sysInfoModel.manageConfigAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_UTIL_RESET;
            sysInfoModel.resetFlag = true;
        }
        else if (target == ui_resetSolderUtilBnt)
        {
            solderModel.manageConfigAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_UTIL_RESET;
            sysInfoModel.resetFlag = true;
        }
        else if (target == ui_resetSolderCoreBnt)
        {
            solderModel.manageConfigAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_CORE_RESET;
            sysInfoModel.resetFlag = true;
        }
        else if (target == ui_resetAHUtilBnt)
        {
            airhotModel.manageConfigAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_UTIL_RESET;
            sysInfoModel.resetFlag = true;
        }
        else if (target == ui_resetAHCoreBnt)
        {
            airhotModel.manageCoreAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_CORE_RESET;
            sysInfoModel.resetFlag = true;
        }
        else if (target == ui_resetHPUtilBnt)
        {
            heatplatformModel.manageConfigAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_UTIL_RESET;
            sysInfoModel.resetFlag = true;
        }
        else if (target == ui_resetHPCoreBnt)
        {
            heatplatformModel.manageCoreAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_CORE_RESET;
            sysInfoModel.resetFlag = true;
        }
        else if (target == ui_resetHPCurveBnt)
        {
            heatplatformModel.manageCurveAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_UTIL_RESET;
            sysInfoModel.resetFlag = true;
        }
        else if (target == ui_resetPowerUtilBnt)
        {
            adjPowerModel.manageConfigAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_UTIL_RESET;
            sysInfoModel.resetFlag = true;
        }
        else if (target == ui_resetAllBnt)
        {
            sysInfoModel.manageConfigAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_UTIL_RESET;
            solderModel.manageConfigAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_UTIL_RESET;
            solderModel.manageConfigAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_CORE_RESET;
            airhotModel.manageConfigAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_UTIL_RESET;
            airhotModel.manageCoreAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_CORE_RESET;
            heatplatformModel.manageConfigAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_UTIL_RESET;
            heatplatformModel.manageCoreAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_CORE_RESET;
            heatplatformModel.manageCurveAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_UTIL_RESET;
            adjPowerModel.manageConfigAction =
                INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CONFIG_UTIL_RESET;

            sysInfoModel.resetFlag = true;
        }
    }
}

void ui_userdata_init(lv_obj_t *father)
{
    ui_father = father;

    /*Create style*/
    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 4);
    // LV_PALETTE_GREY LV_PALETTE_BLUE
    lv_style_set_line_color(&style_line, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_line_rounded(&style_line, true);

    // 系统配置
    lv_obj_t *ui_resetSysLabel = lv_label_create(ui_father);
    lv_obj_align(ui_resetSysLabel, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_label_set_text(ui_resetSysLabel, "系统");
    lv_obj_add_style(ui_resetSysLabel, &label_text_style, 0);

    static lv_point_t sysLine_points[] = {{0, 0}, {0, 30}};
    lv_obj_t *sysLine = lv_line_create(ui_father);
    lv_line_set_points(sysLine, sysLine_points, 2); /*Set the points*/
    lv_obj_add_style(sysLine, &style_line, 0);
    lv_obj_align_to(sysLine, ui_resetSysLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 7, 0);

    // 通用配置重置
    lv_obj_t *ui_resetSysUtilLabel = lv_label_create(ui_father);
    lv_obj_align_to(ui_resetSysUtilLabel, sysLine,
                    LV_ALIGN_OUT_RIGHT_TOP, 5, 5);
    lv_label_set_text(ui_resetSysUtilLabel, SETTING_TEXT_CONFIG_UTIL_RESET);
    lv_obj_add_style(ui_resetSysUtilLabel, &label_text_style, 0);

    ui_resetSysUtilBnt = lv_btn_create(ui_father);
    // lv_obj_remove_style_all(ui_resetSysUtilBnt);
    lv_obj_set_size(ui_resetSysUtilBnt, 40, 20);
    lv_obj_align_to(ui_resetSysUtilBnt, ui_resetSysUtilLabel,
                    LV_ALIGN_OUT_LEFT_MID, 135, 0);
    lv_obj_add_flag(ui_resetSysUtilBnt, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_resetSysUtilBnt, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_resetSysUtilBnt, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_resetSysUtilBnt, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_resetSysUtilBnt, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_resetSysUtilBnt, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    lv_obj_t *ui_resetSysUtilBntLabel = lv_label_create(ui_resetSysUtilBnt);
    lv_obj_align(ui_resetSysUtilBntLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui_resetSysUtilBntLabel, SETTING_TEXT_CONFIG_RESET);
    lv_obj_add_style(ui_resetSysUtilBntLabel, &label_text_style, 0);

    // 烙铁配置
    lv_obj_t *ui_resetSolderLabel = lv_label_create(ui_father);
    lv_obj_align_to(ui_resetSolderLabel, ui_resetSysLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 50);
    lv_label_set_text(ui_resetSolderLabel, "烙铁");
    lv_obj_add_style(ui_resetSolderLabel, &label_text_style, 0);

    static lv_point_t solderLine_points[] = {{0, 0}, {0, 55}};
    lv_obj_t *solderLine = lv_line_create(ui_father);
    lv_line_set_points(solderLine, solderLine_points, 2); /*Set the points*/
    lv_obj_add_style(solderLine, &style_line, 0);
    lv_obj_align_to(solderLine, ui_resetSolderLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 7, 0);

    // 烙铁通用配置重置
    lv_obj_t *ui_resetSolderUtilLabel = lv_label_create(ui_father);
    lv_obj_align_to(ui_resetSolderUtilLabel, solderLine,
                    LV_ALIGN_OUT_RIGHT_TOP, 5, 5);
    lv_label_set_text(ui_resetSolderUtilLabel, SETTING_TEXT_CONFIG_UTIL_RESET);
    lv_obj_add_style(ui_resetSolderUtilLabel, &label_text_style, 0);

    ui_resetSolderUtilBnt = lv_btn_create(ui_father);
    // lv_obj_remove_style_all(ui_resetSolderUtilBnt);
    lv_obj_set_size(ui_resetSolderUtilBnt, 40, 20);
    lv_obj_align_to(ui_resetSolderUtilBnt, ui_resetSolderUtilLabel,
                    LV_ALIGN_OUT_LEFT_MID, 135, 0);
    lv_obj_add_flag(ui_resetSolderUtilBnt, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_resetSolderUtilBnt, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_resetSolderUtilBnt, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_resetSolderUtilBnt, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_resetSolderUtilBnt, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_resetSolderUtilBnt, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    lv_obj_t *ui_resetSolderUtilBntLabel = lv_label_create(ui_resetSolderUtilBnt);
    lv_obj_align(ui_resetSolderUtilBntLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui_resetSolderUtilBntLabel, SETTING_TEXT_CONFIG_RESET);
    lv_obj_add_style(ui_resetSolderUtilBntLabel, &label_text_style, 0);

    // 烙铁发热芯配置重置
    lv_obj_t *ui_resetSolderCoreLabel = lv_label_create(ui_father);
    lv_obj_align_to(ui_resetSolderCoreLabel, solderLine,
                    LV_ALIGN_OUT_RIGHT_TOP, 5, 30);
    lv_label_set_text(ui_resetSolderCoreLabel, SETTING_TEXT_CONFIG_CORE_RESET);
    lv_obj_add_style(ui_resetSolderCoreLabel, &label_text_style, 0);

    ui_resetSolderCoreBnt = lv_btn_create(ui_father);
    // lv_obj_remove_style_all(ui_resetSolderCoreBnt);
    lv_obj_set_size(ui_resetSolderCoreBnt, 40, 20);
    lv_obj_align_to(ui_resetSolderCoreBnt, ui_resetSolderCoreLabel,
                    LV_ALIGN_OUT_LEFT_MID, 135, 0);
    lv_obj_add_flag(ui_resetSolderCoreBnt, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_resetSolderCoreBnt, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_resetSolderCoreBnt, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_resetSolderCoreBnt, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_resetSolderCoreBnt, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_resetSolderCoreBnt, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    lv_obj_t *ui_resetSolderCoreBntLabel = lv_label_create(ui_resetSolderCoreBnt);
    lv_obj_align(ui_resetSolderCoreBntLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui_resetSolderCoreBntLabel, SETTING_TEXT_CONFIG_RESET);
    lv_obj_add_style(ui_resetSolderCoreBntLabel, &label_text_style, 0);

    // 风枪配置
    lv_obj_t *ui_resetAHLabel = lv_label_create(ui_father);
    lv_obj_align_to(ui_resetAHLabel, ui_resetSolderLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 75);
    lv_label_set_text(ui_resetAHLabel, "风枪");
    lv_obj_add_style(ui_resetAHLabel, &label_text_style, 0);

    static lv_point_t aHLine_points[] = {{0, 0}, {0, 55}};
    lv_obj_t *aHLine = lv_line_create(ui_father);
    lv_line_set_points(aHLine, aHLine_points, 2); /*Set the points*/
    lv_obj_add_style(aHLine, &style_line, 0);
    lv_obj_align_to(aHLine, ui_resetAHLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 7, 0);

    // 风枪通用配置重置
    lv_obj_t *ui_resetAHUtilLabel = lv_label_create(ui_father);
    lv_obj_align_to(ui_resetAHUtilLabel, aHLine,
                    LV_ALIGN_OUT_RIGHT_TOP, 5, 5);
    lv_label_set_text(ui_resetAHUtilLabel, SETTING_TEXT_CONFIG_UTIL_RESET);
    lv_obj_add_style(ui_resetAHUtilLabel, &label_text_style, 0);

    ui_resetAHUtilBnt = lv_btn_create(ui_father);
    // lv_obj_remove_style_all(ui_resetAHUtilBnt);
    lv_obj_set_size(ui_resetAHUtilBnt, 40, 20);
    lv_obj_align_to(ui_resetAHUtilBnt, ui_resetAHUtilLabel,
                    LV_ALIGN_OUT_LEFT_MID, 135, 0);
    lv_obj_add_flag(ui_resetAHUtilBnt, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_resetAHUtilBnt, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_resetAHUtilBnt, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_resetAHUtilBnt, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_resetAHUtilBnt, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_resetAHUtilBnt, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    lv_obj_t *ui_resetAHUtilBntLabel = lv_label_create(ui_resetAHUtilBnt);
    lv_obj_align(ui_resetAHUtilBntLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui_resetAHUtilBntLabel, SETTING_TEXT_CONFIG_RESET);
    lv_obj_add_style(ui_resetAHUtilBntLabel, &label_text_style, 0);

    // 风枪发热芯配置重置
    lv_obj_t *ui_resetAHCoreLabel = lv_label_create(ui_father);
    lv_obj_align_to(ui_resetAHCoreLabel, aHLine,
                    LV_ALIGN_OUT_RIGHT_TOP, 5, 30);
    lv_label_set_text(ui_resetAHCoreLabel, SETTING_TEXT_CONFIG_CORE_RESET);
    lv_obj_add_style(ui_resetAHCoreLabel, &label_text_style, 0);

    ui_resetAHCoreBnt = lv_btn_create(ui_father);
    // lv_obj_remove_style_all(ui_resetAHCoreBnt);
    lv_obj_set_size(ui_resetAHCoreBnt, 40, 20);
    lv_obj_align_to(ui_resetAHCoreBnt, ui_resetAHCoreLabel,
                    LV_ALIGN_OUT_LEFT_MID, 135, 0);
    lv_obj_add_flag(ui_resetAHCoreBnt, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_resetAHCoreBnt, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_resetAHCoreBnt, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_resetAHCoreBnt, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_resetAHCoreBnt, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_resetAHCoreBnt, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    lv_obj_t *ui_resetAHCoreBntLabel = lv_label_create(ui_resetAHCoreBnt);
    lv_obj_align(ui_resetAHCoreBntLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui_resetAHCoreBntLabel, SETTING_TEXT_CONFIG_RESET);
    lv_obj_add_style(ui_resetAHCoreBntLabel, &label_text_style, 0);

    // 加热台配置
    lv_obj_t *ui_resetHPLabel = lv_label_create(ui_father);
    lv_obj_align_to(ui_resetHPLabel, ui_resetAHLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 75);
    lv_label_set_text(ui_resetHPLabel, "加热台");
    lv_obj_add_style(ui_resetHPLabel, &label_text_style, 0);

    static lv_point_t hpLine_points[] = {{0, 0}, {0, 80}};
    lv_obj_t *hpLine = lv_line_create(ui_father);
    lv_line_set_points(hpLine, hpLine_points, 2); /*Set the points*/
    lv_obj_add_style(hpLine, &style_line, 0);
    lv_obj_align_to(hpLine, ui_resetHPLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 7, 0);

    // 加热台通用配置重置
    lv_obj_t *ui_resetHPUtilLabel = lv_label_create(ui_father);
    lv_obj_align_to(ui_resetHPUtilLabel, hpLine,
                    LV_ALIGN_OUT_RIGHT_TOP, 5, 5);
    lv_label_set_text(ui_resetHPUtilLabel, SETTING_TEXT_CONFIG_UTIL_RESET);
    lv_obj_add_style(ui_resetHPUtilLabel, &label_text_style, 0);

    ui_resetHPUtilBnt = lv_btn_create(ui_father);
    // lv_obj_remove_style_all(ui_resetHPUtilBnt);
    lv_obj_set_size(ui_resetHPUtilBnt, 40, 20);
    lv_obj_align_to(ui_resetHPUtilBnt, ui_resetHPUtilLabel,
                    LV_ALIGN_OUT_LEFT_MID, 135, 0);
    lv_obj_add_flag(ui_resetHPUtilBnt, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_resetHPUtilBnt, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_resetHPUtilBnt, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_resetHPUtilBnt, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_resetHPUtilBnt, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_resetHPUtilBnt, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    lv_obj_t *ui_resetHPUtilBntLabel = lv_label_create(ui_resetHPUtilBnt);
    lv_obj_align(ui_resetHPUtilBntLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui_resetHPUtilBntLabel, SETTING_TEXT_CONFIG_RESET);
    lv_obj_add_style(ui_resetHPUtilBntLabel, &label_text_style, 0);

    // 加热台发热芯配置重置
    lv_obj_t *ui_resetHPCoreLabel = lv_label_create(ui_father);
    lv_obj_align_to(ui_resetHPCoreLabel, hpLine,
                    LV_ALIGN_OUT_RIGHT_TOP, 5, 30);
    lv_label_set_text(ui_resetHPCoreLabel, SETTING_TEXT_CONFIG_CORE_RESET);
    lv_obj_add_style(ui_resetHPCoreLabel, &label_text_style, 0);

    ui_resetHPCoreBnt = lv_btn_create(ui_father);
    // lv_obj_remove_style_all(ui_resetHPCoreBnt);
    lv_obj_set_size(ui_resetHPCoreBnt, 40, 20);
    lv_obj_align_to(ui_resetHPCoreBnt, ui_resetHPCoreLabel,
                    LV_ALIGN_OUT_LEFT_MID, 135, 0);
    lv_obj_add_flag(ui_resetHPCoreBnt, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_resetHPCoreBnt, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_resetHPCoreBnt, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_resetHPCoreBnt, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_resetHPCoreBnt, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_resetHPCoreBnt, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    lv_obj_t *ui_resetHPCoreBntLabel = lv_label_create(ui_resetHPCoreBnt);
    lv_obj_align(ui_resetHPCoreBntLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui_resetHPCoreBntLabel, SETTING_TEXT_CONFIG_RESET);
    lv_obj_add_style(ui_resetHPCoreBntLabel, &label_text_style, 0);

    // 加热台曲线配置重置
    lv_obj_t *ui_resetHPCurveLabel = lv_label_create(ui_father);
    lv_obj_align_to(ui_resetHPCurveLabel, hpLine,
                    LV_ALIGN_OUT_RIGHT_TOP, 5, 55);
    lv_label_set_text(ui_resetHPCurveLabel, SETTING_TEXT_CONFIG_CURVE_RESET);
    lv_obj_add_style(ui_resetHPCurveLabel, &label_text_style, 0);

    ui_resetHPCurveBnt = lv_btn_create(ui_father);
    // lv_obj_remove_style_all(ui_resetHPCurveBnt);
    lv_obj_set_size(ui_resetHPCurveBnt, 40, 20);
    lv_obj_align_to(ui_resetHPCurveBnt, ui_resetHPCurveLabel,
                    LV_ALIGN_OUT_LEFT_MID, 135, 0);
    lv_obj_add_flag(ui_resetHPCurveBnt, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_resetHPCurveBnt, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_resetHPCurveBnt, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_resetHPCurveBnt, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_resetHPCurveBnt, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_resetHPCurveBnt, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    lv_obj_t *ui_resetHPCurveBntLabel = lv_label_create(ui_resetHPCurveBnt);
    lv_obj_align(ui_resetHPCurveBntLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui_resetHPCurveBntLabel, SETTING_TEXT_CONFIG_RESET);
    lv_obj_add_style(ui_resetHPCurveBntLabel, &label_text_style, 0);

    // 可调电源配置
    lv_obj_t *ui_resetPowerLabel = lv_label_create(ui_father);
    lv_obj_align_to(ui_resetPowerLabel, ui_resetHPLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 100);
    lv_label_set_text(ui_resetPowerLabel, "可调电源");
    lv_obj_add_style(ui_resetPowerLabel, &label_text_style, 0);

    static lv_point_t powerLine_points[] = {{0, 0}, {0, 30}};
    lv_obj_t *powerLine = lv_line_create(ui_father);
    lv_line_set_points(powerLine, powerLine_points, 2); /*Set the points*/
    lv_obj_add_style(powerLine, &style_line, 0);
    lv_obj_align_to(powerLine, ui_resetPowerLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 7, 0);

    // 可调电源通用配置重置
    lv_obj_t *ui_resetPowerUtilLabel = lv_label_create(ui_father);
    lv_obj_align_to(ui_resetPowerUtilLabel, powerLine,
                    LV_ALIGN_OUT_RIGHT_TOP, 5, 5);
    lv_label_set_text(ui_resetPowerUtilLabel, SETTING_TEXT_CONFIG_UTIL_RESET);
    lv_obj_add_style(ui_resetPowerUtilLabel, &label_text_style, 0);

    ui_resetPowerUtilBnt = lv_btn_create(ui_father);
    // lv_obj_remove_style_all(ui_resetPowerUtilBnt);
    lv_obj_set_size(ui_resetPowerUtilBnt, 40, 20);
    lv_obj_align_to(ui_resetPowerUtilBnt, ui_resetPowerUtilLabel,
                    LV_ALIGN_OUT_LEFT_MID, 135, 0);
    lv_obj_add_flag(ui_resetPowerUtilBnt, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_resetPowerUtilBnt, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_resetPowerUtilBnt, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_resetPowerUtilBnt, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_resetPowerUtilBnt, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_resetPowerUtilBnt, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    lv_obj_t *ui_resetPowerUtilBntLabel = lv_label_create(ui_resetPowerUtilBnt);
    lv_obj_align(ui_resetPowerUtilBntLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui_resetPowerUtilBntLabel, SETTING_TEXT_CONFIG_RESET);
    lv_obj_add_style(ui_resetPowerUtilBntLabel, &label_text_style, 0);

    // 所有配置重置
    lv_obj_t *ui_resetAllLabel = lv_label_create(ui_father);
    lv_obj_align_to(ui_resetAllLabel, ui_resetPowerLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 50);
    lv_label_set_text(ui_resetAllLabel, SETTING_TEXT_CONFIG_ALL_RESET);
    lv_obj_add_style(ui_resetAllLabel, &label_text_style, 0);

    ui_resetAllBnt = lv_btn_create(ui_father);
    // lv_obj_remove_style_all(ui_resetAllBnt);
    lv_obj_set_size(ui_resetAllBnt, 55, 20);
    lv_obj_align_to(ui_resetAllBnt, ui_resetAllLabel,
                    LV_ALIGN_OUT_LEFT_MID, 160, 0);
    lv_obj_add_flag(ui_resetAllBnt, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_resetAllBnt, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_resetAllBnt, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_resetAllBnt, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_resetAllBnt, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_resetAllBnt, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    lv_obj_t *ui_resetAllBntLabel = lv_label_create(ui_resetAllBnt);
    lv_obj_align(ui_resetAllBntLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui_resetAllBntLabel, SETTING_TEXT_CONFIG_FACTORY);
    lv_obj_add_style(ui_resetAllBntLabel, &label_text_style, 0);

    // 返回
    ui_subBackBtn = lv_btn_create(father);
    lv_obj_remove_style_all(ui_subBackBtn);
    lv_obj_align_to(ui_subBackBtn, ui_resetAllLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 40);
    lv_obj_add_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_subBackBtn, &back_btn_style, LV_STATE_DEFAULT);
    lv_obj_add_style(ui_subBackBtn, &back_btn_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_subBackBtnLabel = lv_label_create(ui_subBackBtn);
    lv_obj_center(ui_subBackBtnLabel);
    lv_obj_add_style(ui_subBackBtnLabel, &label_text_style, 0);
    lv_label_set_text(ui_subBackBtnLabel, "返回");

    ui_userdata_group(NULL);
}

void ui_userdata_back_btn_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == ui_subBackBtn)
        {
            lv_obj_t *father = ui_father;
            lv_obj_clean(ui_father);
            ui_father = NULL;
            ui_sys_setting_init(father);
            ui_sys_setting_init_group(father);
        }
    }
}

void ui_userdata_group(lv_obj_t *father)
{
    lv_obj_add_event_cb(ui_resetSysUtilBnt, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_resetSolderUtilBnt, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_resetSolderCoreBnt, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_resetAHUtilBnt, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_resetAHCoreBnt, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_resetHPUtilBnt, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_resetHPCoreBnt, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_resetHPCurveBnt, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_resetPowerUtilBnt, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_resetAllBnt, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_subBackBtn, ui_userdata_back_btn_pressed, LV_EVENT_PRESSED, 0);

    if (NULL != sub_btn_group)
    {
        lv_group_del(sub_btn_group);
        sub_btn_group = NULL;
    }
    sub_btn_group = lv_group_create();
    lv_group_add_obj(sub_btn_group, ui_resetSysUtilBnt);
    lv_group_add_obj(sub_btn_group, ui_resetSolderUtilBnt);
    lv_group_add_obj(sub_btn_group, ui_resetSolderCoreBnt);
    lv_group_add_obj(sub_btn_group, ui_resetAHUtilBnt);
    lv_group_add_obj(sub_btn_group, ui_resetAHCoreBnt);
    lv_group_add_obj(sub_btn_group, ui_resetHPUtilBnt);
    lv_group_add_obj(sub_btn_group, ui_resetHPCoreBnt);
    lv_group_add_obj(sub_btn_group, ui_resetHPCurveBnt);
    lv_group_add_obj(sub_btn_group, ui_resetPowerUtilBnt);
    lv_group_add_obj(sub_btn_group, ui_resetAllBnt);
    lv_group_add_obj(sub_btn_group, ui_subBackBtn);
    lv_indev_set_group(knobs_indev, sub_btn_group);
}

#endif