// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.0.5
// LVGL VERSION: 8.2
// PROJECT: SquareLine_Project

#ifndef _VERIFICATION_H
#define _VERIFICATION_H

#ifdef __cplusplus
extern "C"
{
#endif

#if __has_include("lvgl.h")
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#define MAIN_INFO_NUM 3

    // 字库解决方法 https://blog.csdn.net/a200327/article/details/120628170
    LV_FONT_DECLARE(sh_ch_font_14);

    extern lv_obj_t *desktop_screen;

    void verification_init(lv_indev_t *indev, const char *userId);
    void verification_release(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
