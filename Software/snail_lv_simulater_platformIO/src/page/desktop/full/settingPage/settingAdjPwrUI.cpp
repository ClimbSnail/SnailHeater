#include "./settingUIFull.h"

#ifdef FULL_UI

#include "../ui.h"
#include "../../desktop_model.h"

static lv_obj_t *ui_father = NULL;

static lv_obj_t *ui_pwrCurZeroBtn;
static lv_obj_t *ui_activeCloseSolderSwitch;
static lv_obj_t *ui_calibMsgBox = NULL;

static lv_obj_t *powerQuickSetupVol0;
static lv_obj_t *powerQuickSetupVol1;
static lv_obj_t *powerQuickSetupVol2;
static lv_timer_t *ui_calibTimer = NULL; // 校准的定时器
static lv_obj_t *ui_timer_arc = NULL;
static lv_obj_t *ui_timer_arc_label = NULL;

static lv_group_t *msg_btn_group = NULL;

static void adjCalibTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    if (NULL != ui_timer_arc)
    {
        lv_obj_del(ui_timer_arc);
        ui_timer_arc = NULL;
    }

    if (NULL != ui_calibMsgBox)
    {
        lv_obj_del(ui_calibMsgBox);
        ui_calibMsgBox = NULL;
    }
    lv_indev_set_group(knobs_indev, sub_btn_group);

    if (INFO_MANAGE_ACTION_ADJPWR_CALIB_FINISH == adjPowerModel.manageCalibAction)
    {
        adjPowerModel.manageCalibAction = INFO_MANAGE_ACTION_ADJPWR_CALIB_IDLE;
    }
}

static void set_angle(void *obj, int32_t v)
{
    if (100 <= v)
    {
        lv_obj_set_style_text_font(ui_timer_arc_label, &FontDeyi_24, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text_fmt(ui_timer_arc_label, "%s", "成功");
    }
    else
    {
        lv_arc_set_value((lv_obj_t *)obj, v);
        lv_label_set_text_fmt(ui_timer_arc_label, "%ld%%", v);
    }
    lv_obj_align(ui_timer_arc_label, LV_ALIGN_CENTER, 0, 0);
}

static void calib_msgbox_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_current_target(e);
    LV_LOG_USER("Button %s clicked", lv_msgbox_get_active_btn_text(obj));

    if (!strcmp(lv_msgbox_get_active_btn_text(obj), SETTING_TEXT_BACK))
    {
        lv_indev_set_group(knobs_indev, sub_btn_group);
        lv_group_remove_obj(lv_msgbox_get_btns(ui_calibMsgBox));
        if (NULL != ui_calibMsgBox)
        {
            lv_obj_del(ui_calibMsgBox);
            ui_calibMsgBox = NULL;
        }
    }
    else
    {
        ui_timer_arc = lv_arc_create(ui_father);
        lv_arc_set_rotation(ui_timer_arc, 270);
        lv_arc_set_bg_angles(ui_timer_arc, 0, 360);
        lv_obj_remove_style(ui_timer_arc, NULL, LV_PART_KNOB);  /*Be sure the knob is not displayed*/
        lv_obj_clear_flag(ui_timer_arc, LV_OBJ_FLAG_CLICKABLE); /*To not allow adjusting by click*/
        lv_obj_center(ui_timer_arc);

        ui_timer_arc_label = lv_label_create(ui_timer_arc);
        lv_label_set_text_fmt(ui_timer_arc_label, "%d", 0);
        lv_obj_align(ui_timer_arc_label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_add_style(ui_timer_arc_label, &label_text_style, 0);
        lv_obj_set_style_text_color(ui_timer_arc_label,
                                    // lv_obj_get_style_arc_color_filtered(ui_timer_arc, LV_PART_MAIN),
                                    lv_obj_get_style_arc_color(ui_timer_arc, LV_PART_INDICATOR),
                                    LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(ui_timer_arc_label, &FontRoboto_24, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, ui_timer_arc);
        lv_anim_set_exec_cb(&a, set_angle);
        lv_anim_set_time(&a, 50000);
        // 重复次数。默认值为1。LV_ANIM_REPEAT_INFINIT用于无限重复
        lv_anim_set_repeat_count(&a, 1);
        lv_anim_set_repeat_delay(&a, 300);
        lv_anim_set_values(&a, 0, 100);
        lv_anim_start(&a);
        adjPowerModel.manageCalibAction = INFO_MANAGE_ACTION_ADJPWR_CALIB_START;

        ui_calibTimer = lv_timer_create(adjCalibTimer_timeout, 52000, NULL);
        lv_timer_set_repeat_count(ui_calibTimer, 1);
    }
}

static void ui_bnt_obj_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_PRESSED)
    {
        if (target == ui_pwrCurZeroBtn)
        {
            static const char *btns[] = {SETTING_TEXT_BACK, "开始校准", ""};

            ui_calibMsgBox = lv_msgbox_create(ui_father,
                                              SETTING_TEXT_ADJPWR_MSGBOX_INFO_TITLE,
                                              SETTING_TEXT_ADJPWR_MSGBOX_INFO, btns, false);
            lv_obj_add_style(ui_calibMsgBox, &label_text_style, 0);
            lv_obj_add_event_cb(ui_calibMsgBox, calib_msgbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
            lv_obj_center(ui_calibMsgBox);

            lv_obj_t *btnmatrix = lv_msgbox_get_btns(ui_calibMsgBox);
            lv_group_add_obj(msg_btn_group, btnmatrix);

            lv_indev_set_group(knobs_indev, msg_btn_group);
            lv_group_focus_obj(btnmatrix);
        }
    }
}

static void ui_adj_obj_change(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        if (target == ui_activeCloseSolderSwitch)
        {
            if (lv_obj_has_state(ui_activeCloseSolderSwitch, LV_STATE_CHECKED))
                sysInfoModel.utilConfig.activeCloseSolder = ENABLE_STATE_OPEN;
            else
                sysInfoModel.utilConfig.activeCloseSolder = ENABLE_STATE_CLOSE;
        }
    }
}

void ui_adjpwr_setting_init(lv_obj_t *father)
{
    ui_father = father;
    // 校准
    lv_obj_t *ui_pwrCurZeroLabel = lv_label_create(father);
    lv_obj_align(ui_pwrCurZeroLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_label_set_text(ui_pwrCurZeroLabel, SETTING_TEXT_ADJPWR_AUTO_CALIB);
    lv_obj_add_style(ui_pwrCurZeroLabel, &label_text_style, 0);

    ui_pwrCurZeroBtn = lv_btn_create(father);
    lv_obj_add_flag(ui_pwrCurZeroBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_pwrCurZeroBtn, 40, 20);
    lv_obj_align(ui_pwrCurZeroBtn, LV_ALIGN_TOP_LEFT, 105, 10);
    // lv_obj_set_style_outline_color(ui_pwrCurZeroBtn, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    // lv_obj_set_style_outline_opa(ui_pwrCurZeroBtn, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    // lv_obj_set_style_outline_pad(ui_pwrCurZeroBtn, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_bg_color(ui_pwrCurZeroBtn, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_pwrCurZeroBtn, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_pwrCurZeroBtn, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_pwrCurZeroBtn, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    lv_obj_t *ui_pwrCurZeroBtnLabel = lv_label_create(ui_pwrCurZeroBtn);
    lv_obj_align(ui_pwrCurZeroBtnLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui_pwrCurZeroBtnLabel, "进入");
    lv_obj_add_style(ui_pwrCurZeroBtnLabel, &label_text_style, 0);
    // lv_obj_set_style_bg_color(ui_pwrCurZeroBtnLabel, LV_PART_MAIN|LV_STATE_DEFAULT);
    // lv_obj_add_style(ui_pwrCurZeroBtnLabel, &setting_btn_focused_style, LV_STATE_FOCUSED);
    // lv_obj_add_style(ui_pwrCurZeroBtnLabel, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 是否主动关闭烙铁
    lv_obj_t *ui_activeCloseSolderLabel = lv_label_create(father);
    lv_obj_align_to(ui_activeCloseSolderLabel, ui_pwrCurZeroLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    lv_label_set_text(ui_activeCloseSolderLabel, SETTING_TEXT_ACTIVE_CLOSE_SOLDER);
    lv_obj_add_style(ui_activeCloseSolderLabel, &label_text_style, 0);

    ui_activeCloseSolderSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_activeCloseSolderSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_activeCloseSolderSwitch, 40, 20);
    lv_obj_align_to(ui_activeCloseSolderSwitch, ui_activeCloseSolderLabel,
                    LV_ALIGN_OUT_LEFT_MID, 140, 0);
    if (sysInfoModel.utilConfig.activeCloseSolder)
    {
        lv_obj_add_state(ui_activeCloseSolderSwitch, LV_STATE_CHECKED); // 反向
    }
    else
    {
        lv_obj_clear_state(ui_activeCloseSolderSwitch, LV_STATE_CHECKED);
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_activeCloseSolderSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_activeCloseSolderSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_activeCloseSolderSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 电压快速通道修改
    lv_obj_t *powerQuickSetupVolLabel = lv_label_create(father);
    lv_obj_align_to(powerQuickSetupVolLabel, ui_activeCloseSolderLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_label_set_text(powerQuickSetupVolLabel, TEXT_TEMP_POWER_VOL_CHANNEL_MD);
    lv_obj_add_style(powerQuickSetupVolLabel, &label_text_style, 0);

    powerQuickSetupVol0 = lv_numberbtn_create(father);
    lv_obj_t *powerQuickSetupVolLabel0 = lv_label_create(powerQuickSetupVol0);
    lv_numberbtn_set_label_and_format(powerQuickSetupVol0,
                                      powerQuickSetupVolLabel0, "%.1f", 0.1);
    lv_numberbtn_set_range(powerQuickSetupVol0, 0.6, ADJ_POWER_VOL_MAX / 1000.0);
    lv_numberbtn_set_value(powerQuickSetupVol0, adjPowerModel.utilConfig.quickSetupVoltage_0 / 1000.0);
    lv_obj_set_size(powerQuickSetupVol0, 50, 20);
    lv_obj_align_to(powerQuickSetupVol0, powerQuickSetupVolLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    // lv_obj_remove_style_all(powerQuickSetupVol0);
    lv_obj_add_flag(powerQuickSetupVol0, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(powerQuickSetupVol0, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(powerQuickSetupVol0, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(powerQuickSetupVol0, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(powerQuickSetupVol0, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(powerQuickSetupVol0, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(powerQuickSetupVol0, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(powerQuickSetupVol0, &setting_btn_pressed_style, LV_STATE_EDITED);

    powerQuickSetupVol1 = lv_numberbtn_create(father);
    lv_obj_t *powerQuickSetupVolLabel1 = lv_label_create(powerQuickSetupVol1);
    lv_numberbtn_set_label_and_format(powerQuickSetupVol1,
                                      powerQuickSetupVolLabel1, "%.1f", 0.1);
    lv_numberbtn_set_range(powerQuickSetupVol1, 0.6, ADJ_POWER_VOL_MAX / 1000.0);
    lv_numberbtn_set_value(powerQuickSetupVol1, adjPowerModel.utilConfig.quickSetupVoltage_1 / 1000.0);
    lv_obj_set_size(powerQuickSetupVol1, 50, 20);
    lv_obj_align_to(powerQuickSetupVol1, powerQuickSetupVol0,
                    LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    // lv_obj_remove_style_all(powerQuickSetupVol1);
    lv_obj_add_flag(powerQuickSetupVol1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(powerQuickSetupVol1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(powerQuickSetupVol1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(powerQuickSetupVol1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(powerQuickSetupVol1, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(powerQuickSetupVol1, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(powerQuickSetupVol1, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(powerQuickSetupVol1, &setting_btn_pressed_style, LV_STATE_EDITED);

    powerQuickSetupVol2 = lv_numberbtn_create(father);
    lv_obj_t *powerQuickSetupVolLabel2 = lv_label_create(powerQuickSetupVol2);
    lv_numberbtn_set_label_and_format(powerQuickSetupVol2,
                                      powerQuickSetupVolLabel2, "%.1f", 0.1);
    lv_numberbtn_set_range(powerQuickSetupVol2, 0.6, ADJ_POWER_VOL_MAX / 1000.0);
    lv_numberbtn_set_value(powerQuickSetupVol2, adjPowerModel.utilConfig.quickSetupVoltage_2 / 1000.0);
    lv_obj_set_size(powerQuickSetupVol2, 50, 20);
    lv_obj_align_to(powerQuickSetupVol2, powerQuickSetupVol1,
                    LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    // lv_obj_remove_style_all(powerQuickSetupVol2);
    lv_obj_add_flag(powerQuickSetupVol2, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(powerQuickSetupVol2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(powerQuickSetupVol2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(powerQuickSetupVol2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(powerQuickSetupVol2, ALL_GREY_COLOR,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(powerQuickSetupVol2, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(powerQuickSetupVol2, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(powerQuickSetupVol2, &setting_btn_pressed_style, LV_STATE_EDITED);

    ui_subBackBtn = lv_btn_create(father);
    lv_obj_remove_style_all(ui_subBackBtn);
    lv_obj_align_to(ui_subBackBtn, powerQuickSetupVolLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 20, 40);
    lv_obj_add_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_subBackBtn, &back_btn_style, LV_STATE_DEFAULT);
    lv_obj_add_style(ui_subBackBtn, &back_btn_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_subBackBtnLabel = lv_label_create(ui_subBackBtn);
    lv_obj_center(ui_subBackBtnLabel);
    lv_obj_add_style(ui_subBackBtnLabel, &label_text_style, 0);
    lv_label_set_text(ui_subBackBtnLabel, SETTING_TEXT_BACK);
}

static void ui_powerQuickSetupVol_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == powerQuickSetupVol0)
        {
            adjPowerModel.utilConfig.quickSetupVoltage_0 = lv_numberbtn_get_value(powerQuickSetupVol0) * 1000;
        }
        else if (target == powerQuickSetupVol1)
        {
            adjPowerModel.utilConfig.quickSetupVoltage_1 = lv_numberbtn_get_value(powerQuickSetupVol1) * 1000;
        }
        else if (target == powerQuickSetupVol2)
        {
            adjPowerModel.utilConfig.quickSetupVoltage_2 = lv_numberbtn_get_value(powerQuickSetupVol2) * 1000;
        }
    }
}

void ui_adjpwr_setting_init_group(lv_obj_t *father)
{
    lv_obj_add_event_cb(ui_pwrCurZeroBtn, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_activeCloseSolderSwitch, ui_adj_obj_change, LV_EVENT_VALUE_CHANGED, 0);
    lv_obj_add_event_cb(powerQuickSetupVol0, ui_powerQuickSetupVol_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(powerQuickSetupVol1, ui_powerQuickSetupVol_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(powerQuickSetupVol2, ui_powerQuickSetupVol_pressed, LV_EVENT_PRESSED, NULL);

    lv_obj_add_event_cb(ui_subBackBtn, ui_tab_back_btn_pressed, LV_EVENT_PRESSED, 0);

    if (NULL != sub_btn_group)
    {
        lv_group_del(sub_btn_group);
        sub_btn_group = NULL;
    }
    sub_btn_group = lv_group_create();
    msg_btn_group = lv_group_create();
    lv_group_add_obj(sub_btn_group, ui_pwrCurZeroBtn);
#if SH_HARDWARE_VER >= SH_ESP32S2_WROOM_V25
    lv_group_add_obj(sub_btn_group, ui_activeCloseSolderSwitch);
#endif
    lv_group_add_obj(sub_btn_group, powerQuickSetupVol0);
    lv_group_add_obj(sub_btn_group, powerQuickSetupVol1);
    lv_group_add_obj(sub_btn_group, powerQuickSetupVol2);
    lv_group_add_obj(sub_btn_group, ui_subBackBtn);
    lv_indev_set_group(knobs_indev, sub_btn_group);
}

void ui_adjpwr_setting_release(void *param)
{
    if (NULL != ui_father)
    {
        lv_obj_clean(ui_father);
        ui_father = NULL;
    }

    if (NULL != sub_btn_group)
    {
        lv_group_del(sub_btn_group);
        sub_btn_group = NULL;
    }

    if (NULL != msg_btn_group)
    {
        lv_group_del(msg_btn_group);
        msg_btn_group = NULL;
    }
}

#endif