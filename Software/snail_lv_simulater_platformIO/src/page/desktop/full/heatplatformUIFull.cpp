#include "./ui.h"
#include "../desktop_model.h"

#ifdef FULL_UI

static lv_timer_t *hpTimer = NULL;
static lv_obj_t *hpPageUI = NULL;
static lv_obj_t *ui_curTempLabel;
static lv_obj_t *ui_curTempCLabel;
static lv_obj_t *ui_targetTempButton;
static lv_obj_t *ui_enableSwitch;
static lv_obj_t *ui_powerBar;
static lv_obj_t *ui_setAirDutyButton;
static lv_obj_t *ui_setAirTextLabel;

static lv_obj_t *ui_fastSetTempButton0;
static lv_obj_t *ui_fastSetTempButton1;
static lv_obj_t *ui_fastSetTempButton2;

static lv_obj_t *ui_paramKp;
static lv_obj_t *ui_paramKi;
static lv_obj_t *ui_paramKd;

static lv_obj_t *curvelineDropdown = NULL;
static lv_obj_t *curvelineSwitchBtn;
static lv_obj_t *curvelineSwitchLabel;
static lv_obj_t *startTimeLabel;
static lv_timer_t *ui_searchTimer = NULL; // 读取曲线的定时器
static lv_point_t curveline_points[MAX_STAGE_NUM];
static lv_obj_t *curveline;
static lv_point_t cur_point[2];
static lv_obj_t *curveCurline;

static lv_group_t *btn_group = NULL;

// 图标的历史数据
#define CHART_TEMP_LEN 70
static uint8_t chartTempData[CHART_TEMP_LEN] = {0};
static uint8_t chartTempDataSaveInd; // 循环储存的下标

static void ui_set_temp_btn_pressed(lv_event_t *e);
static void ui_set_air_duty_btn_pressed(lv_event_t *e);
static void ui_enable_switch_pressed(lv_event_t *e);
static void ui_curvelineDropdown_pressed(lv_event_t *e);
static void ui_line_switch_pressed(lv_event_t *e);
static void managerInfoTimer_timeout(lv_timer_t *timer);

static void hpPageUI_focused(lv_event_t *e)
{
    lv_group_focus_obj(ui_backBtn);
    lv_indev_set_group(knobs_indev, btn_group);
}

static void setTargetTemp(int temp)
{
    heatplatformModel.utilConfig.targetTemp = temp;
    lv_numberbtn_set_value(ui_targetTempButton, heatplatformModel.utilConfig.targetTemp);
}

static void ui_fast_temp_btn1_pressed(lv_event_t *e)
{
    setTargetTemp(heatplatformModel.utilConfig.quickSetupTemp_0);
}
static void ui_fast_temp_btn2_pressed(lv_event_t *e)
{
    setTargetTemp(heatplatformModel.utilConfig.quickSetupTemp_1);
}
static void ui_fast_temp_btn3_pressed(lv_event_t *e)
{
    setTargetTemp(heatplatformModel.utilConfig.quickSetupTemp_2);
}

static void draw_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);

    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
    if (dsc->part == LV_PART_ITEMS)
    {

        if (!dsc->p1 || !dsc->p2)
            return;

        lv_draw_mask_line_param_t line_mask_param;
        lv_draw_mask_line_points_init(&line_mask_param, dsc->p1->x, dsc->p1->y, dsc->p2->x, dsc->p2->y, LV_DRAW_MASK_LINE_SIDE_BOTTOM);
        int16_t line_mask_id = lv_draw_mask_add(&line_mask_param, NULL);

        lv_draw_rect_dsc_t draw_rect_dsc;
        lv_draw_rect_dsc_init(&draw_rect_dsc);
        draw_rect_dsc.bg_opa = LV_OPA_70;
        draw_rect_dsc.bg_color = dsc->line_dsc->color;

        lv_area_t a;
        a.x1 = dsc->p1->x;
        a.x2 = dsc->p2->x - 1;
        a.y1 = LV_MIN(dsc->p1->y, dsc->p2->y);
        a.y2 = obj->coords.y2;
        lv_draw_rect(dsc->draw_ctx, &draw_rect_dsc, &a);

        lv_draw_mask_free_param(&line_mask_param);
        lv_draw_mask_remove_id(line_mask_id);
    }
    else if (dsc->part == LV_PART_MAIN)
    {
        // if (GET_BIT(cfgKey1, CFG_KEY1_HEAT_PLAT_GRID))
        if (ENABLE_STATE_OPEN == sysInfoModel.utilConfig.uiParam.uiGlobalParam.heatplatGridEnable)
        {
            if (dsc->line_dsc == NULL || dsc->p1 == NULL || dsc->p2 == NULL)
                return;

            if (dsc->p1->x == dsc->p2->x)
            {
                dsc->line_dsc->color = IS_WHITE_THEME ? WHITE_THEME_CHART_COLOR2 : BLACK_THEME_CHART_COLOR2;

                dsc->line_dsc->width = 1;
                dsc->line_dsc->dash_gap = 4;
                dsc->line_dsc->dash_width = 4;
            }
            else
            {
                dsc->line_dsc->color = IS_WHITE_THEME ? WHITE_THEME_CHART_COLOR2 : BLACK_THEME_CHART_COLOR2;
                if (dsc->id == 0)
                {
                    dsc->line_dsc->color = IS_WHITE_THEME ? WHITE_THEME_CHART_COLOR1 : BLACK_THEME_CHART_COLOR1;
                    dsc->line_dsc->width = 1;
                    dsc->line_dsc->dash_gap = 0;
                    dsc->line_dsc->dash_width = 0;
                }
                else if (dsc->id == 2)
                {
                    dsc->line_dsc->width = 1;
                    dsc->line_dsc->dash_gap = 0;
                    dsc->line_dsc->dash_width = 0;
                }
                else
                {
                    dsc->line_dsc->width = 1;
                    dsc->line_dsc->dash_gap = 4;
                    dsc->line_dsc->dash_width = 4;
                }
            }
        }
    }
}

static void hpTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    ui_updateHeatplatformData();
}

static void ui_set_pid_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == ui_paramKp)
        {
            heatplatformModel.coreConfig.kp = lv_numberbtn_get_value(ui_paramKp);
        }
        else if (target == ui_paramKi)
        {
            heatplatformModel.coreConfig.ki = lv_numberbtn_get_value(ui_paramKi);
        }
        else if (target == ui_paramKd)
        {
            heatplatformModel.coreConfig.kd = lv_numberbtn_get_value(ui_paramKd);
        }
    }
}

static void setCurveline(lv_obj_t *obj, const char *text)
{
    char name[16];
    int maxSize = lv_dropdown_get_option_cnt(obj);
    int cnt;
    for (cnt = 0; cnt < maxSize; cnt++)
    {
        lv_dropdown_set_selected(obj, cnt);
        lv_dropdown_get_selected_str(obj, name, 16);
        int ret = strcmp(text, name);
        if (ret == 0)
        {
            break;
        }
    }
}

static bool hpPageUI_init(lv_obj_t *father)
{
    if (NULL != hpPageUI)
    {
        lv_obj_del(hpPageUI);
        hpPageUI = NULL;
    }
    top_layer_set_name();
    theme_color_init();

    hpPageUI = lv_btn_create(father);
    hpUIObj.mainButtonUI = hpPageUI;

    lv_obj_remove_style_all(hpPageUI);
    lv_obj_set_size(hpPageUI, EACH_PAGE_SIZE_X, EACH_PAGE_SIZE_Y);
    lv_obj_center(hpPageUI);
    lv_obj_add_flag(hpPageUI, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(hpPageUI, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_style_bg_color(hpPageUI, IS_WHITE_THEME ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(hpPageUI, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_ButtonTmp = hpPageUI;

    lv_obj_t *lb1 = lv_label_create(ui_ButtonTmp);
    lv_label_set_text(lb1, TEXT_TEMP_PRESET);
    lv_obj_align(lb1, LV_ALIGN_TOP_LEFT, 10, 12);
    lv_obj_add_style(lb1, &label_text_style, 0);

    ui_fastSetTempButton0 = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_fastSetTempButton0);
    lv_obj_set_size(ui_fastSetTempButton0, 38, 20);
    lv_obj_align(ui_fastSetTempButton0, LV_ALIGN_TOP_LEFT, 74, 10);
    lv_obj_add_flag(ui_fastSetTempButton0, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_fastSetTempButton0, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_fastSetTempButton0, &btn_type1_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_fastSetTempButton0, &btn_type1_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_fastSetTempLabel0 = lv_label_create(ui_fastSetTempButton0);
    lv_obj_set_align(ui_fastSetTempLabel0, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_fastSetTempLabel0, "%d", heatplatformModel.utilConfig.quickSetupTemp_0);

    ui_fastSetTempButton1 = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_fastSetTempButton1);
    lv_obj_set_size(ui_fastSetTempButton1, 38, 20);
    lv_obj_align(ui_fastSetTempButton1, LV_ALIGN_TOP_LEFT, 122, 10);
    lv_obj_add_flag(ui_fastSetTempButton1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_fastSetTempButton1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_fastSetTempButton1, &btn_type1_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_fastSetTempButton1, &btn_type1_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_fastSetTempLabel1 = lv_label_create(ui_fastSetTempButton1);
    lv_obj_set_align(ui_fastSetTempLabel1, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_fastSetTempLabel1, "%d", heatplatformModel.utilConfig.quickSetupTemp_1);

    ui_fastSetTempButton2 = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_fastSetTempButton2);
    lv_obj_set_size(ui_fastSetTempButton2, 38, 20);
    lv_obj_align(ui_fastSetTempButton2, LV_ALIGN_TOP_LEFT, 170, 10);
    lv_obj_add_flag(ui_fastSetTempButton2, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_fastSetTempButton2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_fastSetTempButton2, &btn_type1_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_fastSetTempButton2, &btn_type1_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_fastSetTempLabel2 = lv_label_create(ui_fastSetTempButton2);
    lv_obj_set_align(ui_fastSetTempLabel2, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_fastSetTempLabel2, "%d", heatplatformModel.utilConfig.quickSetupTemp_2);

    ui_curTempLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_curTempLabel, 105, 52);
    lv_obj_align(ui_curTempLabel, LV_ALIGN_TOP_LEFT, 68, 44);
    if (heatplatformModel.curTemp >= DISCONNCT_TEMP)
    {
        // 未连接
        lv_label_set_text(ui_curTempLabel, NO_CONNECT_MSG);
    }
    else
    {
        lv_label_set_text_fmt(ui_curTempLabel, "%d", heatplatformModel.curTemp);
    }
    lv_obj_set_style_text_align(ui_curTempLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_color(ui_curTempLabel, HEAT_PLAT_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_curTempLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_curTempLabel, &FontRoboto_52, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui_curTempLabel, 3, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_curTempCLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_curTempCLabel, LV_ALIGN_TOP_LEFT, 172, 56);
    lv_label_set_text(ui_curTempCLabel, "°C");
    lv_obj_set_style_text_color(ui_curTempCLabel, lv_color_hex(0x999798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_curTempCLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_curTempCLabel, &FontJost_36, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *lb2 = lv_label_create(ui_ButtonTmp);
    lv_label_set_text(lb2, TEXT_SET_TEMP);
    lv_obj_align(lb2, LV_ALIGN_TOP_LEFT, 10, 46);
    lv_obj_add_style(lb2, &label_text_style, 0);

    // 目标温度
    ui_targetTempButton = lv_numberbtn_create(ui_ButtonTmp);

    lv_obj_t *targetTempLabel = lv_label_create(ui_targetTempButton);
    lv_numberbtn_set_label_and_format(ui_targetTempButton,
                                      targetTempLabel, "%d°C", 1);
    lv_numberbtn_set_range(ui_targetTempButton, 0, 500);
    lv_numberbtn_set_value(ui_targetTempButton, heatplatformModel.utilConfig.targetTemp);
    lv_obj_remove_style_all(ui_targetTempButton);
    lv_obj_set_size(ui_targetTempButton, 60, 20);
    lv_obj_align(ui_targetTempButton, LV_ALIGN_TOP_LEFT, 4, 64);
    lv_obj_add_flag(ui_targetTempButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_targetTempButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_targetTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_targetTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_targetTempButton, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_targetTempButton, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_targetTempButton, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_targetTempButton, &btn_type1_pressed_style, LV_STATE_EDITED);

    // 使能开关
    ui_enableSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_add_flag(ui_enableSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_enableSwitch, 40, 20);
    lv_obj_align(ui_enableSwitch, LV_ALIGN_CENTER, 104, -86);
    if (heatplatformModel.enable == ENABLE_STATE_OPEN)
    {
        lv_obj_add_state(ui_enableSwitch, LV_STATE_CHECKED); // 开
    }
    else
    {
        // if (GET_BIT(cfgKey1, CFG_KEY1_HP_AUTO_HEAT))
        if (ENABLE_STATE_OPEN == sysInfoModel.utilConfig.uiParam.uiGlobalParam.hpAutoHeatEnable)
        {
            heatplatformModel.enable = ENABLE_STATE_OPEN;
            lv_obj_add_state(ui_enableSwitch, LV_STATE_CHECKED);
        }
        else
        {
            lv_obj_clear_state(ui_enableSwitch, LV_STATE_CHECKED);
        }
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_enableSwitch, HEAT_PLAT_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_enableSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_enableSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 风速字样
    ui_setAirTextLabel = lv_label_create(ui_ButtonTmp);
    lv_label_set_text(ui_setAirTextLabel, TEXT_AIR_SPEED);
    lv_obj_align(ui_setAirTextLabel, LV_ALIGN_CENTER, 106, -57);
    lv_obj_add_style(ui_setAirTextLabel, &label_text_style, 0);

    // 风速调节按钮
    ui_setAirDutyButton = lv_numberbtn_create(ui_ButtonTmp);

    lv_obj_t *setAirDutyLabel = lv_label_create(ui_setAirDutyButton);
    lv_numberbtn_set_label_and_format(ui_setAirDutyButton,
                                      setAirDutyLabel, "%d%%", 1);
    lv_numberbtn_set_range(ui_setAirDutyButton, 5, 100);
    lv_numberbtn_set_value(ui_setAirDutyButton, heatplatformModel.utilConfig.workAirSpeed);
    lv_obj_remove_style_all(ui_setAirDutyButton);
    lv_obj_set_size(ui_setAirDutyButton, 64, 30);
    // lv_obj_align(ui_setAirDutyButton, LV_ALIGN_CENTER, 102, -65);
    lv_obj_align(ui_setAirDutyButton, LV_ALIGN_CENTER, 102, -36);
    lv_obj_add_flag(ui_setAirDutyButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_setAirDutyButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_setAirDutyButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_setAirDutyButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_setAirDutyButton, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setAirDutyButton, &FontJost_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_setAirDutyButton, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_setAirDutyButton, &btn_type1_pressed_style, LV_STATE_EDITED);

    ui_powerBar = lv_bar_create(ui_ButtonTmp);
    lv_obj_set_size(ui_powerBar, SH_SCREEN_WIDTH - 30, 6);
    lv_obj_align(ui_powerBar, LV_ALIGN_CENTER, 0, -5);
    lv_bar_set_range(ui_powerBar, 0, 1000);
    lv_bar_set_value(ui_powerBar, 0, LV_ANIM_ON);

    lv_style_set_bg_color(&bar_style_indic, HEAT_PLAT_THEME_COLOR2);
    lv_style_set_bg_grad_color(&bar_style_indic, HEAT_PLAT_THEME_COLOR1);
    lv_obj_add_style(ui_powerBar, &bar_style_bg, LV_PART_MAIN);
    lv_obj_add_style(ui_powerBar, &bar_style_indic, LV_PART_INDICATOR);

    chartTemp = lv_chart_create(ui_ButtonTmp);
    lv_obj_remove_style_all(chartTemp);
    lv_obj_set_style_bg_color(chartTemp, IS_WHITE_THEME ? WHITE_THEME_CHART_COLOR1 : BLACK_THEME_CHART_COLOR1, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(chartTemp, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_size(chartTemp, SH_SCREEN_WIDTH, CURVE_HIGH);
    lv_obj_align(chartTemp, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_chart_set_type(chartTemp, LV_CHART_TYPE_LINE);
    // lv_obj_set_style_size(chartTemp, 0, LV_PART_INDICATOR);
    lv_chart_set_div_line_count(chartTemp, 5, 12);
    lv_chart_set_point_count(chartTemp, CHART_TEMP_LEN);

    lv_obj_add_event_cb(chartTemp, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
    lv_chart_set_update_mode(chartTemp, LV_CHART_UPDATE_MODE_SHIFT);

    // static char curvelineName[] = "恒温\n折线";
    // curvelineDropdown = lv_dropdown_create(chartTemp);
    // // lv_dropdown_set_options(curvelineDropdown, curvelineName);
    // lv_dropdown_set_options_static(curvelineDropdown, curvelineName);
    // lv_dropdown_set_selected(curvelineDropdown, heatplatformModel.curveConfig.id);
    // // setCurveline(curvelineDropdown, heatplatformModel.curveConfig.id);
    // lv_obj_set_size(curvelineDropdown, 60, LV_SIZE_CONTENT);
    // lv_obj_align(curvelineDropdown, LV_ALIGN_TOP_LEFT, 5, 5);
    // lv_obj_add_flag(curvelineDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    // lv_obj_set_style_text_font(curvelineDropdown, &FontDeyi_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_outline_color(curvelineDropdown, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    // lv_obj_set_style_outline_opa(curvelineDropdown, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    // lv_obj_set_style_outline_pad(curvelineDropdown, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    // lv_obj_add_style(curvelineDropdown, &label_text_style, 0);

    // 曲线切换按钮
    curvelineSwitchBtn = lv_btn_create(chartTemp);
    lv_obj_remove_style_all(curvelineSwitchBtn);
    lv_obj_set_size(curvelineSwitchBtn, 38, 20);
    lv_obj_align(curvelineSwitchBtn, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_add_flag(curvelineSwitchBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(curvelineSwitchBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(curvelineSwitchBtn, &btn_type1_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(curvelineSwitchBtn, &btn_type1_focused_style, LV_STATE_FOCUSED);

    curvelineSwitchLabel = lv_label_create(curvelineSwitchBtn);
    lv_label_set_text(curvelineSwitchLabel, TEXT_SWITVHCH_LINE);
    lv_obj_align(curvelineSwitchLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(curvelineSwitchLabel, &label_text_style, 0);

    startTimeLabel = lv_label_create(chartTemp);
    lv_label_set_text_fmt(startTimeLabel, "%03d:%02d",
                          heatplatformModel.curRunTime / 60000,
                          heatplatformModel.curRunTime % 60000 / 1000);
    lv_obj_align(startTimeLabel, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_obj_add_style(startTimeLabel, &label_text_style, 0);

    chartSer1 = lv_chart_add_series(chartTemp, HEAT_PLAT_THEME_COLOR1, LV_CHART_AXIS_PRIMARY_Y);
    // 这里调用后台记录的数组来填充数据
    for (uint32_t i = chartTempDataSaveInd + 1; i != chartTempDataSaveInd;)
    {
        lv_chart_set_next_value(chartTemp, chartSer1, chartTempData[i]);
        i = (i + 1) % CHART_TEMP_LEN;
    }

    // 曲线数据
    double div_x = (SH_SCREEN_WIDTH - 20) / 1.0 / heatplatformModel.curveAllTime;
    for (uint32_t i = 0; i < heatplatformModel.curveConfig.stageNum; ++i)
    {
        curveline_points[i].x = div_x * heatplatformModel.curveConfig.time[i] + 10;
        curveline_points[i].y = CURVE_HIGH - (heatplatformModel.curveConfig.temperatur[i] >> 2);
    }

    curveline = lv_line_create(chartTemp);
    lv_line_set_points(curveline, curveline_points,
                       heatplatformModel.curveConfig.stageNum); /*Set the points*/
    lv_obj_set_style_line_color(curveline, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_line_width(curveline, 3, 0);
    lv_obj_set_style_line_rounded(curveline, true, 0);

    cur_point[0].x = curveline_points[1].x;
    cur_point[0].y = curveline_points[1].y;
    cur_point[1].x = curveline_points[1].x;
    cur_point[1].y = curveline_points[1].y + 1;
    curveCurline = lv_line_create(chartTemp);
    lv_line_set_points(curveCurline, cur_point, 2); /*Set the points*/
    lv_obj_set_style_line_color(curveCurline, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_line_width(curveCurline, 8, 0);
    lv_obj_set_style_line_rounded(curveCurline, true, 0);

    if (ENABLE_STATE::ENABLE_STATE_OPEN == heatplatformModel.utilConfig.fastPID)
    {
        ui_paramKp = lv_numberbtn_create(chartTemp);
        lv_obj_t *solderParamKpLabel = lv_label_create(ui_paramKp);
        lv_numberbtn_set_label_and_format(ui_paramKp,
                                          solderParamKpLabel, "P->%d", 1);
        lv_numberbtn_set_range(ui_paramKp, 0, 200);
        lv_numberbtn_set_value(ui_paramKp, heatplatformModel.coreConfig.kp);
        // lv_obj_remove_style_all(ui_paramKp);
        lv_obj_set_size(ui_paramKp, 50, 20);
        lv_obj_align_to(ui_paramKp, chartTemp,
                        LV_ALIGN_TOP_LEFT, 60, 0);
        lv_obj_add_flag(ui_paramKp, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_clear_flag(ui_paramKp, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(ui_paramKp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui_paramKp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(ui_paramKp, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(ui_paramKp, &FontDeyi_16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_style(ui_paramKp, &btn_type1_focused_style, LV_STATE_FOCUSED);
        lv_obj_add_style(ui_paramKp, &btn_type1_pressed_style, LV_STATE_EDITED);

        ui_paramKi = lv_numberbtn_create(chartTemp);
        lv_obj_t *solderParamKiLabel = lv_label_create(ui_paramKi);
        lv_numberbtn_set_label_and_format(ui_paramKi,
                                          solderParamKiLabel, "I->%.1lf", 0.1);
        lv_numberbtn_set_range(ui_paramKi, 0, 200);
        lv_numberbtn_set_value(ui_paramKi, heatplatformModel.coreConfig.ki);
        // lv_obj_remove_style_all(ui_paramKi);
        lv_obj_set_size(ui_paramKi, 50, 20);
        lv_obj_align_to(ui_paramKi, ui_paramKp,
                        LV_ALIGN_OUT_RIGHT_MID, 5, 0);
        lv_obj_add_flag(ui_paramKi, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_clear_flag(ui_paramKi, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(ui_paramKi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui_paramKi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(ui_paramKi, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(ui_paramKi, &FontDeyi_16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_style(ui_paramKi, &btn_type1_focused_style, LV_STATE_FOCUSED);
        lv_obj_add_style(ui_paramKi, &btn_type1_pressed_style, LV_STATE_EDITED);

        ui_paramKd = lv_numberbtn_create(chartTemp);
        lv_obj_t *solderParamKdLabel = lv_label_create(ui_paramKd);
        lv_numberbtn_set_label_and_format(ui_paramKd,
                                          solderParamKdLabel, "D->%d", 1);
        lv_numberbtn_set_range(ui_paramKd, 0, 200);
        lv_numberbtn_set_value(ui_paramKd, heatplatformModel.coreConfig.kd);
        // lv_obj_remove_style_all(ui_paramKd);
        lv_obj_set_style_border_width(ui_paramKd, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_size(ui_paramKd, 50, 20);
        lv_obj_align_to(ui_paramKd, ui_paramKi,
                        LV_ALIGN_OUT_RIGHT_MID, 5, 0);
        lv_obj_set_style_text_color(ui_paramKd, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(ui_paramKd, &FontDeyi_16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_style(ui_paramKd, &btn_type1_focused_style, LV_STATE_FOCUSED);
        lv_obj_add_style(ui_paramKd, &btn_type1_pressed_style, LV_STATE_EDITED);
    }

    lv_obj_add_event_cb(ui_fastSetTempButton0, ui_fast_temp_btn1_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_fastSetTempButton1, ui_fast_temp_btn2_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_fastSetTempButton2, ui_fast_temp_btn3_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_targetTempButton, ui_set_temp_btn_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_setAirDutyButton, ui_set_air_duty_btn_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_enableSwitch, ui_enable_switch_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    // lv_obj_add_event_cb(curvelineDropdown, ui_curvelineDropdown_pressed, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(curvelineSwitchBtn, ui_line_switch_pressed, LV_EVENT_PRESSED, NULL);
    if (ENABLE_STATE::ENABLE_STATE_OPEN == heatplatformModel.utilConfig.fastPID)
    {
        lv_obj_add_event_cb(ui_paramKp, ui_set_pid_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_paramKi, ui_set_pid_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_paramKd, ui_set_pid_pressed, LV_EVENT_PRESSED, NULL);
    }

    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_backBtn);
    lv_group_add_obj(btn_group, ui_enableSwitch);
    lv_group_add_obj(btn_group, ui_targetTempButton);
    lv_group_add_obj(btn_group, ui_fastSetTempButton0);
    lv_group_add_obj(btn_group, ui_fastSetTempButton1);
    lv_group_add_obj(btn_group, ui_fastSetTempButton2);
    // lv_group_add_obj(btn_group, curvelineDropdown);
    lv_group_add_obj(btn_group, curvelineSwitchBtn);
    lv_group_add_obj(btn_group, ui_setAirDutyButton);
    if (heatplatformModel.enable == ENABLE_STATE_OPEN)
    {
        lv_group_focus_obj(ui_enableSwitch);
    }
    else
    {
        lv_group_focus_obj(ui_backBtn);
    }

    if (ENABLE_STATE::ENABLE_STATE_OPEN == heatplatformModel.utilConfig.fastPID)
    {
        lv_group_add_obj(btn_group, ui_paramKp);
        lv_group_add_obj(btn_group, ui_paramKi);
        lv_group_add_obj(btn_group, ui_paramKd);
    }
    lv_indev_set_group(knobs_indev, btn_group);

    hpTimer = lv_timer_create(hpTimer_timeout, DATA_REFRESH_MS, NULL);
    lv_timer_set_repeat_count(hpTimer, -1);
    return true;
}

static void hpPageUI_release()
{
    if (NULL != hpTimer)
    {
        lv_timer_del(hpTimer);
        hpTimer = NULL;
    }

    if (NULL != btn_group)
    {
        lv_group_del(btn_group);
        btn_group = NULL;
    }

    if (NULL != hpPageUI)
    {
        lv_obj_del(hpPageUI);
        hpPageUI = NULL;
        hpUIObj.mainButtonUI = NULL;
        ui_paramKp = NULL;
        ui_paramKi = NULL;
        ui_paramKd = NULL;
    }
}

static void ui_line_switch_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_PRESSED)
    {
        LV_LOG_USER("ui_line_switch_pressed heatplatformModel.curveConfig.id = % u",
                    heatplatformModel.curveConfig.id);
        // 正在编辑
        heatplatformModel.curveConfig.id = (heatplatformModel.curveConfig.id + 1) % 2;
        // 动作修改成读取
        heatplatformModel.manageCurveAction = INFO_MANAGE_ACTION_CURVE_READ;

        ui_searchTimer = lv_timer_create(managerInfoTimer_timeout, 600, NULL);
        lv_timer_set_repeat_count(ui_searchTimer, 1);
    }
}

static void ui_enable_switch_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        bool isEnable = lv_obj_has_state(target, LV_STATE_CHECKED); // 返回 bool 类型， 开-true ； 关-false
        heatplatformModel.enable = isEnable ? ENABLE_STATE_OPEN : ENABLE_STATE_CLOSE;
    }
}

static void managerInfoTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    if (INFO_MANAGE_ACTION_CURVE_READ_OK == heatplatformModel.manageCurveAction)
    {
        heatplatformModel.manageCurveAction = INFO_MANAGE_ACTION_CURVE_IDLE;
        // 读取成功
        // lv_dropdown_set_selected(curvelineDropdown, heatplatformModel.curveConfig.id);

        // 曲线数据
        double div_x = (SH_SCREEN_WIDTH - 20) / 1.0 / heatplatformModel.curveAllTime;
        for (uint32_t i = 0; i < heatplatformModel.curveConfig.stageNum; ++i)
        {
            curveline_points[i].x = div_x * heatplatformModel.curveConfig.time[i] + 10;
            curveline_points[i].y = CURVE_HIGH - (heatplatformModel.curveConfig.temperatur[i] >> 2);
        }

        lv_line_set_points(curveline, curveline_points,
                           heatplatformModel.curveConfig.stageNum); /*Set the points*/
    }
}

static void ui_curvelineDropdown_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        // 正在编辑
        heatplatformModel.curveConfig.id = lv_dropdown_get_selected(curvelineDropdown);
        // 动作修改成读取
        heatplatformModel.manageCurveAction = INFO_MANAGE_ACTION_CURVE_READ;

        ui_searchTimer = lv_timer_create(managerInfoTimer_timeout, 600, NULL);
        lv_timer_set_repeat_count(ui_searchTimer, 1);
    }
}

void ui_updateHeatplatformPIDParam(void)
{
    // 便捷pid
    if (ENABLE_STATE::ENABLE_STATE_OPEN == heatplatformModel.utilConfig.fastPID)
    {
        lv_numberbtn_set_value(ui_paramKp, heatplatformModel.coreConfig.kp);
        lv_numberbtn_set_value(ui_paramKi, heatplatformModel.coreConfig.ki);
        lv_numberbtn_set_value(ui_paramKd, heatplatformModel.coreConfig.kd);
    }
}

void ui_updateHeatplatformData(void)
{
    if (NULL == hpPageUI)
    {
        return;
    }

    if (heatplatformModel.curTemp >= DISCONNCT_TEMP)
    {
        // 未连接
        lv_label_set_text(ui_curTempLabel, NO_CONNECT_MSG);
    }
    else
    {
        lv_label_set_text_fmt(ui_curTempLabel, "%d", heatplatformModel.curTemp);
        lv_bar_set_value(ui_powerBar, heatplatformModel.powerRatio, LV_ANIM_ON);
        chartTempData[chartTempDataSaveInd] = heatplatformModel.curTemp >> 2;
        lv_chart_set_next_value(chartTemp, chartSer1, chartTempData[chartTempDataSaveInd]);
        chartTempDataSaveInd = (chartTempDataSaveInd + 1) % CHART_TEMP_LEN;

        // 单位时间的x的div
        double div_x = (SH_SCREEN_WIDTH - 20) / 1.0 / heatplatformModel.curveAllTime;
        // heatplatformModel.curveRunTime = (heatplatformModel.curveRunTime + 1) %
        //                                    heatplatformModel.curveAllTime;
        cur_point[0].x = heatplatformModel.curveRunTime * div_x;
        cur_point[0].y = CURVE_HIGH - (heatplatformModel.curTemp >> 2);
        cur_point[1].x = cur_point[0].x;
        cur_point[1].y = cur_point[0].y + 1;
        lv_line_set_points(curveCurline, cur_point, 2); /*Set the points*/

        lv_label_set_text_fmt(startTimeLabel, "%03d:%02d",
                              heatplatformModel.curRunTime / 60000,
                              heatplatformModel.curRunTime % 60000 / 1000);
    }
}

static void ui_set_air_duty_btn_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        LV_LOG_USER("button_pressed LV_EVENT_PRESSED % u", event_code);

        heatplatformModel.utilConfig.workAirSpeed = lv_numberbtn_get_value(ui_setAirDutyButton);
    }
}

static void ui_set_temp_btn_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        LV_LOG_USER("button_pressed LV_EVENT_PRESSED % u", event_code);

        heatplatformModel.utilConfig.targetTemp = lv_numberbtn_get_value(ui_targetTempButton);
    }
}

FE_UI_OBJ hpUIObj = {hpPageUI, hpPageUI_init,
                     hpPageUI_release, hpPageUI_focused};

#endif