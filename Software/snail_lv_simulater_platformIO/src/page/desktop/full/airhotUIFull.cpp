#include "./ui.h"
#include "../desktop_model.h"

#ifdef FULL_UI

static lv_timer_t *airhotTimer = NULL;
static lv_obj_t *airhotPageUI = NULL;
static lv_obj_t *ui_curTempLabel;
static lv_obj_t *ui_curTempCLabel;
static lv_obj_t *ui_targetTempButton;
static lv_obj_t *ui_setAirDutyButton;

static lv_obj_t *ui_powerBar;

static lv_obj_t *ui_imgFan;
static lv_anim_t fan_anim;

static lv_obj_t *ui_fastSetTempButton0;
static lv_obj_t *ui_fastSetTempButton1;
static lv_obj_t *ui_fastSetTempButton2;

static lv_obj_t *ui_paramKp;
static lv_obj_t *ui_paramKi;
static lv_obj_t *ui_paramKd;

static lv_group_t *btn_group = NULL;

// 图标的历史数据
#define CHART_TEMP_LEN 28
static uint8_t chartTempData[CHART_TEMP_LEN] = {0};
static uint8_t chartTempDataSaveInd; // 循环储存的下标
static uint8_t pre_airFanSpeed;      // 记录上一次的风枪风速

static void ui_set_temp_btn_pressed(lv_event_t *e);
static void ui_set_air_duty_btn_change(lv_event_t *e);
static void airhotPageUI_release();

static void airhotPageUI_focused(lv_event_t *e)
{
    lv_indev_set_group(knobs_indev, btn_group);
    lv_group_focus_obj(ui_backBtn);
}

static void rotate_anim_cb(void *lvobj, int32_t v)
{
    lv_img_set_angle((lv_obj_t *)lvobj, v);
}

static void setFineAdjTemp(int temp)
{
    airhotModel.utilConfig.targetTemp = temp;
    lv_numberbtn_set_value(ui_targetTempButton, airhotModel.utilConfig.targetTemp);
}

static void ui_fast_temp_btn1_pressed(lv_event_t *e)
{
    setFineAdjTemp(airhotModel.utilConfig.quickSetupTemp_0);
}
static void ui_fast_temp_btn2_pressed(lv_event_t *e)
{
    setFineAdjTemp(airhotModel.utilConfig.quickSetupTemp_1);
}
static void ui_fast_temp_btn3_pressed(lv_event_t *e)
{
    setFineAdjTemp(airhotModel.utilConfig.quickSetupTemp_2);
}

static void ui_set_pid_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == ui_paramKp)
        {
            airhotModel.coreConfig.kp = lv_numberbtn_get_value(ui_paramKp);
        }
        else if (target == ui_paramKi)
        {
            airhotModel.coreConfig.ki = lv_numberbtn_get_value(ui_paramKi);
        }
        else if (target == ui_paramKd)
        {
            airhotModel.coreConfig.kd = lv_numberbtn_get_value(ui_paramKd);
        }
    }
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
        if (ENABLE_STATE_OPEN == sysInfoModel.utilConfig.uiParam.uiGlobalParam.airhotGridEnable)
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
    ui_updateAirhotData();
}

static bool airhotPageUI_init(lv_obj_t *father)
{
    airhotPageUI_release();

    top_layer_set_name();
    theme_color_init();

    airhotPageUI = lv_btn_create(father);
    airhotUIObj.mainButtonUI = airhotPageUI;
    lv_obj_t *ui_ButtonTmp = airhotPageUI;

    lv_obj_remove_style_all(ui_ButtonTmp);
    lv_obj_set_size(ui_ButtonTmp, EACH_PAGE_SIZE_X, EACH_PAGE_SIZE_Y);
    lv_obj_center(ui_ButtonTmp);
    lv_obj_add_flag(ui_ButtonTmp, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_ButtonTmp, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_style_bg_color(ui_ButtonTmp, IS_WHITE_THEME ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_ButtonTmp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

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
    lv_label_set_text_fmt(ui_fastSetTempLabel0, "%d", airhotModel.utilConfig.quickSetupTemp_0);

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
    lv_label_set_text_fmt(ui_fastSetTempLabel1, "%d", airhotModel.utilConfig.quickSetupTemp_1);

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
    lv_label_set_text_fmt(ui_fastSetTempLabel2, "%d", airhotModel.utilConfig.quickSetupTemp_2);

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
    lv_obj_set_style_text_color(ui_curTempCLabel, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
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
    lv_numberbtn_set_range(ui_targetTempButton, MIN_SET_TEMPERATURE, MAX_SET_TEMPERATURE);
    lv_numberbtn_set_value(ui_targetTempButton, airhotModel.utilConfig.targetTemp);
    lv_obj_remove_style_all(ui_targetTempButton);
    lv_obj_set_size(ui_targetTempButton, 60, 20);
    lv_obj_align(ui_targetTempButton, LV_ALIGN_TOP_LEFT, 4, 64);
    lv_obj_add_flag(ui_targetTempButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_targetTempButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_targetTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_targetTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_targetTempButton, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_targetTempButton, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_targetTempButton, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_targetTempButton, &btn_type1_pressed_style, LV_STATE_EDITED);

    // 风扇
    ui_imgFan = lv_img_create(ui_ButtonTmp);
    lv_img_set_src(ui_imgFan, &img_fan_speed);
    lv_obj_align(ui_imgFan, LV_ALIGN_CENTER, 103, -76);
    lv_anim_init(&fan_anim);
    lv_anim_set_exec_cb(&fan_anim, rotate_anim_cb);
    lv_anim_set_var(&fan_anim, ui_imgFan);
    lv_anim_set_values(&fan_anim, 0, 3600);
    if (airhotModel.curAirSpeed != 0)
    {
        lv_anim_set_repeat_count(&fan_anim, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_time(&fan_anim, 4960 - (airhotModel.curAirSpeed - 1) * 40);
        lv_anim_start(&fan_anim);
    }
    else
    {
        lv_anim_set_repeat_count(&fan_anim, 1);
        lv_anim_start(&fan_anim);
    }
    pre_airFanSpeed = airhotModel.curAirSpeed;

    ui_setAirDutyButton = lv_numberbtn_create(ui_ButtonTmp);

    lv_obj_t *airDutyLabel = lv_label_create(ui_setAirDutyButton);
    lv_numberbtn_set_label_and_format(ui_setAirDutyButton,
                                      airDutyLabel, "%d%%", 1);
    lv_numberbtn_set_range(ui_setAirDutyButton, 15, 100);
    lv_numberbtn_set_value(ui_setAirDutyButton, airhotModel.utilConfig.workAirSpeed);
    lv_obj_remove_style_all(ui_setAirDutyButton);
    lv_obj_set_size(ui_setAirDutyButton, 64, 30);
    lv_obj_align(ui_setAirDutyButton, LV_ALIGN_CENTER, 102, -35);
    lv_obj_add_flag(ui_setAirDutyButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_setAirDutyButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_setAirDutyButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_setAirDutyButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_setAirDutyButton, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setAirDutyButton, &FontJost_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_setAirDutyButton, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_setAirDutyButton, &btn_type1_pressed_style, LV_STATE_EDITED);

    ui_powerBar = lv_bar_create(ui_ButtonTmp);
    lv_obj_set_size(ui_powerBar, SH_SCREEN_WIDTH - 30, 6);
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
    lv_obj_set_size(chartTemp, SH_SCREEN_WIDTH, CURVE_HIGH);
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

    if (ENABLE_STATE::ENABLE_STATE_OPEN == airhotModel.utilConfig.fastPID)
    {
        ui_paramKp = lv_numberbtn_create(chartTemp);
        lv_obj_t *solderParamKpLabel = lv_label_create(ui_paramKp);
        lv_numberbtn_set_label_and_format(ui_paramKp,
                                          solderParamKpLabel, "P->%d", 1);
        lv_numberbtn_set_range(ui_paramKp, 0, 200);
        lv_numberbtn_set_value(ui_paramKp, airhotModel.coreConfig.kp);
        // lv_obj_remove_style_all(ui_paramKp);
        lv_obj_set_size(ui_paramKp, 50, 20);
        lv_obj_align_to(ui_paramKp, chartTemp,
                        LV_ALIGN_TOP_LEFT, 60, 0);
        lv_obj_add_flag(ui_paramKp, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_clear_flag(ui_paramKp, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(ui_paramKp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui_paramKp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(ui_paramKp, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(ui_paramKp, &FontDeyi_16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_style(ui_paramKp, &btn_type1_focused_style, LV_STATE_FOCUSED);
        lv_obj_add_style(ui_paramKp, &btn_type1_pressed_style, LV_STATE_EDITED);

        ui_paramKi = lv_numberbtn_create(chartTemp);
        lv_obj_t *solderParamKiLabel = lv_label_create(ui_paramKi);
        lv_numberbtn_set_label_and_format(ui_paramKi,
                                          solderParamKiLabel, "I->%.1lf", 0.1);
        lv_numberbtn_set_range(ui_paramKi, 0, 200);
        lv_numberbtn_set_value(ui_paramKi, airhotModel.coreConfig.ki);
        // lv_obj_remove_style_all(ui_paramKi);
        lv_obj_set_size(ui_paramKi, 50, 20);
        lv_obj_align_to(ui_paramKi, ui_paramKp,
                        LV_ALIGN_OUT_RIGHT_MID, 5, 0);
        lv_obj_add_flag(ui_paramKi, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_clear_flag(ui_paramKi, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(ui_paramKi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(ui_paramKi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(ui_paramKi, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(ui_paramKi, &FontDeyi_16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_style(ui_paramKi, &btn_type1_focused_style, LV_STATE_FOCUSED);
        lv_obj_add_style(ui_paramKi, &btn_type1_pressed_style, LV_STATE_EDITED);

        ui_paramKd = lv_numberbtn_create(chartTemp);
        lv_obj_t *solderParamKdLabel = lv_label_create(ui_paramKd);
        lv_numberbtn_set_label_and_format(ui_paramKd,
                                          solderParamKdLabel, "D->%d", 1);
        lv_numberbtn_set_range(ui_paramKd, 0, 200);
        lv_numberbtn_set_value(ui_paramKd, airhotModel.coreConfig.kd);
        // lv_obj_remove_style_all(ui_paramKd);
        lv_obj_set_style_border_width(ui_paramKd, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_size(ui_paramKd, 50, 20);
        lv_obj_align_to(ui_paramKd, ui_paramKi,
                        LV_ALIGN_OUT_RIGHT_MID, 5, 0);
        lv_obj_set_style_text_color(ui_paramKd, ALL_GREY_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(ui_paramKd, &FontDeyi_16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_style(ui_paramKd, &btn_type1_focused_style, LV_STATE_FOCUSED);
        lv_obj_add_style(ui_paramKd, &btn_type1_pressed_style, LV_STATE_EDITED);
    }

    lv_obj_add_event_cb(ui_fastSetTempButton0, ui_fast_temp_btn1_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_fastSetTempButton1, ui_fast_temp_btn2_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_fastSetTempButton2, ui_fast_temp_btn3_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_targetTempButton, ui_set_temp_btn_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_setAirDutyButton, ui_set_air_duty_btn_change, LV_EVENT_VALUE_CHANGED, NULL);
    if (ENABLE_STATE::ENABLE_STATE_OPEN == airhotModel.utilConfig.fastPID)
    {
        lv_obj_add_event_cb(ui_paramKp, ui_set_pid_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_paramKi, ui_set_pid_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_paramKd, ui_set_pid_pressed, LV_EVENT_PRESSED, NULL);
    }

    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_backBtn);
    lv_group_add_obj(btn_group, ui_targetTempButton);
    lv_group_add_obj(btn_group, ui_fastSetTempButton0);
    lv_group_add_obj(btn_group, ui_fastSetTempButton1);
    lv_group_add_obj(btn_group, ui_fastSetTempButton2);
    lv_group_add_obj(btn_group, ui_setAirDutyButton);
    if (ENABLE_STATE::ENABLE_STATE_OPEN == airhotModel.utilConfig.fastPID)
    {
        lv_group_add_obj(btn_group, ui_paramKp);
        lv_group_add_obj(btn_group, ui_paramKi);
        lv_group_add_obj(btn_group, ui_paramKd);
    }
    lv_indev_set_group(knobs_indev, btn_group);
    lv_group_focus_obj(ui_backBtn);

    airhotTimer = lv_timer_create(airhotTimer_timeout, DATA_REFRESH_MS, NULL);
    lv_timer_set_repeat_count(airhotTimer, -1);
    return true;
}

static void airhotPageUI_release()
{
    if (NULL != airhotTimer)
    {
        lv_timer_del(airhotTimer);
        airhotTimer = NULL;
    }

    if (NULL != btn_group)
    {
        lv_group_del(btn_group);
        btn_group = NULL;
    }

    if (NULL != airhotPageUI)
    {
        lv_anim_del(&fan_anim, NULL);
        lv_obj_del(airhotPageUI);
        airhotPageUI = NULL;
        airhotUIObj.mainButtonUI = NULL;
        ui_paramKp = NULL;
        ui_paramKi = NULL;
        ui_paramKd = NULL;
    }
}

void ui_updateAirhotPIDParam(void)
{
    // 便捷pid
    if (ENABLE_STATE::ENABLE_STATE_OPEN == airhotModel.utilConfig.fastPID)
    {
        lv_numberbtn_set_value(ui_paramKp, airhotModel.coreConfig.kp);
        lv_numberbtn_set_value(ui_paramKi, airhotModel.coreConfig.ki);
        lv_numberbtn_set_value(ui_paramKd, airhotModel.coreConfig.kd);
    }
}

void ui_updateAirhotData(void)
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

        if (airhotModel.curAirSpeed != pre_airFanSpeed)
        {
            if (airhotModel.curAirSpeed != 0)
            {
                lv_anim_set_repeat_count(&fan_anim, LV_ANIM_REPEAT_INFINITE);
                lv_anim_set_time(&fan_anim, 4960 - (airhotModel.curAirSpeed - 1) * 40);
                lv_anim_start(&fan_anim);
            }
            else
            {
                lv_anim_set_repeat_count(&fan_anim, 1);
                lv_anim_start(&fan_anim);
            }
            pre_airFanSpeed = airhotModel.curAirSpeed;
        }
    }
}

static void ui_set_temp_btn_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        airhotModel.utilConfig.targetTemp = lv_numberbtn_get_value(ui_targetTempButton);
    }
}

static void ui_set_air_duty_btn_change(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        airhotModel.utilConfig.workAirSpeed = lv_numberbtn_get_value(ui_setAirDutyButton);

        LV_LOG_USER("set_tempArc LV_EVENT_VALUE_CHANGED % u", event_code);
    }
}

FE_UI_OBJ airhotUIObj = {airhotPageUI, airhotPageUI_init,
                         airhotPageUI_release, airhotPageUI_focused};

#endif