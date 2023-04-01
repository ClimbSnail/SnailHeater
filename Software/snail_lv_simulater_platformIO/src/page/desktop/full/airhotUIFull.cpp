#include "./ui.h"
#include "../desktop_model.h"

#ifdef FULL_UI

static lv_timer_t *airhotTimer = NULL;
static lv_obj_t *airhotPageUI = NULL;
static lv_obj_t *ui_curTempLabel;
static lv_obj_t *ui_curTempCLabel;
static lv_obj_t *ui_fineAdjTempButton;
static lv_obj_t *ui_fineAdjTempLabel;
static lv_obj_t *ui_fineAdjTempArc = NULL;
static lv_obj_t *ui_setAirDutyButton;
static lv_obj_t *ui_setAirDutyLabel;
static lv_obj_t *ui_setAirArc = NULL;

static lv_obj_t *ui_powerBar;

static lv_obj_t *ui_imgFan;
static lv_anim_t fan_anim;

static lv_obj_t *ui_fastSetTempButton1;
static lv_obj_t *ui_fastSetTempButton2;
static lv_obj_t *ui_fastSetTempButton3;
static lv_obj_t *ui_fastSetTempLabel1;
static lv_obj_t *ui_fastSetTempLabel2;
static lv_obj_t *ui_fastSetTempLabel3;

static lv_obj_t *lb1;
static lv_obj_t *lb2;

static lv_group_t *btn_group = NULL;
static lv_group_t *setTempArcGroup = NULL;
static lv_group_t *setAirArcGroup = NULL;

// 图标的历史数据
#define CHART_TEMP_LEN 28
static uint8_t chartTempData[CHART_TEMP_LEN] = {0};
static uint8_t chartTempDataSaveInd; // 循环储存的下标

static void ui_set_temp_btn_pressed(lv_event_t *e);
static void ui_set_air_btn_pressed(lv_event_t *e);

static void airhotPageUI_focused(lv_event_t *e)
{
    lv_group_focus_obj(ui_backBtn);
    lv_indev_set_group(knobs_indev, btn_group);
}

static void rotate_anim_cb(void *lvobj, int32_t v)
{
    lv_img_set_angle((lv_obj_t *)lvobj, v);
}

static void setFineAdjTemp(int temp)
{
    airhotModel.fineAdjTemp = temp;
    airhotModel.targetTemp = airhotModel.fineAdjTemp;
    lv_label_set_text_fmt(ui_fineAdjTempLabel, "%d°C", airhotModel.fineAdjTemp);
}

static void ui_fast_temp_btn1_pressed(lv_event_t *e)
{
    // airhotModel.tempEnable.allValue = 0; // 重设置前必须清空
    // airhotModel.tempEnable.bitValue.quickSetupTempEnable_0 = ENABLE_STATE_OPEN;
    // airhotModel.targetTemp = airhotModel.quickSetupTemp_0;
    setFineAdjTemp(airhotModel.quickSetupTemp_0);
}
static void ui_fast_temp_btn2_pressed(lv_event_t *e)
{
    // airhotModel.tempEnable.allValue = 0; // 重设置前必须清空
    // airhotModel.tempEnable.bitValue.quickSetupTempEnable_1 = ENABLE_STATE_OPEN;
    // airhotModel.targetTemp = airhotModel.quickSetupTemp_1;
    setFineAdjTemp(airhotModel.quickSetupTemp_1);
}
static void ui_fast_temp_btn3_pressed(lv_event_t *e)
{
    // airhotModel.tempEnable.allValue = 0; // 重设置前必须清空
    // airhotModel.tempEnable.bitValue.quickSetupTempEnable_2 = ENABLE_STATE_OPEN;
    // airhotModel.targetTemp = airhotModel.quickSetupTemp_2;
    setFineAdjTemp(airhotModel.quickSetupTemp_2);
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
        // if (GET_BIT(cfgKey1, CFG_KEY1_AIR_HOT_GRID))
        if (ENABLE_STATE_OPEN == sysInfoModel.uiGlobalParam.airhotGridEnable)
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

static void airhotTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    ui_updateAirhotCurTempAndPowerDuty();
}

static bool airhotPageUI_init(lv_obj_t *father)
{
    if (NULL != airhotPageUI)
    {
        airhotPageUI = NULL;
    }
    top_layer_set_name();
    theme_color_init();

    airhotPageUI = lv_btn_create(father);
    airhotUIObj.mainButtonUI = airhotPageUI;

    lv_obj_remove_style_all(airhotPageUI);
    lv_obj_set_size(airhotPageUI, EACH_PAGE_SIZE_X, EACH_PAGE_SIZE_Y);
    lv_obj_center(airhotPageUI);
    lv_obj_add_flag(airhotPageUI, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(airhotPageUI, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_style_bg_color(airhotPageUI, IS_WHITE_THEME ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(airhotPageUI, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_ButtonTmp = airhotPageUI;

    lb1 = lv_label_create(ui_ButtonTmp);
    lv_label_set_text(lb1, TEXT_TEMP_PRESET);
    lv_obj_align(lb1, LV_ALIGN_TOP_LEFT, 10, 12);
    lv_obj_add_style(lb1, &label_text_style, 0);

    ui_fastSetTempButton1 = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_fastSetTempButton1);
    lv_obj_set_size(ui_fastSetTempButton1, 38, 20);
    lv_obj_align(ui_fastSetTempButton1, LV_ALIGN_TOP_LEFT, 74, 10);
    lv_obj_add_flag(ui_fastSetTempButton1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_fastSetTempButton1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_fastSetTempButton1, &btn_type1_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_fastSetTempButton1, &btn_type1_focused_style, LV_STATE_FOCUSED);

    ui_fastSetTempLabel1 = lv_label_create(ui_fastSetTempButton1);
    lv_obj_set_align(ui_fastSetTempLabel1, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_fastSetTempLabel1, "%d", airhotModel.quickSetupTemp_0);

    ui_fastSetTempButton2 = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_fastSetTempButton2);
    lv_obj_set_size(ui_fastSetTempButton2, 38, 20);
    lv_obj_align(ui_fastSetTempButton2, LV_ALIGN_TOP_LEFT, 122, 10);
    lv_obj_add_flag(ui_fastSetTempButton2, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_fastSetTempButton2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_fastSetTempButton2, &btn_type1_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_fastSetTempButton2, &btn_type1_focused_style, LV_STATE_FOCUSED);

    ui_fastSetTempLabel2 = lv_label_create(ui_fastSetTempButton2);
    lv_obj_set_align(ui_fastSetTempLabel2, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_fastSetTempLabel2, "%d", airhotModel.quickSetupTemp_1);

    ui_fastSetTempButton3 = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_fastSetTempButton3);
    lv_obj_set_size(ui_fastSetTempButton3, 38, 20);
    lv_obj_align(ui_fastSetTempButton3, LV_ALIGN_TOP_LEFT, 170, 10);
    lv_obj_add_flag(ui_fastSetTempButton3, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_fastSetTempButton3, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_fastSetTempButton3, &btn_type1_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_fastSetTempButton3, &btn_type1_focused_style, LV_STATE_FOCUSED);

    ui_fastSetTempLabel3 = lv_label_create(ui_fastSetTempButton3);
    lv_obj_set_align(ui_fastSetTempLabel3, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_fastSetTempLabel3, "%d", airhotModel.quickSetupTemp_2);

    ui_curTempLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_curTempLabel, 105, 52);
    lv_obj_align(ui_curTempLabel, LV_ALIGN_TOP_LEFT, 68, 44);
    if (airhotModel.curTemp >= DISCONNCT_TEMP)
    {
        // 未连接
        lv_label_set_text(ui_curTempLabel, NO_CONNECT_MSG);
    }
    else
    {
        lv_label_set_text_fmt(ui_curTempLabel, "%d", airhotModel.curTemp);
    }
    lv_obj_set_style_text_align(ui_curTempLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_color(ui_curTempLabel, AIR_HOT_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_curTempLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_curTempLabel, &FontRoboto_52, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui_curTempLabel, 3, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_curTempCLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_curTempCLabel, LV_ALIGN_TOP_LEFT, 172, 56);
    lv_label_set_text(ui_curTempCLabel, "°C");
    lv_obj_set_style_text_color(ui_curTempCLabel, lv_color_hex(0x999798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_curTempCLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_curTempCLabel, &FontJost_36, LV_PART_MAIN | LV_STATE_DEFAULT);

    lb2 = lv_label_create(ui_ButtonTmp);
    lv_label_set_text(lb2, TEXT_SET_TEMP);
    lv_obj_align(lb2, LV_ALIGN_TOP_LEFT, 10, 46);
    lv_obj_add_style(lb2, &label_text_style, 0);

    // 细调温度
    ui_fineAdjTempButton = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_fineAdjTempButton);
    lv_obj_set_size(ui_fineAdjTempButton, 60, 20);
    lv_obj_align(ui_fineAdjTempButton, LV_ALIGN_TOP_LEFT, 4, 64);
    lv_obj_add_flag(ui_fineAdjTempButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_fineAdjTempButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_fineAdjTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_fineAdjTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_fineAdjTempButton, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_fineAdjTempButton, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_fineAdjTempButton, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_fineAdjTempButton, &btn_type1_pressed_style, LV_STATE_EDITED);

    ui_fineAdjTempLabel = lv_label_create(ui_fineAdjTempButton);
    lv_obj_set_size(ui_fineAdjTempLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(ui_fineAdjTempLabel);
    lv_label_set_text_fmt(ui_fineAdjTempLabel, "%d°C", airhotModel.fineAdjTemp);

    // 风扇
    ui_imgFan = lv_img_create(ui_ButtonTmp);
    lv_img_set_src(ui_imgFan, &img_fan_speed);
    lv_obj_align(ui_imgFan, LV_ALIGN_CENTER, 103, -76);
    lv_anim_init(&fan_anim);
    lv_anim_set_exec_cb(&fan_anim, rotate_anim_cb);
    lv_anim_set_var(&fan_anim, ui_imgFan);
    lv_anim_set_values(&fan_anim, 0, 3600);
    lv_anim_set_repeat_count(&fan_anim, LV_ANIM_REPEAT_INFINITE);

    if (airhotModel.workAirSpeed != 0)
    {
        lv_anim_set_time(&fan_anim, 4960 - (airhotModel.workAirSpeed - 1) * 40);
        lv_anim_start(&fan_anim);
    }

    ui_setAirDutyButton = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_setAirDutyButton);
    lv_obj_set_size(ui_setAirDutyButton, 64, 30);
    lv_obj_align(ui_setAirDutyButton, LV_ALIGN_CENTER, 102, -35);
    lv_obj_add_flag(ui_setAirDutyButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_setAirDutyButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_setAirDutyButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_setAirDutyButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_setAirDutyButton, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setAirDutyButton, &FontJost_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_setAirDutyButton, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_setAirDutyButton, &btn_type1_pressed_style, LV_STATE_EDITED);

    ui_setAirDutyLabel = lv_label_create(ui_setAirDutyButton);
    lv_obj_set_size(ui_setAirDutyLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(ui_setAirDutyLabel);
    lv_label_set_text_fmt(ui_setAirDutyLabel, "%d%%", airhotModel.workAirSpeed);

    ui_powerBar = lv_bar_create(ui_ButtonTmp);
    lv_obj_set_size(ui_powerBar, 250, 6);
    lv_obj_align(ui_powerBar, LV_ALIGN_CENTER, 0, -5);
    lv_bar_set_range(ui_powerBar, 0, 1000);
    lv_bar_set_value(ui_powerBar, 0, LV_ANIM_ON);

    lv_style_set_bg_color(&bar_style_indic, AIR_HOT_THEME_COLOR2);
    lv_style_set_bg_grad_color(&bar_style_indic, AIR_HOT_THEME_COLOR1);
    lv_obj_add_style(ui_powerBar, &bar_style_bg, LV_PART_MAIN);
    lv_obj_add_style(ui_powerBar, &bar_style_indic, LV_PART_INDICATOR);

    // 温度曲线
    chartTemp = lv_chart_create(ui_ButtonTmp);
    lv_obj_remove_style_all(chartTemp);
    lv_obj_set_style_bg_color(chartTemp, IS_WHITE_THEME ? WHITE_THEME_CHART_COLOR1 : BLACK_THEME_CHART_COLOR1, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(chartTemp, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_size(chartTemp, 280, 100);
    lv_obj_align(chartTemp, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_chart_set_type(chartTemp, LV_CHART_TYPE_LINE);
    // lv_obj_set_style_size(chartTemp, 0, LV_PART_INDICATOR);
    lv_chart_set_div_line_count(chartTemp, 5, 12);

    lv_chart_set_point_count(chartTemp, CHART_TEMP_LEN);

    lv_obj_add_event_cb(chartTemp, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
    lv_chart_set_update_mode(chartTemp, LV_CHART_UPDATE_MODE_SHIFT);
    chartSer1 = lv_chart_add_series(chartTemp, AIR_HOT_THEME_COLOR1, LV_CHART_AXIS_PRIMARY_Y);

    // 这里调用后台记录的数组来填充数据
    for (uint32_t i = chartTempDataSaveInd + 1; i != chartTempDataSaveInd;)
    {
        lv_chart_set_next_value(chartTemp, chartSer1, chartTempData[i]);
        i = (i + 1) % CHART_TEMP_LEN;
    }

    lv_obj_add_event_cb(ui_fastSetTempButton1, ui_fast_temp_btn1_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_fastSetTempButton2, ui_fast_temp_btn2_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_fastSetTempButton3, ui_fast_temp_btn3_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_fineAdjTempButton, ui_set_temp_btn_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_setAirDutyButton, ui_set_air_btn_pressed, LV_EVENT_PRESSED, NULL);

    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_backBtn);
    lv_group_add_obj(btn_group, ui_fineAdjTempButton);
    lv_group_add_obj(btn_group, ui_fastSetTempButton1);
    lv_group_add_obj(btn_group, ui_fastSetTempButton2);
    lv_group_add_obj(btn_group, ui_fastSetTempButton3);
    lv_group_add_obj(btn_group, ui_setAirDutyButton);
    lv_group_focus_obj(ui_backBtn);
    lv_indev_set_group(knobs_indev, btn_group);

    setTempArcGroup = lv_group_create();
    setAirArcGroup = lv_group_create();

    airhotTimer = lv_timer_create(airhotTimer_timeout, DATA_REFRESH_MS, NULL);
    lv_timer_set_repeat_count(airhotTimer, -1);
    return true;
}

static void airhotPageUI_release()
{
    if (NULL != airhotPageUI)
    {
        lv_obj_del(airhotPageUI);
        airhotPageUI = NULL;
        airhotUIObj.mainButtonUI = NULL;
    }

    if (NULL != btn_group)
    {
        lv_group_del(btn_group);
        btn_group = NULL;
    }
    if (NULL != setTempArcGroup)
    {
        lv_group_del(setTempArcGroup);
        setTempArcGroup = NULL;
    }
    if (NULL != setAirArcGroup)
    {
        lv_group_del(setAirArcGroup);
        setAirArcGroup = NULL;
    }
    if (NULL != airhotTimer)
    {
        lv_timer_del(airhotTimer);
        airhotTimer = NULL;
    }
}

void ui_updateAirhotCurTempAndPowerDuty(void)
{
    if (NULL == airhotPageUI)
    {
        return;
    }

    if (airhotModel.curTemp >= DISCONNCT_TEMP)
    {
        // 未连接
        lv_label_set_text(ui_curTempLabel, NO_CONNECT_MSG);
    }
    else
    {
        lv_label_set_text_fmt(ui_curTempLabel, "%d", airhotModel.curTemp);
        lv_bar_set_value(ui_powerBar, airhotModel.powerRatio, LV_ANIM_ON);
        chartTempData[chartTempDataSaveInd] = (uint8_t)(airhotModel.curTemp >> 2);
        lv_chart_set_next_value(chartTemp, chartSer1, chartTempData[chartTempDataSaveInd]);
        chartTempDataSaveInd = (chartTempDataSaveInd + 1) % CHART_TEMP_LEN;
    }
}

static void ui_set_tempArc_changed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        lv_label_set_text_fmt(ui_fineAdjTempLabel, "%d°C", lv_arc_get_value(ui_fineAdjTempArc));
    }
}

static void ui_set_tempArc_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        airhotModel.tempEnable.allValue = 0; // 重设置前必须清空
        airhotModel.tempEnable.bitValue.fineAdjTempEnable = ENABLE_STATE_OPEN;
        airhotModel.fineAdjTemp = lv_arc_get_value(ui_fineAdjTempArc);
        airhotModel.targetTemp = airhotModel.fineAdjTemp;

        lv_group_focus_freeze(setTempArcGroup, false);
        lv_indev_set_group(knobs_indev, btn_group);

        LV_LOG_USER("set_tempArc LV_EVENT_PRESSED % u", event_code);
        if (NULL != ui_fineAdjTempArc)
        {
            lv_group_remove_obj(ui_fineAdjTempArc);
            lv_obj_del(ui_fineAdjTempArc);
            ui_fineAdjTempArc = NULL;
        }
        lv_obj_clear_state(ui_fineAdjTempButton, LV_STATE_EDITED);
    }
}

static void ui_set_temp_btn_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        LV_LOG_USER("button_pressed LV_EVENT_PRESSED % u", event_code);

        lv_obj_add_state(ui_fineAdjTempButton, LV_STATE_EDITED);
        if (NULL != ui_fineAdjTempArc)
        {
            lv_group_remove_obj(ui_fineAdjTempArc);
            lv_obj_del(ui_fineAdjTempArc);
            ui_fineAdjTempArc = NULL;
        }
        ui_fineAdjTempArc = lv_arc_create(ui_fineAdjTempButton);
        lv_obj_set_size(ui_fineAdjTempArc, 30, 30);
        lv_obj_center(ui_fineAdjTempArc);
        lv_arc_set_range(ui_fineAdjTempArc, 0, 500);
        lv_arc_set_value(ui_fineAdjTempArc, airhotModel.fineAdjTemp);
        // 设置隐藏
        lv_obj_set_style_opa(ui_fineAdjTempArc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_group_add_obj(setTempArcGroup, ui_fineAdjTempArc);
        lv_indev_set_group(knobs_indev, setTempArcGroup);
        lv_group_focus_obj(ui_fineAdjTempArc);

        lv_obj_add_event_cb(ui_fineAdjTempArc, ui_set_tempArc_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_fineAdjTempArc, ui_set_tempArc_changed, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

static void ui_set_airArc_changed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        lv_label_set_text_fmt(ui_setAirDutyLabel, "%d%%", lv_arc_get_value(ui_setAirArc));
    }
}

static void ui_set_airArc_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        airhotModel.workAirSpeed = lv_arc_get_value(ui_setAirArc);
        if (airhotModel.workAirSpeed != 0)
        {
            lv_anim_set_repeat_count(&fan_anim, LV_ANIM_REPEAT_INFINITE);
            lv_anim_set_time(&fan_anim, 4960 - (airhotModel.workAirSpeed - 1) * 40);
            lv_anim_start(&fan_anim);
        }
        else
        {
            lv_anim_set_repeat_count(&fan_anim, 1);
            lv_anim_start(&fan_anim);
        }

        lv_group_focus_freeze(setAirArcGroup, false);
        lv_indev_set_group(knobs_indev, btn_group);

        LV_LOG_USER("set_tempArc LV_EVENT_PRESSED % u", event_code);

        if (NULL != ui_setAirArc)
        {
            lv_group_remove_obj(ui_setAirArc);
            lv_obj_del(ui_setAirArc);
            ui_setAirArc = NULL;
        }
        lv_obj_clear_state(ui_setAirDutyButton, LV_STATE_EDITED);
    }
}

static void ui_set_air_btn_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        LV_LOG_USER("button_pressed LV_EVENT_PRESSED % u", event_code);
        lv_obj_add_state(ui_setAirDutyButton, LV_STATE_EDITED);
        if (NULL != ui_setAirArc)
        {
            lv_group_remove_obj(ui_setAirArc);
            lv_obj_del(ui_setAirArc);
            ui_setAirArc = NULL;
        }
        ui_setAirArc = lv_arc_create(ui_setAirDutyButton);
        lv_obj_set_size(ui_setAirArc, 30, 30);
        lv_obj_center(ui_setAirArc);
        lv_arc_set_range(ui_setAirArc, 5, 100);
        lv_arc_set_value(ui_setAirArc, airhotModel.workAirSpeed);
        // 设置隐藏
        lv_obj_set_style_opa(ui_setAirArc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_group_add_obj(setAirArcGroup, ui_setAirArc);
        lv_indev_set_group(knobs_indev, setAirArcGroup);
        lv_group_focus_obj(ui_setAirArc);

        lv_obj_add_event_cb(ui_setAirArc, ui_set_airArc_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_setAirArc, ui_set_airArc_changed, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

FE_UI_OBJ airhotUIObj = {airhotPageUI, airhotPageUI_init,
                         airhotPageUI_release, airhotPageUI_focused};

#endif