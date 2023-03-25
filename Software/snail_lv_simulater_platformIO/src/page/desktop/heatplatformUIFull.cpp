#include "ui.h"
#include "desktop_model.h"

#ifdef NEW_UI

static lv_timer_t *hpTimer;
static lv_obj_t *hpPageUI = NULL;
static lv_obj_t *ui_curTempLabel;
static lv_obj_t *ui_curTempCLabel;
static lv_obj_t *ui_fineAdjTempButton;
static lv_obj_t *ui_fineAdjTempLabel;
static lv_obj_t *ui_enableSwitch;
static lv_obj_t *ui_fineAdjTempArc;
static lv_obj_t *ui_backButton;
static lv_obj_t *ui_backButtonLabel;
static lv_obj_t *ui_powerBar;
static lv_obj_t *ui_setAirDutyButton;
static lv_obj_t *ui_setAirDutyLabel;
static lv_obj_t *ui_setAirArc;
static lv_obj_t *ui_setAirTextLabel;

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

// 图标的历史数据
#define CHART_TEMP_LEN 70
static uint8_t chartTempData[CHART_TEMP_LEN] = {0};
static uint8_t chartTempDataSaveInd; // 循环储存的下标

static void ui_set_temp_btn_pressed(lv_event_t *e);
static void ui_back_btn_pressed(lv_event_t *e);
static void ui_enable_switch_pressed(lv_event_t *e);

static void hpPageUI_focused()
{
    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_backButton);
    lv_group_add_obj(btn_group, ui_enableSwitch);
    lv_group_add_obj(btn_group, ui_fineAdjTempButton);
    lv_group_add_obj(btn_group, ui_fastSetTempButton1);
    lv_group_add_obj(btn_group, ui_fastSetTempButton2);
    lv_group_add_obj(btn_group, ui_fastSetTempButton3);
    if (heatplatformModel.enable == ENABLE_STATE_OPEN)
    {
        lv_group_focus_obj(ui_enableSwitch);
    }
    else
    {
        lv_group_focus_obj(ui_backButton);
    }
    lv_indev_set_group(knobs_indev, btn_group);
}

static void ui_fast_temp_btn1_pressed(lv_event_t *e)
{
    heatplatformModel.tempEnable.allValue = 0; // 重设置前必须清空
    heatplatformModel.tempEnable.bitValue.quickSetupTempEnable_0 = ENABLE_STATE_OPEN;
    heatplatformModel.targetTemp = solderModel.quickSetupTemp_0;
}
static void ui_fast_temp_btn2_pressed(lv_event_t *e)
{
    heatplatformModel.tempEnable.allValue = 0; // 重设置前必须清空
    heatplatformModel.tempEnable.bitValue.quickSetupTempEnable_1 = ENABLE_STATE_OPEN;
    heatplatformModel.targetTemp = solderModel.quickSetupTemp_1;
}
static void ui_fast_temp_btn3_pressed(lv_event_t *e)
{
    heatplatformModel.tempEnable.allValue = 0; // 重设置前必须清空
    heatplatformModel.tempEnable.bitValue.quickSetupTempEnable_2 = ENABLE_STATE_OPEN;
    heatplatformModel.targetTemp = solderModel.quickSetupTemp_2;
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
        if (ENABLE_STATE_OPEN == sysInfoModel.uiGlobalParam.heatplatGridEnable)
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
    ui_updateHeatplatformCurTempAndPowerDuty();
}

static bool hpPageUI_init(lv_obj_t *father)
{
    if (NULL != hpPageUI)
    {
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
    lv_label_set_text_fmt(ui_fastSetTempLabel1, "%d", heatplatformModel.quickSetupTemp_0);

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
    lv_label_set_text_fmt(ui_fastSetTempLabel2, "%d", heatplatformModel.quickSetupTemp_1);

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
    lv_label_set_text_fmt(ui_fastSetTempLabel3, "%d", heatplatformModel.quickSetupTemp_2);

    ui_curTempLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_curTempLabel, LV_ALIGN_TOP_LEFT, 68, 44);
    lv_label_set_text_fmt(ui_curTempLabel, "%3d", heatplatformModel.curTemp);
    lv_obj_set_style_text_color(ui_curTempLabel, HEAT_PLAT_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
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
    lv_label_set_text_fmt(ui_fineAdjTempLabel, "%d°C", heatplatformModel.fineAdjTemp);

    ui_setAirTextLabel = lv_label_create(ui_ButtonTmp);
    lv_label_set_text(ui_setAirTextLabel, TEXT_AIR_SPEED);
    lv_obj_align(ui_setAirTextLabel, LV_ALIGN_TOP_LEFT, 220, 12);
    lv_obj_add_style(ui_setAirTextLabel, &label_text_style, 0);

    ui_setAirDutyButton = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_setAirDutyButton);
    lv_obj_set_size(ui_setAirDutyButton, 64, 30);
    lv_obj_align(ui_setAirDutyButton, LV_ALIGN_CENTER, 102, -65);
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
    lv_label_set_text_fmt(ui_setAirDutyLabel, "%d%%", heatplatformModel.workAirSpeed);
    // 上面的模型要改

    // 使能开关
    ui_enableSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_add_flag(ui_enableSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_enableSwitch, 40, 20);
    lv_obj_align(ui_enableSwitch, LV_ALIGN_CENTER, 104, -36);

    if (heatplatformModel.enable == ENABLE_STATE_OPEN)
    {
        lv_obj_add_state(ui_enableSwitch, LV_STATE_CHECKED); // 开
    }
    else
    {
        // if (GET_BIT(cfgKey1, CFG_KEY1_HP_AUTO_HEAT))
        if (ENABLE_STATE_OPEN == sysInfoModel.uiGlobalParam.hpAutoHeatEnable)
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

    lv_style_set_bg_color(&bar_style_indic, HEAT_PLAT_THEME_COLOR2);
    lv_style_set_bg_grad_color(&bar_style_indic, HEAT_PLAT_THEME_COLOR1);
    lv_obj_add_style(ui_powerBar, &bar_style_bg, LV_PART_MAIN);
    lv_obj_add_style(ui_powerBar, &bar_style_indic, LV_PART_INDICATOR);

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
    chartSer1 = lv_chart_add_series(chartTemp, HEAT_PLAT_THEME_COLOR1, LV_CHART_AXIS_PRIMARY_Y);

    // 这里调用后台记录的数组来填充数据
    for (uint32_t i = chartTempDataSaveInd + 1; i != chartTempDataSaveInd;)
    {
        lv_chart_set_next_value(chartTemp, chartSer1, chartTempData[i]);
        i = (i + 1) % CHART_TEMP_LEN;
    }

    lv_obj_add_event_cb(ui_backButton, ui_back_btn_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_fastSetTempButton1, ui_fast_temp_btn1_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_fastSetTempButton2, ui_fast_temp_btn2_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_fastSetTempButton3, ui_fast_temp_btn3_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_fineAdjTempButton, ui_set_temp_btn_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_enableSwitch, ui_enable_switch_pressed, LV_EVENT_VALUE_CHANGED, NULL);

    setTempArcGroup = lv_group_create();

    hpTimer = lv_timer_create(hpTimer_timeout, 300, NULL);
    lv_timer_set_repeat_count(hpTimer, -1);
    return true;
}

static void hpPageUI_release()
{
    if (NULL == hpPageUI)
    {
        return;
    }
    if (NULL != btn_group)
    {
        lv_group_del(btn_group);
    }
    lv_timer_del(hpTimer);
    lv_obj_clean(hpPageUI);
    hpPageUI = NULL;
    hpUIObj.mainButtonUI = NULL;
}

static void ui_enable_switch_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        bool isEnable = lv_obj_has_state(target, LV_STATE_CHECKED); // 返回 bool 类型， 开-1 ； 关-2
        heatplatformModel.enable = isEnable ? ENABLE_STATE_OPEN : ENABLE_STATE_CLOSE;
    }
}
static void ui_back_btn_pressed(lv_event_t *e)
{
    // 返回项被按下
    show_menu();
}

void ui_updateHeatplatformCurTempAndPowerDuty(void)
{
    if (NULL == hpPageUI)
    {
        return;
    }

    if (heatplatformModel.curTemp >= DISCONNCT_TEMP)
    {
        // 未连接
        lv_label_set_text(ui_curTempLabel, "Err");
    }
    else
    {
        lv_label_set_text_fmt(ui_curTempLabel, "%3d", heatplatformModel.curTemp);
        lv_bar_set_value(ui_powerBar, heatplatformModel.powerRatio, LV_ANIM_ON);
        chartTempData[chartTempDataSaveInd] = heatplatformModel.curTemp >> 2;
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
        heatplatformModel.tempEnable.allValue = 0; // 重设置前必须清空
        heatplatformModel.tempEnable.bitValue.fineAdjTempEnable = ENABLE_STATE_OPEN;
        heatplatformModel.fineAdjTemp = lv_arc_get_value(ui_fineAdjTempArc);
        heatplatformModel.targetTemp = heatplatformModel.fineAdjTemp;

        lv_group_focus_freeze(setTempArcGroup, false);
        lv_indev_set_group(knobs_indev, btn_group);

        LV_LOG_USER("set_tempArc LV_EVENT_PRESSED % u", event_code);
        if (NULL != ui_fineAdjTempArc)
        {
            lv_group_remove_obj(ui_fineAdjTempArc);
            lv_obj_clean(ui_fineAdjTempArc);
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
            lv_obj_clean(ui_fineAdjTempArc);
        }
        ui_fineAdjTempArc = lv_arc_create(ui_fineAdjTempButton);
        lv_obj_set_size(ui_fineAdjTempArc, 30, 30);
        lv_obj_center(ui_fineAdjTempArc);
        lv_arc_set_range(ui_fineAdjTempArc, 0, 500);
        lv_arc_set_value(ui_fineAdjTempArc, heatplatformModel.fineAdjTemp);
        // 设置隐藏
        lv_obj_set_style_opa(ui_fineAdjTempArc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_group_add_obj(setTempArcGroup, ui_fineAdjTempArc);
        lv_indev_set_group(knobs_indev, setTempArcGroup);
        lv_group_focus_obj(ui_fineAdjTempArc);

        lv_obj_add_event_cb(ui_fineAdjTempArc, ui_set_tempArc_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_fineAdjTempArc, ui_set_tempArc_changed, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

FE_FULL_UI_OBJ hpUIObj = {hpPageUI, hpPageUI_init,
                          hpPageUI_release, hpPageUI_focused};

#endif