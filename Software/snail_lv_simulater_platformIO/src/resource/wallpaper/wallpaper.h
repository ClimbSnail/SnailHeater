#ifndef WALLPAPER_H
#define WALLPAPER_H

#ifdef __cplusplus
extern "C"
{
#endif

#if __has_include("lvgl.h")
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#if SH_HARDWARE_VER >= SH_ESP32S2_WROOM_V26
    extern const lv_img_dsc_t lighthouse_320x240;
    extern const lv_img_dsc_t bridge_320x240;
    extern const lv_img_dsc_t coast_320x240;
    extern const lv_img_dsc_t naturalScenery_320x240;
    extern const lv_img_dsc_t nightView_320x240;
    extern const lv_img_dsc_t sunsetGlow_320x240;
#else
    extern const lv_img_dsc_t lighthouse_280x240;
    extern const lv_img_dsc_t bridge_280x240;
    extern const lv_img_dsc_t coast_280x240;
    extern const lv_img_dsc_t naturalScenery_280x240;
    extern const lv_img_dsc_t nightView_280x240;
    extern const lv_img_dsc_t sunsetGlow_280x240;
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif