#include "./settingUIFull.h"

#ifdef FULL_UI

#include "../ui.h"
#include "../../desktop_model.h"

lv_obj_t *ui_subBackBtn = NULL;
lv_group_t *sub_btn_group = NULL;

lv_style_t setting_btn_focused_style;
lv_style_t setting_btn_pressed_style;

static lv_obj_t *settingPageUI = NULL;
static lv_group_t *btn_group = NULL;
static lv_obj_t *settingTabview = NULL;
static lv_obj_t *qq_group_icon;
// tab
static lv_obj_t *ui_tabSystem = NULL;
static lv_obj_t *ui_tabSoder = NULL;
static lv_obj_t *ui_tabAirhot = NULL;
static lv_obj_t *ui_tabHp = NULL;
static lv_obj_t *ui_tabAdjPwr = NULL;
static lv_obj_t *ui_tabBack = NULL;

static lv_timer_t *dispPageTimer = NULL;
static int pageId = 1000; // 当前刷新完成的页面id

static void settingPageUI_focused(lv_event_t *e)
{
    lv_group_focus_obj(ui_backBtn);
    lv_indev_set_group(knobs_indev, btn_group);
}

void ui_tab_back_btn_pressed(lv_event_t *e)
{
    lv_obj_clear_state(ui_subBackBtn, LV_STATE_FOCUSED);
    lv_indev_set_group(knobs_indev, btn_group);
    lv_group_focus_obj(lv_tabview_get_tab_btns(settingTabview));
}

static void ui_info_init(lv_obj_t *father)
{
    // 设置不显示滚动条
    lv_obj_set_style_bg_opa(father, LV_OPA_0,
                            LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    // qq群图片
    qq_group_icon = lv_img_create(father);
    lv_img_set_src(qq_group_icon, &qq_group);
    lv_obj_align(qq_group_icon, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_style_img_recolor_opa(qq_group_icon, LV_OPA_70, 0);
    // lv_obj_set_style_img_recolor(qq_group_icon, lv_color_hex(0xff3964), 0);
}

static void value_change_tab(lv_event_t *e)
{
    // 进入按键组
    // uint16_t id = lv_tabview_get_tab_act(settingTabview);

    uint16_t id = lv_btnmatrix_get_selected_btn(lv_tabview_get_tab_btns(settingTabview));

    if (pageId != id)
    {
        // 界面未刷机，本次不绑定案件组
        return;
    }

    LV_LOG_USER("value_change_tab id = %d", id);

    switch (id)
    {
    case 0:
    {
        ui_sys_setting_init_group(ui_tabSystem);
    }
    break;
    case 1:
    {
        ui_solder_setting_init_group(ui_tabSoder);
    }
    break;
    case 2:
    {
        ui_airhot_setting_init_group(ui_tabAirhot);
    }
    break;
    case 3:
    {
        ui_hp_setting_init_group(ui_tabHp);
    }
    break;
    case 4:
    {
        ui_adjpwr_setting_init_group(ui_tabAdjPwr);
    }
    break;
    case 5:
    {
        lv_event_send(ui_backBtn, LV_EVENT_PRESSED, 0);
    }
    break;
    default:
        break;
    }
}

// 暂时废弃
static void focused_tab(lv_event_t *e)
{
    // uint16_t id = lv_tabview_get_tab_act(settingTabview);
    uint16_t id = lv_btnmatrix_get_selected_btn(lv_tabview_get_tab_btns(settingTabview));
    LV_LOG_USER("my focused id = %d", id);

    ui_sys_setting_release(ui_tabSystem);
    ui_solder_setting_release(ui_tabSoder);
    ui_airhot_setting_release(ui_tabAirhot);
    ui_hp_setting_release(ui_tabHp);
    ui_adjpwr_setting_release(ui_tabAdjPwr);
    lv_obj_clean(ui_tabBack);

    lv_tabview_set_act(settingTabview, id, LV_ANIM_ON);

    if (NULL != sub_btn_group)
    {
        lv_group_del(sub_btn_group);
        sub_btn_group = NULL;
    }

    switch (id)
    {
    case 0:
    {
        ui_sys_setting_init(ui_tabSystem);
    }
    break;
    case 1:
    {
        ui_solder_setting_init(ui_tabSoder);
    }
    break;
    case 2:
    {
        ui_airhot_setting_init(ui_tabAirhot);
    }
    break;
    case 3:
    {
        ui_hp_setting_init(ui_tabHp);
    }
    break;
    case 4:
    {
        ui_adjpwr_setting_init(ui_tabAdjPwr);
    }
    break;
    case 5:
    {
        ui_info_init(ui_tabBack);
    }
    break;
    default:
        break;
    }
}

static void dispPage_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    // uint16_t id = lv_tabview_get_tab_act(settingTabview);
    uint16_t id = lv_btnmatrix_get_selected_btn(lv_tabview_get_tab_btns(settingTabview));

    if (pageId == id)
    {
        return;
    }

    ui_sys_setting_release(ui_tabSystem);
    ui_solder_setting_release(ui_tabSoder);
    ui_airhot_setting_release(ui_tabAirhot);
    ui_hp_setting_release(ui_tabHp);
    ui_adjpwr_setting_release(ui_tabAdjPwr);
    lv_obj_clean(ui_tabBack);

    lv_tabview_set_act(settingTabview, id, LV_ANIM_ON);

    if (NULL != sub_btn_group)
    {
        lv_group_del(sub_btn_group);
        sub_btn_group = NULL;
    }

    switch (id)
    {
    case 0:
    {
        ui_sys_setting_init(ui_tabSystem);
    }
    break;
    case 1:
    {
        ui_solder_setting_init(ui_tabSoder);
    }
    break;
    case 2:
    {
        ui_airhot_setting_init(ui_tabAirhot);
    }
    break;
    case 3:
    {
        ui_hp_setting_init(ui_tabHp);
    }
    break;
    case 4:
    {
        ui_adjpwr_setting_init(ui_tabAdjPwr);
    }
    break;
    case 5:
    {
        ui_info_init(ui_tabBack);
    }
    break;
    default:
        break;
    }

    pageId = id;
    LV_LOG_USER("my focused id = %d, pageId = %d", id, pageId);
}

static bool settingPageUI_init(lv_obj_t *father)
{
    bool isWhiteTheme = IS_WHITE_THEME;
    if (NULL != settingPageUI)
    {
        settingPageUI = NULL;
    }
    top_layer_set_name();
    theme_color_init();

    lv_style_set_border_color(&setting_btn_focused_style, HEAT_PLAT_THEME_COLOR1);
    lv_style_set_radius(&setting_btn_focused_style, 4);
    lv_style_set_border_width(&setting_btn_focused_style, 2);
    lv_style_set_bg_color(&setting_btn_focused_style, HEAT_PLAT_THEME_COLOR1);
    // lv_style_set_text_color(&setting_btn_focused_style, theme_color1[currPageIndex]);

    lv_style_set_border_color(&setting_btn_pressed_style, HEAT_PLAT_THEME_COLOR1);
    lv_style_set_radius(&setting_btn_pressed_style, 4);
    lv_style_set_border_width(&setting_btn_pressed_style, 2);
    lv_style_set_bg_color(&setting_btn_pressed_style, HEAT_PLAT_THEME_COLOR1);
    lv_style_set_text_color(&setting_btn_pressed_style, HEAT_PLAT_THEME_COLOR1);

    settingPageUI = lv_btn_create(father);
    settingUIObj.mainButtonUI = settingPageUI;

    lv_obj_remove_style_all(settingPageUI);
    lv_obj_set_size(settingPageUI, EACH_PAGE_SIZE_X, EACH_PAGE_SIZE_Y);
    lv_obj_center(settingPageUI);
    lv_obj_add_flag(settingPageUI, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(settingPageUI, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_style_bg_color(settingPageUI, isWhiteTheme ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(settingPageUI, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_ButtonTmp = settingPageUI;

    // Tabview
    settingTabview = lv_tabview_create(ui_ButtonTmp, LV_DIR_LEFT, 50);
    lv_obj_add_style(settingTabview, &label_text_style, 0);
    lv_obj_add_style(settingTabview, &black_white_theme_style, 0);
    lv_obj_set_style_border_color(settingTabview,
                                  lv_palette_darken(LV_PALETTE_GREY, 3),
                                  LV_PART_MAIN | LV_STATE_ANY);

    lv_obj_t *tab_btns = lv_tabview_get_tab_btns(settingTabview);
    lv_obj_add_style(tab_btns, &label_text_style, 0);
    lv_obj_add_style(tab_btns, &black_white_theme_style, 0);
    // lv_obj_set_style_bg_color(tab_btns, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    // lv_obj_set_style_text_color(tab_btns, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);
    lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_NONE, LV_PART_ITEMS | LV_STATE_CHECKED);
    // lv_obj_set_style_border_color(tab_btns, SETTING_THEME_COLOR1, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_border_color(tab_btns, lv_palette_darken(LV_PALETTE_GREY, 3), LV_PART_MAIN | LV_STATE_ANY);

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    ui_tabSystem = lv_tabview_add_tab(settingTabview, "系统");
    ui_tabSoder = lv_tabview_add_tab(settingTabview, "烙铁");
    ui_tabAirhot = lv_tabview_add_tab(settingTabview, "风枪");
    ui_tabHp = lv_tabview_add_tab(settingTabview, "热台");
    ui_tabAdjPwr = lv_tabview_add_tab(settingTabview, "电源");
    ui_tabBack = lv_tabview_add_tab(settingTabview, "返回");

    // lv_obj_add_event_cb(tab_btns, focused_tab, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(settingTabview, value_change_tab, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_clear_flag(lv_tabview_get_content(settingTabview), LV_OBJ_FLAG_SCROLLABLE);

    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_backBtn);
    lv_group_add_obj(btn_group, tab_btns);
    // lv_group_focus_obj(tab_btns);
    lv_indev_set_group(knobs_indev, btn_group);

    // lv_event_send(tab_btns, LV_EVENT_PRESSED, 0);

    dispPageTimer = lv_timer_create(dispPage_timeout, 1000, NULL);
    lv_timer_set_repeat_count(dispPageTimer, -1);

    return true;
}

void settingPageUI_release()
{
    if (NULL != dispPageTimer)
    {
        lv_timer_del(dispPageTimer);
        dispPageTimer = NULL;
    }

    // 需要手动清理下各个子页面
    ui_sys_setting_release(ui_tabSystem);
    ui_solder_setting_release(ui_tabSoder);
    ui_airhot_setting_release(ui_tabAirhot);
    ui_hp_setting_release(ui_tabHp);
    ui_adjpwr_setting_release(ui_tabAdjPwr);
    lv_obj_clean(ui_tabBack);

    if (NULL != btn_group)
    {
        lv_group_del(btn_group);
        btn_group = NULL;
    }

    if (NULL != sub_btn_group)
    {
        lv_group_del(sub_btn_group);
        sub_btn_group = NULL;
    }

    if (NULL != settingPageUI)
    {
        lv_obj_del(settingPageUI);
        settingPageUI = NULL;
        settingUIObj.mainButtonUI = NULL;
    }

    // 保存设置
    sysInfoModel.saveConfAPI(NULL);
}

FE_UI_OBJ settingUIObj = {settingPageUI, settingPageUI_init,
                          settingPageUI_release, settingPageUI_focused};

#endif