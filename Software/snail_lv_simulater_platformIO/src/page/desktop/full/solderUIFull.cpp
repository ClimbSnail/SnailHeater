#include "./ui.h"
#include "../desktop_model.h"

#ifdef FULL_UI

static lv_timer_t *solderTimer = NULL;
static lv_obj_t *solderPageUI = NULL;
static lv_obj_t *ui_curTempLabel;
static lv_obj_t *ui_curTempCLabel;
static lv_obj_t *ui_targetTempButton;
static lv_obj_t *ui_powerBar;

static lv_obj_t *ui_fastSetTempButton0;
static lv_obj_t *ui_fastSetTempButton1;
static lv_obj_t *ui_fastSetTempButton2;
static lv_obj_t *ui_solderManualSwitch; // 手动开关
static lv_obj_t *solder_wake_type_img;
static lv_obj_t *solder_type_text_img;
static lv_obj_t *solder_type_val_text = NULL;
static lv_obj_t *vol_val_text = NULL;

static lv_obj_t *soldering_icon;

static lv_obj_t *ui_paramKp = NULL;
static lv_obj_t *ui_paramKi = NULL;
static lv_obj_t *ui_paramKd = NULL;

static lv_group_t *btn_group = NULL;

// 图标的历史数据
#define CHART_TEMP_LEN 28
static uint8_t chartTempData[CHART_TEMP_LEN] = {0};
static uint8_t chartTempDataSaveInd = 0; // 循环储存的下标

static void ui_set_temp_btn_pressed(lv_event_t *e);
static void ui_manual_switch_pressed(lv_event_t *e);
void solderPageUI_release();

static void solderPageUI_focused(lv_event_t *e)
{
    lv_indev_set_group(knobs_indev, btn_group);
    lv_group_focus_obj(ui_backBtn);
}

static void setFineAdjTemp(int temp)
{
    solderModel.fineAdjTemp = temp;
    solderModel.utilConfig.targetTemp = temp;
    lv_numberbtn_set_value(ui_targetTempButton, solderModel.utilConfig.targetTemp);
}

static void ui_fast_temp_btn1_pressed(lv_event_t *e)
{
    // solderModel.tempEnable.allValue = 0; // 重设置前必须清空
    // solderModel.tempEnable.bitValue.quickSetupTempEnable_0 = ENABLE_STATE_OPEN;
    // solderModel.targetTemp = solderModel.quickSetupTemp_0;
    setFineAdjTemp(solderModel.utilConfig.quickSetupTemp_0);
}
static void ui_fast_temp_btn2_pressed(lv_event_t *e)
{
    // solderModel.tempEnable.allValue = 0; // 重设置前必须清空
    // solderModel.tempEnable.bitValue.quickSetupTempEnable_1 = ENABLE_STATE_OPEN;
    // solderModel.targetTemp = solderModel.quickSetupTemp_1;
    setFineAdjTemp(solderModel.utilConfig.quickSetupTemp_1);
}
static void ui_fast_temp_btn3_pressed(lv_event_t *e)
{
    // solderModel.tempEnable.allValue = 0; // 重设置前必须清空
    // solderModel.tempEnable.bitValue.quickSetupTempEnable_2 = ENABLE_STATE_OPEN;
    // solderModel.targetTemp = solderModel.quickSetupTemp_2;
    setFineAdjTemp(solderModel.utilConfig.quickSetupTemp_2);
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
        if (ENABLE_STATE_OPEN == sysInfoModel.utilConfig.uiParam.uiGlobalParam.solderGridEnable)
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

static void setSolderWakeType()
{
    if (NULL == solder_wake_type_img)
    {
        return;
    }

    switch (solderModel.coreConfig.wakeSwitchType)
    {
    case SOLDER_SHAKE_TYPE_NONE:
        lv_img_set_src(solder_wake_type_img, &img_wake_type_none);
        break;
    case SOLDER_SHAKE_TYPE_HIGH:
        lv_img_set_src(solder_wake_type_img, &img_wake_type_high);
        break;
    case SOLDER_SHAKE_TYPE_LOW:
        lv_img_set_src(solder_wake_type_img, &img_wake_type_low);
        break;
    case SOLDER_SHAKE_TYPE_CHANGE:
        lv_img_set_src(solder_wake_type_img, &img_wake_type_change);
        break;
    default:
        lv_img_set_src(solder_wake_type_img, &img_wake_type_low);
        break;
    }
}

static void setSolderCoreType()
{
    if (NULL == solder_type_text_img)
    {
        return;
    }

    switch (solderModel.coreConfig.solderType)
    {
    case SOLDER_TYPE_UNHNOWN:
        lv_img_set_src(solder_type_text_img, &img_name_noc_ico);
        lv_img_set_src(soldering_icon, &img_soldering_gray);
        break;
    case SOLDER_TYPE_T12:
        lv_img_set_src(solder_type_text_img, &img_name_t12_ico);
        lv_img_set_src(soldering_icon, &T12_CORE_INDEXED_4);
        break;
    case SOLDER_TYPE_JBC210:
        lv_img_set_src(solder_type_text_img, &img_name_c210_ico);
        lv_img_set_src(soldering_icon, &C210_CORE_INDEXED_4);
        break;
    case SOLDER_TYPE_JBC245:
        lv_img_set_src(solder_type_text_img, &img_name_c245_ico);
        lv_img_set_src(soldering_icon, &C245_CORE_INDEXED_4);
        break;
    case SOLDER_TYPE_JBC470:
        lv_img_set_src(solder_type_text_img, &img_name_c470_ico);
        lv_img_set_src(soldering_icon, &C470_CORE_INDEXED_4);
        break;
    case SOLDER_TYPE_JBC115:
        lv_img_set_src(solder_type_text_img, &img_name_c115_ico);
        lv_img_set_src(soldering_icon, &C115_CORE_INDEXED_4);
        break;
    case SOLDER_TYPE_T20:
        lv_img_set_src(solder_type_text_img, &img_name_t20_ico);
        lv_img_set_src(soldering_icon, &T20_CORE_COLOR_ALPHA);
        break;
    case SOLDER_TYPE_TK:
        lv_img_set_src(solder_type_text_img, &img_name_tx_ico);
        lv_img_set_src(soldering_icon, &T936_CORE_INDEXED_4);
        break;
    default:
        lv_img_set_src(solder_type_text_img, &img_name_noc_ico);
        lv_img_set_src(soldering_icon, &img_name_noc_ico);
        break;
    }
}

static void ui_set_pid_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_PRESSED == event_code)
    {
        if (target == ui_paramKp)
        {
            solderModel.coreConfig.kp = lv_numberbtn_get_value(ui_paramKp);
        }
        else if (target == ui_paramKi)
        {
            solderModel.coreConfig.ki = lv_numberbtn_get_value(ui_paramKi);
        }
        else if (target == ui_paramKd)
        {
            solderModel.coreConfig.kd = lv_numberbtn_get_value(ui_paramKd);
        }
    }
}

static void solderTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    if (NULL != solder_type_val_text)
    {
        lv_label_set_text_fmt(solder_type_val_text, "%d mV", solderModel.typeValue);
    }

    if (NULL != vol_val_text)
    {
        lv_label_set_text_fmt(vol_val_text, "%.2lf V", solderModel.volValue / 1000.0);
    }

    ui_updateSolderData();
}

static bool solderPageUI_init(lv_obj_t *father)
{
    solderPageUI_release();

    top_layer_set_name();
    theme_color_init();

    solderPageUI = lv_btn_create(father);
    solderUIObj.mainButtonUI = solderPageUI;
    lv_obj_t *ui_ButtonTmp = solderPageUI;

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
    lv_label_set_text_fmt(ui_fastSetTempLabel0, "%d", solderModel.utilConfig.quickSetupTemp_0);

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
    lv_label_set_text_fmt(ui_fastSetTempLabel1, "%d", solderModel.utilConfig.quickSetupTemp_1);

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
    lv_label_set_text_fmt(ui_fastSetTempLabel2, "%d", solderModel.utilConfig.quickSetupTemp_2);

    ui_solderManualSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_add_flag(ui_solderManualSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_solderManualSwitch, 19, 20);
    lv_obj_align_to(ui_solderManualSwitch, ui_fastSetTempButton2,
                    LV_ALIGN_OUT_RIGHT_MID, 8, 30);
    lv_obj_set_style_outline_color(ui_solderManualSwitch, SOLDER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_solderManualSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_solderManualSwitch, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_bg_color(ui_solderManualSwitch, SOLDER_THEME_COLOR1, LV_PART_INDICATOR | LV_STATE_CHECKED);
    // lv_obj_set_style_opa(ui_solderManualSwitch, 255, LV_STATE_DISABLED);
    if (ENABLE_STATE_OPEN == solderModel.softwareSwitch)
    {
        lv_obj_add_state(ui_solderManualSwitch, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_clear_state(ui_solderManualSwitch, LV_STATE_CHECKED);
    }

    ui_curTempLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_set_size(ui_curTempLabel, 105, 52);
    lv_obj_align(ui_curTempLabel, LV_ALIGN_TOP_LEFT, 68, 44);

    if (solderModel.curTemp >= DISCONNCT_TEMP)
    {
        // 未连接
        lv_label_set_text(ui_curTempLabel, NO_CONNECT_MSG);
    }
    else
    {
        lv_label_set_text_fmt(ui_curTempLabel, "%d", solderModel.curTemp);
    }
    lv_obj_set_style_text_align(ui_curTempLabel, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_color(ui_curTempLabel, SOLDER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
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
    lv_numberbtn_set_value(ui_targetTempButton, solderModel.utilConfig.targetTemp);
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

    // 烙铁图片
    soldering_icon = lv_img_create(ui_ButtonTmp);
    lv_img_set_src(soldering_icon, &T20_CORE_INDEXED_4);
    lv_obj_align(soldering_icon, LV_ALIGN_CENTER, 105, -58);
    // lv_obj_set_style_img_recolor_opa(soldering_icon, LV_OPA_70, 0);
    // lv_obj_set_style_img_recolor(soldering_icon, lv_color_hex(0xff3964), 0);

    // 分割线
    /*Create an array for the points of the line*/
    static lv_point_t line_points[] = {{0, 5}, {5, 5}};
    /*Create style*/
    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 3);
    lv_style_set_line_color(&style_line, lv_color_hex(0xfb8e00));
    lv_style_set_line_rounded(&style_line, true);
    /*Create a line and apply the new style*/
    lv_obj_t *main_line = lv_line_create(ui_ButtonTmp);
    lv_line_set_points(main_line, line_points, 2); /*Set the points*/
    lv_obj_add_style(main_line, &style_line, 0);
    lv_obj_align_to(main_line, soldering_icon,
                    LV_ALIGN_OUT_RIGHT_TOP, 5, 50);

    solder_wake_type_img = lv_img_create(ui_ButtonTmp);
    lv_obj_align_to(solder_wake_type_img, main_line,
                    LV_ALIGN_OUT_TOP_MID, -7, -45);
    lv_obj_set_style_img_recolor_opa(solder_wake_type_img, LV_OPA_70, 0);
    lv_obj_set_style_img_recolor(solder_wake_type_img, SOLDER_THEME_COLOR3, 0);
    setSolderWakeType();

    solder_type_text_img = lv_img_create(ui_ButtonTmp);
    lv_obj_align_to(solder_type_text_img, main_line,
                    LV_ALIGN_OUT_BOTTOM_MID, -6, 3);
    lv_obj_set_style_img_recolor_opa(solder_type_text_img, LV_OPA_70, 0);
    lv_obj_set_style_img_recolor(solder_type_text_img, SOLDER_THEME_COLOR1, 0);
    setSolderCoreType();

    // 功率bar
    ui_powerBar = lv_bar_create(ui_ButtonTmp);
    lv_obj_set_size(ui_powerBar, SH_SCREEN_WIDTH - 30, 6);
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
    lv_obj_set_size(chartTemp, SH_SCREEN_WIDTH, CURVE_HIGH);
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

#ifdef TEST_VER
    // 类型值
    // if (false)
    {
        solder_type_val_text = lv_label_create(ui_ButtonTmp);
        lv_obj_set_align(solder_type_val_text, LV_ALIGN_CENTER);
        lv_obj_set_pos(solder_type_val_text, 0, 75);
        lv_label_set_text_fmt(solder_type_val_text, "%d mV", solderModel.typeValue);
        lv_obj_set_size(solder_type_val_text, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_add_flag(solder_type_val_text, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_clear_flag(solder_type_val_text, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_text_color(solder_type_val_text, AIR_HOT_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(solder_type_val_text, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    // if (false)
    {
        vol_val_text = lv_label_create(ui_ButtonTmp);
        lv_obj_set_align(vol_val_text, LV_ALIGN_CENTER);
        lv_obj_set_pos(vol_val_text, 0, 30);
        lv_label_set_text_fmt(vol_val_text, "%.2lf V", solderModel.volValue / 1000.0);
        lv_obj_set_size(vol_val_text, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_add_flag(vol_val_text, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_clear_flag(vol_val_text, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_text_color(vol_val_text, AIR_HOT_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(vol_val_text, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
#endif

    if (ENABLE_STATE::ENABLE_STATE_OPEN == solderModel.utilConfig.fastPID)
    {
        ui_paramKp = lv_numberbtn_create(chartTemp);
        lv_obj_t *solderParamKpLabel = lv_label_create(ui_paramKp);
        lv_numberbtn_set_label_and_format(ui_paramKp,
                                          solderParamKpLabel, "P->%d", 1);
        lv_numberbtn_set_range(ui_paramKp, 0, 200);
        lv_numberbtn_set_value(ui_paramKp, solderModel.coreConfig.kp);
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
        lv_numberbtn_set_value(ui_paramKi, solderModel.coreConfig.ki);
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
        lv_numberbtn_set_value(ui_paramKd, solderModel.coreConfig.kd);
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
    // lv_obj_add_event_cb(ui_targetTempButton, ui_setIntellectRateFlag, LV_EVENT_ALL, NULL);
    // lv_obj_add_event_cb(ui_targetTempButton, ui_setIntellectRateFlag, LV_EVENT_FOCUSED, NULL);
    // lv_obj_add_event_cb(ui_targetTempButton, ui_setIntellectRateFlag, LV_EVENT_DEFOCUSED, NULL);

    if (ENABLE_STATE::ENABLE_STATE_OPEN == solderModel.utilConfig.fastPID)
    {
        lv_obj_add_event_cb(ui_paramKp, ui_set_pid_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_paramKi, ui_set_pid_pressed, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_paramKd, ui_set_pid_pressed, LV_EVENT_PRESSED, NULL);
    }
    lv_obj_add_event_cb(ui_solderManualSwitch, ui_manual_switch_pressed, LV_EVENT_VALUE_CHANGED, NULL);

    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_backBtn);
    lv_group_add_obj(btn_group, ui_targetTempButton);
    lv_group_add_obj(btn_group, ui_fastSetTempButton0);
    lv_group_add_obj(btn_group, ui_fastSetTempButton1);
    lv_group_add_obj(btn_group, ui_fastSetTempButton2);
    if (SOLDER_SHAKE_TYPE_NONE == solderModel.coreConfig.wakeSwitchType)
    {
        lv_group_add_obj(btn_group, ui_solderManualSwitch);
        // lv_obj_set_style_text_opa(ui_solderManualSwitch, 150, LV_PART_MAIN);
        lv_obj_clear_flag(ui_solderManualSwitch, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_group_remove_obj(ui_solderManualSwitch);
        lv_obj_add_flag(ui_solderManualSwitch, LV_OBJ_FLAG_HIDDEN);
    }
    if (ENABLE_STATE::ENABLE_STATE_OPEN == solderModel.utilConfig.fastPID)
    {
        lv_group_add_obj(btn_group, ui_paramKp);
        lv_group_add_obj(btn_group, ui_paramKi);
        lv_group_add_obj(btn_group, ui_paramKd);
    }
    lv_indev_set_group(knobs_indev, btn_group);
    lv_group_focus_obj(ui_backBtn);

    solderTimer = lv_timer_create(solderTimer_timeout, DATA_REFRESH_MS, NULL);
    lv_timer_set_repeat_count(solderTimer, -1);
    return true;
}

void solderPageUI_release()
{
    if (NULL != solderTimer)
    {
        lv_timer_del(solderTimer);
        solderTimer = NULL;
    }

    if (NULL != btn_group)
    {
        lv_group_del(btn_group);
        btn_group = NULL;
    }

    if (NULL != solderPageUI)
    {
        lv_obj_del(solderPageUI);
        solderPageUI = NULL;
        solderUIObj.mainButtonUI = NULL;
        ui_paramKp = NULL;
        ui_paramKi = NULL;
        ui_paramKd = NULL;
    }
}

void ui_updateSolderPIDParam(void)
{
    // 便捷pid
    if (ENABLE_STATE::ENABLE_STATE_OPEN == solderModel.utilConfig.fastPID)
    {
        if (NULL != ui_paramKp)
        {
            lv_numberbtn_set_value(ui_paramKp, solderModel.coreConfig.kp);
            lv_numberbtn_set_value(ui_paramKi, solderModel.coreConfig.ki);
            lv_numberbtn_set_value(ui_paramKd, solderModel.coreConfig.kd);
        }
    }
}

void ui_updateSolderData(void)
{
    if (NULL == solderPageUI)
    {
        return;
    }
    if (solderModel.curTemp >= DISCONNCT_TEMP)
    {
        // 未连接
        lv_label_set_text(ui_curTempLabel, NO_CONNECT_MSG);
        lv_obj_set_style_img_recolor_opa(soldering_icon, LV_OPA_30, 0);
    }
    else
    {
        lv_label_set_text_fmt(ui_curTempLabel, "%d", solderModel.curTemp);

        setSolderWakeType();
        setSolderCoreType();

        uint8_t opa = 0;
        if (solderModel.curTemp >= solderModel.utilConfig.targetTemp)
        {
            opa = 178;
        }
        else
        {
            opa = 76 + 100 * solderModel.curTemp / solderModel.utilConfig.targetTemp;
        }
        lv_obj_set_style_img_recolor_opa(soldering_icon, opa, 0);
        lv_bar_set_value(ui_powerBar, solderModel.powerRatio, LV_ANIM_ON);

        chartTempData[chartTempDataSaveInd] = (uint8_t)(solderModel.curTemp >> 2);
        lv_chart_set_next_value(chartTemp, chartSer1, chartTempData[chartTempDataSaveInd]);
        chartTempDataSaveInd = (chartTempDataSaveInd + 1) % CHART_TEMP_LEN;

        // 自动添加"手动按钮"到按键组
        if (SOLDER_SHAKE_TYPE_NONE == solderModel.coreConfig.wakeSwitchType &&
            lv_obj_has_flag(ui_solderManualSwitch, LV_OBJ_FLAG_HIDDEN))
        {
            lv_group_add_obj(btn_group, ui_solderManualSwitch);
            // lv_obj_set_style_text_opa(ui_solderManualSwitch, 150, LV_PART_MAIN);
            lv_obj_clear_flag(ui_solderManualSwitch, LV_OBJ_FLAG_HIDDEN);
        }
        else if (SOLDER_SHAKE_TYPE_NONE != solderModel.coreConfig.wakeSwitchType &&
                 !lv_obj_has_flag(ui_solderManualSwitch, LV_OBJ_FLAG_HIDDEN))
        {
            lv_group_remove_obj(ui_solderManualSwitch);
            lv_obj_add_flag(ui_solderManualSwitch, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void ui_manual_switch_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        if (target == ui_solderManualSwitch)
        {
            if (lv_obj_has_state(target, LV_STATE_CHECKED))
            {
                solderModel.softwareSwitch = ENABLE_STATE::ENABLE_STATE_OPEN;
            }
            else
            {
                solderModel.softwareSwitch = ENABLE_STATE::ENABLE_STATE_CLOSE;
            }
        }
    }
}

static void ui_set_temp_btn_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (LV_EVENT_PRESSED == event_code)
    {
        solderModel.tempEnable.allValue = 0; // 重设置前必须清空
        solderModel.tempEnable.bitValue.fineAdjTempEnable = ENABLE_STATE_OPEN;
        solderModel.fineAdjTemp = lv_numberbtn_get_value(ui_targetTempButton);
        solderModel.utilConfig.targetTemp = lv_numberbtn_get_value(ui_targetTempButton);
    }
}

FE_UI_OBJ solderUIObj = {solderPageUI, solderPageUI_init,
                         solderPageUI_release, solderPageUI_focused};

#endif