
#ifndef STARTUP_H
#define STARTUP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); //等待动画完成

    void startupInitScreen(void);
    void startupUI(const char *sw_ver, const char *hw_ver);
    void startupDel(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_settings;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif