#include "ui.h"
#include "desktop_model.h"

#ifdef NEW_UI

static lv_timer_t *airhotTimer;
static lv_obj_t *airhotPageUI = NULL;
static lv_obj_t *ui_curTempLabel;
static lv_obj_t *ui_curTempCLabel;
static lv_obj_t *ui_setTempButton;
static lv_obj_t *ui_setTempLabel;
static lv_obj_t *ui_setTempArc;
static lv_obj_t *ui_setAirDutyButton;
static lv_obj_t *ui_setAirDutyLabel;
static lv_obj_t *ui_setAirArc;

static lv_obj_t *ui_backButton;
static lv_obj_t *ui_backButtonLabel;
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

static lv_group_t *btn_group;
static lv_group_t *setTempArcGroup;
static lv_group_t *setAirArcGroup;

static void ui_set_temp_btn_pressed(lv_event_t *e);
static void ui_set_air_btn_pressed(lv_event_t *e);
static void ui_back_btn_pressed(lv_event_t *e);

static void airhotPageUI_focused()
{
    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_backButton);
    lv_group_add_obj(btn_group, ui_setTempButton);
    lv_group_add_obj(btn_group, ui_fastSetTempButton1);
    lv_group_add_obj(btn_group, ui_fastSetTempButton2);
    lv_group_add_obj(btn_group, ui_fastSetTempButton3);
    lv_group_add_obj(btn_group, ui_setAirDutyButton);
    lv_group_focus_obj(ui_backButton);
    lv_indev_set_group(knobs_indev, btn_group);
}

static void rotate_anim_cb(void *lvobj, int32_t v)
{
    lv_img_set_angle((lv_obj_t *)lvobj, v);
}
static void ui_fast_temp_btn1_pressed(lv_event_t *e)
{
    airhotModel.tempEnable.allValue = 0; // 重设置前必须清空
    airhotModel.tempEnable.bitValue.predefinedTempEnable_0 = ENABLE_STATE_OPEN;
    airhotModel.targetTemp = airhotModel.predefinedTemp_0;
    lv_label_set_text_fmt(ui_setTempLabel, "%d°C", airhotModel.targetTemp);
}
static void ui_fast_temp_btn2_pressed(lv_event_t *e)
{
    airhotModel.tempEnable.allValue = 0; // 重设置前必须清空
    airhotModel.tempEnable.bitValue.predefinedTempEnable_1 = ENABLE_STATE_OPEN;
    airhotModel.targetTemp = airhotModel.predefinedTemp_1;
    lv_label_set_text_fmt(ui_setTempLabel, "%d°C", airhotModel.targetTemp);
}
static void ui_fast_temp_btn3_pressed(lv_event_t *e)
{
    airhotModel.tempEnable.allValue = 0; // 重设置前必须清空
    airhotModel.tempEnable.bitValue.predefinedTempEnable_2 = ENABLE_STATE_OPEN;
    airhotModel.targetTemp = airhotModel.predefinedTemp_2;
    lv_label_set_text_fmt(ui_setTempLabel, "%d°C", airhotModel.targetTemp);
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
    //lv_obj_set_style_bg_color(airhotPageUI, IS_WHITE_THEME ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
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
    lv_label_set_text_fmt(ui_fastSetTempLabel1, "%d", airhotModel.predefinedTemp_0);

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
    lv_label_set_text_fmt(ui_fastSetTempLabel2, "%d", airhotModel.predefinedTemp_1);

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
    lv_label_set_text_fmt(ui_fastSetTempLabel3, "%d", airhotModel.predefinedTemp_2);

    ui_curTempLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_curTempLabel, LV_ALIGN_TOP_LEFT, 68, 44);
    lv_label_set_text_fmt(ui_curTempLabel, "%3d", airhotModel.curTemp);
    lv_obj_set_style_text_color(ui_curTempLabel, AIR_HOT_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_curTempLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_curTempLabel, &FontJost_52, LV_PART_MAIN | LV_STATE_DEFAULT);
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

    // 温度调节
    ui_setTempButton = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_setTempButton);
    lv_obj_set_size(ui_setTempButton, 60, 20);
    lv_obj_align(ui_setTempButton, LV_ALIGN_TOP_LEFT, 4, 64);
    lv_obj_add_flag(ui_setTempButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_setTempButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_setTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_setTempButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_setTempButton, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_setTempButton, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_setTempButton, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_setTempButton, &btn_type1_pressed_style, LV_STATE_EDITED);

    ui_setTempLabel = lv_label_create(ui_setTempButton);
    lv_obj_set_size(ui_setTempLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(ui_setTempLabel);
    lv_label_set_text_fmt(ui_setTempLabel, "%d°C", airhotModel.targetTemp);

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

    // 返回图标
    ui_backButton = lv_btn_create(ui_PanelTop);
    lv_obj_remove_style_all(ui_backButton);
    lv_obj_align(ui_backButton, LV_ALIGN_TOP_RIGHT, -10, 0);
    lv_obj_add_flag(ui_backButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_backButton, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_backButton, &back_btn_style, LV_STATE_DEFAULT);
    lv_obj_add_style(ui_backButton, &back_btn_focused_style, LV_STATE_FOCUSED);

    ui_backButtonLabel = lv_label_create(ui_backButton);
    lv_obj_center(ui_backButtonLabel);
    lv_label_set_text(ui_backButtonLabel, LV_SYMBOL_LEFT);

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

    lv_chart_set_point_count(chartTemp, 70);

    lv_obj_add_event_cb(chartTemp, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
    lv_chart_set_update_mode(chartTemp, LV_CHART_UPDATE_MODE_SHIFT);
    chartSer1 = lv_chart_add_series(chartTemp, AIR_HOT_THEME_COLOR1, LV_CHART_AXIS_PRIMARY_Y);

    // 这里调用后台记录的数组来填充数据
    uint32_t i;
    for (i = 0; i < 70; i++)
    {
        lv_chart_set_next_value(chartTemp, chartSer1, lv_rand(70, 90));
    }

    lv_obj_add_event_cb(ui_backButton, ui_back_btn_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_fastSetTempButton1, ui_fast_temp_btn1_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_fastSetTempButton2, ui_fast_temp_btn2_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_fastSetTempButton3, ui_fast_temp_btn3_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_setTempButton, ui_set_temp_btn_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_setAirDutyButton, ui_set_air_btn_pressed, LV_EVENT_PRESSED, NULL);

    setTempArcGroup = lv_group_create();
    setAirArcGroup = lv_group_create();

    airhotTimer = lv_timer_create(airhotTimer_timeout, 1000, NULL);
    lv_timer_set_repeat_count(airhotTimer, -1);
    return true;
}

static void airhotPageUI_release()
{
    if (NULL == airhotPageUI)
    {
        return;
    }
    if (NULL != btn_group)
    {
        lv_group_del(btn_group);
    }
    lv_timer_del(airhotTimer);
    lv_obj_clean(airhotPageUI);
    airhotPageUI = NULL;
    airhotUIObj.mainButtonUI = NULL;
}

void ui_updateAirhotCurTempAndPowerDuty(void)
{
    if (NULL == airhotPageUI)
    {
        return;
    }

    // todo
    // airhotModel.curTemp = lv_rand(200, 350);
    // airhotModel.powerRatio = lv_rand(400, 950);

    if (airhotModel.curTemp >= DISCONNCT_TEMP)
    {
        // 未连接
        lv_label_set_text(ui_curTempLabel, "Err");
    }
    else
    {
        lv_label_set_text_fmt(ui_curTempLabel, "%3d", airhotModel.curTemp);
        lv_bar_set_value(ui_powerBar, airhotModel.powerRatio, LV_ANIM_ON);
        lv_chart_set_next_value(chartTemp, chartSer1, airhotModel.curTemp / 4);
    }
}

static void ui_back_btn_pressed(lv_event_t *e)
{
    // 返回项被按下
    // lv_group_del(btn_group);
    show_menu();
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

static void ui_set_tempArc_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        airhotModel.tempEnable.allValue = 0; // 重设置前必须清空
        airhotModel.tempEnable.bitValue.refinementEnable = ENABLE_STATE_OPEN;
        airhotModel.targetTemp = lv_arc_get_value(ui_setTempArc);

        lv_group_focus_freeze(setTempArcGroup, false);
        lv_indev_set_group(knobs_indev, btn_group);

        LV_LOG_USER("set_tempArc LV_EVENT_PRESSED % u", event_code);
        if (NULL != ui_setTempArc)
        {
            lv_group_remove_obj(ui_setTempArc);
            lv_obj_clean(ui_setTempArc);
        }
        lv_obj_clear_state(ui_setTempButton, LV_STATE_EDITED);
    }
}

static void ui_set_temp_btn_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        LV_LOG_USER("button_pressed LV_EVENT_PRESSED % u", event_code);

        lv_obj_add_state(ui_setTempButton, LV_STATE_EDITED);
        if (NULL != ui_setTempArc)
        {
            lv_group_remove_obj(ui_setTempArc);
            lv_obj_clean(ui_setTempArc);
        }
        ui_setTempArc = lv_arc_create(ui_setTempButton);
        lv_obj_set_size(ui_setTempArc, 30, 30);
        lv_obj_center(ui_setTempArc);
        lv_arc_set_range(ui_setTempArc, 0, 500);
        lv_arc_set_value(ui_setTempArc, airhotModel.targetTemp);
        // 设置隐藏
        lv_obj_set_style_opa(ui_setTempArc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_group_add_obj(setTempArcGroup, ui_setTempArc);
        lv_indev_set_group(knobs_indev, setTempArcGroup);
        lv_group_focus_obj(ui_setTempArc);

        lv_obj_add_event_cb(ui_setTempArc, ui_set_tempArc_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_setTempArc, ui_set_tempArc_changed, LV_EVENT_VALUE_CHANGED, NULL);
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
            lv_obj_clean(ui_setAirArc);
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
            lv_obj_clean(ui_setAirArc);
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

FE_FULL_UI_OBJ airhotUIObj = {airhotPageUI, airhotPageUI_init,
                              airhotPageUI_release, airhotPageUI_focused};

#endif