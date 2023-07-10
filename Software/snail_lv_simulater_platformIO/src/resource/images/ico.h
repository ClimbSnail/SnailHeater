#ifndef IMAGES_ICO_H
#define IMAGES_ICO_H


#ifdef __cplusplus
extern "C"
{
#endif

#if __has_include("lvgl.h")
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

    extern const lv_img_dsc_t back_16;
    
    extern const lv_img_dsc_t fan;
    extern const lv_img_dsc_t wind_16;
    extern const lv_img_dsc_t setting_ico_32;
    extern const lv_img_dsc_t solder_16;
    extern const lv_img_dsc_t wake_16;
    extern const lv_img_dsc_t heat_platform_48;
    extern const lv_img_dsc_t knobs_16;
    extern const lv_img_dsc_t hardware_16;

    extern const lv_img_dsc_t lighthouse;
    extern const lv_img_dsc_t bridge;
    extern const lv_img_dsc_t coast;
    extern const lv_img_dsc_t naturalScenery;
    extern const lv_img_dsc_t night_view;
    extern const lv_img_dsc_t sunsetGlow;
    
    
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif