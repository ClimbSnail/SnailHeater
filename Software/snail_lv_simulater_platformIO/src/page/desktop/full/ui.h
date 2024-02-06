#include "../ui_select.h" // 必须添加此行

#ifdef FULL_UI // 使能宏

#ifndef _UI_H
#define _UI_H

#ifdef __cplusplus
extern "C"
{
#endif

#if __has_include("lvgl.h")
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "./themeColor.h"
#include "./uiObjBase.h"
#include "../language_info.h"
#include "./numberbtn/lv_numberbtn.h"
#include "resource/images/ico.h"
#include "resource/wallpaper/wallpaper.h"

#define DATA_REFRESH_MS 300 // 数据刷新的时间

    typedef enum
    {
        PAGE_INDEX_SOLDER = 0,
        PAGE_INDEX_AIR_HOT,
        PAGE_INDEX_HEAT_PLAT,
        PAGE_INDEX_ADJ_POWER,
        PAGE_INDEX_SPOTWELDER,
        PAGE_INDEX_SIGNAL,
        PAGE_INDEX_SETTING,
        PAGE_INDEX_MAXSIZE

    } PAGE_INDEX;

#define UI_OBJ_NUM PAGE_INDEX_MAXSIZE
    extern uint8_t currPageIndex; // 当前页所在的页面索引，对应上面的枚举 PAGE_INDEX
    const char modeName[UI_OBJ_NUM][20] = {"烙铁", "风枪",
                                           "热台", "电源",
                                           "点焊",
                                           "信号",
                                           "设置"};

#define EACH_PAGE_SIZE_X SH_SCREEN_WIDTH
#define EACH_PAGE_SIZE_Y 216
    LV_FONT_DECLARE(FontDeyi_16);
    LV_FONT_DECLARE(FontDeyi_24);
    LV_FONT_DECLARE(FontDeyi_36);
    LV_FONT_DECLARE(FontDeyi_20);
    LV_FONT_DECLARE(FontJost_52);
    LV_FONT_DECLARE(FontRoboto_52);
    LV_FONT_DECLARE(FontRoboto_36);
    LV_FONT_DECLARE(FontRoboto_24);
    LV_FONT_DECLARE(FontJost_14);
    LV_FONT_DECLARE(FontJost_18);
    LV_FONT_DECLARE(FontJost_24);
    LV_FONT_DECLARE(FontJost_36);
    LV_FONT_DECLARE(FontJost);
    LV_IMG_DECLARE(img_soldering_gray);
    LV_IMG_DECLARE(qq_group);
    LV_IMG_DECLARE(img_fan_speed);
    LV_IMG_DECLARE(img_text_c245);
    LV_IMG_DECLARE(img_text_c210);
    LV_IMG_DECLARE(img_text_t12);
    LV_IMG_DECLARE(img_text_noc);

    LV_IMG_DECLARE(img_name_noc_ico);
    LV_IMG_DECLARE(img_name_c105_ico);
    LV_IMG_DECLARE(img_name_c115_ico);
    LV_IMG_DECLARE(img_name_c210_ico);
    LV_IMG_DECLARE(img_name_c245_ico);
    LV_IMG_DECLARE(img_name_c470_ico);
    LV_IMG_DECLARE(img_name_t12_ico);
    LV_IMG_DECLARE(img_name_t20_ico);
    LV_IMG_DECLARE(img_name_936_ico);

    LV_IMG_DECLARE(img_wake_type_none);
    LV_IMG_DECLARE(img_wake_type_high);
    LV_IMG_DECLARE(img_wake_type_low);
    LV_IMG_DECLARE(img_wake_type_change);

    LV_IMG_DECLARE(signal_sine);
    LV_IMG_DECLARE(signal_square);
    LV_IMG_DECLARE(earphone3);
    LV_IMG_DECLARE(lineSwitch);
    LV_IMG_DECLARE(setting);

    LV_IMG_DECLARE(img_top_bar_white);
    LV_IMG_DECLARE(img_top_bar_black);
    LV_FONT_DECLARE(lv_font_montserrat_18);
    LV_FONT_DECLARE(lv_font_montserrat_48);

    void show_arc_menu();
    void hide_arc_menu();

    // // 顶层的4个数字标记
    // extern lv_obj_t *ui_airHotLabel;
    // extern lv_obj_t *ui_heatPlatLabel;
    // extern lv_obj_t *ui_adjPowerLabel;
    // extern lv_obj_t *ui_solderingLabel;
    // 顶层左边的文本/logo
    extern lv_obj_t *ui_topLogoLabel;
#define CURVE_HIGH 100
    extern lv_obj_t *chartTemp; // 曲线图对象
    extern lv_chart_series_t *chartSer1;
    extern lv_chart_series_t *chartSer2;

    extern lv_style_t bar_style_bg; // 功率bar的样式
    extern lv_style_t bar_style_indic;

    typedef enum
    {
        CFG_KEY1_SOLDER_GRID = 0,
        CFG_KEY1_AIR_HOT_GRID,
        CFG_KEY1_HEAT_PLAT_GRID,
        CFG_KEY1_USER1,
        CFG_KEY1_USER2,
        CFG_KEY1_USER3,
        CFG_KEY1_HP_AUTO_HEAT,
        CFG_KEY1_WHITE_THEME,

    } CONFIG_KEY1_ENUM;

    const lv_color_t theme_color1[UI_OBJ_NUM] = {
        SOLDER_THEME_COLOR1,
        AIR_HOT_THEME_COLOR1,
        HEAT_PLAT_THEME_COLOR1,
        ADJ_POWER_THEME_COLOR1,
        SPOT_WELDER_THEME_COLOR1,
        SIGNAL_THEME_COLOR1,
        SETTING_THEME_COLOR1};

// 位操作宏
#define SET_BIT(x, y) x |= (1 << y)  // 将x右起第y位置1，y从0开始，0表示第一位置1
#define CLR_BIT(x, y) x &= ~(1 << y) // 将x右起第y位置0，y从0开始，0表示第一位置0
#define REVERSE_BIT(x, y) x ^= (1 << y)
#define GET_BIT(x, y) ((x) >> (y) & 1)

// 菜单风格，1，最老的，2，roller 3，最新的圆盘
#define USE_MENU_STYLE 3

#if USE_MENU_STYLE == 3
#include "./arcmenu/lv_arcmenu.h"
#define ARC_MENU_NUM UI_OBJ_NUM
#define ARC_MENU_ARC_WIDTH 50
#define ARC_MENU_WIDTH 160
#define ARC_MENU_HEIGHT 160
#endif

// 是否是白色主题的快捷宏
// #define IS_WHITE_THEME GET_BIT(cfgKey1, CFG_KEY1_WHITE_THEME)
#define IS_WHITE_THEME sysInfoModel.utilConfig.uiParam.uiGlobalParam.whiteThemeEnable

    void top_layer_init(lv_obj_t *parent); // 绘制顶层状态栏
    void theme_color_init();               // 根据模式设置主题色

    extern lv_style_t label_text_style;        // 用于各种默认标签的样式
    extern lv_style_t black_white_theme_style; // 全局黑白样式
    extern lv_obj_t *ui_PanelTopBgImg;
    extern lv_style_t back_btn_style;
    extern lv_style_t back_btn_focused_style;
    extern lv_obj_t *ui_PanelTop; // 顶部状态容器
    extern lv_obj_t *ui_backBtn;  // 顶部返回按钮

#if USE_MENU_STYLE == 3
    extern lv_style_t menu_button_style;
    extern lv_style_t menu_button_checked_style;
    extern lv_style_t menu_arc_style;
#endif
#if USE_MENU_STYLE == 2
    extern lv_obj_t *rollerMenu; // 菜单
#endif
    extern lv_style_t btn_type1_style;
    extern lv_style_t btn_type1_focused_style;
    extern lv_style_t btn_type1_pressed_style;
    extern lv_indev_t *knobs_indev;

    extern uint8_t cfgKey1; // 关于UI的全局配置参数，会持久化保存

    extern FE_UI_OBJ solderUIObj;
    extern FE_UI_OBJ hpUIObj;
    extern FE_UI_OBJ airhotUIObj;
    extern FE_UI_OBJ settingUIObj;
    extern FE_UI_OBJ spotWelderUIObj;
    extern FE_UI_OBJ signalUIObj;
    extern FE_UI_OBJ adjPowerUIObj;

    void ui_init(lv_indev_t *lv_indev_delete);
    void ui_release(); // 基本不会调用此函数
    void ui_reload();
    void ui_page_move_center_by_ind(int index); // 后台请求主动页面切换

    void ui_updateSolderData(void);
    void ui_updateSolderPIDParam(void);
    void ui_updateHeatplatformData(void);
    void ui_updateHeatplatformPIDParam(void);
    void ui_updateAirhotData(void);
    void ui_updateAirhotPIDParam(void);
    void ui_updateAdjPowerData(void);
    void ui_updateAdjPowerWorkState(void);
    void top_layer_set_name();

    void ui_setIntellectRateFlag(lv_event_t *e); // 设置旋钮的智能加速

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

#endif
