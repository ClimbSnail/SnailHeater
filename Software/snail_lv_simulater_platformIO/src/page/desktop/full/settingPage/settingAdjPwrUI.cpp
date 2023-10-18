#include "./settingUIFull.h"

#ifdef FULL_UI

#include "../ui.h"
#include "../../desktop_model.h"

static lv_obj_t *ui_father = NULL;

static lv_obj_t *ui_pwrCurZeroLabel;
static lv_obj_t *ui_pwrCurZeroBtn;
static lv_obj_t *ui_pwrCurZeroBtnLabel;

static void ui_adjpwr_zero_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_PRESSED)
    {
        adjPowerModel.curToZeroFlag = true;
    }
}

void ui_adjpwr_setting_init(lv_obj_t *father)
{
    ui_father = father;
    // 静态电流
    ui_pwrCurZeroLabel = lv_label_create(father);
    lv_obj_align(ui_pwrCurZeroLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_label_set_text(ui_pwrCurZeroLabel, SETTING_TEXT_ADJPWR_CUR_ZERO);
    lv_obj_add_style(ui_pwrCurZeroLabel, &label_text_style, 0);

    ui_pwrCurZeroBtn = lv_btn_create(father);
    lv_obj_add_flag(ui_pwrCurZeroBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_pwrCurZeroBtn, 40, 20);
    lv_obj_align(ui_pwrCurZeroBtn, LV_ALIGN_TOP_LEFT, 105, 10);
    // lv_obj_set_style_outline_color(ui_pwrCurZeroBtn, SETTING_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_pwrCurZeroBtn, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_pwrCurZeroBtn, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    ui_pwrCurZeroBtnLabel = lv_label_create(ui_pwrCurZeroBtn);
    lv_obj_align(ui_pwrCurZeroBtnLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(ui_pwrCurZeroBtnLabel, "归零");
    lv_obj_add_style(ui_pwrCurZeroBtnLabel, &label_text_style, 0);

    ui_subBackBtn = lv_btn_create(father);
    lv_obj_remove_style_all(ui_subBackBtn);
    lv_obj_align(ui_subBackBtn, LV_ALIGN_TOP_LEFT, 10, 130);
    lv_obj_add_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_subBackBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_subBackBtn, &back_btn_style, LV_STATE_DEFAULT);
    lv_obj_add_style(ui_subBackBtn, &back_btn_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_subBackBtnLabel = lv_label_create(ui_subBackBtn);
    lv_obj_center(ui_subBackBtnLabel);
    lv_obj_add_style(ui_subBackBtnLabel, &label_text_style, 0);
    lv_label_set_text(ui_subBackBtnLabel, "返回");
}

void ui_adjpwr_setting_init_group(lv_obj_t *father)
{
    lv_obj_add_event_cb(ui_pwrCurZeroBtn, ui_adjpwr_zero_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_subBackBtn, ui_tab_back_btn_pressed, LV_EVENT_PRESSED, 0);

    sub_btn_group = lv_group_create();
    lv_group_add_obj(sub_btn_group, ui_subBackBtn);
    lv_group_add_obj(sub_btn_group, ui_pwrCurZeroBtn);
    lv_indev_set_group(knobs_indev, sub_btn_group);
}

void ui_adjpwr_setting_release(void *param)
{
    if (NULL != ui_father)
    {
        lv_obj_clean(ui_father);
        ui_father = NULL;
    }
}

#endif