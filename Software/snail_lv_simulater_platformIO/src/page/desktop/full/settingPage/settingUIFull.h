#include "../../ui_select.h" // 必须添加此行

#ifdef FULL_UI // 判断使能宏

#ifndef _SETTING_UI_FULL_H
#define _SETTING_UI_FULL_H

#ifdef __cplusplus
extern "C"
{
#endif

#if __has_include("lvgl.h")
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); // 等待动画完成

    extern lv_obj_t *ui_subBackBtn;
    extern lv_group_t *sub_btn_group;

    void ui_tab_back_btn_pressed(lv_event_t *e);

    void ui_sys_setting_init(lv_obj_t *father);
    void ui_solder_setting_init(lv_obj_t *father);
    void ui_airhot_setting_init(lv_obj_t *father);
    void ui_hp_setting_init(lv_obj_t *father);
    void ui_adjpwr_setting_init(lv_obj_t *father);

    void ui_sys_setting_release(void *param = NULL);
    void ui_solder_setting_release(void *param = NULL);
    void ui_airhot_setting_release(void *param = NULL);
    void ui_hp_setting_release(void *param = NULL);
    void ui_adjpwr_setting_release(void *param = NULL);

    void ui_sys_setting_init_group(lv_obj_t *father);
    void ui_solder_setting_init_group(lv_obj_t *father);
    void ui_airhot_setting_init_group(lv_obj_t *father);
    void ui_hp_setting_init_group(lv_obj_t *father);
    void ui_adjpwr_setting_init_group(lv_obj_t *father);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

#endif
