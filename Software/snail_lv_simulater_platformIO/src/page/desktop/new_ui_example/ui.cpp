// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.0.5
// LVGL VERSION: 8.2
// PROJECT: SquareLine_Project

#include "./ui.h"

#ifdef NEW_UI_EXAMPLE // 判断使能宏

#include "../desktop_model.h"

lv_style_t focused_style;
static lv_obj_t *desktop_screen;
lv_indev_t *knobs_indev;
static lv_obj_t *ui_PanelMain;
static lv_group_t *main_btn_group = NULL;

void main_screen_init(lv_indev_t *indev)
{
    // desktop_screen
    desktop_screen = lv_obj_create(NULL);

    lv_obj_clear_flag(desktop_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_grad_color(desktop_screen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_style_init(&focused_style);

    lv_style_set_outline_color(&focused_style,
                               lv_palette_main(LV_PALETTE_RED)); // 边框颜色
    lv_style_set_outline_width(&focused_style, 2);               // 边框宽度
    lv_style_set_outline_pad(&focused_style, 1);
    lv_style_set_outline_opa(&focused_style, 255);                     // 透明度
    lv_style_set_border_color(&focused_style, lv_color_hex(0xA90730)); // 边框颜色
    lv_style_set_border_opa(&focused_style, 255);                      // 透明度
    lv_style_set_border_width(&focused_style, 2);                      // 边框宽度
    lv_style_set_radius(&focused_style, 6);
    // 设置控件圆角半径
    // ui_PanelMain
    ui_PanelMain = lv_obj_create(desktop_screen);
    lv_obj_set_size(ui_PanelMain, 280, 240);
    lv_obj_align(ui_PanelMain, LV_ALIGN_CENTER, 0, 0);
    // 设置控件圆角半径
    lv_obj_set_style_radius(ui_PanelMain, 0, LV_STATE_DEFAULT);
    // 设置边框宽度
    lv_obj_set_style_border_width(ui_PanelMain, 0, LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui_PanelMain, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_PanelMain, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_opa(ui_PanelMain, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color_filtered(ui_PanelMain, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 删除操作组
    if (NULL != main_btn_group)
    {
        lv_group_del(main_btn_group);
    }
    main_btn_group = lv_group_create();
    lv_indev_set_group(knobs_indev, main_btn_group);
}

void ui_init(lv_indev_t *indev)
{
    knobs_indev = indev;
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE),
                                              lv_palette_main(LV_PALETTE_RED),
                                              false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    main_screen_init(knobs_indev);
    lv_disp_load_scr(desktop_screen);
}

#endif