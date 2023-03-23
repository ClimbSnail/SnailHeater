// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.0.5
// LVGL VERSION: 8.2
// PROJECT: SquareLine_Project

#include "ui.h"
#include "desktop_model.h"

#ifdef NEW_UI

// 如果需要启用新的菜单样式，定义这个
#define USE_NEW_MENU

uint8_t currPageIndex = PAGE_INDEX_SOLDER;

// 存放一些布尔参数值，会写入nvs
/*左起
1、白底黑底
2、进入加热台页面自动加热
3、
4、
5、
6、加热台页面曲线图显示网格
7、风枪页面曲线图显示网格
8、烙铁页面曲线图显示网格
*/
uint8_t cfgKey1 = 0b10111100;

///////////////////// VARIABLES ////////////////////
lv_style_t back_btn_focused_style; // 只用在返回按钮上
lv_obj_t *desktop_screen;
lv_obj_t *ui_PanelMain;
lv_obj_t *ui_PanelTop;

#ifdef USE_NEW_MENU
lv_obj_t *rollerMenu;
#define UI_MENU_WIDTH 50
#else
lv_obj_t *ui_PanelMenu;
lv_obj_t *ui_MenuButton[UI_OBJ_NUM];
lv_obj_t *ui_MenuLabel[UI_OBJ_NUM];
#define UI_MENU_WIDTH 40
lv_style_t menu_button_style;
lv_style_t menu_button_focus_style;
#endif

// type1是平时灰色边框灰色字体，选中红色边框红色字体，按下背景变白
lv_style_t btn_type1_style;
lv_style_t btn_type1_focused_style;
lv_style_t btn_type1_pressed_style;

lv_style_t label_text_style; // 用于各种默认标签的样式

lv_style_t bar_style_bg;
lv_style_t bar_style_indic;

lv_obj_t *ui_airHotLabel = NULL;
lv_obj_t *ui_heatPlatLabel = NULL;
lv_obj_t *ui_adjPowerLabel = NULL;
lv_obj_t *ui_solderingLabel = NULL;

static lv_timer_t *topLayerTimer = NULL;
lv_obj_t *ui_topLogoLabel;
lv_obj_t *chartTemp;
lv_chart_series_t *chartSer1;

lv_indev_t *knobs_indev;
lv_group_t *menu_btn_group = NULL;

static FE_FULL_UI_OBJ *ui_Page[UI_OBJ_NUM];

static bool is_menu_show = false;
static lv_anim_t menu_anim;

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
#error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP != 0
#error "#error LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

void top_layer_release(void);

///////////////////// SCREENS ////////////////////
// 测试用
void memory_print(void)
{
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    LV_LOG_USER("[LVGL] Mem used:%d,(%d%),frag:%d%,biggest free:%d\n", (int)mon.total_size - mon.free_size, mon.used_pct, mon.frag_pct, (int)mon.free_biggest_size);
}

static void set_x_pos(void *menu, int32_t v)
{
    lv_obj_align((lv_obj_t *)menu, LV_ALIGN_RIGHT_MID, v, 0);
}

static void set_menu_focus_on(lv_anim_t *a)
{
#ifdef USE_NEW_MENU
    lv_group_focus_obj(rollerMenu);
    lv_roller_set_selected(rollerMenu, currPageIndex, LV_ANIM_OFF);
    lv_indev_set_group(knobs_indev, menu_btn_group);
    lv_group_set_editing(menu_btn_group, true);
#else
    lv_group_focus_obj(ui_MenuButton[currPageIndex]);
    lv_indev_set_group(knobs_indev, menu_btn_group);
#endif
}

static void set_menu_focus_out(lv_anim_t *a)
{
    if (NULL != ui_Page[currPageIndex])
    {
        ui_Page[currPageIndex]->ui_btn_group_init();
    }
}

void hide_menu()
{
    if (is_menu_show)
    {
        is_menu_show = false;
        lv_anim_del(&menu_anim, set_x_pos);
        lv_anim_init(&menu_anim);
        lv_anim_set_exec_cb(&menu_anim, set_x_pos);
        lv_anim_set_values(&menu_anim, 0, UI_MENU_WIDTH);
        lv_anim_set_ready_cb(&menu_anim, set_menu_focus_out);
        lv_anim_set_repeat_count(&menu_anim, 0);
        lv_anim_set_time(&menu_anim, 300);
#ifdef USE_NEW_MENU
        lv_anim_set_var(&menu_anim, rollerMenu);
#else
        lv_anim_set_var(&menu_anim, ui_PanelMenu);
#endif
        lv_anim_start(&menu_anim);
    }
    memory_print();
}

void show_menu()
{
    if (is_menu_show)
    {
        return;
    }
    else
    {
        is_menu_show = true;
        lv_anim_del(&menu_anim, set_x_pos);
        lv_anim_init(&menu_anim);
        lv_anim_set_exec_cb(&menu_anim, set_x_pos);
        lv_anim_set_values(&menu_anim, UI_MENU_WIDTH, 0);
        lv_anim_set_ready_cb(&menu_anim, set_menu_focus_on);
        lv_anim_set_repeat_count(&menu_anim, 0);
        lv_anim_set_time(&menu_anim, 300);
#ifdef USE_NEW_MENU
        lv_anim_set_var(&menu_anim, rollerMenu);
#else
        lv_anim_set_var(&menu_anim, ui_PanelMenu);
#endif
        lv_anim_start(&menu_anim);
    }
}

#ifdef USE_NEW_MENU

static void roller_menu_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    static int16_t mask_top_id = -1;
    static int16_t mask_bottom_id = -1;
    if (code == LV_EVENT_COVER_CHECK)
    {
        lv_event_set_cover_res(e, LV_COVER_RES_MASKED);
    }
    else if (code == LV_EVENT_DRAW_MAIN_BEGIN)
    {
        const lv_font_t *font = lv_obj_get_style_text_font(obj, LV_PART_MAIN);
        lv_coord_t line_space = lv_obj_get_style_text_line_space(obj, LV_PART_MAIN);
        lv_coord_t font_h = lv_font_get_line_height(font);

        lv_area_t roller_coords;
        lv_obj_get_coords(obj, &roller_coords);

        lv_area_t rect_area;
        rect_area.x1 = roller_coords.x1;
        rect_area.x2 = roller_coords.x2;
        rect_area.y1 = roller_coords.y1;
        rect_area.y2 = roller_coords.y1 + (lv_obj_get_height(obj) - font_h - line_space) / 2;

        lv_draw_mask_fade_param_t *fade_mask_top = (lv_draw_mask_fade_param_t *)lv_mem_buf_get(sizeof(lv_draw_mask_fade_param_t));
        lv_draw_mask_fade_init(fade_mask_top, &rect_area, LV_OPA_50, rect_area.y1, LV_OPA_COVER, rect_area.y2);
        mask_top_id = lv_draw_mask_add(fade_mask_top, NULL);

        rect_area.y1 = rect_area.y2 + font_h + line_space - 1;
        rect_area.y2 = roller_coords.y2;

        lv_draw_mask_fade_param_t *fade_mask_bottom = (lv_draw_mask_fade_param_t *)lv_mem_buf_get(sizeof(lv_draw_mask_fade_param_t));
        lv_draw_mask_fade_init(fade_mask_bottom, &rect_area, LV_OPA_COVER, rect_area.y1, LV_OPA_50, rect_area.y2);
        mask_bottom_id = lv_draw_mask_add(fade_mask_bottom, NULL);
    }
    else if (code == LV_EVENT_DRAW_POST_END)
    {
        lv_draw_mask_fade_param_t *fade_mask_top = (lv_draw_mask_fade_param_t *)lv_draw_mask_remove_id(mask_top_id);
        lv_draw_mask_fade_param_t *fade_mask_bottom = (lv_draw_mask_fade_param_t *)lv_draw_mask_remove_id(mask_bottom_id);
        lv_draw_mask_free_param(fade_mask_top);
        lv_draw_mask_free_param(fade_mask_bottom);
        lv_mem_buf_release(fade_mask_top);
        lv_mem_buf_release(fade_mask_bottom);
        mask_top_id = -1;
        mask_bottom_id = -1;
    }
    else if (code == LV_EVENT_VALUE_CHANGED)
    {
        uint16_t index = lv_roller_get_selected(rollerMenu);
        if (currPageIndex != index)
        {
            ui_Page[currPageIndex]->ui_release();
            currPageIndex = index;
            ui_Page[currPageIndex]->ui_init(ui_PanelMain);
        }
        hide_menu();
    }
}
#else
static void ui_menu_btn_solder_pressed(lv_event_t *e)
{
    if (currPageIndex != PAGE_INDEX_SOLDER)
    {
        ui_Page[currPageIndex]->ui_release();
        currPageIndex = PAGE_INDEX_SOLDER;
        ui_Page[currPageIndex]->ui_init(ui_PanelMain);
    }
    hide_menu();
}

static void ui_menu_btn_airhot_pressed(lv_event_t *e)
{
    if (currPageIndex != PAGE_INDEX_AIR_HOT)
    {
        ui_Page[currPageIndex]->ui_release();
        currPageIndex = PAGE_INDEX_AIR_HOT;
        ui_Page[currPageIndex]->ui_init(ui_PanelMain);
    }
    hide_menu();
}

static void ui_menu_btn_heatplat_pressed(lv_event_t *e)
{
    if (currPageIndex != PAGE_INDEX_HEAT_PLAT)
    {
        ui_Page[currPageIndex]->ui_release();
        currPageIndex = PAGE_INDEX_HEAT_PLAT;
        ui_Page[currPageIndex]->ui_init(ui_PanelMain);
    }
    hide_menu();
}

static void ui_menu_btn_adjpower_pressed(lv_event_t *e)
{
    if (currPageIndex != PAGE_INDEX_ADJ_POWER)
    {
        ui_Page[currPageIndex]->ui_release();
        currPageIndex = PAGE_INDEX_ADJ_POWER;
        ui_Page[currPageIndex]->ui_init(ui_PanelMain);
    }
    hide_menu();
}

static void ui_menu_btn_setting_pressed(lv_event_t *e)
{
    if (currPageIndex != PAGE_INDEX_SETTING)
    {
        ui_Page[currPageIndex]->ui_release();
        currPageIndex = PAGE_INDEX_SETTING;
        ui_Page[currPageIndex]->ui_init(ui_PanelMain);
    }
    hide_menu();
}
#endif

void theme_color_init()
{
    if (IS_WHITE_THEME)
    {
        lv_style_set_text_color(&label_text_style, lv_color_hex(0x666666));
#ifdef USE_NEW_MENU
        lv_obj_set_style_bg_color(rollerMenu, lv_color_hex(0xf0f0f0), LV_PART_MAIN);
        lv_obj_set_style_text_color(rollerMenu, lv_color_hex(0x666666), LV_PART_MAIN);
#endif
    }
    else
    {
        lv_style_set_text_color(&label_text_style, lv_color_hex(0xa7a8a9));
#ifdef USE_NEW_MENU
        lv_obj_set_style_bg_color(rollerMenu, lv_color_hex(0x444444), LV_PART_MAIN);
        lv_obj_set_style_text_color(rollerMenu, lv_color_hex(0xa7a8a9), LV_PART_MAIN);
#endif
    }

    switch (currPageIndex)
    {
    case PAGE_INDEX_SOLDER:
#ifdef USE_NEW_MENU
        lv_obj_set_style_bg_color(rollerMenu, SOLDER_THEME_COLOR1, LV_PART_SELECTED);
#else
        lv_style_set_bg_color(&menu_button_focus_style, SOLDER_THEME_COLOR1);
#endif
        lv_style_set_bg_color(&back_btn_focused_style, SOLDER_THEME_COLOR1);
        lv_style_set_border_color(&btn_type1_focused_style, SOLDER_THEME_COLOR1);
        lv_style_set_text_color(&btn_type1_focused_style, SOLDER_THEME_COLOR1);
        lv_style_set_border_color(&btn_type1_pressed_style, SOLDER_THEME_COLOR1);
        lv_style_set_text_color(&btn_type1_pressed_style, SOLDER_THEME_COLOR1);
        break;
    case PAGE_INDEX_AIR_HOT:
#ifdef USE_NEW_MENU
        lv_obj_set_style_bg_color(rollerMenu, AIR_HOT_THEME_COLOR1, LV_PART_SELECTED);
#else
        lv_style_set_bg_color(&menu_button_focus_style, AIR_HOT_THEME_COLOR1);
#endif
        lv_style_set_bg_color(&back_btn_focused_style, AIR_HOT_THEME_COLOR1);
        lv_style_set_border_color(&btn_type1_focused_style, AIR_HOT_THEME_COLOR1);
        lv_style_set_text_color(&btn_type1_focused_style, AIR_HOT_THEME_COLOR1);
        lv_style_set_border_color(&btn_type1_pressed_style, AIR_HOT_THEME_COLOR1);
        lv_style_set_text_color(&btn_type1_pressed_style, AIR_HOT_THEME_COLOR1);
        break;
    case PAGE_INDEX_HEAT_PLAT:
#ifdef USE_NEW_MENU
        lv_obj_set_style_bg_color(rollerMenu, HEAT_PLAT_THEME_COLOR1, LV_PART_SELECTED);
#else
        lv_style_set_bg_color(&menu_button_focus_style, HEAT_PLAT_THEME_COLOR1);
#endif
        lv_style_set_bg_color(&back_btn_focused_style, HEAT_PLAT_THEME_COLOR1);
        lv_style_set_border_color(&btn_type1_focused_style, HEAT_PLAT_THEME_COLOR1);
        lv_style_set_text_color(&btn_type1_focused_style, HEAT_PLAT_THEME_COLOR1);
        lv_style_set_border_color(&btn_type1_pressed_style, HEAT_PLAT_THEME_COLOR1);
        lv_style_set_text_color(&btn_type1_pressed_style, HEAT_PLAT_THEME_COLOR1);
        break;
    case PAGE_INDEX_ADJ_POWER:
#ifdef USE_NEW_MENU
        lv_obj_set_style_bg_color(rollerMenu, ADJ_POWER_THEME_COLOR1, LV_PART_SELECTED);
#else
        lv_style_set_bg_color(&menu_button_focus_style, ADJ_POWER_THEME_COLOR1);
#endif
        lv_style_set_bg_color(&back_btn_focused_style, ADJ_POWER_THEME_COLOR1);
        lv_style_set_border_color(&btn_type1_focused_style, ADJ_POWER_THEME_COLOR1);
        lv_style_set_text_color(&btn_type1_focused_style, ADJ_POWER_THEME_COLOR1);
        lv_style_set_border_color(&btn_type1_pressed_style, ADJ_POWER_THEME_COLOR1);
        lv_style_set_text_color(&btn_type1_pressed_style, ADJ_POWER_THEME_COLOR1);
        break;
    case PAGE_INDEX_SETTING:
#ifdef USE_NEW_MENU
        lv_obj_set_style_bg_color(rollerMenu, SETTING_THEME_COLOR1, LV_PART_SELECTED);
#else
        lv_style_set_bg_color(&menu_button_focus_style, SETTING_THEME_COLOR1);
#endif
        lv_style_set_bg_color(&back_btn_focused_style, SETTING_THEME_COLOR1);
        lv_style_set_border_color(&btn_type1_focused_style, SETTING_THEME_COLOR1);
        lv_style_set_text_color(&btn_type1_focused_style, SETTING_THEME_COLOR1);
        lv_style_set_border_color(&btn_type1_pressed_style, SETTING_THEME_COLOR1);
        lv_style_set_text_color(&btn_type1_pressed_style, SETTING_THEME_COLOR1);
        break;
    default:
        // 默认用烙铁的
#ifdef USE_NEW_MENU
        lv_obj_set_style_bg_color(rollerMenu, SOLDER_THEME_COLOR1, LV_PART_SELECTED);
#else
        lv_style_set_bg_color(&menu_button_focus_style, SOLDER_THEME_COLOR1);
#endif
        lv_style_set_bg_color(&back_btn_focused_style, SOLDER_THEME_COLOR1);
        lv_style_set_border_color(&btn_type1_focused_style, SOLDER_THEME_COLOR1);
        lv_style_set_text_color(&btn_type1_focused_style, SOLDER_THEME_COLOR1);
        lv_style_set_border_color(&btn_type1_pressed_style, SOLDER_THEME_COLOR1);
        lv_style_set_text_color(&btn_type1_pressed_style, SOLDER_THEME_COLOR1);
        break;
    }
}

void update_top_info()
{
    if (currPageIndex == PAGE_INDEX_SETTING)
        return;
    // 顶部状态栏
    uint8_t x_pos = 70;
    bool isWhiteTheme = IS_WHITE_THEME;

    for (uint8_t ind = 0; ind < UI_OBJ_NUM; ind++)
    {
        if (ind != currPageIndex)
        {
            switch (ind)
            {
            case PAGE_INDEX_SOLDER:
                lv_label_set_text_fmt(ui_solderingLabel, "%3d", solderModel.curTemp);
                break;
            case PAGE_INDEX_AIR_HOT:
                lv_label_set_text_fmt(ui_airHotLabel, "%3d", airhotModel.curTemp);
                break;
            case PAGE_INDEX_HEAT_PLAT:
                lv_label_set_text_fmt(ui_heatPlatLabel, "%3d", heatplatformModel.curTemp);
                break;
            case PAGE_INDEX_ADJ_POWER:
                lv_label_set_text_fmt(ui_adjPowerLabel, "%.1lf", adjPowerModel.voltage / 1000.0);
                break;
            case PAGE_INDEX_SETTING:
                break;
            default:
                break;
            }
            x_pos += 50;
        }
    }
}

static void toplayTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    update_top_info(); // 更新数据
}

void top_layer_init()
{
    top_layer_release();
    bool isWhiteTheme = IS_WHITE_THEME;
    lv_obj_set_style_bg_color(ui_PanelTop, isWhiteTheme ? lv_color_hex(0xf9f9f9) : lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clean(ui_PanelTop);
    ui_topLogoLabel = lv_label_create(ui_PanelTop);
    lv_obj_set_size(ui_topLogoLabel, 50, 24);
    lv_label_set_text(ui_topLogoLabel, modeName[currPageIndex]);
    lv_obj_align(ui_topLogoLabel, LV_ALIGN_TOP_LEFT, 10, 0);
    lv_obj_set_style_text_color(ui_topLogoLabel, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_topLogoLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_topLogoLabel, &FontDeyi_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_color_t theme_color;
    switch (currPageIndex)
    {
    case PAGE_INDEX_SOLDER:
        theme_color = SOLDER_THEME_COLOR1;
        break;
    case PAGE_INDEX_AIR_HOT:
        theme_color = AIR_HOT_THEME_COLOR1;
        break;
    case PAGE_INDEX_HEAT_PLAT:
        theme_color = HEAT_PLAT_THEME_COLOR1;
        break;
    case PAGE_INDEX_ADJ_POWER:
        theme_color = ADJ_POWER_THEME_COLOR1;
        break;
    case PAGE_INDEX_SETTING:
        theme_color = SETTING_THEME_COLOR1;
        break;
    default:
        theme_color = SOLDER_THEME_COLOR1;
        break;
    }
    lv_obj_set_style_bg_color(ui_topLogoLabel, theme_color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_topLogoLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_topLogoLabel, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_topLogoLabel, LV_TEXT_ALIGN_CENTER, 0);

    if (currPageIndex == PAGE_INDEX_SETTING)
        return;
    // 顶部状态栏
    uint8_t x_pos = 70;

    for (uint8_t i = 0; i < UI_OBJ_NUM; i++)
    {
        if (i != currPageIndex)
        {
            switch (i)
            {
            case PAGE_INDEX_SOLDER:
                ui_solderingLabel = lv_label_create(ui_PanelTop);
                lv_obj_set_size(ui_solderingLabel, 40, 24);
                lv_label_set_text_fmt(ui_solderingLabel, "%3d", solderModel.curTemp);
                lv_obj_align(ui_solderingLabel, LV_ALIGN_TOP_LEFT, x_pos, 0);
                lv_obj_set_style_pad_top(ui_solderingLabel, 5, 0);
                lv_obj_set_style_text_align(ui_solderingLabel, LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_set_style_text_color(ui_solderingLabel, SOLDER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_opa(ui_solderingLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_font(ui_solderingLabel, &FontJost_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(ui_solderingLabel, isWhiteTheme ? lv_color_hex(0xeeeeee) : BLACK_THEME_CHART_COLOR2, 0);
                lv_obj_set_style_bg_opa(ui_solderingLabel, 255, 0);
                break;
            case PAGE_INDEX_AIR_HOT:
                ui_airHotLabel = lv_label_create(ui_PanelTop);
                lv_obj_set_size(ui_airHotLabel, 40, 24);
                lv_label_set_text_fmt(ui_airHotLabel, "%3d", airhotModel.curTemp);
                lv_obj_align(ui_airHotLabel, LV_ALIGN_TOP_LEFT, x_pos, 0);
                lv_obj_set_style_pad_top(ui_airHotLabel, 5, 0);
                lv_obj_set_style_text_align(ui_airHotLabel, LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_set_style_text_color(ui_airHotLabel, AIR_HOT_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_opa(ui_airHotLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_font(ui_airHotLabel, &FontJost_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(ui_airHotLabel, isWhiteTheme ? lv_color_hex(0xeeeeee) : BLACK_THEME_CHART_COLOR2, 0);
                lv_obj_set_style_bg_opa(ui_airHotLabel, 255, 0);
                break;
            case PAGE_INDEX_HEAT_PLAT:
                ui_heatPlatLabel = lv_label_create(ui_PanelTop);
                lv_obj_set_size(ui_heatPlatLabel, 40, 24);
                lv_label_set_text_fmt(ui_heatPlatLabel, "%3d", heatplatformModel.curTemp);
                lv_obj_align(ui_heatPlatLabel, LV_ALIGN_TOP_LEFT, x_pos, 0);
                lv_obj_set_style_pad_top(ui_heatPlatLabel, 5, 0);
                lv_obj_set_style_text_align(ui_heatPlatLabel, LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_set_style_text_color(ui_heatPlatLabel, HEAT_PLAT_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_opa(ui_heatPlatLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_font(ui_heatPlatLabel, &FontJost_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(ui_heatPlatLabel, isWhiteTheme ? lv_color_hex(0xeeeeee) : BLACK_THEME_CHART_COLOR2, 0);
                lv_obj_set_style_bg_opa(ui_heatPlatLabel, 255, 0);
                break;
            case PAGE_INDEX_ADJ_POWER:
                ui_adjPowerLabel = lv_label_create(ui_PanelTop);
                lv_obj_set_size(ui_adjPowerLabel, 40, 24);
                lv_label_set_text_fmt(ui_adjPowerLabel, "%.1lf", adjPowerModel.voltage / 1000.0);
                lv_obj_align(ui_adjPowerLabel, LV_ALIGN_TOP_LEFT, x_pos, 0);
                lv_obj_set_style_pad_top(ui_adjPowerLabel, 5, 0);
                lv_obj_set_style_text_align(ui_adjPowerLabel, LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_set_style_text_color(ui_adjPowerLabel, ADJ_POWER_THEME_COLOR1, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_opa(ui_adjPowerLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_font(ui_adjPowerLabel, &FontJost_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(ui_adjPowerLabel, isWhiteTheme ? lv_color_hex(0xeeeeee) : BLACK_THEME_CHART_COLOR2, 0);
                lv_obj_set_style_bg_opa(ui_adjPowerLabel, 255, 0);
                break;
            case PAGE_INDEX_SETTING:
                break;
            default:
                break;
            }
            x_pos += 50;
        }
    }

    topLayerTimer = lv_timer_create(toplayTimer_timeout, 1000, NULL);
    lv_timer_set_repeat_count(topLayerTimer, -1);
}

void top_layer_release(void)
{
    if (NULL != ui_solderingLabel)
    {
        lv_obj_del(ui_solderingLabel);
        ui_solderingLabel = NULL;
    }
    if (NULL != ui_airHotLabel)
    {
        lv_obj_del(ui_airHotLabel);
        ui_airHotLabel = NULL;
    }
    if (NULL != ui_heatPlatLabel)
    {
        lv_obj_del(ui_heatPlatLabel);
        ui_heatPlatLabel = NULL;
    }
    if (NULL != ui_adjPowerLabel)
    {
        lv_obj_del(ui_adjPowerLabel);
        ui_adjPowerLabel = NULL;
    }
    if (NULL != topLayerTimer)
    {
        lv_timer_del(topLayerTimer);
        topLayerTimer = NULL;
    }
}

#ifdef USE_NEW_MENU
void roller_menu_init()
{
    rollerMenu = lv_roller_create(desktop_screen);
    lv_roller_set_options(rollerMenu,
                          "烙铁\n"
                          "风枪\n"
                          "热台\n"
                          "电源\n"
                          "设置",
                          LV_ROLLER_MODE_INFINITE);

    lv_obj_set_style_text_font(rollerMenu, &FontDeyi_16, 0);
    lv_roller_set_visible_row_count(rollerMenu, 5);
    lv_obj_set_width(rollerMenu, UI_MENU_WIDTH);
    lv_obj_set_height(rollerMenu, 240);
    lv_obj_set_style_border_width(rollerMenu, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(rollerMenu, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rollerMenu, IS_WHITE_THEME ? lv_color_hex(0xf0f0f0) : lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_text_color(rollerMenu, IS_WHITE_THEME ? lv_color_hex(0x666666) : lv_color_hex(0xa7a8a9), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rollerMenu, 255, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rollerMenu, 255, LV_PART_SELECTED);
    lv_obj_set_style_text_color(rollerMenu, lv_color_hex(0xffffff), LV_PART_SELECTED);
    lv_obj_set_style_text_font(rollerMenu, &FontDeyi_20, LV_PART_SELECTED);
    lv_obj_set_style_outline_width(rollerMenu, 0, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_width(rollerMenu, 0, LV_PART_MAIN | LV_STATE_EDITED);
    lv_obj_set_style_text_line_space(rollerMenu, 30, 0);

    lv_obj_align(rollerMenu, LV_ALIGN_RIGHT_MID, UI_MENU_WIDTH, 0);
    lv_obj_add_event_cb(rollerMenu, roller_menu_event_handler, LV_EVENT_ALL, NULL);

    menu_btn_group = lv_group_create();

    lv_group_add_obj(menu_btn_group, rollerMenu);
}
#else
void menu_init()
{
    // 初始化导航按键
    ui_PanelMenu = lv_obj_create(desktop_screen);
    lv_obj_remove_style_all(ui_PanelMenu);
    lv_obj_set_size(ui_PanelMenu, 40, 240);
    lv_obj_align(ui_PanelMenu, LV_ALIGN_RIGHT_MID, 40, 0);
    lv_obj_set_style_radius(ui_PanelMenu, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_PanelMenu, 0, 0);
    lv_obj_set_style_border_width(ui_PanelMenu, 0, LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui_PanelMenu, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_PanelMenu, lv_color_hex(0xeeeeee), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_PanelMenu, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_style_init(&menu_button_style);
    lv_style_set_text_align(&menu_button_style, LV_TEXT_ALIGN_CENTER);
    // lv_style_set_bg_opa(&menu_button_style, LV_OPA_COVER);
    // lv_style_set_bg_color(&menu_button_style, lv_color_hex(0xe1e4eb));
    lv_style_set_text_color(&menu_button_style, lv_color_hex(0x5d5d5f));
    lv_style_init(&menu_button_focus_style);
    lv_style_set_bg_opa(&menu_button_focus_style, LV_OPA_COVER);
    lv_style_set_text_color(&menu_button_focus_style, lv_color_hex(0xffffff));

    ui_MenuButton[PAGE_INDEX_SOLDER] = lv_btn_create(ui_PanelMenu);
    lv_obj_remove_style_all(ui_MenuButton[PAGE_INDEX_SOLDER]);
    lv_obj_set_size(ui_MenuButton[PAGE_INDEX_SOLDER], 40, 40);
    lv_obj_align(ui_MenuButton[PAGE_INDEX_SOLDER], LV_ALIGN_TOP_LEFT, 0, 24);
    lv_obj_add_style(ui_MenuButton[PAGE_INDEX_SOLDER], &menu_button_style, LV_PART_MAIN);
    lv_obj_add_style(ui_MenuButton[PAGE_INDEX_SOLDER], &menu_button_focus_style, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_flag(ui_MenuButton[PAGE_INDEX_SOLDER], LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_MenuButton[PAGE_INDEX_SOLDER], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_font(ui_MenuButton[PAGE_INDEX_SOLDER], &FontDeyi_16, LV_PART_MAIN);

    ui_MenuLabel[PAGE_INDEX_SOLDER] = lv_label_create(ui_MenuButton[PAGE_INDEX_SOLDER]);
    lv_obj_center(ui_MenuLabel[PAGE_INDEX_SOLDER]);
    lv_label_set_text(ui_MenuLabel[PAGE_INDEX_SOLDER], "烙铁");

    ui_MenuButton[PAGE_INDEX_AIR_HOT] = lv_btn_create(ui_PanelMenu);
    lv_obj_remove_style_all(ui_MenuButton[PAGE_INDEX_AIR_HOT]);
    lv_obj_set_size(ui_MenuButton[PAGE_INDEX_AIR_HOT], 40, 40);
    lv_obj_align(ui_MenuButton[PAGE_INDEX_AIR_HOT], LV_ALIGN_TOP_LEFT, 0, 65);
    lv_obj_add_style(ui_MenuButton[PAGE_INDEX_AIR_HOT], &menu_button_style, LV_PART_MAIN);
    lv_obj_add_style(ui_MenuButton[PAGE_INDEX_AIR_HOT], &menu_button_focus_style, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_flag(ui_MenuButton[PAGE_INDEX_AIR_HOT], LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_MenuButton[PAGE_INDEX_AIR_HOT], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_font(ui_MenuButton[PAGE_INDEX_AIR_HOT], &FontDeyi_16, LV_PART_MAIN);

    ui_MenuLabel[PAGE_INDEX_AIR_HOT] = lv_label_create(ui_MenuButton[PAGE_INDEX_AIR_HOT]);
    lv_obj_center(ui_MenuLabel[PAGE_INDEX_AIR_HOT]);
    lv_label_set_text(ui_MenuLabel[PAGE_INDEX_AIR_HOT], "风枪");

    ui_MenuButton[PAGE_INDEX_HEAT_PLAT] = lv_btn_create(ui_PanelMenu);
    lv_obj_remove_style_all(ui_MenuButton[PAGE_INDEX_HEAT_PLAT]);
    lv_obj_set_size(ui_MenuButton[PAGE_INDEX_HEAT_PLAT], 40, 40);
    lv_obj_align(ui_MenuButton[PAGE_INDEX_HEAT_PLAT], LV_ALIGN_TOP_LEFT, 0, 106);
    lv_obj_add_style(ui_MenuButton[PAGE_INDEX_HEAT_PLAT], &menu_button_style, LV_PART_MAIN);
    lv_obj_add_style(ui_MenuButton[PAGE_INDEX_HEAT_PLAT], &menu_button_focus_style, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_flag(ui_MenuButton[PAGE_INDEX_HEAT_PLAT], LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_MenuButton[PAGE_INDEX_HEAT_PLAT], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_font(ui_MenuButton[PAGE_INDEX_HEAT_PLAT], &FontDeyi_16, LV_PART_MAIN);

    ui_MenuLabel[PAGE_INDEX_HEAT_PLAT] = lv_label_create(ui_MenuButton[PAGE_INDEX_HEAT_PLAT]);
    lv_obj_center(ui_MenuLabel[PAGE_INDEX_HEAT_PLAT]);
    lv_label_set_text(ui_MenuLabel[PAGE_INDEX_HEAT_PLAT], "热台");

    ui_MenuButton[PAGE_INDEX_ADJ_POWER] = lv_btn_create(ui_PanelMenu);
    lv_obj_remove_style_all(ui_MenuButton[PAGE_INDEX_ADJ_POWER]);
    lv_obj_set_size(ui_MenuButton[PAGE_INDEX_ADJ_POWER], 40, 40);
    lv_obj_align(ui_MenuButton[PAGE_INDEX_ADJ_POWER], LV_ALIGN_TOP_LEFT, 0, 147);
    lv_obj_add_style(ui_MenuButton[PAGE_INDEX_ADJ_POWER], &menu_button_style, LV_PART_MAIN);
    lv_obj_add_style(ui_MenuButton[PAGE_INDEX_ADJ_POWER], &menu_button_focus_style, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_flag(ui_MenuButton[PAGE_INDEX_ADJ_POWER], LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_MenuButton[PAGE_INDEX_ADJ_POWER], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_font(ui_MenuButton[PAGE_INDEX_ADJ_POWER], &FontDeyi_16, LV_PART_MAIN);

    ui_MenuLabel[PAGE_INDEX_ADJ_POWER] = lv_label_create(ui_MenuButton[PAGE_INDEX_ADJ_POWER]);
    lv_obj_center(ui_MenuLabel[PAGE_INDEX_ADJ_POWER]);
    lv_label_set_text(ui_MenuLabel[PAGE_INDEX_ADJ_POWER], "电源");

    ui_MenuButton[PAGE_INDEX_SETTING] = lv_btn_create(ui_PanelMenu);
    lv_obj_remove_style_all(ui_MenuButton[PAGE_INDEX_SETTING]);
    lv_obj_set_size(ui_MenuButton[PAGE_INDEX_SETTING], 40, 40);
    lv_obj_align(ui_MenuButton[PAGE_INDEX_SETTING], LV_ALIGN_TOP_LEFT, 0, 188);
    lv_obj_add_style(ui_MenuButton[PAGE_INDEX_SETTING], &menu_button_style, LV_PART_MAIN);
    lv_obj_add_style(ui_MenuButton[PAGE_INDEX_SETTING], &menu_button_focus_style, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_flag(ui_MenuButton[PAGE_INDEX_SETTING], LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_MenuButton[PAGE_INDEX_SETTING], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_font(ui_MenuButton[PAGE_INDEX_SETTING], &FontDeyi_16, LV_PART_MAIN);

    ui_MenuLabel[PAGE_INDEX_SETTING] = lv_label_create(ui_MenuButton[PAGE_INDEX_SETTING]);
    lv_obj_center(ui_MenuLabel[PAGE_INDEX_SETTING]);
    lv_label_set_text(ui_MenuLabel[PAGE_INDEX_SETTING], "设置");

    lv_obj_add_event_cb(ui_MenuButton[PAGE_INDEX_SOLDER], ui_menu_btn_solder_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_MenuButton[PAGE_INDEX_AIR_HOT], ui_menu_btn_airhot_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_MenuButton[PAGE_INDEX_HEAT_PLAT], ui_menu_btn_heatplat_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_MenuButton[PAGE_INDEX_ADJ_POWER], ui_menu_btn_adjpower_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_MenuButton[PAGE_INDEX_SETTING], ui_menu_btn_setting_pressed, LV_EVENT_PRESSED, NULL);

    menu_btn_group = lv_group_create();
    for (uint8_t i = 0; i < UI_OBJ_NUM; i++)
    {
        lv_group_add_obj(menu_btn_group, ui_MenuButton[i]);
    }
}
#endif

void main_screen_init(lv_indev_t *indev)
{
    bool isWhiteTheme = IS_WHITE_THEME;
    desktop_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(desktop_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(desktop_screen, isWhiteTheme ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_style_init(&back_btn_focused_style);
    lv_style_set_text_color(&back_btn_focused_style, lv_color_white());
    lv_style_set_bg_opa(&back_btn_focused_style, LV_OPA_COVER);

    // 带颜色边框的按钮样式，通常用在快捷设定温度上
    lv_style_init(&btn_type1_style);
    lv_style_set_radius(&btn_type1_style, 4);
    lv_style_set_border_width(&btn_type1_style, 1);
    lv_style_set_text_opa(&btn_type1_style, 255);
    lv_style_set_text_font(&btn_type1_style, &FontJost_14);
    lv_style_set_border_color(&btn_type1_style, lv_color_hex(0x666666));
    lv_style_set_text_color(&btn_type1_style, lv_color_hex(0x989798));

    lv_style_init(&btn_type1_focused_style);
    lv_style_set_radius(&btn_type1_focused_style, 4);
    lv_style_set_border_width(&btn_type1_focused_style, 1);

    lv_style_init(&btn_type1_pressed_style);
    lv_style_set_radius(&btn_type1_pressed_style, 4);
    lv_style_set_border_width(&btn_type1_pressed_style, 1);
    lv_style_set_bg_color(&btn_type1_pressed_style, isWhiteTheme ? lv_color_black() : lv_color_white());
    lv_style_set_bg_opa(&btn_type1_pressed_style, LV_OPA_COVER);

    lv_style_init(&bar_style_bg);
    lv_style_init(&bar_style_indic);

    lv_style_set_bg_color(&bar_style_bg, lv_color_hex3(0xccc));
    lv_style_set_radius(&bar_style_bg, 0);
    lv_style_set_anim_time(&bar_style_bg, 400);
    lv_style_set_bg_opa(&bar_style_indic, LV_OPA_COVER);
    lv_style_set_radius(&bar_style_indic, 0);
    lv_style_set_bg_grad_dir(&bar_style_indic, LV_GRAD_DIR_HOR);

    lv_style_set_bg_color(&bar_style_indic, SOLDER_THEME_COLOR2);
    lv_style_set_bg_grad_color(&bar_style_indic, SOLDER_THEME_COLOR1);

    lv_style_init(&label_text_style);
    lv_style_set_text_font(&label_text_style, &FontDeyi_16);
    lv_style_set_text_color(&label_text_style, isWhiteTheme ? lv_color_hex(0x666666) : lv_color_hex(0xa7a8a9));

    ui_PanelMain = lv_obj_create(desktop_screen);
    lv_obj_remove_style_all(ui_PanelMain);
    lv_obj_set_size(ui_PanelMain, 280, 216);
    lv_obj_align(ui_PanelMain, LV_ALIGN_CENTER, 0, 12);
    lv_obj_set_style_radius(ui_PanelMain, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_PanelMain, 0, 0);
    lv_obj_set_style_border_width(ui_PanelMain, 0, LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui_PanelMain, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_PanelMain, isWhiteTheme ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Page[0] = &solderUIObj;
    ui_Page[1] = &airhotUIObj;
    ui_Page[2] = &hpUIObj;
    ui_Page[3] = &adjPowerUIObj;
    ui_Page[4] = &settingUIObj;

    ui_PanelTop = lv_obj_create(desktop_screen);
    lv_obj_remove_style_all(ui_PanelTop);
    lv_obj_clear_flag(ui_PanelTop, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(ui_PanelTop, 280, 24);
    lv_obj_align(ui_PanelTop, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_set_style_bg_color(ui_PanelTop, isWhiteTheme ? lv_color_hex(0xf9f9f9) : lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_PanelTop, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

#ifdef USE_NEW_MENU
    roller_menu_init();
#else
    menu_init();
#endif

    bool ret_code = ui_Page[currPageIndex]->ui_init(ui_PanelMain);
    // 页面初始化后直接焦点，不再多按一次
    ui_Page[currPageIndex]->ui_btn_group_init();
    lv_scr_load(desktop_screen);
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

void ui_release()
{
    top_layer_release();
}

void ui_page_move_center_by_ind(int index)
{
    // uint16_t index = lv_roller_get_selected(rollerMenu);
    // todo 待排查
    // if (currPageIndex != index)
    // {
    //     ui_Page[currPageIndex]->ui_release();
    //     currPageIndex = index;
    //     ui_Page[currPageIndex]->ui_init(ui_PanelMain);
    // }
    // hide_menu();
}

#endif