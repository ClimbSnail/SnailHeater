#include "./settingUIFull.h"

#ifdef FULL_UI

#include "../ui.h"
#include "../../desktop_model.h"

static lv_obj_t *ui_father = NULL;

static lv_obj_t *ui_heatplatGridSwitch;
static lv_obj_t *ui_autoHeatSwitch;
static lv_obj_t *heatplatQuickSetupTemp0;
static lv_obj_t *heatplatQuickSetupTemp1;
static lv_obj_t *heatplatQuickSetupTemp2;
static lv_obj_t *ui_coolingFinishTempBtn;
static lv_obj_t *fastPIDSwitch;

static lv_obj_t *ui_heatplatParamKp;
static lv_obj_t *ui_heatplatParamKi;
static lv_obj_t *ui_heatplatParamKd;

static lv_obj_t *ui_heatplatTempCalibrateAbleSwitch;
static lv_obj_t *ui_heatplatTempCalibrateObj;
static lv_obj_t *ui_heatplatTempCalibrate;
static lv_obj_t *ui_heatplatRealTemp[DISPLAY_TEMP_MAX];

static lv_obj_t *ui_saveBtn = NULL; // 保存修改

static void ui_auto_heat_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        if (lv_obj_has_state(target, LV_STATE_CHECKED))
            // SET_BIT(cfgKey1,CFG_KEY1_HP_AUTO_HEAT);
            sysInfoModel.utilConfig.uiParam.uiGlobalParam.hpAutoHeatEnable = ENABLE_STATE_OPEN;
        else
            // CLR_BIT(cfgKey1,CFG_KEY1_HP_AUTO_HEAT);
            sysInfoModel.utilConfig.uiParam.uiGlobalParam.hpAutoHeatEnable = ENABLE_STATE_CLOSE;
    }
}

static void ui_heatplatSetTemp_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == heatplatQuickSetupTemp0)
        {
            heatplatformModel.utilConfig.quickSetupTemp_0 = lv_numberbtn_get_value(heatplatQuickSetupTemp0);
        }
        else if (target == heatplatQuickSetupTemp1)
        {
            heatplatformModel.utilConfig.quickSetupTemp_1 = lv_numberbtn_get_value(heatplatQuickSetupTemp1);
        }
        else if (target == heatplatQuickSetupTemp2)
        {
            heatplatformModel.utilConfig.quickSetupTemp_2 = lv_numberbtn_get_value(heatplatQuickSetupTemp2);
        }
        else if (target == ui_coolingFinishTempBtn)
        {
            heatplatformModel.utilConfig.coolingFinishTemp = lv_numberbtn_get_value(ui_coolingFinishTempBtn);
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
                heatplatformModel.utilConfig.fastPID = ENABLE_STATE_OPEN;
            else
                heatplatformModel.utilConfig.fastPID = ENABLE_STATE_CLOSE;
        }
    }
}

static void ui_heatplat_set_core_pid_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == ui_heatplatParamKp)
        {
            heatplatformModel.editCoreConfig.kp = lv_numberbtn_get_value(ui_heatplatParamKp);
        }
        else if (target == ui_heatplatParamKi)
        {
            heatplatformModel.editCoreConfig.ki = lv_numberbtn_get_value(ui_heatplatParamKi);
        }
        else if (target == ui_heatplatParamKd)
        {
            heatplatformModel.editCoreConfig.kd = lv_numberbtn_get_value(ui_heatplatParamKd);
        }
    }
}

static void ui_hp_bnt_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == ui_saveBtn)
        {
            heatplatformModel.manageConfigAction = INFO_MANAGE_ACTION_HP_WRITE;
            heatplatformModel.manageCoreAction = INFO_MANAGE_ACTION_HP_WRITE;
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
            if (target == ui_heatplatRealTemp[index])
            {
                heatplatformModel.editCoreConfig.realTemp[index] =
                    lv_numberbtn_get_value(ui_heatplatRealTemp[index]);
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
        if (target == ui_heatplatTempCalibrateAbleSwitch)
        {
            heatplatformModel.editCoreConfig.useCalibrationParam = (ENABLE_STATE)flag;

            if (heatplatformModel.editCoreConfig.useCalibrationParam)
            {
                lv_group_remove_obj(ui_saveBtn);
                // 添加校准界面按键
                for (int index = DISPLAY_TEMP_0 + 1; index < DISPLAY_TEMP_MAX; ++index)
                {
                    lv_group_add_obj(sub_btn_group, ui_heatplatRealTemp[index]);
                }
                lv_group_add_obj(sub_btn_group, ui_saveBtn);
            }
            else
            {
                // 移除校准界面按键
                for (int index = DISPLAY_TEMP_0 + 1; index < DISPLAY_TEMP_MAX; ++index)
                {
                    lv_group_remove_obj(ui_heatplatRealTemp[index]);
                }
            }
        }
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
            sysInfoModel.utilConfig.uiParam.uiGlobalParam.heatplatGridEnable = ENABLE_STATE_OPEN;
        else
            // CLR_BIT(cfgKey1,CFG_KEY1_HEAT_PLAT_GRID);
            sysInfoModel.utilConfig.uiParam.uiGlobalParam.heatplatGridEnable = ENABLE_STATE_CLOSE;
    }
}

void ui_hp_setting_init(lv_obj_t *father)
{
    ui_father = father;

    // 初始默认值 todo 存在bug
    heatplatformModel.editCoreConfig = heatplatformModel.coreConfig;

    // 加热台曲线网格
    lv_obj_t *ui_heatplatGridLabel = lv_label_create(father);
    lv_obj_align(ui_heatplatGridLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_label_set_text(ui_heatplatGridLabel, SETTING_TEXT_HEAT_PLAT_GRID);
    lv_obj_add_style(ui_heatplatGridLabel, &label_text_style, 0);

    ui_heatplatGridSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_heatplatGridSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_heatplatGridSwitch, 40, 20);
    lv_obj_align(ui_heatplatGridSwitch, LV_ALIGN_TOP_LEFT, 105, 10);
    // if (GET_BIT(cfgKey1, CFG_KEY1_HEAT_PLAT_GRID))
    if (ENABLE_STATE_OPEN == sysInfoModel.utilConfig.uiParam.uiGlobalParam.heatplatGridEnable)
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

    // 预设温度修改
    lv_obj_t *heatplatQuickSetupTempLabel = lv_label_create(father);
    lv_obj_align_to(heatplatQuickSetupTempLabel, ui_heatplatGridLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_label_set_text(heatplatQuickSetupTempLabel, TEXT_TEMP_PRESET_MD);
    lv_obj_add_style(heatplatQuickSetupTempLabel, &label_text_style, 0);

    heatplatQuickSetupTemp0 = lv_numberbtn_create(father);
    lv_obj_t *heatplatQuickSetupTempLabel0 = lv_label_create(heatplatQuickSetupTemp0);
    lv_numberbtn_set_label_and_format(heatplatQuickSetupTemp0,
                                      heatplatQuickSetupTempLabel0, "%d", 1);
    lv_numberbtn_set_range(heatplatQuickSetupTemp0, MIN_SET_TEMPERATURE, MAX_SET_TEMPERATURE);
    lv_numberbtn_set_value(heatplatQuickSetupTemp0, heatplatformModel.utilConfig.quickSetupTemp_0);
    lv_obj_set_size(heatplatQuickSetupTemp0, 50, 20);
    lv_obj_align_to(heatplatQuickSetupTemp0, heatplatQuickSetupTempLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    // lv_obj_remove_style_all(heatplatQuickSetupTemp0);
    lv_obj_add_flag(heatplatQuickSetupTemp0, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(heatplatQuickSetupTemp0, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(heatplatQuickSetupTemp0, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(heatplatQuickSetupTemp0, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(heatplatQuickSetupTemp0, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(heatplatQuickSetupTemp0, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(heatplatQuickSetupTemp0, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(heatplatQuickSetupTemp0, &setting_btn_pressed_style, LV_STATE_EDITED);

    heatplatQuickSetupTemp1 = lv_numberbtn_create(father);
    lv_obj_t *heatplatQuickSetupTempLabel1 = lv_label_create(heatplatQuickSetupTemp1);
    lv_numberbtn_set_label_and_format(heatplatQuickSetupTemp1,
                                      heatplatQuickSetupTempLabel1, "%d", 1);
    lv_numberbtn_set_range(heatplatQuickSetupTemp1, MIN_SET_TEMPERATURE, MAX_SET_TEMPERATURE);
    lv_numberbtn_set_value(heatplatQuickSetupTemp1, heatplatformModel.utilConfig.quickSetupTemp_1);
    lv_obj_set_size(heatplatQuickSetupTemp1, 50, 20);
    lv_obj_align_to(heatplatQuickSetupTemp1, heatplatQuickSetupTemp0,
                    LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    // lv_obj_remove_style_all(heatplatQuickSetupTemp1);
    lv_obj_add_flag(heatplatQuickSetupTemp1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(heatplatQuickSetupTemp1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(heatplatQuickSetupTemp1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(heatplatQuickSetupTemp1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(heatplatQuickSetupTemp1, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(heatplatQuickSetupTemp1, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(heatplatQuickSetupTemp1, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(heatplatQuickSetupTemp1, &setting_btn_pressed_style, LV_STATE_EDITED);

    heatplatQuickSetupTemp2 = lv_numberbtn_create(father);
    lv_obj_t *heatplatQuickSetupTempLabel2 = lv_label_create(heatplatQuickSetupTemp2);
    lv_numberbtn_set_label_and_format(heatplatQuickSetupTemp2,
                                      heatplatQuickSetupTempLabel2, "%d", 1);
    lv_numberbtn_set_range(heatplatQuickSetupTemp2, MIN_SET_TEMPERATURE, MAX_SET_TEMPERATURE);
    lv_numberbtn_set_value(heatplatQuickSetupTemp2, heatplatformModel.utilConfig.quickSetupTemp_2);
    lv_obj_set_size(heatplatQuickSetupTemp2, 50, 20);
    lv_obj_align_to(heatplatQuickSetupTemp2, heatplatQuickSetupTemp1,
                    LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    // lv_obj_remove_style_all(heatplatQuickSetupTemp2);
    lv_obj_add_flag(heatplatQuickSetupTemp2, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(heatplatQuickSetupTemp2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(heatplatQuickSetupTemp2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(heatplatQuickSetupTemp2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(heatplatQuickSetupTemp2, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(heatplatQuickSetupTemp2, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(heatplatQuickSetupTemp2, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(heatplatQuickSetupTemp2, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 冷却终止温度
    lv_obj_t *ui_coolingFinishTempLabel = lv_label_create(father);
    lv_obj_align_to(ui_coolingFinishTempLabel, heatplatQuickSetupTempLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 40);
    lv_label_set_text(ui_coolingFinishTempLabel, SETTING_TEXT_COOL_FINISH_TEMP);
    lv_obj_add_style(ui_coolingFinishTempLabel, &label_text_style, 0);

    ui_coolingFinishTempBtn = lv_numberbtn_create(father);
    lv_obj_t *ui_coolingFinishTempBtnLabel = lv_label_create(ui_coolingFinishTempBtn);
    lv_numberbtn_set_label_and_format(ui_coolingFinishTempBtn,
                                      ui_coolingFinishTempBtnLabel, "%d", 1);
    lv_numberbtn_set_range(ui_coolingFinishTempBtn, 0, 100);
    lv_numberbtn_set_value(ui_coolingFinishTempBtn, heatplatformModel.utilConfig.coolingFinishTemp);
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
    if (ENABLE_STATE_OPEN == heatplatformModel.utilConfig.fastPID)
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
    lv_obj_t *ui_heatplatPIDLabel = lv_label_create(father);
    lv_obj_align_to(ui_heatplatPIDLabel, fastPIDLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    lv_label_set_text(ui_heatplatPIDLabel, "PID参数");
    lv_obj_set_size(ui_heatplatPIDLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(ui_heatplatPIDLabel, &label_text_style, 0);

    ui_heatplatParamKp = lv_numberbtn_create(father);
    lv_obj_t *heatplatParamKpLabel = lv_label_create(ui_heatplatParamKp);
    lv_numberbtn_set_label_and_format(ui_heatplatParamKp,
                                      heatplatParamKpLabel, "P->%d", 1);
    lv_numberbtn_set_range(ui_heatplatParamKp, 0, 200);
    lv_numberbtn_set_value(ui_heatplatParamKp, heatplatformModel.editCoreConfig.kp);
    // lv_obj_remove_style_all(ui_heatplatParamKp);
    lv_obj_set_size(ui_heatplatParamKp, 50, 20);
    lv_obj_align_to(ui_heatplatParamKp, ui_heatplatPIDLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_obj_add_flag(ui_heatplatParamKp, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_heatplatParamKp, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_heatplatParamKp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_heatplatParamKp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_heatplatParamKp, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_heatplatParamKp, &setting_btn_pressed_style, LV_STATE_EDITED);

    ui_heatplatParamKi = lv_numberbtn_create(father);
    lv_obj_t *heatplatParamKiLabel = lv_label_create(ui_heatplatParamKi);
    lv_numberbtn_set_label_and_format(ui_heatplatParamKi,
                                      heatplatParamKiLabel, "I->%.1f", 0.1);
    lv_numberbtn_set_range(ui_heatplatParamKi, 0, 200);
    lv_numberbtn_set_value(ui_heatplatParamKi, heatplatformModel.editCoreConfig.ki);
    // lv_obj_remove_style_all(ui_heatplatParamKi);
    lv_obj_set_size(ui_heatplatParamKi, 50, 20);
    lv_obj_align_to(ui_heatplatParamKi, ui_heatplatParamKp,
                    LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_add_flag(ui_heatplatParamKi, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_heatplatParamKi, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_heatplatParamKi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_heatplatParamKi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_heatplatParamKi, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_heatplatParamKi, &setting_btn_pressed_style, LV_STATE_EDITED);

    ui_heatplatParamKd = lv_numberbtn_create(father);
    lv_obj_t *heatplatParamKdLabel = lv_label_create(ui_heatplatParamKd);
    lv_numberbtn_set_label_and_format(ui_heatplatParamKd,
                                      heatplatParamKdLabel, "D->%d", 1);
    lv_numberbtn_set_range(ui_heatplatParamKd, 0, 200);
    lv_numberbtn_set_value(ui_heatplatParamKd, heatplatformModel.editCoreConfig.kd);
    // lv_obj_remove_style_all(ui_heatplatParamKd);
    lv_obj_set_style_border_width(ui_heatplatParamKd, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_size(ui_heatplatParamKd, 50, 20);
    lv_obj_align_to(ui_heatplatParamKd, ui_heatplatParamKi,
                    LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_add_style(ui_heatplatParamKd, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_heatplatParamKd, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 使能温度标定
    lv_obj_t *ui_heatplatTempCalibrateAbleLaleb = lv_label_create(father);
    lv_obj_align_to(ui_heatplatTempCalibrateAbleLaleb, ui_heatplatPIDLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 45);
    lv_label_set_text(ui_heatplatTempCalibrateAbleLaleb, "启用温度标定");
    lv_obj_add_style(ui_heatplatTempCalibrateAbleLaleb, &label_text_style, 0);

    ui_heatplatTempCalibrateAbleSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_heatplatTempCalibrateAbleSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_heatplatTempCalibrateAbleSwitch, 40, 20);
    lv_obj_align_to(ui_heatplatTempCalibrateAbleSwitch, ui_heatplatTempCalibrateAbleLaleb,
                    LV_ALIGN_OUT_RIGHT_MID, 30, 0);
    if (heatplatformModel.editCoreConfig.useCalibrationParam)
    {
        lv_obj_add_state(ui_heatplatTempCalibrateAbleSwitch, LV_STATE_CHECKED); // 反向
    }
    else
    {
        lv_obj_clear_state(ui_heatplatTempCalibrateAbleSwitch, LV_STATE_CHECKED);
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_heatplatTempCalibrateAbleSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_heatplatTempCalibrateAbleSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_heatplatTempCalibrateAbleSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 温度标定
    ui_heatplatTempCalibrateObj = lv_obj_create(father);
    lv_obj_set_size(ui_heatplatTempCalibrateObj, SH_SCREEN_WIDTH - 75,
                    LV_SIZE_CONTENT);
    lv_obj_add_style(ui_heatplatTempCalibrateObj, &black_white_theme_style,
                     LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(ui_heatplatTempCalibrateObj, ui_heatplatTempCalibrateAbleLaleb,
                    LV_ALIGN_OUT_BOTTOM_LEFT, -10, 10);

    ui_heatplatTempCalibrate = lv_label_create(ui_heatplatTempCalibrateObj);
    lv_label_set_text(ui_heatplatTempCalibrate, "温度标定 格式(显示->真实)");
    lv_obj_add_style(ui_heatplatTempCalibrate, &label_text_style, 0);

    for (int index = DISPLAY_TEMP_0 + 1; index < DISPLAY_TEMP_MAX; ++index)
    {
        ui_heatplatRealTemp[index] = lv_numberbtn_create(ui_heatplatTempCalibrateObj);
        lv_obj_t *heatplatRealTempLabel = lv_label_create(ui_heatplatRealTemp[index]);
        lv_obj_add_style(heatplatRealTempLabel, &label_text_style, 0);
        char format[8] = "";
        snprintf(format, 8, "%d->%%d", index * DISPLAY_TEMP_STEP);
        lv_numberbtn_set_label_and_format(ui_heatplatRealTemp[index],
                                          heatplatRealTempLabel, format, 1);
        lv_numberbtn_set_range(ui_heatplatRealTemp[index], 0, 800);
        lv_numberbtn_set_value(ui_heatplatRealTemp[index],
                               heatplatformModel.editCoreConfig.realTemp[index]);
        // lv_obj_remove_style_all(ui_heatplatRealTemp[index]);
        lv_obj_set_size(ui_heatplatRealTemp[index], 60, 20);
        lv_obj_align_to(ui_heatplatRealTemp[index], ui_heatplatTempCalibrate,
                        LV_ALIGN_BOTTOM_LEFT, ((index + 2) % 3) * 65 - 10,
                        ((index + 2) / 3) * 30);

        lv_obj_add_flag(ui_heatplatRealTemp[index], LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_clear_flag(ui_heatplatRealTemp[index], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(ui_heatplatRealTemp[index], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui_heatplatRealTemp[index], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_style(ui_heatplatRealTemp[index], &setting_btn_focused_style, LV_STATE_FOCUSED);
        lv_obj_add_style(ui_heatplatRealTemp[index], &setting_btn_pressed_style, LV_STATE_EDITED);
    }

    ui_saveBtn = lv_btn_create(father);
    lv_obj_remove_style_all(ui_saveBtn);
    lv_obj_align_to(ui_saveBtn, ui_heatplatTempCalibrateObj,
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

    ui_subBackBtn = lv_btn_create(father);
    lv_obj_remove_style_all(ui_subBackBtn);
    lv_obj_align_to(ui_subBackBtn, ui_heatplatTempCalibrateObj,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 120, 20);
    lv_obj_add_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_subBackBtn, &back_btn_style, LV_STATE_DEFAULT);
    lv_obj_add_style(ui_subBackBtn, &setting_btn_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_subBackBtnLabel = lv_label_create(ui_subBackBtn);
    lv_obj_center(ui_subBackBtnLabel);
    lv_obj_add_style(ui_subBackBtnLabel, &label_text_style, 0);
    lv_label_set_text(ui_subBackBtnLabel, SETTING_TEXT_BACK);
}

void ui_hp_setting_init_group(lv_obj_t *father)
{
    // lv_obj_add_event_cb(ui_autoHeatSwitch, ui_auto_heat_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_heatplatGridSwitch, ui_heat_plat_grid_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(heatplatQuickSetupTemp0, ui_heatplatSetTemp_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(heatplatQuickSetupTemp1, ui_heatplatSetTemp_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(heatplatQuickSetupTemp2, ui_heatplatSetTemp_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_coolingFinishTempBtn, ui_heatplatSetTemp_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(fastPIDSwitch, ui_fastPIDSwitch_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_heatplatParamKp, ui_heatplat_set_core_pid_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_heatplatParamKi, ui_heatplat_set_core_pid_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_heatplatParamKd, ui_heatplat_set_core_pid_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_heatplatTempCalibrateAbleSwitch, ui_useCalibration_pressed,
                        LV_EVENT_VALUE_CHANGED, NULL);
    for (int index = DISPLAY_TEMP_0 + 1; index < DISPLAY_TEMP_MAX; ++index)
    {
        lv_obj_add_event_cb(ui_heatplatRealTemp[index], ui_real_temp_pressed, LV_EVENT_PRESSED, NULL);
    }
    lv_obj_add_event_cb(ui_saveBtn, ui_hp_bnt_pressed, LV_EVENT_PRESSED, 0);
    lv_obj_add_event_cb(ui_subBackBtn, ui_tab_back_btn_pressed, LV_EVENT_PRESSED, 0);

    if (NULL != sub_btn_group)
    {
        lv_group_del(sub_btn_group);
        sub_btn_group = NULL;
    }
    sub_btn_group = lv_group_create();
    // lv_group_add_obj(sub_btn_group, ui_autoHeatSwitch);
    lv_group_add_obj(sub_btn_group, ui_subBackBtn);
    lv_group_add_obj(sub_btn_group, ui_heatplatGridSwitch);
    lv_group_add_obj(sub_btn_group, heatplatQuickSetupTemp0);
    lv_group_add_obj(sub_btn_group, heatplatQuickSetupTemp1);
    lv_group_add_obj(sub_btn_group, heatplatQuickSetupTemp2);
    lv_group_add_obj(sub_btn_group, ui_coolingFinishTempBtn);
    lv_group_add_obj(sub_btn_group, fastPIDSwitch);
    lv_group_add_obj(sub_btn_group, ui_heatplatParamKp);
    lv_group_add_obj(sub_btn_group, ui_heatplatParamKi);
    lv_group_add_obj(sub_btn_group, ui_heatplatParamKd);
    lv_group_add_obj(sub_btn_group, ui_heatplatTempCalibrateAbleSwitch);
    for (int index = DISPLAY_TEMP_0 + 1;
         index < DISPLAY_TEMP_MAX && heatplatformModel.editCoreConfig.useCalibrationParam;
         ++index)
    {
        lv_group_add_obj(sub_btn_group, ui_heatplatRealTemp[index]);
    }
    lv_group_add_obj(sub_btn_group, ui_saveBtn);
    lv_indev_set_group(knobs_indev, sub_btn_group);
}

void ui_hp_setting_release(void *param)
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