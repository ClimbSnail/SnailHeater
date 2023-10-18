#include "../ui_select.h" // 必须添加此行

#ifdef MULTI_UI // 判断使能宏

#ifndef _UI_H
#define _UI_H

#ifdef __cplusplus
extern "C"
{
#endif

#if __has_include("lvgl.h")
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "./uiObjBase.h"
#include "../language_info.h"
#include "resource/images/ico.h"

#define DATA_REFRESH_MS 300 // 数据刷新的时间

#define START_UI_OBJ_X 140 // 280/2=140
#define WIN_INFO_SIZE 3
#define UI_OBJ_NUM 5

#define EACH_PAGE_SIZE_X 110
#define EACH_PAGE_SIZE_Y 200

    // 字库解决方法 https://blog.csdn.net/a200327/article/details/120628170
    LV_FONT_DECLARE(sh_number_40);
    LV_FONT_DECLARE(sh_number_50);
    LV_FONT_DECLARE(sh_number_70);
    LV_FONT_DECLARE(sh_number_90);
    LV_FONT_DECLARE(sh_ch_font_14);
    LV_FONT_DECLARE(sh_ch_font_18);

    extern lv_style_t focused_style;

    extern lv_indev_t *knobs_indev; // 旋钮设备

#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); // 等待动画完成

    void ui_init(lv_indev_t *lv_indev_delete);
    void ui_main_pressed(lv_event_t *e);
    void ui_page_move_center_by_ind(int index); // 后台请求主动页面切换

    static uint8_t lastMiddlePos = 0;

    extern int16_t btnPosXY[WIN_INFO_SIZE][2];

    extern FE_UI_OBJ solderUIObj;
    extern FE_UI_OBJ airhotUIObj;
    extern FE_UI_OBJ hpUIObj;
    extern FE_UI_OBJ adjPowerUIObj;
    extern FE_UI_OBJ sysInfoUIObj;

    void ui_updateSolderData(void);
    void ui_updateAirhotData(void);
    void ui_updateHeatplatformData(void);
    void ui_updateAdjPowerData(void);
    void ui_updateAdjPowerWorkState(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

#endif
