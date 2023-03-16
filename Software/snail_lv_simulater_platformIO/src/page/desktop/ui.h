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

#define START_UI_OBJ_X 140 // 280/2=140
#define WIN_INFO_SIZE 3
#define UI_OBJ_NUM 5
    // #define NEW_UI

#ifdef NEW_UI
#define EACH_PAGE_SIZE_X 260
#define EACH_PAGE_SIZE_Y 220
#else
#define EACH_PAGE_SIZE_X 110
#define EACH_PAGE_SIZE_Y 200
#endif

    // 字库解决方法 https://blog.csdn.net/a200327/article/details/120628170
    LV_FONT_DECLARE(sh_number_40);
    LV_FONT_DECLARE(sh_number_50);
    LV_FONT_DECLARE(sh_number_70);
    LV_FONT_DECLARE(sh_number_90);
    LV_FONT_DECLARE(sh_ch_font_14);
    LV_FONT_DECLARE(sh_ch_font_18);

    extern lv_style_t focused_style;
    extern lv_obj_t *desktop_screen;
    extern lv_obj_t *ui_PanelMain;

    extern lv_indev_t *knobs_indev; // 旋钮设备

#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); // 等待动画完成

    void ui_init(lv_indev_t *lv_indev_delete);
    void ui_main_pressed(lv_event_t *e);
    void ui_Button_move_center(int middleIndex);

    static uint8_t lastMiddlePos = 0;

    extern int16_t btnPosXY[WIN_INFO_SIZE][2];

    extern FE_UI_OBJ solderUIObj;

#ifndef NEW_UI
    extern FE_UI_OBJ airhotUIObj;
    extern FE_UI_OBJ hpUIObj;
    extern FE_UI_OBJ adjPowerUIObj;
    extern FE_UI_OBJ sysInfoUIObj;
#endif

    void ui_updateSolderCurTempAndPowerDuty(void);
#ifndef NEW_UI
    void ui_updateAirhotCurTempAndPowerDuty(void);
    void ui_updateHeatplatformCurTempAndPowerDuty(void);
    void ui_updateAdjPowerCurVoltage(void);
    void ui_updateAdjPowerWorkState(void);
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
