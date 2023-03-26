#include "../ui_select.h" // 必须添加此行

#ifdef NEW_UI_EXAMPLE // 判断使能宏

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

#include "../language_info.h"
#include "../desktop_model.h"
#include "resource/images/ico.h"

#define DATA_REFRESH_MS 300 // 数据刷新的时间

    // 字库解决方法 https://blog.csdn.net/a200327/article/details/120628170
    LV_FONT_DECLARE(sh_number_40);

    extern lv_indev_t *knobs_indev; // 旋钮设备

    void ui_init(lv_indev_t *lv_indev_delete);

    // void ui_updateSolderCurTempAndPowerDuty(void);
    // void ui_updateAirhotCurTempAndPowerDuty(void);
    // void ui_updateHeatplatformCurTempAndPowerDuty(void);
    // void ui_updateAdjPowerCurVoltage(void);
    // void ui_updateAdjPowerWorkState(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

#endif
