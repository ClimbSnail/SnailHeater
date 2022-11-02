// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.0.5
// LVGL VERSION: 8.2
// PROJECT: SquareLine_Project

#ifndef _SQUARELINE_PROJECT_UI_H
#define _SQUARELINE_PROJECT_UI_H

#ifdef __cplusplus
extern "C"
{
#endif

#if __has_include("lvgl.h")
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "uiBase.h"
#include "language_info.h"
#include "resource/images/ico.h"

#define START_UI_OBJ_X 140
#define WIN_INFO_SIZE 3
#define UI_OBJ_NUM 5

    // 字库解决方法 https://blog.csdn.net/a200327/article/details/120628170
    LV_FONT_DECLARE(sh_number_40);
    LV_FONT_DECLARE(sh_number_50);
    LV_FONT_DECLARE(sh_ch_font_14);
    LV_FONT_DECLARE(sh_ch_font_18);
    // LV_FONT_DECLARE(lv_font_montserrat_10);
    // LV_FONT_DECLARE(lv_font_montserrat_16);
    // LV_FONT_DECLARE(lv_font_montserrat_18);
    // LV_FONT_DECLARE(lv_font_montserrat_28);

    extern lv_style_t focused_style;
    extern lv_obj_t *desktop_screen;
    extern lv_obj_t *ui_PanelMain;

    extern lv_indev_t *knobs_indev; // 旋钮设备

#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); //等待动画完成

    void ui_init(lv_indev_t *lv_indev_delete);
    void ui_main_pressed(lv_event_t *e);
    void ui_Button_move_center(int middleIndex);

    static uint8_t lastMiddlePos = 1;

    extern int16_t btnPosXY[WIN_INFO_SIZE][2];

    extern struct FE_UI_OBJ solderUIObj;
    extern struct FE_UI_OBJ airhotUIObj;
    extern struct FE_UI_OBJ hpUIObj;
    extern struct FE_UI_OBJ adjPowerUIObj;
    extern struct FE_UI_OBJ sysInfoUIObj;

    void ui_updateHeatplatformCurTempAndPowerDuty(void);
    void ui_updateAirhotCurTempAndPowerDuty(void);
    void ui_updateSolderCurTempAndPowerDuty(void);
    void ui_updateAdjPowerCurVoltage(void);
    void ui_updateAdjPowerWorkState(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
