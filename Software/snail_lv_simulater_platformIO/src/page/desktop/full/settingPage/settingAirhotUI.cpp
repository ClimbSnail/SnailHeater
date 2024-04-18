#include "./settingUIFull.h"

#ifdef FULL_UI

#include "../ui.h"
#include "../../desktop_model.h"

static lv_obj_t *ui_father = NULL;

static lv_obj_t *ui_airhotGridLabel;
static lv_obj_t *ui_airhotGridSwitch;
static lv_obj_t *airhotQuickSetupTemp0;
static lv_obj_t *airhotQuickSetupTemp1;
static lv_obj_t *airhotQuickSetupTemp2;
static lv_obj_t *ui_coolingFinishTempBtn;
static lv_obj_t *fastPIDSwitch;

static lv_obj_t *ui_airhotPIDLabel;
static lv_obj_t *ui_airhotParamKp;
static lv_obj_t *ui_airhotParamKi;
static lv_obj_t *ui_airhotParamKd;

static lv_obj_t *ui_airhotTempCalibrateAbleLaleb;
static lv_obj_t *ui_airhotTempCalibrateAbleSwitch;
static lv_obj_t *ui_airhotTempCalibrateObj;
static lv_obj_t *ui_airhotTempCalibrate;
static lv_obj_t *ui_airhotRealTemp[DISPLAY_TEMP_MAX];

static lv_obj_t *ui_saveBtn = NULL; // 保存修改

static void ui_air_hot_grid_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        if (lv_obj_has_state(target, LV_STATE_CHECKED))
            // SET_BIT(cfgKey1,CFG_KEY1_AIR_HOT_GRID);
            sysInfoModel.utilConfig.uiParam.uiGlobalParam.airhotGridEnable = ENABLE_STATE_OPEN;
        else
            // CLR_BIT(cfgKey1,CFG_KEY1_AIR_HOT_GRID);
            sysInfoModel.utilConfig.uiParam.uiGlobalParam.airhotGridEnable = ENABLE_STATE_CLOSE;
    }
}

static void ui_airhotSetTemp_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == airhotQuickSetupTemp0)
        {
            airhotModel.utilConfig.quickSetupTemp_0 = lv_numberbtn_get_value(airhotQuickSetupTemp0);
        }
        else if (target == airhotQuickSetupTemp1)
        {
            airhotModel.utilConfig.quickSetupTemp_1 = lv_numberbtn_get_value(airhotQuickSetupTemp1);
        }
        else if (target == airhotQuickSetupTemp2)
        {
            airhotModel.utilConfig.quickSetupTemp_2 = lv_numberbtn_get_value(airhotQuickSetupTemp2);
        }
        else if (target == ui_coolingFinishTempBtn)
        {
            airhotModel.utilConfig.coolingFinishTemp = lv_numberbtn_get_value(ui_coolingFinishTempBtn);
        }
    }
}

static void ui_fastPIDSwitch_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        if (target == fastPIDSwitch)
        {
            if (lv_obj_has_state(target, LV_STATE_CHECKED))
                airhotModel.utilConfig.fastPID = ENABLE_STATE_OPEN;
            else
                airhotModel.utilConfig.fastPID = ENABLE_STATE_CLOSE;
        }
    }
}

static void ui_airhot_set_core_pid_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == ui_airhotParamKp)
        {
            airhotModel.editCoreConfig.kp = lv_numberbtn_get_value(ui_airhotParamKp);
        }
        else if (target == ui_airhotParamKi)
        {
            airhotModel.editCoreConfig.ki = lv_numberbtn_get_value(ui_airhotParamKi);
        }
        else if (target == ui_airhotParamKd)
        {
            airhotModel.editCoreConfig.kd = lv_numberbtn_get_value(ui_airhotParamKd);
        }
    }
}

static void ui_airhot_bnt_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == ui_saveBtn)
        {
            airhotModel.manageCoreAction = INFO_MANAGE_ACTION_HA_WRITE;
            lv_obj_clear_state(ui_saveBtn, LV_STATE_FOCUSED);
            lv_group_focus_obj(ui_subBackBtn);
        }
    }
}

static void ui_real_temp_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        for (int index = DISPLAY_TEMP_0 + 1; index < DISPLAY_TEMP_MAX; ++index)
        {
            if (target == ui_airhotRealTemp[index])
            {
                airhotModel.editCoreConfig.realTemp[index] =
                    lv_numberbtn_get_value(ui_airhotRealTemp[index]);
            }
        }
    }
}

static void ui_useCalibration_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        bool flag = lv_obj_has_state(target, LV_STATE_CHECKED);
        if (target == ui_airhotTempCalibrateAbleSwitch)
        {
            airhotModel.editCoreConfig.useCalibrationParam = (ENABLE_STATE)flag;

            if (airhotModel.editCoreConfig.useCalibrationParam)
            {
                lv_group_remove_obj(ui_saveBtn);
                // 添加校准界面按键
                for (int index = DISPLAY_TEMP_0 + 1; index < DISPLAY_TEMP_MAX; ++index)
                {
                    lv_group_add_obj(sub_btn_group, ui_airhotRealTemp[index]);
                }
                lv_group_add_obj(sub_btn_group, ui_saveBtn);
            }
            else
            {
                // 移除校准界面按键
                for (int index = DISPLAY_TEMP_0 + 1; index < DISPLAY_TEMP_MAX; ++index)
                {
                    lv_group_remove_obj(ui_airhotRealTemp[index]);
                }
            }
        }
    }
}

void ui_airhot_setting_init(lv_obj_t *father)
{
    ui_father = father;

    // 初始默认值 todo 存在bug
    airhotModel.editCoreConfig = airhotModel.coreConfig;

    // 风枪曲线网格
    ui_airhotGridLabel = lv_label_create(father);
    lv_obj_align(ui_airhotGridLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_label_set_text(ui_airhotGridLabel, SETTING_TEXT_AIR_HOT_GRID);
    lv_obj_add_style(ui_airhotGridLabel, &label_text_style, 0);

    ui_airhotGridSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_airhotGridSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_airhotGridSwitch, 40, 20);
    lv_obj_align(ui_airhotGridSwitch, LV_ALIGN_TOP_LEFT, 105, 10);
    // if (GET_BIT(cfgKey1, CFG_KEY1_AIR_HOT_GRID))
    if (ENABLE_STATE_OPEN == sysInfoModel.utilConfig.uiParam.uiGlobalParam.airhotGridEnable)
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

    // 预设温度修改
    lv_obj_t *airhotQuickSetupTempLabel = lv_label_create(father);
    lv_obj_align_to(airhotQuickSetupTempLabel, ui_airhotGridLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_label_set_text(airhotQuickSetupTempLabel, TEXT_TEMP_PRESET_MD);
    lv_obj_add_style(airhotQuickSetupTempLabel, &label_text_style, 0);

    airhotQuickSetupTemp0 = lv_numberbtn_create(father);
    lv_obj_t *airhotQuickSetupTempLabel0 = lv_label_create(airhotQuickSetupTemp0);
    lv_numberbtn_set_label_and_format(airhotQuickSetupTemp0,
                                      airhotQuickSetupTempLabel0, "%d", 1);
    lv_numberbtn_set_range(airhotQuickSetupTemp0, MIN_SET_TEMPERATURE, MAX_SET_TEMPERATURE);
    lv_numberbtn_set_value(airhotQuickSetupTemp0, airhotModel.utilConfig.quickSetupTemp_0);
    lv_obj_set_size(airhotQuickSetupTemp0, 50, 20);
    lv_obj_align_to(airhotQuickSetupTemp0, airhotQuickSetupTempLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    // lv_obj_remove_style_all(airhotQuickSetupTemp0);
    lv_obj_add_flag(airhotQuickSetupTemp0, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(airhotQuickSetupTemp0, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(airhotQuickSetupTemp0, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(airhotQuickSetupTemp0, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(airhotQuickSetupTemp0, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(airhotQuickSetupTemp0, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(airhotQuickSetupTemp0, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(airhotQuickSetupTemp0, &setting_btn_pressed_style, LV_STATE_EDITED);

    airhotQuickSetupTemp1 = lv_numberbtn_create(father);
    lv_obj_t *airhotQuickSetupTempLabel1 = lv_label_create(airhotQuickSetupTemp1);
    lv_numberbtn_set_label_and_format(airhotQuickSetupTemp1,
                                      airhotQuickSetupTempLabel1, "%d", 1);
    lv_numberbtn_set_range(airhotQuickSetupTemp1, MIN_SET_TEMPERATURE, MAX_SET_TEMPERATURE);
    lv_numberbtn_set_value(airhotQuickSetupTemp1, airhotModel.utilConfig.quickSetupTemp_1);
    lv_obj_set_size(airhotQuickSetupTemp1, 50, 20);
    lv_obj_align_to(airhotQuickSetupTemp1, airhotQuickSetupTemp0,
                    LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    // lv_obj_remove_style_all(airhotQuickSetupTemp1);
    lv_obj_add_flag(airhotQuickSetupTemp1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(airhotQuickSetupTemp1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(airhotQuickSetupTemp1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(airhotQuickSetupTemp1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(airhotQuickSetupTemp1, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(airhotQuickSetupTemp1, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(airhotQuickSetupTemp1, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(airhotQuickSetupTemp1, &setting_btn_pressed_style, LV_STATE_EDITED);

    airhotQuickSetupTemp2 = lv_numberbtn_create(father);
    lv_obj_t *airhotQuickSetupTempLabel2 = lv_label_create(airhotQuickSetupTemp2);
    lv_numberbtn_set_label_and_format(airhotQuickSetupTemp2,
                                      airhotQuickSetupTempLabel2, "%d", 1);
    lv_numberbtn_set_range(airhotQuickSetupTemp2, MIN_SET_TEMPERATURE, MAX_SET_TEMPERATURE);
    lv_numberbtn_set_value(airhotQuickSetupTemp2, airhotModel.utilConfig.quickSetupTemp_2);
    lv_obj_set_size(airhotQuickSetupTemp2, 50, 20);
    lv_obj_align_to(airhotQuickSetupTemp2, airhotQuickSetupTemp1,
                    LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    // lv_obj_remove_style_all(airhotQuickSetupTemp2);
    lv_obj_add_flag(airhotQuickSetupTemp2, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(airhotQuickSetupTemp2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(airhotQuickSetupTemp2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(airhotQuickSetupTemp2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(airhotQuickSetupTemp2, ALL_GREY_COLOR,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(airhotQuickSetupTemp2, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(airhotQuickSetupTemp2, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(airhotQuickSetupTemp2, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 冷却终止温度
    lv_obj_t *ui_coolingFinishTempLabel = lv_label_create(father);
    lv_obj_align_to(ui_coolingFinishTempLabel, airhotQuickSetupTempLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 40);
    lv_label_set_text(ui_coolingFinishTempLabel, SETTING_TEXT_COOL_FINISH_TEMP);
    lv_obj_add_style(ui_coolingFinishTempLabel, &label_text_style, 0);

    ui_coolingFinishTempBtn = lv_numberbtn_create(father);
    lv_obj_t *ui_coolingFinishTempBtnLabel = lv_label_create(ui_coolingFinishTempBtn);
    lv_numberbtn_set_label_and_format(ui_coolingFinishTempBtn,
                                      ui_coolingFinishTempBtnLabel, "%d", 1);
    lv_numberbtn_set_range(ui_coolingFinishTempBtn, 0, 100);
    lv_numberbtn_set_value(ui_coolingFinishTempBtn, airhotModel.utilConfig.coolingFinishTemp);
    lv_obj_set_size(ui_coolingFinishTempBtn, 50, 20);
    lv_obj_align_to(ui_coolingFinishTempBtn, ui_coolingFinishTempLabel,
                    LV_ALIGN_OUT_LEFT_MID, 145, 0);
    // lv_obj_remove_style_all(ui_coolingFinishTempBtn);
    lv_obj_add_flag(ui_coolingFinishTempBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_coolingFinishTempBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_coolingFinishTempBtn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_coolingFinishTempBtn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_coolingFinishTempBtn, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_coolingFinishTempBtn, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_coolingFinishTempBtn, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_coolingFinishTempBtn, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 快捷PID
    lv_obj_t *fastPIDLabel = lv_label_create(father);
    lv_obj_align(fastPIDLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_align_to(fastPIDLabel, ui_coolingFinishTempLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    lv_label_set_text(fastPIDLabel, SETTING_TEXT_FAST_PID);
    lv_obj_add_style(fastPIDLabel, &label_text_style, 0);

    fastPIDSwitch = lv_switch_create(father);
    lv_obj_add_flag(fastPIDSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(fastPIDSwitch, 40, 20);
    lv_obj_align_to(fastPIDSwitch, fastPIDLabel,
                    LV_ALIGN_OUT_LEFT_MID, 140, 0);
    // if (GET_BIT(cfgKey1, CFG_KEY1_SOLDER_GRID))
    if (ENABLE_STATE_OPEN == airhotModel.utilConfig.fastPID)
    {
        lv_obj_add_state(fastPIDSwitch, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_clear_state(fastPIDSwitch, LV_STATE_CHECKED);
    }
    lv_obj_set_style_outline_color(fastPIDSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(fastPIDSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(fastPIDSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // PID参数
    ui_airhotPIDLabel = lv_label_create(father);
    lv_obj_align_to(ui_airhotPIDLabel, fastPIDLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    lv_label_set_text(ui_airhotPIDLabel, "PID参数");
    lv_obj_set_size(ui_airhotPIDLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(ui_airhotPIDLabel, &label_text_style, 0);

    ui_airhotParamKp = lv_numberbtn_create(father);
    lv_obj_t *airhotParamKpLabel = lv_label_create(ui_airhotParamKp);
    lv_numberbtn_set_label_and_format(ui_airhotParamKp,
                                      airhotParamKpLabel, "P->%d", 1);
    lv_numberbtn_set_range(ui_airhotParamKp, 0, 200);
    lv_numberbtn_set_value(ui_airhotParamKp, airhotModel.editCoreConfig.kp);
    // lv_obj_remove_style_all(ui_airhotParamKp);
    lv_obj_set_size(ui_airhotParamKp, 50, 20);
    lv_obj_align_to(ui_airhotParamKp, ui_airhotPIDLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_obj_add_flag(ui_airhotParamKp, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_airhotParamKp, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_airhotParamKp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_airhotParamKp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_airhotParamKp, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_airhotParamKp, &setting_btn_pressed_style, LV_STATE_EDITED);

    ui_airhotParamKi = lv_numberbtn_create(father);
    lv_obj_t *airhotParamKiLabel = lv_label_create(ui_airhotParamKi);
    lv_numberbtn_set_label_and_format(ui_airhotParamKi,
                                      airhotParamKiLabel, "I->%.1f", 0.1);
    lv_numberbtn_set_range(ui_airhotParamKi, 0, 200);
    lv_numberbtn_set_value(ui_airhotParamKi, airhotModel.editCoreConfig.ki);
    // lv_obj_remove_style_all(ui_airhotParamKi);
    lv_obj_set_size(ui_airhotParamKi, 50, 20);
    lv_obj_align_to(ui_airhotParamKi, ui_airhotParamKp,
                    LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_add_flag(ui_airhotParamKi, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_airhotParamKi, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_airhotParamKi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_airhotParamKi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_airhotParamKi, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_airhotParamKi, &setting_btn_pressed_style, LV_STATE_EDITED);

    ui_airhotParamKd = lv_numberbtn_create(father);
    lv_obj_t *airhotParamKdLabel = lv_label_create(ui_airhotParamKd);
    lv_numberbtn_set_label_and_format(ui_airhotParamKd,
                                      airhotParamKdLabel, "D->%d", 1);
    lv_numberbtn_set_range(ui_airhotParamKd, 0, 200);
    lv_numberbtn_set_value(ui_airhotParamKd, airhotModel.editCoreConfig.kd);
    // lv_obj_remove_style_all(ui_airhotParamKd);
    lv_obj_set_style_border_width(ui_airhotParamKd, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_size(ui_airhotParamKd, 50, 20);
    lv_obj_align_to(ui_airhotParamKd, ui_airhotParamKi,
                    LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_add_style(ui_airhotParamKd, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_airhotParamKd, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 使能温度标定
    ui_airhotTempCalibrateAbleLaleb = lv_label_create(father);
    lv_obj_align_to(ui_airhotTempCalibrateAbleLaleb, ui_airhotPIDLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 45);
    lv_label_set_text(ui_airhotTempCalibrateAbleLaleb, "启用温度标定");
    lv_obj_add_style(ui_airhotTempCalibrateAbleLaleb, &label_text_style, 0);

    ui_airhotTempCalibrateAbleSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_airhotTempCalibrateAbleSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_airhotTempCalibrateAbleSwitch, 40, 20);
    lv_obj_align_to(ui_airhotTempCalibrateAbleSwitch, ui_airhotTempCalibrateAbleLaleb,
                    LV_ALIGN_OUT_RIGHT_MID, 30, 0);
    if (airhotModel.editCoreConfig.useCalibrationParam)
    {
        lv_obj_add_state(ui_airhotTempCalibrateAbleSwitch, LV_STATE_CHECKED); // 反向
    }
    else
    {
        lv_obj_clear_state(ui_airhotTempCalibrateAbleSwitch, LV_STATE_CHECKED);
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_airhotTempCalibrateAbleSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_airhotTempCalibrateAbleSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_airhotTempCalibrateAbleSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 温度标定
    ui_airhotTempCalibrateObj = lv_obj_create(father);
    lv_obj_set_size(ui_airhotTempCalibrateObj, SH_SCREEN_WIDTH - 75,
                    LV_SIZE_CONTENT);
    lv_obj_add_style(ui_airhotTempCalibrateObj, &black_white_theme_style,
                     LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(ui_airhotTempCalibrateObj, ui_airhotTempCalibrateAbleLaleb,
                    LV_ALIGN_OUT_BOTTOM_LEFT, -10, 10);

    ui_airhotTempCalibrate = lv_label_create(ui_airhotTempCalibrateObj);
    lv_label_set_text(ui_airhotTempCalibrate, "温度标定 格式(显示->真实)");
    lv_obj_add_style(ui_airhotTempCalibrate, &label_text_style, 0);

    for (int index = DISPLAY_TEMP_0 + 1; index < DISPLAY_TEMP_MAX; ++index)
    {
        ui_airhotRealTemp[index] = lv_numberbtn_create(ui_airhotTempCalibrateObj);
        lv_obj_t *airhotRealTempLabel = lv_label_create(ui_airhotRealTemp[index]);
        lv_obj_add_style(airhotRealTempLabel, &label_text_style, 0);
        char format[8] = "";
        snprintf(format, 8, "%d->%%d", index * DISPLAY_TEMP_STEP);
        lv_numberbtn_set_label_and_format(ui_airhotRealTemp[index],
                                          airhotRealTempLabel, format, 1);
        lv_numberbtn_set_range(ui_airhotRealTemp[index], 0, 800);
        lv_numberbtn_set_value(ui_airhotRealTemp[index],
                               airhotModel.editCoreConfig.realTemp[index]);
        // lv_obj_remove_style_all(ui_airhotRealTemp[0]);
        lv_obj_set_size(ui_airhotRealTemp[index], 60, 20);
        lv_obj_align_to(ui_airhotRealTemp[index], ui_airhotTempCalibrate,
                        LV_ALIGN_BOTTOM_LEFT, ((index + 2) % 3) * 65 - 10,
                        ((index + 2) / 3) * 30);

        lv_obj_add_flag(ui_airhotRealTemp[index], LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_clear_flag(ui_airhotRealTemp[index], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(ui_airhotRealTemp[index], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui_airhotRealTemp[index], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_style(ui_airhotRealTemp[index], &setting_btn_focused_style, LV_STATE_FOCUSED);
        lv_obj_add_style(ui_airhotRealTemp[index], &setting_btn_pressed_style, LV_STATE_EDITED);
    }

    ui_subBackBtn = lv_btn_create(father);
    lv_obj_remove_style_all(ui_subBackBtn);
    lv_obj_align_to(ui_subBackBtn, ui_airhotTempCalibrateObj,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 120, 20);
    lv_obj_add_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_subBackBtn, &back_btn_style, LV_STATE_DEFAULT);
    lv_obj_add_style(ui_subBackBtn, &setting_btn_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_subBackBtnLabel = lv_label_create(ui_subBackBtn);
    lv_obj_center(ui_subBackBtnLabel);
    lv_obj_add_style(ui_subBackBtnLabel, &label_text_style, 0);
    lv_label_set_text(ui_subBackBtnLabel, "返回");

    ui_saveBtn = lv_btn_create(father);
    lv_obj_remove_style_all(ui_saveBtn);
    lv_obj_align_to(ui_saveBtn, ui_airhotTempCalibrateObj,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 20, 20);
    lv_obj_set_size(ui_saveBtn, 65, 25);
    lv_obj_add_flag(ui_saveBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_saveBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_saveBtn, &back_btn_style, LV_STATE_DEFAULT);
    lv_obj_add_style(ui_saveBtn, &setting_btn_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_saveLabel = lv_label_create(ui_saveBtn);
    lv_obj_center(ui_saveLabel);
    lv_obj_add_style(ui_saveLabel, &label_text_style, 0);
    lv_label_set_text(ui_saveLabel, SETTING_TEXT_EDIT_SAVE);
}

void ui_airhot_setting_init_group(lv_obj_t *father)
{
    lv_obj_add_event_cb(ui_airhotGridSwitch, ui_air_hot_grid_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(airhotQuickSetupTemp0, ui_airhotSetTemp_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(airhotQuickSetupTemp1, ui_airhotSetTemp_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(airhotQuickSetupTemp2, ui_airhotSetTemp_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_coolingFinishTempBtn, ui_airhotSetTemp_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(fastPIDSwitch, ui_fastPIDSwitch_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_airhotParamKp, ui_airhot_set_core_pid_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_airhotParamKi, ui_airhot_set_core_pid_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_airhotParamKd, ui_airhot_set_core_pid_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_airhotTempCalibrateAbleSwitch, ui_useCalibration_pressed,
                        LV_EVENT_VALUE_CHANGED, NULL);
    for (int index = DISPLAY_TEMP_0 + 1; index < DISPLAY_TEMP_MAX; ++index)
    {
        lv_obj_add_event_cb(ui_airhotRealTemp[index], ui_real_temp_pressed, LV_EVENT_PRESSED, NULL);
    }
    lv_obj_add_event_cb(ui_saveBtn, ui_airhot_bnt_pressed, LV_EVENT_PRESSED, 0);
    lv_obj_add_event_cb(ui_subBackBtn, ui_tab_back_btn_pressed, LV_EVENT_PRESSED, 0);

    if (NULL != sub_btn_group)
    {
        lv_group_del(sub_btn_group);
        sub_btn_group = NULL;
    }
    sub_btn_group = lv_group_create();
    lv_group_add_obj(sub_btn_group, ui_subBackBtn);
    lv_group_add_obj(sub_btn_group, ui_airhotGridSwitch);
    lv_group_add_obj(sub_btn_group, airhotQuickSetupTemp0);
    lv_group_add_obj(sub_btn_group, airhotQuickSetupTemp1);
    lv_group_add_obj(sub_btn_group, airhotQuickSetupTemp2);
    lv_group_add_obj(sub_btn_group, ui_coolingFinishTempBtn);
    lv_group_add_obj(sub_btn_group, fastPIDSwitch);
    lv_group_add_obj(sub_btn_group, ui_airhotParamKp);
    lv_group_add_obj(sub_btn_group, ui_airhotParamKi);
    lv_group_add_obj(sub_btn_group, ui_airhotParamKd);
    lv_group_add_obj(sub_btn_group, ui_airhotTempCalibrateAbleSwitch);
    for (int index = DISPLAY_TEMP_0 + 1;
         index < DISPLAY_TEMP_MAX && airhotModel.editCoreConfig.useCalibrationParam;
         ++index)
    {
        lv_group_add_obj(sub_btn_group, ui_airhotRealTemp[index]);
    }
    lv_group_add_obj(sub_btn_group, ui_saveBtn);
    lv_indev_set_group(knobs_indev, sub_btn_group);
}

void ui_airhot_setting_release(void *param)
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
}

#endif