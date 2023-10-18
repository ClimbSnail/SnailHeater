#include "./settingUIFull.h"

#ifdef FULL_UI

#include "../ui.h"
#include "../../desktop_model.h"

static lv_obj_t *ui_father = NULL;

static lv_obj_t *ui_solderGridLabel;
static lv_obj_t *ui_solderGridSwitch;
static lv_obj_t *ui_solderAutoTypeSwitchLabel;
static lv_obj_t *ui_solderAutoTypeSwitch;
static lv_obj_t *ui_autoTypeSwitchMsgBox = NULL;
static lv_obj_t *ui_solderNameLabel;
static lv_obj_t *ui_solderNameDropdown;
static lv_obj_t *ui_solderManageLabel;
static lv_obj_t *ui_solderManageBnt;
static lv_obj_t *ui_solderManageText;

static lv_obj_t *ui_core_manage_page;
static lv_obj_t *ui_search_arc = NULL;
static lv_timer_t *ui_searchTimer = NULL; // 读取烙铁芯的定时器
static lv_obj_t *ui_solderNumLabel;
static lv_obj_t *ui_solderNumDropdown;
static lv_obj_t *ui_solderWakeLabel;
static lv_obj_t *ui_solderWakeDropdown;
static lv_obj_t *ui_coreManageExitBtn = NULL; // 退出
static lv_obj_t *ui_coreManageSaveBtn = NULL; // 保存修改
static lv_group_t *coreManage_btn_group = NULL;
static lv_group_t *msgbox_group = NULL;


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
            sysInfoModel.uiGlobalParam.solderGridEnable = ENABLE_STATE_OPEN;
        else
            // CLR_BIT(cfgKey1,CFG_KEY1_SOLDER_GRID);
            sysInfoModel.uiGlobalParam.solderGridEnable = ENABLE_STATE_CLOSE;
    }
}

static void auto_type_msgbox_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_current_target(e);
    LV_LOG_USER("Button %s clicked", lv_msgbox_get_active_btn_text(obj));

    if (strcmp(lv_msgbox_get_active_btn_text(obj), SETTING_TEXT_SOLDER_MSGBOX_ENTER))
    {
        solderModel.utilConfig.autoTypeSwitch = ENABLE_STATE_OPEN;

        lv_obj_set_style_text_opa(ui_solderNameLabel, 150, LV_PART_MAIN);
        lv_obj_clear_flag(ui_solderNameDropdown, LV_OBJ_FLAG_HIDDEN);
        // 使手动类型不可操作
        lv_group_remove_obj(ui_solderNameDropdown);
    }
    else
    {
        // lv_obj_clear_state(ui_solderAutoTypeSwitch, LV_STATE_CHECKED);
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
            // static const char *btns[] = {SETTING_TEXT_SOLDER_MSGBOX_ENTER, ""};
            static const char *btns[] = {""};

            ui_autoTypeSwitchMsgBox = lv_msgbox_create(ui_father, SETTING_TEXT_SOLDER_MSGBOX_INFO_TITLE, SETTING_TEXT_SOLDER_MSGBOX_INFO, btns, true);
            lv_obj_add_style(ui_autoTypeSwitchMsgBox, &label_text_style, 0);
            lv_obj_add_event_cb(ui_autoTypeSwitchMsgBox, auto_type_msgbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
            lv_obj_center(ui_autoTypeSwitchMsgBox);

            // if (NULL != msgbox_group)
            // {
            //     lv_group_del(msgbox_group);
            //     msgbox_group = NULL;
            // }
            // msgbox_group = lv_group_create();

            // lv_obj_add_flag(ui_autoTypeSwitchMsgBox, LV_OBJ_FLAG_SCROLLABLE);
            // lv_obj_t *bnts = lv_msgbox_get_btns(ui_autoTypeSwitchMsgBox);
            // lv_btnmatrix_get_map(bnts);
            // lv_obj_t *close_bnt = lv_msgbox_get_close_btn(ui_autoTypeSwitchMsgBox);
            // lv_group_add_obj(msgbox_group, ui_autoTypeSwitchMsgBox);
            // lv_indev_set_group(knobs_indev, msgbox_group);
            // lv_group_focus_obj(ui_autoTypeSwitchMsgBox);
            // lv_group_set_editing(msgbox_group, true);

            solderModel.utilConfig.autoTypeSwitch = ENABLE_STATE_OPEN;

            lv_obj_set_style_text_opa(ui_solderNameLabel, 150, LV_PART_MAIN);
            lv_obj_add_flag(ui_solderNameDropdown, LV_OBJ_FLAG_HIDDEN);
            // 使手动类型不可操作
            lv_group_remove_obj(ui_solderNameDropdown);

            ui_searchTimer = lv_timer_create(solderCoreTimer_timeout, 5000, NULL);
            lv_timer_set_repeat_count(ui_searchTimer, 1);
        }
        else
        {
            solderModel.utilConfig.autoTypeSwitch = ENABLE_STATE_CLOSE;

            lv_obj_set_style_text_opa(ui_solderNameLabel, 255, LV_PART_MAIN);
            lv_obj_clear_flag(ui_solderNameDropdown, LV_OBJ_FLAG_HIDDEN);
            // 调整按键组
            lv_group_remove_obj(ui_solderManageBnt);
            lv_group_add_obj(sub_btn_group, ui_solderNameDropdown);
            lv_group_add_obj(sub_btn_group, ui_solderManageBnt);
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
    defualt:
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

    if (NULL != ui_autoTypeSwitchMsgBox)
    {
        lv_obj_del(ui_autoTypeSwitchMsgBox);
        ui_autoTypeSwitchMsgBox = NULL;
    }

    if (SOLDER_CORE_MANAGE_ACTION_READ_OK == solderModel.manageCoreAction)
    {
        solderModel.manageCoreAction = SOLDER_CORE_MANAGE_ACTION_IDLE;
        // 读取成功
        setSolderSwitchType(ui_solderWakeDropdown, solderModel.editCoreConfig.wakeSwitchType);
        lv_obj_set_style_text_opa(ui_solderWakeLabel, 255, LV_PART_MAIN);
        lv_obj_set_style_opa(ui_solderWakeDropdown, 255, LV_PART_MAIN);
        // 调整按键组
        lv_group_add_obj(coreManage_btn_group, ui_solderWakeDropdown);
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
        solderModel.manageCoreAction = SOLDER_CORE_MANAGE_ACTION_READ;
        lv_obj_set_style_text_opa(ui_solderWakeLabel, 150, LV_PART_MAIN);
        lv_obj_set_style_opa(ui_solderWakeDropdown, 150, LV_PART_MAIN);
        lv_obj_set_style_text_opa(ui_solderWakeLabel, 150, LV_PART_MAIN);
        lv_obj_set_style_opa(ui_solderWakeDropdown, 150, LV_PART_MAIN);
        // 调整按键组
        lv_group_remove_obj(ui_solderWakeDropdown);
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

        solderModel.manageCoreAction = SOLDER_CORE_MANAGE_ACTION_IDLE;

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

        solderModel.manageCoreAction = SOLDER_CORE_MANAGE_ACTION_WRITE;

        lv_group_focus_obj(ui_coreManageExitBtn);
    }
}

static void ui_solder_wake_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        uint16_t index = lv_dropdown_get_selected(ui_solderWakeDropdown); // 获取索引
        solderModel.editCoreConfig.wakeSwitchType = index;
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
    ui_solderNumLabel = lv_label_create(father);
    lv_obj_align(ui_solderNumLabel, LV_ALIGN_TOP_LEFT, 10, 15);
    lv_label_set_text(ui_solderNumLabel, SETTING_TEXT_SOLDER_NUM);
    lv_obj_add_style(ui_solderNumLabel, &label_text_style, 0);

    // static char solderName[] = "T12\nC210\nC245";
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
    lv_obj_align(main_line, LV_ALIGN_CENTER, 0, -45);

    ui_solderWakeLabel = lv_label_create(father);
    lv_obj_align(ui_solderWakeLabel, LV_ALIGN_TOP_LEFT, 10, 75);
    lv_label_set_text(ui_solderWakeLabel, SETTING_TEXT_SOLDER_WAKEUP);
    lv_obj_add_style(ui_solderWakeLabel, &label_text_style, 0);

    ui_solderWakeDropdown = lv_dropdown_create(father);
    lv_dropdown_set_options(ui_solderWakeDropdown, "None\nHigh\nLow\nChange");
    // 初始已当前选择的烙铁芯为主
    setSolderSwitchType(ui_solderWakeDropdown, solderModel.editCoreConfig.wakeSwitchType);
    lv_obj_set_size(ui_solderWakeDropdown, 80, LV_SIZE_CONTENT);
    lv_obj_align(ui_solderWakeDropdown, LV_ALIGN_TOP_LEFT, 105, 65);
    lv_obj_add_flag(ui_solderWakeDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_text_font(ui_solderWakeDropdown, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_solderWakeDropdown, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_solderWakeDropdown, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_solderWakeDropdown, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    ui_coreManageExitBtn = lv_btn_create(father);
    lv_obj_remove_style_all(ui_coreManageExitBtn);
    lv_obj_align(ui_coreManageExitBtn, LV_ALIGN_BOTTOM_MID, -55, 0);
    lv_obj_add_flag(ui_coreManageExitBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_coreManageExitBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_coreManageExitBtn, &back_btn_style, LV_STATE_DEFAULT);
    lv_obj_add_style(ui_coreManageExitBtn, &back_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_set_size(ui_coreManageExitBtn, 70, LV_SIZE_CONTENT);

    lv_obj_t *ui_manageCoreExitLabel = lv_label_create(ui_coreManageExitBtn);
    lv_obj_center(ui_manageCoreExitLabel);
    lv_obj_add_style(ui_manageCoreExitLabel, &label_text_style, 0);
    lv_label_set_text(ui_manageCoreExitLabel, SETTING_TEXT_SOLDER_CORE_EXIT_MD);

    ui_coreManageSaveBtn = lv_btn_create(father);
    lv_obj_remove_style_all(ui_coreManageSaveBtn);
    lv_obj_align(ui_coreManageSaveBtn, LV_ALIGN_BOTTOM_MID, 55, 0);
    lv_obj_add_flag(ui_coreManageSaveBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_coreManageSaveBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_coreManageSaveBtn, &back_btn_style, LV_STATE_DEFAULT);
    lv_obj_add_style(ui_coreManageSaveBtn, &back_btn_focused_style, LV_STATE_FOCUSED);
    lv_obj_set_size(ui_coreManageSaveBtn, 70, LV_SIZE_CONTENT);

    lv_obj_t *ui_saveCoreLabel = lv_label_create(ui_coreManageSaveBtn);
    lv_obj_center(ui_saveCoreLabel);
    lv_obj_add_style(ui_saveCoreLabel, &label_text_style, 0);
    lv_label_set_text(ui_saveCoreLabel, SETTING_TEXT_SOLDER_CORE_SAVE);

    lv_obj_add_event_cb(ui_solderNumDropdown, ui_solder_num_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_solderWakeDropdown, ui_solder_wake_pressed, LV_EVENT_VALUE_CHANGED, NULL);
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
    lv_group_add_obj(coreManage_btn_group, ui_coreManageSaveBtn);
    lv_indev_set_group(knobs_indev, coreManage_btn_group);
}

static void ui_solder_core_manage_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        lv_obj_clean(ui_father);

        ui_solder_core_manage_init(ui_father);
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
    ui_solderGridLabel = lv_label_create(father);
    lv_obj_align(ui_solderGridLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_label_set_text(ui_solderGridLabel, SETTING_TEXT_SOLDER_GRID);
    lv_obj_add_style(ui_solderGridLabel, &label_text_style, 0);

    ui_solderGridSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_solderGridSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_solderGridSwitch, 40, 20);
    lv_obj_align(ui_solderGridSwitch, LV_ALIGN_TOP_LEFT, 105, 10);
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

    // 烙铁芯自动识别
    ui_solderAutoTypeSwitchLabel = lv_label_create(father);
    lv_obj_align(ui_solderAutoTypeSwitchLabel, LV_ALIGN_TOP_LEFT, 10, 50);
    lv_label_set_text(ui_solderAutoTypeSwitchLabel, SETTING_TEXT_SOLDER_AUTO_TYPE);
    lv_obj_add_style(ui_solderAutoTypeSwitchLabel, &label_text_style, 0);

    ui_solderAutoTypeSwitch = lv_switch_create(father);
    lv_obj_add_flag(ui_solderAutoTypeSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_solderAutoTypeSwitch, 40, 20);
    lv_obj_align(ui_solderAutoTypeSwitch, LV_ALIGN_TOP_LEFT, 105, 50);
    lv_obj_set_style_outline_color(ui_solderAutoTypeSwitch, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_solderAutoTypeSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_solderAutoTypeSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    // lv_obj_set_style_opa(ui_solderAutoTypeSwitch, 255, LV_STATE_DISABLED);
    if (VERSION_INFO_OUT_BOARD_V25 > sysInfoModel.outBoardVersion)
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
    lv_obj_align(ui_solderNameLabel, LV_ALIGN_TOP_LEFT, 10, 90);
    lv_label_set_text(ui_solderNameLabel, SETTING_TEXT_SOLDER_TYPE);
    lv_obj_add_style(ui_solderNameLabel, &label_text_style, 0);

    // static char solderName[] = "T12\nC210\nC245";
    ui_solderNameDropdown = lv_dropdown_create(father);
    lv_dropdown_set_options(ui_solderNameDropdown, solderModel.coreNameList);
    // lv_dropdown_set_options_static(ui_solderNameDropdown, solderName);
    setSolderName(ui_solderNameDropdown, solderModel.curCoreName);
    lv_obj_set_size(ui_solderNameDropdown, 80, LV_SIZE_CONTENT);
    lv_obj_align(ui_solderNameDropdown, LV_ALIGN_TOP_LEFT, 105, 80);
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
    ui_solderManageLabel = lv_label_create(father);
    lv_obj_align(ui_solderManageLabel, LV_ALIGN_TOP_LEFT, 10, 130);
    lv_label_set_text(ui_solderManageLabel, SETTING_TEXT_SOLDER_CORE_MANAGE);
    lv_obj_add_style(ui_solderManageLabel, &label_text_style, 0);

    ui_solderManageBnt = lv_btn_create(father);
    // lv_obj_remove_style_all(ui_solderManageBnt);
    lv_obj_set_size(ui_solderManageBnt, 50, 25);
    lv_obj_align(ui_solderManageBnt, LV_ALIGN_TOP_LEFT, 105, 125);
    lv_obj_add_flag(ui_solderManageBnt, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_solderManageBnt, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_solderManageBnt, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_solderManageBnt, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_solderManageBnt, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_solderManageBnt, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    ui_solderManageText = lv_label_create(ui_solderManageBnt);
    lv_obj_align(ui_solderManageText, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui_solderManageText, SETTING_TEXT_SOLDER_CORE_MANAGE_ENTER);
    lv_obj_add_style(ui_solderManageText, &label_text_style, 0);
    ui_subBackBtn = lv_btn_create(father);
    lv_obj_remove_style_all(ui_subBackBtn);
    lv_obj_align(ui_subBackBtn, LV_ALIGN_TOP_LEFT, 10, 170);
    lv_obj_add_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_subBackBtn, &back_btn_style, LV_STATE_DEFAULT);
    lv_obj_add_style(ui_subBackBtn, &back_btn_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_subBackBtnLabel = lv_label_create(ui_subBackBtn);
    lv_obj_center(ui_subBackBtnLabel);
    lv_obj_add_style(ui_subBackBtnLabel, &label_text_style, 0);
    lv_label_set_text(ui_subBackBtnLabel, "返回");

    // ui_searchTimer = lv_timer_create(solderCoreTimer_timeout, 600, NULL);
    // lv_timer_set_repeat_count(ui_searchTimer, 0);
}

void ui_solder_setting_init_group(lv_obj_t *father)
{
    lv_obj_add_event_cb(ui_solderGridSwitch, ui_solder_grid_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_solderAutoTypeSwitch, ui_solder_auto_type_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_solderNameDropdown, ui_solder_name_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_solderManageBnt, ui_solder_core_manage_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_subBackBtn, ui_tab_back_btn_pressed, LV_EVENT_PRESSED, 0);

    sub_btn_group = lv_group_create();
    lv_group_add_obj(sub_btn_group, ui_subBackBtn);
    lv_group_add_obj(sub_btn_group, ui_solderGridSwitch);
    lv_group_add_obj(sub_btn_group, ui_solderAutoTypeSwitch);
    if (ENABLE_STATE_CLOSE == solderModel.utilConfig.autoTypeSwitch)
    {
        lv_group_add_obj(sub_btn_group, ui_solderNameDropdown);
    }
    // lv_group_add_obj(sub_btn_group, ui_solderWakeDropdown);
    lv_group_add_obj(sub_btn_group, ui_solderManageBnt);
    lv_indev_set_group(knobs_indev, sub_btn_group);
}

void ui_solder_setting_release(void *param)
{
    // if (NULL != ui_searchTimer)
    // {
    //     lv_timer_del(ui_searchTimer);
    //     ui_searchTimer = NULL;
    // }

    if (NULL != ui_father)
    {
        lv_obj_clean(ui_father);
        ui_father = NULL;
    }

    ui_search_arc = NULL;

    if (NULL != coreManage_btn_group)
    {
        lv_group_del(coreManage_btn_group);
        coreManage_btn_group = NULL;
    }
}

#endif