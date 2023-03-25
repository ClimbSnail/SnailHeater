// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.0.5
// LVGL VERSION: 8.2
// PROJECT: SquareLine_Project

#ifndef _SQUARELINE_PROJECT_UI_H
#define _SQUARELINE_PROJECT_UI_H

#ifdef __cplusplus
extern "C"
{
#endif

#if __has_include("lvgl.h")
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "uiBase.h"
#include "language_info.h"
#include "resource/images/ico.h"

#define NEW_UI

#ifndef NEW_UI
// 旧的
#define START_UI_OBJ_X 140 // 280/2=140
#define WIN_INFO_SIZE 3
#define UI_OBJ_NUM 5

#define EACH_PAGE_SIZE_X 110
#define EACH_PAGE_SIZE_Y 200

    // 字库解决方法 https://blog.csdn.net/a200327/article/details/120628170
    LV_FONT_DECLARE(sh_number_40);
    LV_FONT_DECLARE(sh_number_50);
    LV_FONT_DECLARE(sh_number_70);
    LV_FONT_DECLARE(sh_number_90);
    LV_FONT_DECLARE(sh_ch_font_14);
    LV_FONT_DECLARE(sh_ch_font_18);

    extern lv_style_t focused_style;
    extern lv_obj_t *desktop_screen;

    extern lv_obj_t *ui_PanelMain;

    extern lv_indev_t *knobs_indev; // 旋钮设备

#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); // 等待动画完成

    void ui_init(lv_indev_t *lv_indev_delete);
    void ui_main_pressed(lv_event_t *e);
    void ui_page_move_center_by_ind(int index); // 后台请求主动页面切换

    static uint8_t lastMiddlePos = 0;

    extern int16_t btnPosXY[WIN_INFO_SIZE][2];

    extern FE_UI_OBJ solderUIObj;
    extern FE_UI_OBJ airhotUIObj;
    extern FE_UI_OBJ hpUIObj;
    extern FE_UI_OBJ adjPowerUIObj;
    extern FE_UI_OBJ sysInfoUIObj;

    void ui_updateSolderCurTempAndPowerDuty(void);
    void ui_updateAirhotCurTempAndPowerDuty(void);
    void ui_updateHeatplatformCurTempAndPowerDuty(void);
    void ui_updateAdjPowerCurVoltage(void);
    void ui_updateAdjPowerWorkState(void);

#else
// 新UI
struct FE_FULL_UI_OBJ
{
    // ui的主体
    lv_obj_t *mainButtonUI;

    // ui的初始化函数
    bool (*ui_init)(lv_obj_t *father);

    // ui的退出函数
    void (*ui_release)();

    // ui的按钮组初始化函数
    void (*ui_btn_group_init)();
};

#define UI_OBJ_NUM 5
#define EACH_PAGE_SIZE_X 280
#define EACH_PAGE_SIZE_Y 216
LV_FONT_DECLARE(FontDeyi_16);
LV_FONT_DECLARE(FontDeyi_20);
LV_FONT_DECLARE(FontJost_52);
LV_FONT_DECLARE(FontRoboto_52);
LV_FONT_DECLARE(FontRoboto_36);
LV_FONT_DECLARE(FontJost_14);
LV_FONT_DECLARE(FontJost_18);
LV_FONT_DECLARE(FontJost_24);
LV_FONT_DECLARE(FontJost_36);
LV_FONT_DECLARE(FontJost);
LV_IMG_DECLARE(img_soldering_gray);
LV_IMG_DECLARE(img_fan_speed);
LV_IMG_DECLARE(img_text_c245);
LV_IMG_DECLARE(img_text_c210);
LV_IMG_DECLARE(img_text_t12);
LV_IMG_DECLARE(img_text_noc);
LV_IMG_DECLARE(img_top_bar_white);
LV_IMG_DECLARE(img_top_bar_black);

void show_menu();
void hide_menu();

// 顶层的4个数字标记
extern lv_obj_t *ui_airHotLabel;
extern lv_obj_t *ui_heatPlatLabel;
extern lv_obj_t *ui_adjPowerLabel;
extern lv_obj_t *ui_solderingLabel;
// 顶层左边的文本/logo
extern lv_obj_t *ui_topLogoLabel;
extern lv_obj_t *chartTemp; // 曲线图对象
extern lv_chart_series_t *chartSer1;

extern lv_style_t bar_style_bg; // 功率bar的样式
extern lv_style_t bar_style_indic;

typedef enum
{
    PAGE_INDEX_SOLDER = 0,
    PAGE_INDEX_AIR_HOT,
    PAGE_INDEX_HEAT_PLAT,
    PAGE_INDEX_ADJ_POWER,
    PAGE_INDEX_SETTING,
    PAGE_INDEX_MAXSIZE

} PAGE_INDEX;

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

extern uint8_t currPageIndex; // 当前页所在的页面索引，对应上面的枚举 PAGE_INDEX
const char modeName[5][20] = {"烙铁", "风枪", "热台", "电源", "设置"};

#define SOLDER_THEME_COLOR1 lv_color_hex(0xdb3156)
#define SOLDER_THEME_COLOR2 lv_color_hex(0x851e34)
#define HEAT_PLAT_THEME_COLOR1 lv_color_hex(0x009be6)
#define HEAT_PLAT_THEME_COLOR2 lv_color_hex(0x006799)
#define AIR_HOT_THEME_COLOR1 lv_color_hex(0xffcc33)
#define AIR_HOT_THEME_COLOR2 lv_color_hex(0xd4aa2a)
#define ADJ_POWER_THEME_COLOR1 lv_color_hex(0x4ed438)
#define SETTING_THEME_COLOR1 lv_color_hex(0x999999)
#define SETTING_THEME_COLOR2 lv_color_hex(0x666666)

const lv_color_t theme_color1[UI_OBJ_NUM] = {
    SOLDER_THEME_COLOR1,
    AIR_HOT_THEME_COLOR1,
    HEAT_PLAT_THEME_COLOR1,
    ADJ_POWER_THEME_COLOR1,
    SETTING_THEME_COLOR1};

// 位操作宏
#define SET_BIT(x, y) x |= (1 << y)  // 将x右起第y位置1，y从0开始，0表示第一位置1
#define CLR_BIT(x, y) x &= ~(1 << y) // 将x右起第y位置0，y从0开始，0表示第一位置0
#define REVERSE_BIT(x, y) x ^= (1 << y)
#define GET_BIT(x, y) ((x) >> (y)&1)

// 是否是白色主题的快捷宏
// #define IS_WHITE_THEME GET_BIT(cfgKey1, CFG_KEY1_WHITE_THEME)
#define IS_WHITE_THEME sysInfoModel.uiGlobalParam.whiteThemeEnable

#define BLACK_THEME_CHART_COLOR1 lv_color_hex(0x222222)
#define BLACK_THEME_CHART_COLOR2 lv_color_hex(0x444444)

#define WHITE_THEME_CHART_COLOR1 lv_color_hex(0xf0f0f0)
#define WHITE_THEME_CHART_COLOR2 lv_color_hex(0xdddddd)

void top_layer_init();   // 绘制顶层状态栏
void theme_color_init(); // 根据模式设置主题色

extern lv_style_t label_text_style;        // 用于各种默认标签的样式
extern lv_style_t black_white_theme_style; // 全局黑白样式
extern lv_obj_t *ui_PanelTopBgImg;
extern lv_style_t back_btn_style;
extern lv_style_t back_btn_focused_style;
extern lv_obj_t *desktop_screen;
extern lv_obj_t *ui_PanelMain; // 主容器
extern lv_obj_t *ui_PanelTop;  // 顶部状态容器

// 如果需要启用新的菜单样式，定义这个
#define USE_NEW_MENU
#ifdef USE_NEW_MENU
extern lv_obj_t *rollerMenu; // 菜单
#endif
extern lv_style_t btn_type1_style;
extern lv_style_t btn_type1_focused_style;
extern lv_style_t btn_type1_pressed_style;
extern lv_indev_t *knobs_indev;

extern uint8_t cfgKey1; // 关于UI的全局配置参数，会持久化保存

extern FE_FULL_UI_OBJ solderUIObj;
extern FE_FULL_UI_OBJ hpUIObj;
extern FE_FULL_UI_OBJ airhotUIObj;
extern FE_FULL_UI_OBJ settingUIObj;
extern FE_FULL_UI_OBJ adjPowerUIObj;

void ui_init(lv_indev_t *lv_indev_delete);
void ui_release();                          // 基本不会调用此函数
void ui_page_move_center_by_ind(int index); // 后台请求主动页面切换

void ui_updateSolderCurTempAndPowerDuty(void);
void ui_updateHeatplatformCurTempAndPowerDuty(void);
void ui_updateAirhotCurTempAndPowerDuty(void);
void ui_updateAdjPowerCurVoltage(void);
void ui_updateAdjPowerWorkState(void);
void top_layer_set_name();

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
