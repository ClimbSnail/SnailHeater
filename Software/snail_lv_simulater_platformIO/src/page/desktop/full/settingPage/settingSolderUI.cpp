#include "./settingUIFull.h"

#ifdef FULL_UI

#include "../ui.h"
#include "../../desktop_model.h"

static lv_obj_t *ui_father = NULL;

static lv_obj_t *ui_solderGridSwitch;
static lv_obj_t *ui_solderAutoTypeSwitch;
static lv_obj_t *ui_msgBox = NULL;
static lv_obj_t *ui_solderNameLabel;
static lv_obj_t *ui_solderNameDropdown;
static lv_obj_t *ui_solderManageBnt;
static lv_obj_t *ui_solderManageText;
static lv_obj_t *solderQuickSetupTemp0;
static lv_obj_t *solderQuickSetupTemp1;
static lv_obj_t *solderQuickSetupTemp2;
static lv_obj_t *ui_easySleepTempBtn;
static lv_obj_t *fastPIDSwitch;
static lv_obj_t *ui_shortCircuitSwitch;
static lv_obj_t *shortCircuitLabel;

static lv_obj_t *ui_core_manage_page;
static lv_obj_t *ui_search_arc = NULL;
static lv_timer_t *ui_searchTimer = NULL; // 读取烙铁芯的定时器
static lv_obj_t *ui_solderNumDropdown;
static lv_obj_t *ui_solderWakeLabel;
static lv_obj_t *ui_solderWakeDropdown;
static lv_obj_t *ui_enterEasySleepTimeBtn;
static lv_obj_t *ui_enterDeepSleepTimeBtn;
static lv_obj_t *ui_solderFreqDropdown;
static lv_obj_t *ui_solderPowerLimit;
static lv_obj_t *ui_solderParamKp;
static lv_obj_t *ui_solderParamKi;
static lv_obj_t *ui_solderParamKd;

static lv_obj_t *ui_solderTempCalibrateAbleSwitch;
static lv_obj_t *ui_solderTempCalibrateObj;
static lv_obj_t *ui_solderTempCalibrate;
static lv_obj_t *ui_solderRealTemp[DISPLAY_TEMP_MAX];

static lv_obj_t *ui_coreManageExitBtn = NULL; // 退出
static lv_obj_t *ui_coreManageSaveBtn = NULL; // 保存修改
static lv_group_t *coreManage_btn_group = NULL;

static lv_group_t *msg_btn_group = NULL;

static void solderCoreTimer_timeout(lv_timer_t *timer);

static void set_angle(void *obj, int32_t v)
{
    lv_arc_set_value((lv_obj_t *)obj, v);
}

static void ui_solder_grid_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        if (lv_obj_has_state(target, LV_STATE_CHECKED))
            // SET_BIT(cfgKey1,CFG_KEY1_SOLDER_GRID);
            sysInfoModel.utilConfig.uiParam.uiGlobalParam.solderGridEnable = ENABLE_STATE_OPEN;
        else
            // CLR_BIT(cfgKey1,CFG_KEY1_SOLDER_GRID);
            sysInfoModel.utilConfig.uiParam.uiGlobalParam.solderGridEnable = ENABLE_STATE_CLOSE;
    }
}

static void auto_type_msgbox_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_current_target(e);
    LV_LOG_USER("Button %s clicked", lv_msgbox_get_active_btn_text(obj));

    if (!strcmp(lv_msgbox_get_active_btn_text(obj), SETTING_TEXT_CONFIRM))
    {
        solderModel.utilConfig.autoTypeSwitch = ENABLE_STATE_OPEN;

        lv_obj_set_style_text_opa(ui_solderNameLabel, 150, LV_PART_MAIN);
        lv_obj_add_flag(ui_solderNameDropdown, LV_OBJ_FLAG_HIDDEN);
        // 使手动类型不可操作
        lv_group_remove_obj(ui_solderNameDropdown);
    }
    else
    {
        lv_obj_clear_state(ui_solderAutoTypeSwitch, LV_STATE_CHECKED);
    }

    lv_indev_set_group(knobs_indev, sub_btn_group);

    if (NULL != ui_msgBox)
    {
        lv_group_remove_obj(lv_msgbox_get_btns(ui_msgBox));
        lv_obj_del(ui_msgBox);
        ui_msgBox = NULL;
    }
}

static void short_circuit_msgbox_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_current_target(e);
    LV_LOG_USER("Button %s clicked", lv_msgbox_get_active_btn_text(obj));

    lv_indev_set_group(knobs_indev, sub_btn_group);
    lv_group_remove_obj(lv_msgbox_get_btns(ui_msgBox));

    if (NULL != ui_msgBox)
    {
        lv_obj_del(ui_msgBox);
        ui_msgBox = NULL;
    }
}

static void ui_solder_auto_type_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        if (lv_obj_has_state(target, LV_STATE_CHECKED))
        {
            if (NULL != ui_msgBox)
            {
                lv_obj_del(ui_msgBox);
                ui_msgBox = NULL;
            }

            static const char *btns2[] = {SETTING_TEXT_BACK, SETTING_TEXT_CONFIRM, ""};

            ui_msgBox = lv_msgbox_create(ui_father,
                                         SETTING_TEXT_SOLDER_MSGBOX_INFO_TITLE,
                                         SETTING_TEXT_SOLDER_MSGBOX_INFO, btns2, false);
            lv_obj_add_style(ui_msgBox, &label_text_style, 0);
            lv_obj_add_event_cb(ui_msgBox, auto_type_msgbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
            lv_obj_center(ui_msgBox);

            lv_obj_t *btnmatrix = lv_msgbox_get_btns(ui_msgBox);
            lv_group_add_obj(msg_btn_group, btnmatrix);

            lv_indev_set_group(knobs_indev, msg_btn_group);
            lv_group_focus_obj(btnmatrix);
        }
        else
        {
            solderModel.utilConfig.autoTypeSwitch = ENABLE_STATE_CLOSE;

            lv_obj_set_style_text_opa(ui_solderNameLabel, 255, LV_PART_MAIN);
            lv_obj_clear_flag(ui_solderNameDropdown, LV_OBJ_FLAG_HIDDEN);
            // 调整按键组
            lv_group_remove_obj(ui_solderManageBnt);
            lv_group_remove_obj(solderQuickSetupTemp0);
            lv_group_remove_obj(solderQuickSetupTemp1);
            lv_group_remove_obj(solderQuickSetupTemp2);
            lv_group_remove_obj(ui_easySleepTempBtn);
            lv_group_remove_obj(fastPIDSwitch);
            lv_group_remove_obj(ui_shortCircuitSwitch);
            // lv_group_remove_obj(ui_subBackBtn);
            lv_group_add_obj(sub_btn_group, ui_solderNameDropdown);
            lv_group_add_obj(sub_btn_group, ui_solderManageBnt);
            lv_group_add_obj(sub_btn_group, solderQuickSetupTemp0);
            lv_group_add_obj(sub_btn_group, solderQuickSetupTemp1);
            lv_group_add_obj(sub_btn_group, solderQuickSetupTemp2);
            lv_group_add_obj(sub_btn_group, ui_easySleepTempBtn);
            lv_group_add_obj(sub_btn_group, fastPIDSwitch);
#if SH_HARDWARE_VER >= SH_ESP32S2_WROOM_V26
            lv_group_add_obj(sub_btn_group, ui_shortCircuitSwitch);
#endif
            // lv_group_add_obj(sub_btn_group, ui_subBackBtn);
        }
    }
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
    default:
    {
        typeInd = 0;
    }
    break;
    }
    lv_dropdown_set_selected(obj, typeInd);
}

static void setSolderFreq(lv_obj_t *obj, SOLDER_PWM_FREQ type)
{
    uint8_t typeInd;

    switch (type)
    {
    case SOLDER_PWM_FREQ_64:
    {
        typeInd = 0;
    }
    break;
    case SOLDER_PWM_FREQ_128:
    {
        typeInd = 1;
    }
    break;
    case SOLDER_PWM_FREQ_256:
    {
        typeInd = 2;
    }
    break;
    case SOLDER_PWM_FREQ_512:
    {
        typeInd = 3;
    }
    break;
    case SOLDER_PWM_FREQ_1024:
    {
        typeInd = 4;
    }
    break;
    case SOLDER_PWM_FREQ_2048:
    {
        typeInd = 5;
    }
    break;
    case SOLDER_PWM_FREQ_4096:
    {
        typeInd = 6;
    }
    break;
    case SOLDER_PWM_FREQ_8192:
    {
        typeInd = 7;
    }
    break;
    case SOLDER_PWM_FREQ_16384:
    {
        typeInd = 8;
    }
    break;
    default:
    {
        typeInd = 0;
    }
    break;
    }

    lv_dropdown_set_selected(obj, typeInd);
}

static void solderCoreTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    if (NULL != ui_search_arc)
    {
        lv_obj_del(ui_search_arc);
        ui_search_arc = NULL;
    }

    if (INFO_MANAGE_ACTION_SOLDER_READ_OK == solderModel.manageCoreAction)
    {
        solderModel.manageCoreAction = INFO_MANAGE_ACTION_SOLDER_IDLE;
        // 读取成功
        setSolderSwitchType(ui_solderWakeDropdown, solderModel.editCoreConfig.wakeSwitchType);
        setSolderFreq(ui_solderFreqDropdown, solderModel.editCoreConfig.solderPwmFreq);
        lv_obj_set_style_text_opa(ui_solderWakeLabel, 255, LV_PART_MAIN);
        lv_obj_set_style_opa(ui_solderWakeDropdown, 255, LV_PART_MAIN);
        lv_obj_set_style_opa(ui_enterEasySleepTimeBtn, 255, LV_PART_MAIN);
        lv_obj_set_style_opa(ui_enterDeepSleepTimeBtn, 255, LV_PART_MAIN);
        lv_obj_set_style_opa(ui_solderFreqDropdown, 255, LV_PART_MAIN);
        lv_numberbtn_set_value(ui_enterEasySleepTimeBtn, solderModel.editCoreConfig.enterEasySleepTime / 1000);
        lv_numberbtn_set_value(ui_enterDeepSleepTimeBtn, solderModel.editCoreConfig.enterDeepSleepTime / 1000);
        lv_numberbtn_set_value(ui_solderPowerLimit, solderModel.editCoreConfig.powerLimit * 100);
        lv_numberbtn_set_value(ui_solderParamKp, solderModel.editCoreConfig.kp);
        lv_numberbtn_set_value(ui_solderParamKi, solderModel.editCoreConfig.ki);
        lv_numberbtn_set_value(ui_solderParamKd, solderModel.editCoreConfig.kd);
        if (solderModel.editCoreConfig.useCalibrationParam)
        {
            lv_obj_add_state(ui_solderTempCalibrateAbleSwitch, LV_STATE_CHECKED); // 反向
        }
        else
        {
            lv_obj_clear_state(ui_solderTempCalibrateAbleSwitch, LV_STATE_CHECKED);
        }

        for (int index = DISPLAY_TEMP_0 + 1; index < DISPLAY_TEMP_MAX; ++index)
        {
            lv_numberbtn_set_value(ui_solderRealTemp[index],
                                   solderModel.editCoreConfig.realTemp[index]);
        }

        // 调整按键组
        lv_group_add_obj(coreManage_btn_group, ui_solderWakeDropdown);
        lv_group_add_obj(coreManage_btn_group, ui_enterEasySleepTimeBtn);
        lv_group_add_obj(coreManage_btn_group, ui_enterDeepSleepTimeBtn);
        lv_group_add_obj(coreManage_btn_group, ui_solderFreqDropdown);
        lv_group_add_obj(coreManage_btn_group, ui_solderPowerLimit);
        lv_group_add_obj(coreManage_btn_group, ui_solderParamKp);
        lv_group_add_obj(coreManage_btn_group, ui_solderParamKi);
        lv_group_add_obj(coreManage_btn_group, ui_solderParamKd);
        lv_group_add_obj(coreManage_btn_group, ui_solderTempCalibrateAbleSwitch);
        for (int index = DISPLAY_TEMP_0 + 1;
             index < DISPLAY_TEMP_MAX && solderModel.editCoreConfig.useCalibrationParam;
             ++index)
        {
            lv_group_add_obj(coreManage_btn_group, ui_solderRealTemp[index]);
        }
        lv_group_add_obj(coreManage_btn_group, ui_coreManageSaveBtn);
        lv_indev_set_group(knobs_indev, coreManage_btn_group);
    }
}

static void ui_solder_name_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        // uint16_t index = lv_dropdown_get_selected(ui_solderNameDropdown); // 获取索引
        // solderModel.coreConfig.solderType = index;

        if (ENABLE_STATE_OPEN == solderModel.utilConfig.autoTypeSwitch)
        {
            return;
        }

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

        uint16_t id = 0;
        for (index = 0; id_text[index] != 0; index++)
        {
            id = id * 10 + id_text[index] - '0';
        }
        solderModel.utilConfig.curCoreID = id;
    }
}

static void ui_solder_num_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        // uint16_t index = lv_dropdown_get_selected(ui_solderNumDropdown); // 获取索引
        // solderModel.editCoreConfig.solderType = index;

        char id_text[16];
        char solderNameStr[16];
        // 烙铁名字的格式 id+'_'+type
        lv_dropdown_get_selected_str(ui_solderNumDropdown, solderNameStr, 16);
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

        uint16_t id = 0;
        for (index = 0; id_text[index] != 0; index++)
        {
            id = id * 10 + id_text[index] - '0';
        }

        // 正在编辑的烙铁芯
        solderModel.editCoreConfig.id = id;
        // 动作修改成读取
        solderModel.manageCoreAction = INFO_MANAGE_ACTION_SOLDER_READ;
        lv_obj_set_style_text_opa(ui_solderWakeLabel, 150, LV_PART_MAIN);
        lv_obj_set_style_opa(ui_solderWakeDropdown, 150, LV_PART_MAIN);
        lv_obj_set_style_opa(ui_solderFreqDropdown, 150, LV_PART_MAIN);

        // 调整按键组
        lv_group_remove_obj(ui_solderWakeDropdown);
        lv_group_remove_obj(ui_enterEasySleepTimeBtn);
        lv_group_remove_obj(ui_enterDeepSleepTimeBtn);
        lv_group_remove_obj(ui_solderFreqDropdown);
        lv_group_remove_obj(ui_solderPowerLimit);
        lv_group_remove_obj(ui_solderParamKp);
        lv_group_remove_obj(ui_solderParamKi);
        lv_group_remove_obj(ui_solderParamKd);
        lv_group_remove_obj(ui_solderTempCalibrateAbleSwitch);
        lv_group_remove_obj(ui_coreManageSaveBtn);
        lv_indev_set_group(knobs_indev, coreManage_btn_group);

        ui_search_arc = lv_arc_create(ui_father);
        lv_arc_set_rotation(ui_search_arc, 270);
        lv_arc_set_bg_angles(ui_search_arc, 0, 360);
        lv_obj_remove_style(ui_search_arc, NULL, LV_PART_KNOB);  /*Be sure the knob is not displayed*/
        lv_obj_clear_flag(ui_search_arc, LV_OBJ_FLAG_CLICKABLE); /*To not allow adjusting by click*/
        lv_obj_center(ui_search_arc);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, ui_search_arc);
        lv_anim_set_exec_cb(&a, set_angle);
        lv_anim_set_time(&a, 400);
        // 重复次数。默认值为1。LV_ANIM_REPEAT_INFINIT用于无限重复
        // lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_repeat_delay(&a, 500);
        lv_anim_set_values(&a, 0, 100);
        lv_anim_start(&a);

        ui_searchTimer = lv_timer_create(solderCoreTimer_timeout, 600, NULL);
        lv_timer_set_repeat_count(ui_searchTimer, 1);
    }
}

void ui_solder_core_exit_btn_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (LV_EVENT_PRESSED == event_code)
    {
        lv_obj_clear_state(ui_coreManageExitBtn, LV_STATE_FOCUSED);
        if (NULL != ui_core_manage_page)
        {
            lv_obj_del(ui_core_manage_page);
            ui_core_manage_page = NULL;
        }

        solderModel.manageCoreAction = INFO_MANAGE_ACTION_SOLDER_IDLE;

        ui_solder_setting_init(ui_father);
        ui_solder_setting_init_group(ui_father);
        // lv_indev_set_group(knobs_indev, sub_btn_group);
        lv_group_focus_obj(ui_solderManageBnt);
    }
}

void ui_solder_core_save_btn_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (LV_EVENT_PRESSED == event_code)
    {
        lv_obj_clear_state(ui_coreManageSaveBtn, LV_STATE_FOCUSED);

        solderModel.manageCoreAction = INFO_MANAGE_ACTION_SOLDER_WRITE;

        lv_group_focus_obj(ui_coreManageExitBtn);
    }
}

static void ui_set_param_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == ui_solderParamKp)
        {
            solderModel.editCoreConfig.kp = lv_numberbtn_get_value(ui_solderParamKp);
        }
        else if (target == ui_solderParamKi)
        {
            solderModel.editCoreConfig.ki = lv_numberbtn_get_value(ui_solderParamKi);
        }
        else if (target == ui_solderParamKd)
        {
            solderModel.editCoreConfig.kd = lv_numberbtn_get_value(ui_solderParamKd);
        }
        else if (target == ui_enterEasySleepTimeBtn)
        {
            solderModel.editCoreConfig.enterEasySleepTime = lv_numberbtn_get_value(ui_enterEasySleepTimeBtn) * 1000;
        }
        else if (target == ui_enterDeepSleepTimeBtn)
        {
            solderModel.editCoreConfig.enterDeepSleepTime = lv_numberbtn_get_value(ui_enterDeepSleepTimeBtn) * 1000;
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
            if (target == ui_solderRealTemp[index])
            {
                solderModel.editCoreConfig.realTemp[index] =
                    lv_numberbtn_get_value(ui_solderRealTemp[index]);
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
        if (target == ui_solderTempCalibrateAbleSwitch)
        {
            solderModel.editCoreConfig.useCalibrationParam = (ENABLE_STATE)flag;

            if (solderModel.editCoreConfig.useCalibrationParam)
            {
                lv_group_remove_obj(ui_coreManageSaveBtn);
                // 添加校准界面按键
                for (int index = DISPLAY_TEMP_0 + 1; index < DISPLAY_TEMP_MAX; ++index)
                {
                    lv_group_add_obj(coreManage_btn_group, ui_solderRealTemp[index]);
                }
                lv_group_add_obj(coreManage_btn_group, ui_coreManageSaveBtn);
            }
            else
            {
                // 移除校准界面按键
                for (int index = DISPLAY_TEMP_0 + 1; index < DISPLAY_TEMP_MAX; ++index)
                {
                    lv_group_remove_obj(ui_solderRealTemp[index]);
                }
            }
        }
    }
}

static void ui_solder_wake_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        uint16_t index = lv_dropdown_get_selected(ui_solderWakeDropdown); // 获取索引
        solderModel.editCoreConfig.wakeSwitchType = (SOLDER_SHAKE_TYPE)index;
    }
}

static void ui_solder_freq_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        uint16_t index = lv_dropdown_get_selected(ui_solderFreqDropdown); // 获取索引
        solderModel.editCoreConfig.solderPwmFreq = (SOLDER_PWM_FREQ)(64 << index);
        LV_LOG_USER("solderModel.editCoreConfig.solderPwmFreq = %lu\n",
                    solderModel.editCoreConfig.solderPwmFreq);
    }
}

static void ui_solder_power_limit_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == ui_solderPowerLimit)
        {
            solderModel.editCoreConfig.powerLimit =
                lv_numberbtn_get_value(ui_solderPowerLimit) / 100.0;
        }
    }
}

void ui_solder_core_manage_init(lv_obj_t *father)
{
    // ui_core_manage_page = lv_label_create(father);
    // lv_obj_set_style_outline_pad(ui_core_manage_page, 0, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    // lv_obj_set_style_shadow_width(ui_core_manage_page, 0, LV_PART_MAIN);
    // lv_obj_set_size(ui_core_manage_page, lv_obj_get_width(father), lv_obj_get_height(father));

    // 初始默认值 todo 存在bug
    solderModel.editCoreConfig = solderModel.coreConfig;

    // 烙铁编号
    lv_obj_t *ui_solderNumLabel = lv_label_create(father);
    lv_obj_align(ui_solderNumLabel, LV_ALIGN_TOP_LEFT, 10, 15);
    lv_label_set_text(ui_solderNumLabel, SETTING_TEXT_SOLDER_NUM);
    lv_obj_add_style(ui_solderNumLabel, &label_text_style, 0);

    // static char solderName[] = "0_XXX\n1_T12\n2_C210\n3_C245\n4_C470\n5_C115\n6_C105";
    ui_solderNumDropdown = lv_dropdown_create(father);
    lv_dropdown_set_options(ui_solderNumDropdown, solderModel.coreNameList);
    // lv_dropdown_set_options_static(ui_solderNumDropdown, solderName);
    setSolderName(ui_solderNumDropdown, solderModel.curCoreName);
    lv_obj_set_size(ui_solderNumDropdown, 80, LV_SIZE_CONTENT);
    lv_obj_align(ui_solderNumDropdown, LV_ALIGN_TOP_LEFT, 105, 5);
    lv_obj_add_flag(ui_solderNumDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_text_font(ui_solderNumDropdown, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_solderNumDropdown, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_solderNumDropdown, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_solderNumDropdown, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 分割线
    /*Create an array for the points of the line*/
    static lv_point_t line_points[] = {{0, 5}, {200, 5}};
    /*Create style*/
    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 4);
    lv_style_set_text_color(&style_line,
                            IS_WHITE_THEME ? WHITE_THEME_LABEL_TEXT_COLOR : BLACK_THEME_LABEL_TEXT_COLOR);
    lv_style_set_line_rounded(&style_line, true);
    /*Create a line and apply the new style*/
    lv_obj_t *main_line = lv_line_create(father);
    lv_line_set_points(main_line, line_points, 2); /*Set the points*/
    lv_obj_add_style(main_line, &style_line, 0);
    lv_obj_align(main_line, LV_ALIGN_CENTER, 0, -30);

    // 唤醒方式
    ui_solderWakeLabel = lv_label_create(father);
    lv_obj_align(ui_solderWakeLabel, LV_ALIGN_TOP_LEFT, 10, 75);
    lv_label_set_text(ui_solderWakeLabel, SETTING_TEXT_SOLDER_WAKEUP);
    lv_obj_add_style(ui_solderWakeLabel, &label_text_style, 0);

    ui_solderWakeDropdown = lv_dropdown_create(father);
    lv_dropdown_set_options(ui_solderWakeDropdown, SOLDER_SHAKE_TYPE_TEXT);
    // 初始已当前选择的烙铁芯为主
    setSolderSwitchType(ui_solderWakeDropdown, solderModel.editCoreConfig.wakeSwitchType);
    lv_obj_set_size(ui_solderWakeDropdown, 80, LV_SIZE_CONTENT);
    lv_obj_align(ui_solderWakeDropdown, LV_ALIGN_TOP_LEFT, 105, 65);
    lv_obj_add_flag(ui_solderWakeDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_text_font(ui_solderWakeDropdown, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_solderWakeDropdown, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_solderWakeDropdown, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_solderWakeDropdown, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 进入浅度休眠时间
    lv_obj_t *enterEasySleepTimeLabel = lv_label_create(father);
    lv_obj_align_to(enterEasySleepTimeLabel, ui_solderWakeLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_label_set_text(enterEasySleepTimeLabel, SETTING_TEXT_SOLDER_EASY_SLEEP_TIME);
    lv_obj_set_size(enterEasySleepTimeLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(enterEasySleepTimeLabel, &label_text_style, 0);

    ui_enterEasySleepTimeBtn = lv_numberbtn_create(father);
    lv_obj_t *enterEasySleepTimeBtnLabel = lv_label_create(ui_enterEasySleepTimeBtn);
    lv_numberbtn_set_label_and_format(ui_enterEasySleepTimeBtn,
                                      enterEasySleepTimeBtnLabel, "%ds", 1);
    lv_numberbtn_set_range(ui_enterEasySleepTimeBtn, SOLDER_EASY_SLEEP_TIME_MIN / 1000,
                           SOLDER_EASY_SLEEP_TIME_MAX / 1000);
    lv_numberbtn_set_value(ui_enterEasySleepTimeBtn, solderModel.editCoreConfig.enterEasySleepTime / 1000);
    lv_obj_set_size(ui_enterEasySleepTimeBtn, 50, 20);
    lv_obj_align_to(ui_enterEasySleepTimeBtn, enterEasySleepTimeLabel,
                    LV_ALIGN_OUT_RIGHT_MID, 25, 0);
    lv_obj_add_flag(ui_enterEasySleepTimeBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_enterEasySleepTimeBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_enterEasySleepTimeBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_enterEasySleepTimeBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_enterEasySleepTimeBtn, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_enterEasySleepTimeBtn, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 进入深度休眠时间
    lv_obj_t *enterDeepSleepTimeLabel = lv_label_create(father);
    lv_obj_align_to(enterDeepSleepTimeLabel, enterEasySleepTimeLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_label_set_text(enterDeepSleepTimeLabel, SETTING_TEXT_SOLDER_DEEP_SLEEP_TIME);
    lv_obj_set_size(enterDeepSleepTimeLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(enterDeepSleepTimeLabel, &label_text_style, 0);

    ui_enterDeepSleepTimeBtn = lv_numberbtn_create(father);
    lv_obj_t *enterDeepSleepTimeBtnLabel = lv_label_create(ui_enterDeepSleepTimeBtn);
    lv_numberbtn_set_label_and_format(ui_enterDeepSleepTimeBtn,
                                      enterDeepSleepTimeBtnLabel, "%ds", 1);
    lv_numberbtn_set_range(ui_enterDeepSleepTimeBtn, SOLDER_EASY_SLEEP_TIME_MIN / 1000,
                           SOLDER_EASY_SLEEP_TIME_MAX / 1000);
    lv_numberbtn_set_value(ui_enterDeepSleepTimeBtn, solderModel.editCoreConfig.enterDeepSleepTime / 1000);
    lv_obj_set_size(ui_enterDeepSleepTimeBtn, 50, 20);
    lv_obj_align_to(ui_enterDeepSleepTimeBtn, enterDeepSleepTimeLabel,
                    LV_ALIGN_OUT_RIGHT_MID, 25, 0);
    lv_obj_add_flag(ui_enterDeepSleepTimeBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_enterDeepSleepTimeBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_enterDeepSleepTimeBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_enterDeepSleepTimeBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_enterDeepSleepTimeBtn, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_enterDeepSleepTimeBtn, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 烙铁芯发热频率
    lv_obj_t *ui_solderFreqLabel = lv_label_create(father);
    lv_obj_align_to(ui_solderFreqLabel, enterDeepSleepTimeLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_label_set_text(ui_solderFreqLabel, SETTING_TEXT_SOLDER_DRIVE_FREQ);
    lv_obj_add_style(ui_solderFreqLabel, &label_text_style, 0);

    ui_solderFreqDropdown = lv_dropdown_create(father);
    lv_dropdown_set_options(ui_solderFreqDropdown, SOLDER_PWM_FREQ_TEXT);
    // 初始已当前选择的烙铁芯为主
    setSolderFreq(ui_solderFreqDropdown, solderModel.editCoreConfig.solderPwmFreq);
    lv_obj_set_size(ui_solderFreqDropdown, 80, LV_SIZE_CONTENT);
    lv_obj_align_to(ui_solderFreqDropdown, ui_solderFreqLabel,
                    LV_ALIGN_OUT_LEFT_MID, 175, 0);
    lv_obj_add_flag(ui_solderFreqDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_text_font(ui_solderFreqDropdown, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_solderFreqDropdown, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_solderFreqDropdown, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_solderFreqDropdown, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 功率限制
    lv_obj_t *ui_solderPowerLimitLabel = lv_label_create(father);
    lv_obj_align_to(ui_solderPowerLimitLabel, ui_solderFreqLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_label_set_text(ui_solderPowerLimitLabel, SETTING_TEXT_SOLDER_POWER_LIMIT);
    lv_obj_add_style(ui_solderPowerLimitLabel, &label_text_style, 0);

    ui_solderPowerLimit = lv_numberbtn_create(father);
    lv_obj_t *solderPowerLimitLabel = lv_label_create(ui_solderPowerLimit);
    lv_numberbtn_set_label_and_format(ui_solderPowerLimit,
                                      solderPowerLimitLabel, "%d%%", 1);
    lv_numberbtn_set_range(ui_solderPowerLimit, 10, 100);
    lv_numberbtn_set_value(ui_solderPowerLimit, solderModel.editCoreConfig.powerLimit * 100);
    // lv_obj_remove_style_all(ui_solderPowerLimit);
    lv_obj_set_size(ui_solderPowerLimit, 50, 20);
    lv_obj_align_to(ui_solderPowerLimit, ui_solderPowerLimitLabel,
                    LV_ALIGN_OUT_RIGHT_MID, 35, 0);
    lv_obj_add_flag(ui_solderPowerLimit, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_solderPowerLimit, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_solderPowerLimit, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_solderPowerLimit, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_solderPowerLimit, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_solderPowerLimit, &setting_btn_pressed_style, LV_STATE_EDITED);

    // PID参数
    lv_obj_t *ui_solderPIDLabel = lv_label_create(father);
    lv_obj_align_to(ui_solderPIDLabel, ui_solderPowerLimitLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_label_set_text(ui_solderPIDLabel, "PID参数");
    lv_obj_set_size(ui_solderPIDLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(ui_solderPIDLabel, &label_text_style, 0);

    ui_solderParamKp = lv_numberbtn_create(father);
    lv_obj_t *solderParamKpLabel = lv_label_create(ui_solderParamKp);
    lv_numberbtn_set_label_and_format(ui_solderParamKp,
                                      solderParamKpLabel, "P->%d", 1);
    lv_numberbtn_set_range(ui_solderParamKp, 0, 200);
    lv_numberbtn_set_value(ui_solderParamKp, solderModel.editCoreConfig.kp);
    // lv_obj_remove_style_all(ui_solderParamKp);
    lv_obj_set_size(ui_solderParamKp, 50, 20);
    lv_obj_align_to(ui_solderParamKp, ui_solderPIDLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_obj_add_flag(ui_solderParamKp, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_solderParamKp, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_solderParamKp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_solderParamKp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_solderParamKp, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_solderParamKp, &setting_btn_pressed_style, LV_STATE_EDITED);

    ui_solderParamKi = lv_numberbtn_create(father);
    lv_obj_t *solderParamKiLabel = lv_label_create(ui_solderParamKi);
    lv_numberbtn_set_label_and_format(ui_solderParamKi,
                                      solderParamKiLabel, "I->%.1f", 0.1);
    lv_numberbtn_set_range(ui_solderParamKi, 0, 200);
    lv_numberbtn_set_value(ui_solderParamKi, solderModel.editCoreConfig.ki);
    // lv_obj_remove_style_all(ui_solderParamKi);
    lv_obj_set_size(ui_solderParamKi, 50, 20);
    lv_obj_align_to(ui_solderParamKi, ui_solderParamKp,
                    LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_add_flag(ui_solderParamKi, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_solderParamKi, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_solderParamKi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_solderParamKi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_solderParamKi, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_solderParamKi, &setting_btn_pressed_style, LV_STATE_EDITED);

    ui_solderParamKd = lv_numberbtn_create(father);
    lv_obj_t *solderParamKdLabel = lv_label_create(ui_solderParamKd);
    lv_numberbtn_set_label_and_format(ui_solderParamKd,
                                      solderParamKdLabel, "D->%d", 1);
    lv_numberbtn_set_range(ui_solderParamKd, 0, 200);
    lv_numberbtn_set_value(ui_solderParamKd, solderModel.editCoreConfig.kd);
    // lv_obj_remove_style_all(ui_solderParamKd);
    lv_obj_set_style_border_width(ui_solderParamKd, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_size(ui_solderParamKd, 50, 20);
    lv_obj_align_to(ui_solderParamKd, ui_solderParamKi,
                    LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_add_style(ui_solderParamKd, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_solderParamKd, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 使能温度标定
    lv_obj_t *ui_solderTempCalibrateAbleLaleb = lv_label_create(father);
    lv_obj_align_to(ui_solderTempCalibrateAbleLaleb, ui_solderPIDLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 45);
    lv_label_set_text(ui_solderTempCalibrateAbleLaleb, "启用温度标定");
    lv_obj_add_style(ui_solderTempCalibrateAbleLaleb, &label_text_style, 0);

    ui_solderTempCalibrateAbleSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_solderTempCalibrateAbleSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_solderTempCalibrateAbleSwitch, 40, 20);
    lv_obj_align_to(ui_solderTempCalibrateAbleSwitch, ui_solderTempCalibrateAbleLaleb,
                    LV_ALIGN_OUT_RIGHT_MID, 30, 0);
    if (solderModel.editCoreConfig.useCalibrationParam)
    {
        lv_obj_add_state(ui_solderTempCalibrateAbleSwitch, LV_STATE_CHECKED); // 反向
    }
    else
    {
        lv_obj_clear_state(ui_solderTempCalibrateAbleSwitch, LV_STATE_CHECKED);
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_solderTempCalibrateAbleSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_solderTempCalibrateAbleSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_solderTempCalibrateAbleSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 温度标定
    ui_solderTempCalibrateObj = lv_obj_create(father);
    lv_obj_set_size(ui_solderTempCalibrateObj, SH_SCREEN_WIDTH - 75,
                    LV_SIZE_CONTENT);
    lv_obj_add_style(ui_solderTempCalibrateObj, &black_white_theme_style,
                     LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(ui_solderTempCalibrateObj, ui_solderTempCalibrateAbleLaleb,
                    LV_ALIGN_OUT_BOTTOM_LEFT, -10, 10);

    ui_solderTempCalibrate = lv_label_create(ui_solderTempCalibrateObj);
    lv_label_set_text(ui_solderTempCalibrate, "温度标定 格式(显示->真实)");
    lv_obj_add_style(ui_solderTempCalibrate, &label_text_style, 0);

    for (int index = DISPLAY_TEMP_0 + 1; index < DISPLAY_TEMP_MAX; ++index)
    {
        ui_solderRealTemp[index] = lv_numberbtn_create(ui_solderTempCalibrateObj);
        lv_obj_t *solderRealTempLabel = lv_label_create(ui_solderRealTemp[index]);
        lv_obj_add_style(solderRealTempLabel, &label_text_style, 0);
        char format[8] = "";
        snprintf(format, 8, "%d->%%d", index * DISPLAY_TEMP_STEP);
        lv_numberbtn_set_label_and_format(ui_solderRealTemp[index],
                                          solderRealTempLabel, format, 1);
        lv_numberbtn_set_range(ui_solderRealTemp[index], 0, 800);
        lv_numberbtn_set_value(ui_solderRealTemp[index],
                               solderModel.editCoreConfig.realTemp[index]);
        // lv_obj_remove_style_all(ui_solderRealTemp[0]);
        // lv_obj_add_style(ui_solderRealTemp[index], &label_text_style, 0);
        lv_obj_set_size(ui_solderRealTemp[index], 60, 20);
        lv_obj_align_to(ui_solderRealTemp[index], ui_solderTempCalibrate,
                        LV_ALIGN_BOTTOM_LEFT, ((index + 2) % 3) * 65 - 10,
                        ((index + 2) / 3) * 30);

        lv_obj_add_flag(ui_solderRealTemp[index], LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_clear_flag(ui_solderRealTemp[index], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(ui_solderRealTemp[index], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui_solderRealTemp[index], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_style(ui_solderRealTemp[index], &setting_btn_focused_style, LV_STATE_FOCUSED);
        lv_obj_add_style(ui_solderRealTemp[index], &setting_btn_pressed_style, LV_STATE_EDITED);
    }

    ui_coreManageExitBtn = lv_btn_create(father);
    lv_obj_remove_style_all(ui_coreManageExitBtn);
    lv_obj_align_to(ui_coreManageExitBtn, ui_solderTempCalibrateObj,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 20, 20);
    lv_obj_add_flag(ui_coreManageExitBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_coreManageExitBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_coreManageExitBtn, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_set_size(ui_coreManageExitBtn, 70, LV_SIZE_CONTENT);

    lv_obj_t *ui_manageCoreExitLabel = lv_label_create(ui_coreManageExitBtn);
    lv_obj_center(ui_manageCoreExitLabel);
    lv_obj_add_style(ui_manageCoreExitLabel, &label_text_style, 0);
    lv_label_set_text(ui_manageCoreExitLabel, SETTING_TEXT_EDIT_EXIT_MD);

    ui_coreManageSaveBtn = lv_btn_create(father);
    lv_obj_remove_style_all(ui_coreManageSaveBtn);
    lv_obj_align_to(ui_coreManageSaveBtn, ui_solderTempCalibrateObj,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 120, 20);
    lv_obj_add_flag(ui_coreManageSaveBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_coreManageSaveBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_coreManageSaveBtn, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_set_size(ui_coreManageSaveBtn, 70, LV_SIZE_CONTENT);

    lv_obj_t *ui_saveCoreLabel = lv_label_create(ui_coreManageSaveBtn);
    lv_obj_center(ui_saveCoreLabel);
    lv_obj_add_style(ui_saveCoreLabel, &label_text_style, 0);
    lv_label_set_text(ui_saveCoreLabel, SETTING_TEXT_EDIT_SAVE);

    lv_obj_add_event_cb(ui_solderNumDropdown, ui_solder_num_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_solderWakeDropdown, ui_solder_wake_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_enterEasySleepTimeBtn, ui_set_param_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_enterDeepSleepTimeBtn, ui_set_param_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_solderFreqDropdown, ui_solder_freq_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_solderPowerLimit, ui_solder_power_limit_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_solderParamKp, ui_set_param_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_solderParamKi, ui_set_param_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_solderParamKd, ui_set_param_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_solderTempCalibrateAbleSwitch, ui_useCalibration_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    for (int index = DISPLAY_TEMP_0 + 1; index < DISPLAY_TEMP_MAX; ++index)
    {
        lv_obj_add_event_cb(ui_solderRealTemp[index], ui_real_temp_pressed, LV_EVENT_PRESSED, NULL);
    }
    lv_obj_add_event_cb(ui_coreManageExitBtn, ui_solder_core_exit_btn_pressed, LV_EVENT_PRESSED, 0);
    lv_obj_add_event_cb(ui_coreManageSaveBtn, ui_solder_core_save_btn_pressed, LV_EVENT_PRESSED, 0);

    if (NULL != coreManage_btn_group)
    {
        lv_group_del(coreManage_btn_group);
        coreManage_btn_group = NULL;
    }
    coreManage_btn_group = lv_group_create();
    lv_group_add_obj(coreManage_btn_group, ui_coreManageExitBtn);
    lv_group_add_obj(coreManage_btn_group, ui_solderNumDropdown);
    lv_group_add_obj(coreManage_btn_group, ui_solderWakeDropdown);
    lv_group_add_obj(coreManage_btn_group, ui_enterEasySleepTimeBtn);
    lv_group_add_obj(coreManage_btn_group, ui_enterDeepSleepTimeBtn);
    lv_group_add_obj(coreManage_btn_group, ui_solderFreqDropdown);
    lv_group_add_obj(coreManage_btn_group, ui_solderPowerLimit);
    lv_group_add_obj(coreManage_btn_group, ui_solderParamKp);
    lv_group_add_obj(coreManage_btn_group, ui_solderParamKi);
    lv_group_add_obj(coreManage_btn_group, ui_solderParamKd);
    lv_group_add_obj(coreManage_btn_group, ui_solderTempCalibrateAbleSwitch);
    for (int index = DISPLAY_TEMP_0 + 1;
         index < DISPLAY_TEMP_MAX && solderModel.editCoreConfig.useCalibrationParam;
         ++index)
    {
        lv_group_add_obj(coreManage_btn_group, ui_solderRealTemp[index]);
    }
    lv_group_add_obj(coreManage_btn_group, ui_coreManageSaveBtn);
    lv_indev_set_group(knobs_indev, coreManage_btn_group);
}

static void ui_solder_bnt_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == ui_solderManageBnt)
        {
            lv_obj_clean(ui_father);

            ui_solder_core_manage_init(ui_father);
        }
    }
}

static void ui_solderSetBnt_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == solderQuickSetupTemp0)
        {
            solderModel.utilConfig.quickSetupTemp_0 = lv_numberbtn_get_value(solderQuickSetupTemp0);
        }
        else if (target == solderQuickSetupTemp1)
        {
            solderModel.utilConfig.quickSetupTemp_1 = lv_numberbtn_get_value(solderQuickSetupTemp1);
        }
        else if (target == solderQuickSetupTemp2)
        {
            solderModel.utilConfig.quickSetupTemp_2 = lv_numberbtn_get_value(solderQuickSetupTemp2);
        }
        else if (target == ui_easySleepTempBtn)
        {
            solderModel.utilConfig.easySleepTemp = lv_numberbtn_get_value(ui_easySleepTempBtn);
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
                solderModel.utilConfig.fastPID = ENABLE_STATE_OPEN;
            else
                solderModel.utilConfig.fastPID = ENABLE_STATE_CLOSE;
        }
        else if (target == ui_shortCircuitSwitch)
        {
            if (lv_obj_has_state(target, LV_STATE_CHECKED))
            {
                solderModel.utilConfig.shortCircuit = ENABLE_STATE_OPEN;

                if (NULL != ui_msgBox)
                {
                    lv_obj_del(ui_msgBox);
                    ui_msgBox = NULL;
                }

                static const char *btns1[] = {SETTING_TEXT_CONFIRM, ""};

                ui_msgBox = lv_msgbox_create(ui_father,
                                             SETTING_TEXT_SOLDER_SHORT_CUR_INFO_TITLE,
                                             SETTING_TEXT_SOLDER_SHORT_CUR_INFO, btns1, false);

                lv_obj_add_style(ui_msgBox, &label_text_style, 0);
                lv_obj_add_event_cb(ui_msgBox, short_circuit_msgbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
                lv_obj_align_to(ui_msgBox, ui_shortCircuitSwitch,
                                LV_ALIGN_CENTER, -25, -30);

                lv_obj_t *btnmatrix = lv_msgbox_get_btns(ui_msgBox);
                lv_group_add_obj(msg_btn_group, btnmatrix);

                lv_indev_set_group(knobs_indev, msg_btn_group);
                lv_group_focus_obj(btnmatrix);
            }
            else
            {
                solderModel.utilConfig.shortCircuit = ENABLE_STATE_CLOSE;
            }
        }
    }
}

void ui_solder_setting_init(lv_obj_t *father)
{
    if (NULL != ui_father)
    {
        lv_obj_clean(ui_father);
        ui_father = NULL;
    }

    ui_father = father;
    // 烙铁曲线网格
    lv_obj_t *ui_solderGridLabel = lv_label_create(father);
    lv_obj_align(ui_solderGridLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_label_set_text(ui_solderGridLabel, SETTING_TEXT_SOLDER_GRID);
    lv_obj_add_style(ui_solderGridLabel, &label_text_style, 0);

    ui_solderGridSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_solderGridSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_solderGridSwitch, 40, 20);
    lv_obj_align_to(ui_solderGridSwitch, ui_solderGridLabel,
                    LV_ALIGN_OUT_LEFT_MID, 140, 0);
    if (ENABLE_STATE_OPEN == sysInfoModel.utilConfig.uiParam.uiGlobalParam.solderGridEnable)
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

    // 烙铁芯自动识别
    lv_obj_t *ui_solderAutoTypeSwitchLabel = lv_label_create(father);
    lv_obj_align_to(ui_solderAutoTypeSwitchLabel, ui_solderGridLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    lv_label_set_text(ui_solderAutoTypeSwitchLabel, SETTING_TEXT_SOLDER_AUTO_TYPE);
    lv_obj_add_style(ui_solderAutoTypeSwitchLabel, &label_text_style, 0);

    ui_solderAutoTypeSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_solderAutoTypeSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_solderAutoTypeSwitch, 40, 20);
    lv_obj_align_to(ui_solderAutoTypeSwitch, ui_solderAutoTypeSwitchLabel,
                    LV_ALIGN_OUT_LEFT_MID, 140, 0);
    lv_obj_set_style_outline_color(ui_solderAutoTypeSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_solderAutoTypeSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_solderAutoTypeSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    // lv_obj_set_style_opa(ui_solderAutoTypeSwitch, 255, LV_STATE_DISABLED);
    if (VERSION_INFO_OUT_BOARD_V25 > sysInfoModel.coreConfig.outBoardVersion)
    {
        // V2.0 V2.1 板子不支持
        solderModel.utilConfig.autoTypeSwitch = ENABLE_STATE_CLOSE;
        lv_obj_clear_state(ui_solderAutoTypeSwitch, LV_STATE_CHECKED);
        lv_obj_add_state(ui_solderAutoTypeSwitch, LV_STATE_DISABLED);
    }
    if (ENABLE_STATE_OPEN == solderModel.utilConfig.autoTypeSwitch)
    {
        lv_obj_add_state(ui_solderAutoTypeSwitch, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_clear_state(ui_solderAutoTypeSwitch, LV_STATE_CHECKED);
    }

    // 开始下拉菜单
    ui_solderNameLabel = lv_label_create(father);
    lv_obj_align_to(ui_solderNameLabel, ui_solderAutoTypeSwitchLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    lv_label_set_text(ui_solderNameLabel, SETTING_TEXT_SOLDER_TYPE);
    lv_obj_add_style(ui_solderNameLabel, &label_text_style, 0);

    // static char solderName[] = "T12\nC210\nC245";
    ui_solderNameDropdown = lv_dropdown_create(father);
    lv_dropdown_set_options(ui_solderNameDropdown, solderModel.coreNameList);
    // lv_dropdown_set_options_static(ui_solderNameDropdown, solderName);
    setSolderName(ui_solderNameDropdown, solderModel.curCoreName);
    lv_obj_set_size(ui_solderNameDropdown, 80, LV_SIZE_CONTENT);
    lv_obj_align_to(ui_solderNameDropdown, ui_solderNameLabel,
                    LV_ALIGN_OUT_LEFT_MID, 155, 0);
    lv_obj_add_flag(ui_solderNameDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_text_font(ui_solderNameDropdown, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_solderNameDropdown, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_solderNameDropdown, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_solderNameDropdown, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    if (ENABLE_STATE_OPEN == solderModel.utilConfig.autoTypeSwitch)
    {
        lv_obj_set_style_text_opa(ui_solderNameLabel, 150, LV_PART_MAIN);
        lv_obj_add_flag(ui_solderNameDropdown, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_set_style_text_opa(ui_solderNameLabel, 255, LV_PART_MAIN);
        lv_obj_clear_flag(ui_solderNameDropdown, LV_OBJ_FLAG_HIDDEN);
    }

    // 烙铁芯管理
    lv_obj_t *ui_solderManageLabel = lv_label_create(father);
    lv_obj_align_to(ui_solderManageLabel, ui_solderNameLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    lv_label_set_text(ui_solderManageLabel, SETTING_TEXT_SOLDER_CORE_MANAGE);
    lv_obj_add_style(ui_solderManageLabel, &label_text_style, 0);

    ui_solderManageBnt = lv_btn_create(father);
    // lv_obj_remove_style_all(ui_solderManageBnt);
    lv_obj_set_size(ui_solderManageBnt, 50, 25);
    lv_obj_align_to(ui_solderManageBnt, ui_solderManageLabel,
                    LV_ALIGN_OUT_LEFT_MID, 145, 0);
    lv_obj_add_flag(ui_solderManageBnt, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_solderManageBnt, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_solderManageBnt, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_solderManageBnt, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_solderManageBnt, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_solderManageBnt, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    ui_solderManageText = lv_label_create(ui_solderManageBnt);
    lv_obj_align(ui_solderManageText, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui_solderManageText, SETTING_TEXT_ENTER);
    lv_obj_add_style(ui_solderManageText, &label_text_style, 0);

    // 预设温度修改
    lv_obj_t *solderQuickSetupTempLabel = lv_label_create(father);
    lv_obj_align_to(solderQuickSetupTempLabel, ui_solderManageLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    lv_label_set_text(solderQuickSetupTempLabel, TEXT_TEMP_PRESET_MD);
    lv_obj_add_style(solderQuickSetupTempLabel, &label_text_style, 0);

    solderQuickSetupTemp0 = lv_numberbtn_create(father);
    lv_obj_t *solderQuickSetupTempLabel0 = lv_label_create(solderQuickSetupTemp0);
    lv_numberbtn_set_label_and_format(solderQuickSetupTemp0,
                                      solderQuickSetupTempLabel0, "%d", 1);
    lv_numberbtn_set_range(solderQuickSetupTemp0, MIN_SET_TEMPERATURE, MAX_SET_TEMPERATURE);
    lv_numberbtn_set_value(solderQuickSetupTemp0, solderModel.utilConfig.quickSetupTemp_0);
    lv_obj_set_size(solderQuickSetupTemp0, 50, 20);
    lv_obj_align_to(solderQuickSetupTemp0, solderQuickSetupTempLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    // lv_obj_remove_style_all(solderQuickSetupTemp0);
    lv_obj_add_flag(solderQuickSetupTemp0, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(solderQuickSetupTemp0, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(solderQuickSetupTemp0, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(solderQuickSetupTemp0, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(solderQuickSetupTemp0, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(solderQuickSetupTemp0, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(solderQuickSetupTemp0, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(solderQuickSetupTemp0, &setting_btn_pressed_style, LV_STATE_EDITED);

    solderQuickSetupTemp1 = lv_numberbtn_create(father);
    lv_obj_t *solderQuickSetupTempLabel1 = lv_label_create(solderQuickSetupTemp1);
    lv_numberbtn_set_label_and_format(solderQuickSetupTemp1,
                                      solderQuickSetupTempLabel1, "%d", 1);
    lv_numberbtn_set_range(solderQuickSetupTemp1, MIN_SET_TEMPERATURE, MAX_SET_TEMPERATURE);
    lv_numberbtn_set_value(solderQuickSetupTemp1, solderModel.utilConfig.quickSetupTemp_1);
    lv_obj_set_size(solderQuickSetupTemp1, 50, 20);
    lv_obj_align_to(solderQuickSetupTemp1, solderQuickSetupTemp0,
                    LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    // lv_obj_remove_style_all(solderQuickSetupTemp1);
    lv_obj_add_flag(solderQuickSetupTemp1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(solderQuickSetupTemp1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(solderQuickSetupTemp1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(solderQuickSetupTemp1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(solderQuickSetupTemp1, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(solderQuickSetupTemp1, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(solderQuickSetupTemp1, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(solderQuickSetupTemp1, &setting_btn_pressed_style, LV_STATE_EDITED);

    solderQuickSetupTemp2 = lv_numberbtn_create(father);
    lv_obj_t *solderQuickSetupTempLabel2 = lv_label_create(solderQuickSetupTemp2);
    lv_numberbtn_set_label_and_format(solderQuickSetupTemp2,
                                      solderQuickSetupTempLabel2, "%d", 1);
    lv_numberbtn_set_range(solderQuickSetupTemp2, MIN_SET_TEMPERATURE, MAX_SET_TEMPERATURE);
    lv_numberbtn_set_value(solderQuickSetupTemp2, solderModel.utilConfig.quickSetupTemp_2);
    lv_obj_set_size(solderQuickSetupTemp2, 50, 20);
    lv_obj_align_to(solderQuickSetupTemp2, solderQuickSetupTemp1,
                    LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    // lv_obj_remove_style_all(solderQuickSetupTemp2);
    lv_obj_add_flag(solderQuickSetupTemp2, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(solderQuickSetupTemp2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(solderQuickSetupTemp2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(solderQuickSetupTemp2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(solderQuickSetupTemp2, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(solderQuickSetupTemp2, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(solderQuickSetupTemp2, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(solderQuickSetupTemp2, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 浅度休眠温度
    lv_obj_t *ui_easySleepTempLabel = lv_label_create(father);
    lv_obj_align_to(ui_easySleepTempLabel, solderQuickSetupTempLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 40);
    lv_label_set_text(ui_easySleepTempLabel, SETTING_TEXT_SOLDER_EASY_SLEEP_TEMP);
    lv_obj_add_style(ui_easySleepTempLabel, &label_text_style, 0);

    ui_easySleepTempBtn = lv_numberbtn_create(father);
    lv_obj_t *ui_easySleepTempBtnLabel = lv_label_create(ui_easySleepTempBtn);
    lv_numberbtn_set_label_and_format(ui_easySleepTempBtn,
                                      ui_easySleepTempBtnLabel, "%d", 1);
    lv_numberbtn_set_range(ui_easySleepTempBtn, SOLDER_SLEEP_TEMP_MIN, SOLDER_SLEEP_TEMP_MAX);
    lv_numberbtn_set_value(ui_easySleepTempBtn, solderModel.utilConfig.easySleepTemp);
    lv_obj_set_size(ui_easySleepTempBtn, 50, 20);
    lv_obj_align_to(ui_easySleepTempBtn, ui_easySleepTempLabel,
                    LV_ALIGN_OUT_LEFT_MID, 145, 0);
    // lv_obj_remove_style_all(ui_easySleepTempBtn);
    lv_obj_add_flag(ui_easySleepTempBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_easySleepTempBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_easySleepTempBtn, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_easySleepTempBtn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_easySleepTempBtn, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_easySleepTempBtn, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_easySleepTempBtn, &setting_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_easySleepTempBtn, &setting_btn_pressed_style, LV_STATE_EDITED);

    // 快捷PID
    lv_obj_t *fastPIDLabel = lv_label_create(father);
    lv_obj_align(fastPIDLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_align_to(fastPIDLabel, ui_easySleepTempLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_label_set_text(fastPIDLabel, SETTING_TEXT_FAST_PID);
    lv_obj_add_style(fastPIDLabel, &label_text_style, 0);

    fastPIDSwitch = lv_switch_create(father);
    lv_obj_add_flag(fastPIDSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(fastPIDSwitch, 40, 20);
    lv_obj_align_to(fastPIDSwitch, fastPIDLabel,
                    LV_ALIGN_OUT_LEFT_MID, 140, 0);
    // if (GET_BIT(cfgKey1, CFG_KEY1_SOLDER_GRID))
    if (ENABLE_STATE_OPEN == solderModel.utilConfig.fastPID)
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

    // 短路保护
    shortCircuitLabel = lv_label_create(father);
    lv_obj_align(shortCircuitLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_align_to(shortCircuitLabel, fastPIDLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_label_set_text(shortCircuitLabel, SETTING_TEXT_SOLDER_SHORT_CIRCUIT);
    lv_obj_add_style(shortCircuitLabel, &label_text_style, 0);

    ui_shortCircuitSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_shortCircuitSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_shortCircuitSwitch, 40, 20);
    lv_obj_align_to(ui_shortCircuitSwitch, shortCircuitLabel,
                    LV_ALIGN_OUT_LEFT_MID, 140, 0);
    // if (GET_BIT(cfgKey1, CFG_KEY1_SOLDER_GRID))
    if (ENABLE_STATE_OPEN == solderModel.utilConfig.shortCircuit)
    {
        lv_obj_add_state(ui_shortCircuitSwitch, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_clear_state(ui_shortCircuitSwitch, LV_STATE_CHECKED);
    }
    lv_obj_set_style_outline_color(ui_shortCircuitSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_shortCircuitSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_shortCircuitSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    ui_subBackBtn = lv_btn_create(father);
    lv_obj_remove_style_all(ui_subBackBtn);
    lv_obj_align_to(ui_subBackBtn, shortCircuitLabel,
                    LV_ALIGN_OUT_BOTTOM_LEFT, 0, 40);
    lv_obj_add_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_subBackBtn, &back_btn_style, LV_STATE_DEFAULT);
    lv_obj_add_style(ui_subBackBtn, &back_btn_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_subBackBtnLabel = lv_label_create(ui_subBackBtn);
    lv_obj_center(ui_subBackBtnLabel);
    lv_obj_add_style(ui_subBackBtnLabel, &label_text_style, 0);
    lv_label_set_text(ui_subBackBtnLabel, SETTING_TEXT_BACK);
}

void ui_solder_setting_init_group(lv_obj_t *father)
{
    lv_obj_add_event_cb(ui_solderGridSwitch, ui_solder_grid_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_solderAutoTypeSwitch, ui_solder_auto_type_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_solderNameDropdown, ui_solder_name_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_solderManageBnt, ui_solder_bnt_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(solderQuickSetupTemp0, ui_solderSetBnt_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(solderQuickSetupTemp1, ui_solderSetBnt_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(solderQuickSetupTemp2, ui_solderSetBnt_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_easySleepTempBtn, ui_solderSetBnt_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(fastPIDSwitch, ui_fastPIDSwitch_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_shortCircuitSwitch, ui_fastPIDSwitch_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_subBackBtn, ui_tab_back_btn_pressed, LV_EVENT_PRESSED, 0);

    if (NULL != sub_btn_group)
    {
        lv_group_del(sub_btn_group);
        sub_btn_group = NULL;
    }
    sub_btn_group = lv_group_create();
    msg_btn_group = lv_group_create();
    lv_group_add_obj(sub_btn_group, ui_subBackBtn);
    lv_group_add_obj(sub_btn_group, ui_solderGridSwitch);
#if SH_HARDWARE_VER >= SH_ESP32S2_WROOM_V25
    lv_group_add_obj(sub_btn_group, ui_solderAutoTypeSwitch);
#endif
    if (ENABLE_STATE_CLOSE == solderModel.utilConfig.autoTypeSwitch)
    {
        lv_group_add_obj(sub_btn_group, ui_solderNameDropdown);
    }
    // lv_group_add_obj(sub_btn_group, ui_solderWakeDropdown);
    lv_group_add_obj(sub_btn_group, ui_solderManageBnt);
    lv_group_add_obj(sub_btn_group, solderQuickSetupTemp0);
    lv_group_add_obj(sub_btn_group, solderQuickSetupTemp1);
    lv_group_add_obj(sub_btn_group, solderQuickSetupTemp2);
    lv_group_add_obj(sub_btn_group, ui_easySleepTempBtn);
    lv_group_add_obj(sub_btn_group, fastPIDSwitch);
#if SH_HARDWARE_VER >= SH_ESP32S2_WROOM_V26
    lv_group_add_obj(sub_btn_group, ui_shortCircuitSwitch);
#else
    lv_obj_set_style_text_opa(shortCircuitLabel, 150, LV_PART_MAIN);
    lv_obj_add_flag(ui_shortCircuitSwitch, LV_OBJ_FLAG_HIDDEN);
#endif
    lv_indev_set_group(knobs_indev, sub_btn_group);
}

void ui_solder_setting_release(void *param)
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

    ui_search_arc = NULL;

    if (NULL != coreManage_btn_group)
    {
        lv_group_del(coreManage_btn_group);
        coreManage_btn_group = NULL;
    }
}

#endif