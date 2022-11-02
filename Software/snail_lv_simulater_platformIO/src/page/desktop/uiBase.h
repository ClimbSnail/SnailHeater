
#ifndef _UI_BASE_H
#define _UI_BASE_H

#ifdef __cplusplus
extern "C"
{
#endif

#if __has_include("lvgl.h")
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

    struct FE_UI_OBJ
    {
        // ui的主体
        lv_obj_t *mainButtonUI;

        // ui的初始化函数
        bool (*ui_init)(lv_obj_t *father);

        void (*ui_release)();

        // 按下选中后触发的回调
        void (*pressed_event_cb)(lv_event_t *e);
    };

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
