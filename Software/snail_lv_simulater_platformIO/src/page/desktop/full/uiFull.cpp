#include "./ui.h"

#ifdef FULL_UI // 判断使能宏

#include "../desktop_model.h"

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
lv_style_t back_btn_style;          // 只用在返回按钮上
lv_style_t back_btn_focused_style;  // 只用在返回按钮上
lv_style_t black_white_theme_style; // 全局黑白样式
static lv_obj_t *desktop_screen = NULL;
static lv_obj_t *ui_PanelMain;
lv_obj_t *ui_PanelTop;
lv_obj_t *ui_PanelTopBgImg;

static lv_timer_t *sysInfoTimer = NULL;  // 系统消息窗的定时器
static lv_obj_t *sysInfoLabel = NULL;    // 系统消息
static lv_group_t *pre_group = NULL;     // 临时保存之前的按键组
static lv_obj_t *lockScreenImage = NULL; // 锁屏壁纸
static int lockScreenImageIndex = 0;     // 锁屏壁纸播放的下标
#define WALLPAPER_NUM 6
static int wallpaperSwitchLeaveTime = 0; // 锁屏切换的打点器

uint32_t lastTimeStamp = 0;

// // 壁纸列表
// #if SH_HARDWARE_VER >= SH_ESP32S2_WROOM_V26
// static lv_img_dsc_t wallpaperList[] = {lighthouse_320x240, bridge_320x240,
//                                        coast_320x240, naturalScenery_320x240,
//                                        nightView_320x240, sunsetGlow_320x240};
// #else
// static lv_img_dsc_t wallpaperList[] = {lighthouse_280x240, bridge_280x240,
//                                        coast_280x240, naturalScenery_280x240,
//                                        nightView_280x240, sunsetGlow_280x240};
// #endif

lv_obj_t *ui_backBtn = NULL;

typedef enum
{
    MENU_HIDE,
    MENU_SHOW,
    MENU_HIDING,
    MENU_SHOWING,
} MENU_STATUS_ENUM;

#if USE_MENU_STYLE == 3
lv_obj_t *arcMenu;
static lv_obj_t *arcMenuLabel[ARC_MENU_NUM];
lv_style_t menu_button_style;
lv_style_t menu_button_checked_style;
lv_style_t menu_arc_style;
#endif
#if USE_MENU_STYLE == 2
lv_obj_t *rollerMenu;
#define UI_MENU_WIDTH 50
#endif
#if USE_MENU_STYLE == 1
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

static lv_obj_t *ui_topLabel1 = NULL;
static lv_obj_t *ui_topLabel2 = NULL;
static lv_obj_t *ui_topLabel3 = NULL;

static lv_timer_t *topLayerTimer = NULL;
static lv_timer_t *autoChangePageTimer = NULL;
static uint8_t targerChangePageInd = PAGE_INDEX_MAXSIZE;
lv_obj_t *ui_topLogoLabel;
lv_obj_t *chartTemp;
lv_chart_series_t *chartSer1;

lv_indev_t *knobs_indev;
static lv_group_t *menu_btn_group = NULL;

static FE_UI_OBJ *ui_Page[PAGE_INDEX_MAXSIZE];

static MENU_STATUS_ENUM menu_status = MENU_HIDE;
static lv_anim_t menu_anim;

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
#error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP != 0
#error "#error LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

static void sysInfoTimer_timeout(lv_timer_t *timer);

void ui_setIntellectRateFlag(lv_event_t *e)
{
    return;
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_user_data(e);

    if (LV_EVENT_PRESSED == event_code || LV_EVENT_DEFOCUSED == event_code)
    {
        LV_LOG_USER("ui_setIntellectRateFlag false event_code = %u\n", event_code);
        sysInfoModel.setIntellectRateFlag(false);
    }
    else if (LV_EVENT_FOCUSED == event_code || LV_EVENT_KEY == event_code)
    {
        LV_LOG_USER("ui_setIntellectRateFlag true event_code = %u\n", event_code);
        sysInfoModel.setIntellectRateFlag(true);
    }
}

///////////////////// SCREENS ////////////////////
// 测试用
void memory_print(void)
{
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    LV_LOG_USER("[LVGL] Mem used:%d(%d%),frag:%d%,biggest free:%d\n", (int)mon.total_size - mon.free_size, mon.used_pct, mon.frag_pct, (int)mon.free_biggest_size);
}

static void set_x_pos(void *menu, int32_t v)
{
    lv_obj_align((lv_obj_t *)menu, LV_ALIGN_RIGHT_MID, v, 0);
}

static void set_bg_opa(void *menu, int32_t v)
{
#if USE_MENU_STYLE == 3
    lv_obj_set_style_bg_opa(lv_layer_top(), v * 0.8, 0);
    lv_obj_set_style_arc_opa(arcMenu, v, LV_PART_MAIN);
    for (uint8_t i = 0; i < ARC_MENU_NUM; i++)
    {
        lv_obj_set_style_text_opa(arcMenuLabel[i], v, 0);
    }
#endif
}

static void set_menu_focus_on(lv_anim_t *a)
{
#if USE_MENU_STYLE == 3
    lv_indev_set_group(knobs_indev, menu_btn_group);
    lv_group_focus_obj(arcMenu);
    lv_group_set_editing(menu_btn_group, true);
#endif
#if USE_MENU_STYLE == 2
    lv_roller_set_selected(rollerMenu, currPageIndex, LV_ANIM_OFF);
    lv_indev_set_group(knobs_indev, menu_btn_group);
    lv_group_focus_obj(rollerMenu);
    lv_group_set_editing(menu_btn_group, true);
#endif
#if USE_MENU_STYLE == 1
    lv_group_focus_obj(ui_MenuButton[currPageIndex]);
    lv_indev_set_group(knobs_indev, menu_btn_group);
#endif

    menu_status = MENU_SHOW;
}

static void set_menu_focus_out(lv_anim_t *a)
{
    if (NULL != ui_Page[currPageIndex])
    {
        ui_Page[currPageIndex]->selected_event_cb(NULL);
    }
    menu_status = MENU_HIDE;
}

void hide_arc_menu()
{
    if (MENU_SHOW == menu_status)
    {
        menu_status = MENU_HIDING;
        // 开启按钮全局加速
        sysInfoModel.setIntellectRateFlag(true);

#if USE_MENU_STYLE == 3
        lv_anim_del(&menu_anim, set_bg_opa);
        lv_anim_init(&menu_anim);
        lv_anim_set_exec_cb(&menu_anim, set_bg_opa);
        lv_anim_set_values(&menu_anim, 255, 0);
        lv_anim_set_ready_cb(&menu_anim, set_menu_focus_out);
        lv_anim_set_deleted_cb(&menu_anim, set_menu_focus_out);
        lv_anim_set_repeat_count(&menu_anim, 0);
        lv_anim_set_time(&menu_anim, 300);
        lv_anim_set_var(&menu_anim, arcMenu);
        lv_anim_start(&menu_anim);
#endif
#if USE_MENU_STYLE == 2
        lv_anim_del(&menu_anim, set_x_pos);
        lv_anim_init(&menu_anim);
        lv_anim_set_exec_cb(&menu_anim, set_x_pos);
        lv_anim_set_values(&menu_anim, 0, UI_MENU_WIDTH);
        lv_anim_set_ready_cb(&menu_anim, set_menu_focus_out);
        lv_anim_set_repeat_count(&menu_anim, 0);
        lv_anim_set_time(&menu_anim, 300);
        lv_anim_set_var(&menu_anim, rollerMenu);
        lv_anim_start(&menu_anim);
#endif
#if USE_MENU_STYLE == 1
        lv_anim_del(&menu_anim, set_x_pos);
        lv_anim_init(&menu_anim);
        lv_anim_set_exec_cb(&menu_anim, set_x_pos);
        lv_anim_set_values(&menu_anim, 0, UI_MENU_WIDTH);
        lv_anim_set_ready_cb(&menu_anim, set_menu_focus_out);
        lv_anim_set_repeat_count(&menu_anim, 0);
        lv_anim_set_time(&menu_anim, 300);
        lv_anim_set_var(&menu_anim, ui_PanelMenu);
        lv_anim_start(&menu_anim);
#endif
    }
    // memory_print();
}

void show_arc_menu()
{
    if (MENU_HIDE == menu_status)
    {
        menu_status = MENU_SHOWING;
        // 关闭按钮全局加速
        sysInfoModel.setIntellectRateFlag(false);

        // #ifndef SIMULATOR
        //         lastTimeStamp = (uint32_t)GET_SYS_MILLIS();
        //         LV_LOG_USER("show_arc_menu lastTimeStamp = %lu\n",
        //                     lastTimeStamp);
        // #endif

#if USE_MENU_STYLE == 3
        lv_anim_del(&menu_anim, set_bg_opa);
        lv_anim_init(&menu_anim);
        lv_anim_set_exec_cb(&menu_anim, set_bg_opa);
        lv_anim_set_values(&menu_anim, 0, 255);
        lv_anim_set_ready_cb(&menu_anim, set_menu_focus_on);
        lv_anim_set_deleted_cb(&menu_anim, set_menu_focus_on);
        lv_anim_set_repeat_count(&menu_anim, 0);
        lv_anim_set_time(&menu_anim, 300);
        lv_anim_set_var(&menu_anim, arcMenu);
        lv_anim_start(&menu_anim);
        // if (lastTimeStamp > 99221709767064)
        // {
        //     lastTimeStamp = 0;
        // }
        // UART_PORT.printf("getState lastTimeStamp = %lu\n", lastTimeStamp);
#endif
#if USE_MENU_STYLE == 2
        lv_anim_del(&menu_anim, set_x_pos);
        lv_anim_init(&menu_anim);
        lv_anim_set_exec_cb(&menu_anim, set_x_pos);
        lv_anim_set_values(&menu_anim, UI_MENU_WIDTH, 0);
        lv_anim_set_ready_cb(&menu_anim, set_menu_focus_on);
        lv_anim_set_repeat_count(&menu_anim, 0);
        lv_anim_set_time(&menu_anim, 300);
        lv_anim_set_var(&menu_anim, rollerMenu);
        lv_anim_start(&menu_anim);
#endif
#if USE_MENU_STYLE == 1
        lv_anim_del(&menu_anim, set_x_pos);
        lv_anim_init(&menu_anim);
        lv_anim_set_exec_cb(&menu_anim, set_x_pos);
        lv_anim_set_values(&menu_anim, UI_MENU_WIDTH, 0);
        lv_anim_set_ready_cb(&menu_anim, set_menu_focus_on);
        lv_anim_set_repeat_count(&menu_anim, 0);
        lv_anim_set_time(&menu_anim, 300);
        lv_anim_set_var(&menu_anim, ui_PanelMenu);
        lv_anim_start(&menu_anim);
#endif
    }
}

#if USE_MENU_STYLE == 2

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
        hide_arc_menu();
    }
}
#endif
#if USE_MENU_STYLE == 1
static void ui_menu_btn_solder_pressed(lv_event_t *e)
{
    if (currPageIndex != PAGE_INDEX_SOLDER)
    {
        ui_Page[currPageIndex]->ui_release();
        currPageIndex = PAGE_INDEX_SOLDER;
        ui_Page[currPageIndex]->ui_init(ui_PanelMain);
    }
    hide_arc_menu();
}

static void ui_menu_btn_airhot_pressed(lv_event_t *e)
{
    if (currPageIndex != PAGE_INDEX_AIR_HOT)
    {
        ui_Page[currPageIndex]->ui_release();
        currPageIndex = PAGE_INDEX_AIR_HOT;
        ui_Page[currPageIndex]->ui_init(ui_PanelMain);
    }
    hide_arc_menu();
}

static void ui_menu_btn_heatplat_pressed(lv_event_t *e)
{
    if (currPageIndex != PAGE_INDEX_HEAT_PLAT)
    {
        ui_Page[currPageIndex]->ui_release();
        currPageIndex = PAGE_INDEX_HEAT_PLAT;
        ui_Page[currPageIndex]->ui_init(ui_PanelMain);
    }
    hide_arc_menu();
}

static void ui_menu_btn_adjpower_pressed(lv_event_t *e)
{
    if (currPageIndex != PAGE_INDEX_ADJ_POWER)
    {
        ui_Page[currPageIndex]->ui_release();
        currPageIndex = PAGE_INDEX_ADJ_POWER;
        ui_Page[currPageIndex]->ui_init(ui_PanelMain);
    }
    hide_arc_menu();
}

static void ui_menu_btn_setting_pressed(lv_event_t *e)
{
    if (currPageIndex != PAGE_INDEX_SETTING)
    {
        ui_Page[currPageIndex]->ui_release();
        currPageIndex = PAGE_INDEX_SETTING;
        ui_Page[currPageIndex]->ui_init(ui_PanelMain);
    }
    hide_arc_menu();
}
#endif

void theme_color_init()
{
#if USE_MENU_STYLE == 3
    lv_obj_set_style_bg_color(arcMenu, theme_color1[currPageIndex], LV_PART_KNOB); // 这个的颜色就是选中菜单的颜色
#endif
#if USE_MENU_STYLE == 2
    lv_obj_set_style_bg_color(rollerMenu, theme_color1[currPageIndex], LV_PART_SELECTED);
#endif
#if USE_MENU_STYLE == 1
    lv_style_set_bg_color(&menu_button_focus_style, theme_color1[currPageIndex]);
#endif
    lv_style_set_bg_color(&back_btn_focused_style, theme_color1[currPageIndex]);
    lv_style_set_border_color(&btn_type1_focused_style, theme_color1[currPageIndex]);
    lv_style_set_text_color(&btn_type1_focused_style, theme_color1[currPageIndex]);
    lv_style_set_border_color(&btn_type1_pressed_style, theme_color1[currPageIndex]);
    lv_style_set_text_color(&btn_type1_pressed_style, theme_color1[currPageIndex]);
}

// 设置温度标签文本(批量处理未连接的情况)
void set_temp_label_text(lv_obj_t *lb, int temp)
{
    if (temp < DISCONNCT_TEMP)
        lv_label_set_text_fmt(lb, "%d", temp);
    else
        lv_label_set_text(lb, "NA");
}

void update_top_info()
{
    // 顶部状态栏
    // 测试数据，到时候删除
    // solderModel.curTemp = lv_rand(250, 380);
    // airhotModel.curTemp = lv_rand(SH_SCREEN_WIDTH, 360);
    // heatplatformModel.curTemp = lv_rand(150, 250);
    // adjPowerModel.voltage = lv_rand(8000, 24000);

    switch (currPageIndex)
    {
    case PAGE_INDEX_SOLDER:
        set_temp_label_text(ui_topLabel1, airhotModel.curTemp);
        lv_obj_set_style_text_color(ui_topLabel1, AIR_HOT_THEME_COLOR1, 0);
        set_temp_label_text(ui_topLabel2, heatplatformModel.curTemp);
        lv_obj_set_style_text_color(ui_topLabel2, HEAT_PLAT_THEME_COLOR1, 0);
        if (ENABLE_STATE_CLOSE == adjPowerModel.workState)
            lv_label_set_text_fmt(ui_topLabel3, "OFF");
        else
            lv_label_set_text_fmt(ui_topLabel3, "%.1lf", adjPowerModel.voltage / 1000.0);
        lv_obj_set_style_text_color(ui_topLabel3, ADJ_POWER_THEME_COLOR1, 0);
        break;
    case PAGE_INDEX_AIR_HOT:
        set_temp_label_text(ui_topLabel1, solderModel.curTemp);
        lv_obj_set_style_text_color(ui_topLabel1, SOLDER_THEME_COLOR1, 0);
        set_temp_label_text(ui_topLabel2, heatplatformModel.curTemp);
        lv_obj_set_style_text_color(ui_topLabel2, HEAT_PLAT_THEME_COLOR1, 0);
        if (ENABLE_STATE_CLOSE == adjPowerModel.workState)
            lv_label_set_text_fmt(ui_topLabel3, "OFF");
        else
            lv_label_set_text_fmt(ui_topLabel3, "%.1lf", adjPowerModel.voltage / 1000.0);
        lv_obj_set_style_text_color(ui_topLabel3, ADJ_POWER_THEME_COLOR1, 0);
        break;
    case PAGE_INDEX_HEAT_PLAT:
        set_temp_label_text(ui_topLabel1, solderModel.curTemp);
        lv_obj_set_style_text_color(ui_topLabel1, SOLDER_THEME_COLOR1, 0);
        set_temp_label_text(ui_topLabel2, airhotModel.curTemp);
        lv_obj_set_style_text_color(ui_topLabel2, AIR_HOT_THEME_COLOR1, 0);
        if (ENABLE_STATE_CLOSE == adjPowerModel.workState)
            lv_label_set_text_fmt(ui_topLabel3, "OFF");
        else
            lv_label_set_text_fmt(ui_topLabel3, "%.1lf", adjPowerModel.voltage / 1000.0);
        lv_obj_set_style_text_color(ui_topLabel3, ADJ_POWER_THEME_COLOR1, 0);
        break;
    case PAGE_INDEX_ADJ_POWER:
        set_temp_label_text(ui_topLabel1, solderModel.curTemp);
        lv_obj_set_style_text_color(ui_topLabel1, SOLDER_THEME_COLOR1, 0);
        set_temp_label_text(ui_topLabel2, airhotModel.curTemp);
        lv_obj_set_style_text_color(ui_topLabel2, AIR_HOT_THEME_COLOR1, 0);
        set_temp_label_text(ui_topLabel3, heatplatformModel.curTemp);
        lv_obj_set_style_text_color(ui_topLabel3, HEAT_PLAT_THEME_COLOR1, 0);
        break;
    case PAGE_INDEX_SETTING:
        break;
    default:
        break;
    }
}

static void toplayTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    update_top_info(); // 更新数据
}

void top_layer_set_name()
{
    // 进入不同的页面的时候更新文本
    lv_label_set_text(ui_topLogoLabel, modeName[currPageIndex]);
    lv_obj_set_style_text_color(ui_topLogoLabel, theme_color1[currPageIndex], LV_PART_MAIN | LV_STATE_DEFAULT);
    if (currPageIndex == PAGE_INDEX_SETTING)
    {
        lv_obj_add_flag(ui_PanelTopBgImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_topLabel1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_topLabel2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_topLabel3, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_clear_flag(ui_PanelTopBgImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_topLabel1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_topLabel2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_topLabel3, LV_OBJ_FLAG_HIDDEN);
    }
}
static void ui_back_btn_pressed(lv_event_t *e)
{
    // 返回项被按下
    LV_LOG_USER("LOOK\n");
    show_arc_menu();
}

void top_layer_init(lv_obj_t *parent)
{
    // 只在第一次调用

    ui_PanelTop = lv_obj_create(desktop_screen);
    lv_obj_remove_style_all(ui_PanelTop);
    lv_obj_clear_flag(ui_PanelTop, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(ui_PanelTop, SH_SCREEN_WIDTH, 24);
    lv_obj_align(ui_PanelTop, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_opa(ui_PanelTop, 0, 0);

    //  顶部状态栏
    ui_topLogoLabel = lv_label_create(ui_PanelTop);
    lv_obj_set_size(ui_topLogoLabel, 40, 24);
    lv_obj_align(ui_topLogoLabel, LV_ALIGN_TOP_LEFT, 22, 0);
    lv_obj_set_style_radius(ui_topLogoLabel, 10, 0);
    lv_obj_set_style_text_font(ui_topLogoLabel, &FontDeyi_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_topLogoLabel, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_topLogoLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(ui_topLogoLabel, 0, 0);

    ui_PanelTopBgImg = lv_img_create(ui_PanelTop);
    lv_img_set_src(ui_PanelTopBgImg, IS_WHITE_THEME ? &img_top_bar_white : &img_top_bar_black);
    lv_obj_center(ui_PanelTopBgImg);

    uint8_t x_pos = -48;
    ui_topLabel1 = lv_label_create(ui_PanelTopBgImg);
    lv_obj_set_size(ui_topLabel1, 47, 24);
    lv_obj_align(ui_topLabel1, LV_ALIGN_TOP_MID, -48, 0);
    lv_obj_set_style_pad_top(ui_topLabel1, 4, 0);
    lv_obj_set_style_text_align(ui_topLabel1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(ui_topLabel1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_topLabel1, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 顶上的分割线，实测好像不太好看，取消了
    // lv_obj_set_style_border_width(ui_topLabel1,1,0);
    // lv_obj_set_style_border_side(ui_topLabel1,LV_BORDER_SIDE_RIGHT,0);
    // lv_obj_set_style_border_color(ui_topLabel1,isWhiteTheme ? lv_color_hex(0xe0e0e0) : lv_color_hex(0x666666),0);

    ui_topLabel2 = lv_label_create(ui_PanelTopBgImg);
    lv_obj_set_size(ui_topLabel2, 47, 24);
    lv_obj_align(ui_topLabel2, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_pad_top(ui_topLabel2, 4, 0);
    lv_obj_set_style_text_align(ui_topLabel2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(ui_topLabel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_topLabel2, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 顶上的分割线，实测好像不太好看，取消了
    // lv_obj_set_style_border_width(ui_topLabel2,1,0);
    // lv_obj_set_style_border_side(ui_topLabel2,LV_BORDER_SIDE_RIGHT,0);
    // lv_obj_set_style_border_color(ui_topLabel2,isWhiteTheme ? lv_color_hex(0xe0e0e0) : lv_color_hex(0x666666),0);
    x_pos += 48;

    ui_topLabel3 = lv_label_create(ui_PanelTopBgImg);
    lv_obj_set_size(ui_topLabel3, 47, 24);
    lv_obj_align(ui_topLabel3, LV_ALIGN_TOP_MID, 48, 0);
    lv_obj_set_style_pad_top(ui_topLabel3, 4, 0);
    lv_obj_set_style_text_align(ui_topLabel3, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_opa(ui_topLabel3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_topLabel3, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_backBtn = lv_btn_create(ui_PanelTop);
    lv_obj_remove_style_all(ui_backBtn);
    lv_obj_align(ui_backBtn, LV_ALIGN_TOP_RIGHT, -10, 0);
    lv_obj_add_flag(ui_backBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_backBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_backBtn, &back_btn_style, LV_STATE_DEFAULT);
    lv_obj_add_style(ui_backBtn, &back_btn_focused_style, LV_STATE_FOCUSED);

    lv_obj_t *ui_backBtnLabel = lv_label_create(ui_backBtn);
    lv_obj_center(ui_backBtnLabel);
    lv_label_set_text(ui_backBtnLabel, LV_SYMBOL_LEFT);

    lv_obj_add_event_cb(ui_backBtn, ui_back_btn_pressed, LV_EVENT_PRESSED, NULL);

    topLayerTimer = lv_timer_create(toplayTimer_timeout, 1000, NULL);
    lv_timer_set_repeat_count(topLayerTimer, -1);
}

#if USE_MENU_STYLE == 3
static void arc_menu_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_KEY
        // && menu_status == MENU_SHOW
    )
    {
        uint16_t index = lv_arcmenu_get_value(arcMenu);
        // #ifndef SIMULATOR
        //         // if (
        //         //     // index != PAGE_INDEX_AIR_HOT ||
        //         //     GET_SYS_MILLIS() - lastTimeStamp > 300)

        //         if (true)
        // #endif

        {
            lv_obj_t *obj = lv_event_get_target(e);
            char c = *((char *)lv_event_get_param(e));
            if (c == LV_KEY_ENTER)
            {
                // 退出编辑模式并且隐藏菜单
                if (currPageIndex != index &&
                    ui_Page[index] != NULL)
                {
                    ui_Page[currPageIndex]->ui_release();
                    currPageIndex = index;
                    ui_Page[currPageIndex]->ui_init(ui_PanelMain);
                }
                // #ifndef SIMULATOR
                //                 LV_LOG_USER("if lastTimeStamp = %lu, nowTimeStamp = %lu\n",
                //                             lastTimeStamp, GET_SYS_MILLIS());
                // #endif
                hide_arc_menu();
            }
        }
        // #ifndef SIMULATOR
        //         else
        //         {
        //             LV_LOG_USER("else lastTimeStamp = %lu, nowTimeStamp = %lu\n",
        //                         lastTimeStamp, GET_SYS_MILLIS());
        //         }
        // #endif
    }
    else if (code == LV_EVENT_VALUE_CHANGED)
    {
        lv_obj_t *obj = lv_event_get_target(e);
        uint8_t ind = lv_arcmenu_get_value(obj);
        for (uint8_t i = 0; i < ARC_MENU_NUM; i++)
        {
            if (ind != i)
                lv_obj_clear_state(arcMenuLabel[i], LV_STATE_CHECKED);
            else
                lv_obj_add_state(arcMenuLabel[i], LV_STATE_CHECKED);
        }
    }
}

void arc_menu_init()
{
    arcMenu = lv_arcmenu_create(lv_layer_top());
    lv_obj_align(arcMenu, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(arcMenu, ARC_MENU_WIDTH, ARC_MENU_HEIGHT);
    lv_arcmenu_set_anim_time(arcMenu, 300);
    lv_arcmenu_set_max_angle(arcMenu, 360);         // 菜单形状，全圆环，360,半圆180
    lv_arcmenu_set_item_num(arcMenu, ARC_MENU_NUM); // 菜单个数
    uint16_t begin_angle = 270 - (180 / ARC_MENU_NUM);
    lv_arcmenu_set_rotation(arcMenu, begin_angle); // 计算起始角度，保证始终在正上方

    // lv_arcmenu_set_type(arcMenu, LV_ARCMENU_MODE1);
    // lv_arcmenu_set_item_pointer_index(arcMenu, 2); //选中项是第3项（0起） ,仅用在MODE2
    lv_obj_clear_flag(arcMenu, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(arcMenu, ARC_MENU_ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arcMenu, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arcMenu, lv_color_hex(0x303030), LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(arcMenu, false, LV_PART_MAIN);

    lv_obj_remove_style(arcMenu, NULL, LV_PART_KNOB);
    // 这个的颜色就是选中菜单的颜色
    lv_obj_set_style_bg_color(arcMenu, theme_color1[currPageIndex], LV_PART_KNOB);

    for (uint8_t i = 0; i < ARC_MENU_NUM; i++)
    {
        arcMenuLabel[i] = lv_label_create(arcMenu);
        lv_label_set_text(arcMenuLabel[i], modeName[i]);
        // lv_obj_set_style_text_align(arcMenuLabel[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(arcMenuLabel[i], &FontDeyi_16, 0);
        lv_obj_set_style_text_color(arcMenuLabel[i], lv_color_hex(0xaaaaaa), LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(arcMenuLabel[i], lv_color_hex(0xffffff), LV_STATE_CHECKED);
        lv_obj_set_style_text_opa(arcMenuLabel[i], LV_OPA_TRANSP, 0);
    }
    lv_arcmenu_set_item_map(arcMenu, arcMenuLabel, ARC_MENU_NUM, 0);

    // 这两个事件的先后顺序很重要，先绑定的先触发
    // lv_obj_add_event_cb(lv_layer_top(), top_layer_event_handler, LV_EVENT_KEY, NULL);
    // lv_obj_add_event_cb(arcMenu, arc_menu_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(arcMenu, arc_menu_event_handler, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(arcMenu, arc_menu_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    menu_btn_group = lv_group_create();
    lv_group_add_obj(menu_btn_group, arcMenu);
}
#endif
#if USE_MENU_STYLE == 2
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
    lv_obj_set_style_bg_color(rollerMenu, IS_WHITE_THEME ? WHITE_THEME_ROLLERMENU_BG_COLOR : BLACK_THEME_ROLLERMENU_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_color(rollerMenu, IS_WHITE_THEME ? WHITE_THEME_ROLLERMENU_TEXT_COLOR : BLACK_THEME_ROLLERMENU_TEXT_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rollerMenu, 255, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rollerMenu, 255, LV_PART_SELECTED);
    lv_obj_set_style_text_color(rollerMenu, ROLLERMENU_ACTIVE_TEXT_COLOR, LV_PART_SELECTED);
    lv_obj_set_style_text_font(rollerMenu, &FontDeyi_20, LV_PART_SELECTED);
    lv_obj_set_style_outline_width(rollerMenu, 0, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_width(rollerMenu, 0, LV_PART_MAIN | LV_STATE_EDITED);
    lv_obj_set_style_text_line_space(rollerMenu, 30, 0);

    lv_obj_align(rollerMenu, LV_ALIGN_RIGHT_MID, UI_MENU_WIDTH, 0);
    lv_obj_add_event_cb(rollerMenu, roller_menu_event_handler, LV_EVENT_ALL, NULL);

    menu_btn_group = lv_group_create();

    lv_group_add_obj(menu_btn_group, rollerMenu);
}
#endif
#if USE_MENU_STYLE == 1
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

static void autoChangePage_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    if (targerChangePageInd != PAGE_INDEX_MAXSIZE)
    {
        if (currPageIndex != targerChangePageInd &&
            currPageIndex != PAGE_INDEX_SETTING &&
            ui_Page[targerChangePageInd] != NULL)
        {
            ui_Page[currPageIndex]->ui_release();
            currPageIndex = targerChangePageInd;
            ui_Page[currPageIndex]->ui_init(ui_PanelMain);
            // 设置菜单项
#if USE_MENU_STYLE == 3
            lv_arcmenu_set_value(arcMenu, currPageIndex, 100);
#endif
        }
        targerChangePageInd = PAGE_INDEX_MAXSIZE; // 清楚标志
        hide_arc_menu();
    }
}

void main_screen_init(lv_indev_t *indev)
{
    bool isWhiteTheme = IS_WHITE_THEME;
    desktop_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(desktop_screen, LV_OBJ_FLAG_SCROLLABLE);

    lv_style_init(&black_white_theme_style);
    lv_style_set_bg_color(&black_white_theme_style, isWhiteTheme ? lv_color_white() : lv_color_black());
    lv_obj_add_style(desktop_screen, &black_white_theme_style, 0);

    lv_style_init(&back_btn_style);
    lv_style_set_width(&back_btn_style, 46);
    lv_style_set_height(&back_btn_style, 24);
    lv_style_set_text_color(&back_btn_style, BTN_BACK_TEXT_COLOR);
    lv_style_set_radius(&back_btn_style, 10);
    lv_style_set_bg_opa(&back_btn_style, 0);

    lv_style_init(&back_btn_focused_style);
    lv_style_set_text_color(&back_btn_focused_style, lv_color_white());
    lv_style_set_bg_opa(&back_btn_focused_style, LV_OPA_COVER);

    // 带颜色边框的按钮样式，通常用在快捷设定温度上
    lv_style_init(&btn_type1_style);
    lv_style_set_radius(&btn_type1_style, 4);
    lv_style_set_border_width(&btn_type1_style, 1);
    lv_style_set_text_opa(&btn_type1_style, 255);
    lv_style_set_text_font(&btn_type1_style, &FontJost_14);
    lv_style_set_border_color(&btn_type1_style, BTN_TYPE1_BORDER_COLOR);
    lv_style_set_text_color(&btn_type1_style, BTN_TYPE1_TEXT_COLOR);

    lv_style_init(&btn_type1_focused_style);
    lv_style_set_radius(&btn_type1_focused_style, 4);
    lv_style_set_border_width(&btn_type1_focused_style, 2);

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
    lv_style_set_text_color(&label_text_style, isWhiteTheme ? WHITE_THEME_LABEL_TEXT_COLOR : BLACK_THEME_LABEL_TEXT_COLOR);

    ui_PanelMain = lv_obj_create(desktop_screen);
    lv_obj_remove_style_all(ui_PanelMain);
    lv_obj_set_size(ui_PanelMain, SH_SCREEN_WIDTH, 216);
    lv_obj_align(ui_PanelMain, LV_ALIGN_CENTER, 0, 12);
    lv_obj_set_style_radius(ui_PanelMain, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui_PanelMain, 0, 0);
    lv_obj_set_style_border_width(ui_PanelMain, 0, LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui_PanelMain, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(ui_PanelMain, 0, 0);

    ui_Page[PAGE_INDEX_SOLDER] = &solderUIObj;
    ui_Page[PAGE_INDEX_AIR_HOT] = &airhotUIObj;
    ui_Page[PAGE_INDEX_HEAT_PLAT] = &hpUIObj;
    ui_Page[PAGE_INDEX_ADJ_POWER] = &adjPowerUIObj;
    ui_Page[PAGE_INDEX_SPOTWELDER] = &spotWelderUIObj;
    ui_Page[PAGE_INDEX_SIGNAL] = &signalUIObj;
    ui_Page[PAGE_INDEX_SETTING] = &settingUIObj;

    top_layer_init(desktop_screen);

#if USE_MENU_STYLE == 3
    lv_obj_add_style(lv_layer_top(), &black_white_theme_style, 0);
    lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);
    arc_menu_init();
#endif
#if USE_MENU_STYLE == 2
    roller_menu_init();
#endif
#if USE_MENU_STYLE == 1
    menu_init();
#endif

    bool ret_code = ui_Page[currPageIndex]->ui_init(ui_PanelMain);
    // 页面初始化后直接焦点，不再多按一次
    // todo 由init时初始化，故删除此操作
    // ui_Page[currPageIndex]->selected_event_cb(NULL);

    autoChangePageTimer = lv_timer_create(autoChangePage_timeout, 1000, NULL);
    lv_timer_set_repeat_count(autoChangePageTimer, -1);

    sysInfoTimer = lv_timer_create(sysInfoTimer_timeout, DATA_REFRESH_MS, NULL);
    lv_timer_set_repeat_count(sysInfoTimer, -1);

    lv_scr_load(desktop_screen);
}

void ui_init(lv_indev_t *indev)
{
    knobs_indev = indev;
    /*这些都不需要，不用他的主题
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE),
                                              lv_palette_main(LV_PALETTE_RED),
                                              false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    */
    main_screen_init(knobs_indev);
    // lv_disp_load_scr(desktop_screen);
}

void ui_reload()
{
    if (NULL != desktop_screen)
    {
        lv_scr_load(desktop_screen);
        // lv_disp_load_scr(desktop_screen);
    }

    // ui_release();
    // ui_init(knobs_indev);
}

void ui_release()
{
    if (NULL != topLayerTimer)
    {
        lv_timer_del(topLayerTimer);
        topLayerTimer = NULL;
    }

    if (NULL != autoChangePageTimer)
    {
        lv_timer_del(autoChangePageTimer);
        autoChangePageTimer = NULL;
    }

    if (NULL != sysInfoTimer)
    {
        lv_timer_del(sysInfoTimer);
        sysInfoTimer = NULL;
    }

    ui_Page[currPageIndex]->ui_release();

    if (NULL != desktop_screen)
    {
        lv_obj_del(desktop_screen);
        desktop_screen = NULL;
    }
}

void ui_page_move_center_by_ind(int index)
{
    targerChangePageInd = index; // 等待定时器切换 autoChangePageTimer
}

static void sysInfoTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    // // 锁屏壁纸
    // if (sysInfoModel.lockScreenFlag)
    // {
    //     // 锁屏壁纸
    //     if (NULL == lockScreenImage)
    //     {
    //         pre_group = knobs_indev->group;
    //         lv_indev_set_group(knobs_indev, NULL);
    //         lockScreenImage = lv_img_create(lv_layer_top());
    //         lv_img_set_src(lockScreenImage, &wallpaperList[lockScreenImageIndex]);
    //         lv_obj_align(lockScreenImage, LV_ALIGN_CENTER, 0, -240);

    //         lv_anim_t ls_anim;
    //         lv_anim_init(&ls_anim);
    //         lv_anim_set_exec_cb(&ls_anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    //         lv_anim_set_var(&ls_anim, lockScreenImage);
    //         /* 动画时长[ms] */
    //         lv_anim_set_time(&ls_anim, 1500);
    //         lv_anim_set_values(&ls_anim, -240, 0);
    //         // lv_anim_path_overshoot lv_anim_path_bounce
    //         lv_anim_set_path_cb(&ls_anim, lv_anim_path_bounce); // 设置一个动画的路径
    //         lv_anim_set_early_apply(&ls_anim, true);
    //         lv_anim_start(&ls_anim); /* 应用动画效果 */

    //         wallpaperSwitchLeaveTime = sysInfoModel.utilConfig.wallpaperSwitchTime * 1000;
    //     }
    //     else
    //     {
    //         wallpaperSwitchLeaveTime -= DATA_REFRESH_MS;
    //         if (wallpaperSwitchLeaveTime <= 0)
    //         {
    //             wallpaperSwitchLeaveTime = sysInfoModel.utilConfig.wallpaperSwitchTime * 1000;
    //             lockScreenImageIndex = (lockScreenImageIndex + 1) % WALLPAPER_NUM;
    //             lv_img_set_src(lockScreenImage, &wallpaperList[lockScreenImageIndex]);
    //         }
    //     }
    // }
    // else
    // if (!sysInfoModel.lockScreenFlag)
    // {
    //     if (NULL != lockScreenImage)
    //     {
    //         lv_obj_del(lockScreenImage);
    //         lockScreenImage = NULL;

    //         lv_indev_set_group(knobs_indev, pre_group);
    //     }
    // }

    // 系统消息
    if (0 < sysInfoModel.sysInfoDispTime && NULL == sysInfoLabel)
    {
        lv_obj_set_style_bg_opa(lv_layer_top(), 100, 0);
        // lv_obj_set_style_arc_opa(arcMenu, v, LV_PART_MAIN);
        // for (uint8_t i = 0; i < ARC_MENU_NUM; i++)
        // {
        //     lv_obj_set_style_text_opa(arcMenuLabel[i], v, 0);
        // }
        sysInfoLabel = lv_label_create(lv_layer_top());
        lv_obj_set_align(sysInfoLabel, LV_ALIGN_CENTER);
        lv_obj_set_pos(sysInfoLabel, 0, 40);
        lv_label_set_text_fmt(sysInfoLabel, "%s", sysInfoModel.sysInfo);
        // lv_label_set_long_mode(sysInfoLabel, );
        lv_obj_set_size(sysInfoLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_add_flag(sysInfoLabel, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_clear_flag(sysInfoLabel, LV_OBJ_FLAG_SCROLLABLE);
        // lv_obj_set_style_text_color(sysInfoLabel, lv_color_hex(0xdb3156), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(sysInfoLabel, theme_color1[currPageIndex], LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(sysInfoLabel, &FontDeyi_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else if (0 < sysInfoModel.sysInfoDispTime)
    {
        // 加强显示
        lv_label_set_text_fmt(sysInfoLabel, "%s", sysInfoModel.sysInfo);
    }

    if (0 > sysInfoModel.sysInfoDispTime)
    {
        // 显示时间到
        if (NULL != sysInfoLabel)
        {
            lv_obj_del(sysInfoLabel);
            sysInfoLabel = NULL;
            lv_obj_set_style_bg_opa(lv_layer_top(), 0, 0);
        }
    }
    else
    {
        sysInfoModel.sysInfoDispTime -= DATA_REFRESH_MS;
    }
}

// 触摸事件处理
void touch_event(unsigned int value)
{
    LV_LOG_USER("LOOK\n");
    if (NULL != ui_backBtn)
    {
        if (MENU_SHOW == menu_status)
        {
            hide_arc_menu();
        }
        else
        {
            show_arc_menu();
        }
        // lv_event_send(ui_backBtn, LV_EVENT_PRESSED, 0);
    }
}

#endif