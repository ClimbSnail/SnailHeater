#include "startup.h"

#include "lvgl.h"

static lv_obj_t *screen1 = NULL;

void startupAnim(lv_obj_t *obj)
{
    /* 初始化动画
     *-----------------------*/
    lv_anim_t a;
    lv_anim_init(&a);
    /* 必选设置
     *------------------*/
    /* 设置“动画制作”功能 */
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    /* 设置“动画制作”功能 */
    lv_anim_set_var(&a, obj);
    /* 动画时长[ms] */
    lv_anim_set_time(&a, 500);
    /* 设置开始和结束值。例如。 0、150 */
    lv_anim_set_values(&a, lv_obj_get_y(obj), 100);

    lv_anim_set_path_cb(&a, lv_anim_path_overshoot); // 设置一个动画的路径
    // /* 可选设置
    //  *------------------*/
    // /* 开始动画之前的等待时间[ms] */
    // lv_anim_set_delay(&a, delay);
    // /* 设置路径（曲线）。默认为线性 */
    // lv_anim_set_path(&a, &path);
    // /* 设置一个回调以在动画准备好时调用。 */
    // lv_anim_set_ready_cb(&a, ready_cb);
    // /* 设置在动画开始时（延迟后）调用的回调。 */
    // lv_anim_set_start_cb(&a, start_cb);
    // /* 在此持续时间内，也向后播放动画。默认值为0（禁用）[ms] */
    // lv_anim_set_playback_time(&a, wait_time);
    // /* 播放前延迟。默认值为0（禁用）[ms] */
    // lv_anim_set_playback_delay(&a, wait_time);
    // /* 重复次数。默认值为1。LV_ANIM_REPEAT_INFINIT用于无限重复 */
    // lv_anim_set_repeat_count(&a, wait_time);
    // /* 重复之前要延迟。默认值为0（禁用）[ms] */
    // lv_anim_set_repeat_delay(&a, wait_time);

    /* true（默认）：立即应用开始值，false：延迟设置动画后再应用开始值。真正开始。 */
    lv_anim_set_early_apply(&a, true);
    /* 应用动画效果
     *------------------*/
    lv_anim_start(&a); /* 应用动画效果 */
}

void startupInitScreen()
{
    if (NULL == screen1)
    {
        screen1 = lv_obj_create(NULL);
        lv_obj_clear_flag(screen1, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(screen1, lv_color_black(), 0);
        lv_scr_load(screen1);
    }
}

void startupUI(const char *sw_ver, const char *hw_ver)
{
    lv_obj_t *hostName = lv_label_create(screen1);
    lv_obj_set_size(hostName, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(hostName, 0, 0);
    lv_obj_set_align(hostName, LV_ALIGN_TOP_MID);
    lv_obj_set_style_text_color(hostName, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(hostName, &lv_font_montserrat_28, 0);
    lv_label_set_recolor(hostName, true); // 先得使能文本重绘色功能
    lv_label_set_text(hostName, "#FFFFFF SnailHeater#");

    lv_obj_t *sw_version = lv_label_create(screen1);
    lv_obj_set_size(sw_version, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_align(sw_version, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_set_pos(sw_version, 20, -5);
    lv_label_set_recolor(sw_version, true); // 先得使能文本重绘色功能
    lv_obj_set_style_text_font(sw_version, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(sw_version, "#FF0000 SW_Ver %s#", sw_ver);

    lv_obj_t *hw_version = lv_label_create(screen1);
    lv_obj_set_size(hw_version, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_align(hw_version, LV_ALIGN_BOTTOM_RIGHT);
    lv_obj_set_pos(hw_version, -20, -5);
    lv_label_set_recolor(hw_version, true); // 先得使能文本重绘色功能
    lv_obj_set_style_text_font(hw_version, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(hw_version, "#FF0000 HW_Ver %s#", hw_ver);

    startupAnim(hostName);

    ANIEND
}

void startupDel(void)
{
    if (NULL != screen1)
    {
        lv_obj_clean(screen1);
        screen1 = NULL;
    }
}