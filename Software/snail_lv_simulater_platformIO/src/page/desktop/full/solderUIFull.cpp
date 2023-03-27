#include "./ui.h"
#include "../desktop_model.h"

#ifdef FULL_UI

static lv_timer_t *solderTimer = NULL;
static lv_obj_t *solderPageUI = NULL;
static lv_obj_t *ui_curTempLabel;
static lv_obj_t *ui_curTempCLabel;
static lv_obj_t *ui_fineAdjTempButton;
static lv_obj_t *ui_fineAdjTempLabel;
static lv_obj_t *ui_fineAdjTempArc = NULL;
static lv_obj_t *ui_solderTypeDropdown;
static lv_obj_t *ui_powerBar;

static lv_obj_t *ui_fastSetTempButton1;
static lv_obj_t *ui_fastSetTempButton2;
static lv_obj_t *ui_fastSetTempButton3;
static lv_obj_t *ui_fastSetTempLabel1;
static lv_obj_t *ui_fastSetTempLabel2;
static lv_obj_t *ui_fastSetTempLabel3;
static lv_obj_t *solder_type_text;

static lv_obj_t *lb1;
static lv_obj_t *lb2;
static lv_obj_t *soldering_icon;

static lv_group_t *btn_group = NULL;
static lv_group_t *setTempArcGroup = NULL;

// 图标的历史数据
#define CHART_TEMP_LEN 28
static uint8_t chartTempData[CHART_TEMP_LEN] = {0};
static uint8_t chartTempDataSaveInd = 0; // 循环储存的下标

static void ui_type_pressed(lv_event_t *e);
static void ui_set_temp_btn_pressed(lv_event_t *e);

static void solderPageUI_focused(lv_event_t *e)
{
    lv_group_focus_obj(ui_backBtn);
    lv_indev_set_group(knobs_indev, btn_group);
}

static void setFineAdjTemp(int temp)
{
    solderModel.fineAdjTemp = temp;
    solderModel.targetTemp = solderModel.fineAdjTemp;
    lv_label_set_text_fmt(ui_fineAdjTempLabel, "%d°C", solderModel.fineAdjTemp);
}

static void ui_fast_temp_btn1_pressed(lv_event_t *e)
{
    // solderModel.tempEnable.allValue = 0; // 重设置前必须清空
    // solderModel.tempEnable.bitValue.quickSetupTempEnable_0 = ENABLE_STATE_OPEN;
    // solderModel.targetTemp = solderModel.quickSetupTemp_0;
    setFineAdjTemp(solderModel.quickSetupTemp_0);
}
static void ui_fast_temp_btn2_pressed(lv_event_t *e)
{
    // solderModel.tempEnable.allValue = 0; // 重设置前必须清空
    // solderModel.tempEnable.bitValue.quickSetupTempEnable_1 = ENABLE_STATE_OPEN;
    // solderModel.targetTemp = solderModel.quickSetupTemp_1;
    setFineAdjTemp(solderModel.quickSetupTemp_1);
}
static void ui_fast_temp_btn3_pressed(lv_event_t *e)
{
    // solderModel.tempEnable.allValue = 0; // 重设置前必须清空
    // solderModel.tempEnable.bitValue.quickSetupTempEnable_2 = ENABLE_STATE_OPEN;
    // solderModel.targetTemp = solderModel.quickSetupTemp_2;
    setFineAdjTemp(solderModel.quickSetupTemp_2);
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
        // if (GET_BIT(cfgKey1, CFG_KEY1_SOLDER_GRID))
        if (ENABLE_STATE_OPEN == sysInfoModel.uiGlobalParam.solderGridEnable)
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

static void solderTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    ui_updateSolderCurTempAndPowerDuty();
}

static bool solderPageUI_init(lv_obj_t *father)
{
    if (NULL != solderPageUI)
    {
        solderPageUI = NULL;
    }
    top_layer_set_name();
    theme_color_init();

    solderPageUI = lv_btn_create(father);
    solderUIObj.mainButtonUI = solderPageUI;

    lv_obj_remove_style_all(solderPageUI);
    lv_obj_set_size(solderPageUI, EACH_PAGE_SIZE_X, EACH_PAGE_SIZE_Y);
    lv_obj_center(solderPageUI);
    lv_obj_add_flag(solderPageUI, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(solderPageUI, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_style_bg_color(solderPageUI, IS_WHITE_THEME ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(solderPageUI, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_ButtonTmp = solderPageUI;

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
    lv_label_set_text_fmt(ui_fastSetTempLabel1, "%d", solderModel.quickSetupTemp_0);

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
    lv_label_set_text_fmt(ui_fastSetTempLabel2, "%d", solderModel.quickSetupTemp_1);

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
    lv_label_set_text_fmt(ui_fastSetTempLabel3, "%d", solderModel.quickSetupTemp_2);

    ui_curTempLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_curTempLabel, 105, 52);
    lv_obj_align(ui_curTempLabel, LV_ALIGN_TOP_LEFT, 68, 44);
    lv_label_set_text_fmt(ui_curTempLabel, "%d", solderModel.curTemp);
    lv_obj_set_style_text_align(ui_curTempLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_color(ui_curTempLabel, SOLDER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
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

    // lv_obj_set_style_text_color(ui_fineAdjTempButton, SOLDER_THEME_COLOR1, LV_STATE_EDITED);

    ui_fineAdjTempLabel = lv_label_create(ui_fineAdjTempButton);
    lv_obj_set_size(ui_fineAdjTempLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(ui_fineAdjTempLabel);
    lv_label_set_text_fmt(ui_fineAdjTempLabel, "%d°C", solderModel.fineAdjTemp);

    // 烙铁图片
    soldering_icon = lv_img_create(ui_ButtonTmp);
    lv_img_set_src(soldering_icon, &img_soldering_gray);
    lv_obj_align(soldering_icon, LV_ALIGN_CENTER, 105, -58);
    lv_obj_set_style_img_recolor_opa(soldering_icon, LV_OPA_70, 0);
    lv_obj_set_style_img_recolor(soldering_icon, lv_color_hex(0xff3964), 0);

    solder_type_text = lv_img_create(ui_ButtonTmp);
    switch (solderModel.type)
    {
    case SOLDER_TYPE_T12:
        lv_img_set_src(solder_type_text, &img_text_t12);
        break;
    case SOLDER_TYPE_JBC210:
        lv_img_set_src(solder_type_text, &img_text_c210);
        break;
    case SOLDER_TYPE_JBC245:
        lv_img_set_src(solder_type_text, &img_text_c245);
        break;
    default:
        lv_img_set_src(solder_type_text, &img_text_noc);
        break;
    }
    lv_obj_align(solder_type_text, LV_ALIGN_CENTER, 118, -58);

    // 功率bar
    ui_powerBar = lv_bar_create(ui_ButtonTmp);
    lv_obj_set_size(ui_powerBar, 250, 6);
    lv_obj_align(ui_powerBar, LV_ALIGN_CENTER, 0, -5);
    lv_bar_set_range(ui_powerBar, 0, 1000);
    lv_bar_set_value(ui_powerBar, 0, LV_ANIM_ON);

    lv_style_set_bg_color(&bar_style_indic, SOLDER_THEME_COLOR2);
    lv_style_set_bg_grad_color(&bar_style_indic, SOLDER_THEME_COLOR1);
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

    chartSer1 = lv_chart_add_series(chartTemp, SOLDER_THEME_COLOR1, LV_CHART_AXIS_PRIMARY_Y);

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

    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_backBtn);
    lv_group_add_obj(btn_group, ui_fineAdjTempButton);
    lv_group_add_obj(btn_group, ui_fastSetTempButton1);
    lv_group_add_obj(btn_group, ui_fastSetTempButton2);
    lv_group_add_obj(btn_group, ui_fastSetTempButton3);
    lv_group_focus_obj(ui_backBtn);
    lv_indev_set_group(knobs_indev, btn_group);

    setTempArcGroup = lv_group_create();

    solderTimer = lv_timer_create(solderTimer_timeout, DATA_REFRESH_MS, NULL);
    lv_timer_set_repeat_count(solderTimer, -1);
    return true;
}

void solderPageUI_release()
{
    if (NULL != solderPageUI)
    {
        lv_obj_del(solderPageUI);
        solderPageUI = NULL;
        solderUIObj.mainButtonUI = NULL;
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

    if (NULL != solderTimer)
    {
        lv_timer_del(solderTimer);
        solderTimer = NULL;
    }
}

void ui_updateSolderCurTempAndPowerDuty(void)
{
    if (NULL == solderPageUI)
    {
        return;
    }
    if (solderModel.curTemp >= DISCONNCT_TEMP)
    {
        // 未连接
        lv_label_set_text(ui_curTempLabel, "Err");
        lv_obj_set_style_img_recolor_opa(soldering_icon, LV_OPA_30, 0);
    }
    else
    {
        lv_label_set_text_fmt(ui_curTempLabel, "%d", solderModel.curTemp);
        uint8_t opa = 0;
        if (solderModel.curTemp >= solderModel.targetTemp)
        {
            opa = 178;
        }
        else
        {
            opa = 76 + 100 * solderModel.curTemp / solderModel.targetTemp;
        }
        lv_obj_set_style_img_recolor_opa(soldering_icon, opa, 0);
        lv_bar_set_value(ui_powerBar, solderModel.powerRatio, LV_ANIM_ON);

        chartTempData[chartTempDataSaveInd] = (uint8_t)(solderModel.curTemp >> 2);
        lv_chart_set_next_value(chartTemp, chartSer1, chartTempData[chartTempDataSaveInd]);
        chartTempDataSaveInd = (chartTempDataSaveInd + 1) % CHART_TEMP_LEN;
    }
}

static void ui_type_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        uint16_t index = lv_dropdown_get_selected(ui_solderTypeDropdown); // 获取索引
        solderModel.type = index;
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

static void ui_solder_set_tempArc_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        solderModel.tempEnable.allValue = 0; // 重设置前必须清空
        solderModel.tempEnable.bitValue.fineAdjTempEnable = ENABLE_STATE_OPEN;
        solderModel.fineAdjTemp = lv_arc_get_value(ui_fineAdjTempArc);
        solderModel.targetTemp = solderModel.fineAdjTemp;

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

        // 给它增加一个状态
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
        lv_arc_set_value(ui_fineAdjTempArc, solderModel.fineAdjTemp);
        // 设置隐藏
        lv_obj_set_style_opa(ui_fineAdjTempArc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_group_add_obj(setTempArcGroup, ui_fineAdjTempArc);
        lv_indev_set_group(knobs_indev, setTempArcGroup);
        lv_group_focus_obj(ui_fineAdjTempArc);

        lv_obj_add_event_cb(ui_fineAdjTempArc, ui_solder_set_tempArc_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_fineAdjTempArc, ui_set_tempArc_changed, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

FE_UI_OBJ solderUIObj = {solderPageUI, solderPageUI_init,
                         solderPageUI_release, solderPageUI_focused};

#endif