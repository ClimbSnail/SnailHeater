// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.0.5
// LVGL VERSION: 8.2
// PROJECT: SquareLine_Project

#include "ui.h"
#include "desktop_model.h"

#ifndef NEW_UI

int16_t btnPosXY[WIN_INFO_SIZE][2] = {{-10 - 110, 0}, {0, 0}, {10 + 110, 0}};

///////////////////// VARIABLES ////////////////////
lv_style_t focused_style;
lv_obj_t *desktop_screen;
lv_obj_t *ui_PanelMain;

lv_indev_t *knobs_indev;
lv_group_t *main_btn_group = NULL;

static FE_UI_OBJ *ui_Button[UI_OBJ_NUM];

static lv_anim_t a0_x;
static lv_anim_t a1_x;
static lv_anim_t a2_x;

static lv_anim_t a0_y;
static lv_anim_t a1_y;
static lv_anim_t a2_y;

static bool btn_anim_lock = false; // 动画锁

#define ANIM_DURATION 300 // 动画的时长
#define OPNEN_ANIM 0      // 开启动画

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
#error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP != 0
#error "#error LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

bool initUIObj(uint8_t midIndex);

void Obj2ObjAnimX(lv_anim_t *a, lv_obj_t *fromObj, int32_t dx)
{
    /* 初始化动画
     *-----------------------*/
    lv_anim_init(a);
    /* 必选设置
     *------------------*/
    /* 设置“动画制作”功能 */
    lv_anim_set_exec_cb(a, (lv_anim_exec_xcb_t)lv_obj_set_x);
    /* 设置“动画制作”功能 */
    lv_anim_set_var(a, fromObj);
    /* 动画时长[ms] */
    lv_anim_set_time(a, ANIM_DURATION);
    /* 设置开始和结束值。例如。 0、150 */
    lv_anim_set_values(a, lv_obj_get_x(fromObj), dx);

    // lv_anim_set_path_cb(a, lv_anim_path_overshoot); //设置一个动画的路径
    lv_anim_set_path_cb(a, lv_anim_path_linear); // 设置一个动画的路径
    // /* 可选设置
    //  *------------------*/
    // /* 开始动画之前的等待时间[ms] */
    // lv_anim_set_delay(a, delay);
    // /* 设置路径（曲线）。默认为线性 */
    // lv_anim_set_path(a, &path);
    // /* 设置一个回调以在动画准备好时调用。 */
    // lv_anim_set_ready_cb(a, ready_cb);
    // /* 设置在动画开始时（延迟后）调用的回调。 */
    // lv_anim_set_start_cb(a, start_cb);
    // /* 在此持续时间内，也向后播放动画。默认值为0（禁用）[ms] */
    // lv_anim_set_playback_time(a, wait_time);
    // /* 播放前延迟。默认值为0（禁用）[ms] */
    // lv_anim_set_playback_delay(a, wait_time);
    // /* 重复次数。默认值为1。LV_ANIM_REPEAT_INFINIT用于无限重复 */
    // lv_anim_set_repeat_count(&a, wait_time);
    // /* 重复之前要延迟。默认值为0（禁用）[ms] */
    // lv_anim_set_repeat_delay(&a, wait_time);

    // /* true（默认）：立即应用开始值，false：延迟设置动画后再应用开始值。真正开始。 */
    // lv_anim_set_early_apply(&a, true);
    // /* 应用动画效果
    //  *------------------*/
    // lv_anim_start(&a); /* 应用动画效果 */
}

void Obj2ObjAnimY(lv_anim_t *a, lv_obj_t *fromObj, int32_t dy)
{
    /* 初始化动画
     *-----------------------*/
    lv_anim_init(a);
    /* 必选设置
     *------------------*/
    /* 设置“动画制作”功能 */
    lv_anim_set_exec_cb(a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    /* 设置“动画制作”功能 */
    lv_anim_set_var(a, fromObj);
    /* 动画时长[ms] */
    lv_anim_set_time(a, ANIM_DURATION);
    /* 设置开始和结束值。例如。 0、150 */
    lv_anim_set_values(a, lv_obj_get_y(fromObj), dy);

    // lv_anim_set_path_cb(a, lv_anim_path_overshoot); //设置一个动画的路径
    lv_anim_set_path_cb(a, lv_anim_path_linear); // 设置一个动画的路径
    // /* 可选设置
    //  *------------------*/
    // /* 开始动画之前的等待时间[ms] */
    // lv_anim_set_delay(a, delay);
    // /* 设置路径（曲线）。默认为线性 */
    // lv_anim_set_path(a, &path);
    // /* 设置一个回调以在动画准备好时调用。 */
    // lv_anim_set_ready_cb(a, ready_cb);
    // /* 设置在动画开始时（延迟后）调用的回调。 */
    // lv_anim_set_start_cb(a, start_cb);
    // /* 在此持续时间内，也向后播放动画。默认值为0（禁用）[ms] */
    // lv_anim_set_playback_time(a, wait_time);
    // /* 播放前延迟。默认值为0（禁用）[ms] */
    // lv_anim_set_playback_delay(a, wait_time);
    // /* 重复次数。默认值为1。LV_ANIM_REPEAT_INFINIT用于无限重复 */
    // lv_anim_set_repeat_count(&a, wait_time);
    // /* 重复之前要延迟。默认值为0（禁用）[ms] */
    // lv_anim_set_repeat_delay(&a, wait_time);

    // /* true（默认）：立即应用开始值，false：延迟设置动画后再应用开始值。真正开始。 */
    // lv_anim_set_early_apply(&a, true);
    // /* 应用动画效果
    //  *------------------*/
    // lv_anim_start(&a); /* 应用动画效果 */
}

void ui_event_Button(lv_event_t *e);
///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////
void ui_event_Button_1(lv_event_t *e)
{
    // return;
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    uint8_t curMiddlePos = 0;
    for (curMiddlePos = 0; curMiddlePos < UI_OBJ_NUM; curMiddlePos++)
    {
        // todo 有可能找不到或者寻找错误
        if (target == ui_Button[curMiddlePos]->mainButtonUI)
        {
            break;
        }
    }

    if (event_code == LV_EVENT_FOCUSED)
    {
        (e);
        if (true == btn_anim_lock)
        {
            // Serial0.println("Return true == btn_anim_lock");
            return;
        }
        btn_anim_lock = true; // 加锁
        // Serial0.println("\nbtn_anim_lock = true");
        initUIObj(curMiddlePos);
        if ((lastMiddlePos + 1) % UI_OBJ_NUM == curMiddlePos)
        {
            // 图标向左移动
#if OPNEN_ANIM == 1
            int32_t dx = lv_obj_get_x(ui_Button[curMiddlePos]);
            Obj2ObjAnimX(&a0_x, ui_Button[curMiddlePos], btnPosXY[1][0]);
            Obj2ObjAnimX(&a1_x, ui_Button[lastMiddlePos], btnPosXY[0][0]);
            Obj2ObjAnimX(&a2_x, ui_Button[(lastMiddlePos + UI_OBJ_NUM - 1) % UI_OBJ_NUM], btnPosXY[2][0]);

            // Obj2ObjAnimY(&a0_y, ui_Button[curMiddlePos], btnPosXY[1][1]);
            // Obj2ObjAnimY(&a1_y, ui_Button[lastMiddlePos], btnPosXY[0][1]);
            // Obj2ObjAnimY(&a2_y, ui_Button[(lastMiddlePos + UI_OBJ_NUM - 1) % UI_OBJ_NUM], btnPosXY[2][1]);
#else
            lv_obj_set_pos(ui_Button[curMiddlePos]->mainButtonUI, btnPosXY[1][0], btnPosXY[1][1]);
            lv_obj_set_pos(ui_Button[lastMiddlePos]->mainButtonUI, btnPosXY[0][0], btnPosXY[0][1]);
            lv_obj_set_pos(ui_Button[(lastMiddlePos + UI_OBJ_NUM - 1) % UI_OBJ_NUM]->mainButtonUI, btnPosXY[2][0], btnPosXY[2][1]);
            lv_scr_load(desktop_screen);
#endif
        }
        else if ((lastMiddlePos + UI_OBJ_NUM - 1) % UI_OBJ_NUM == curMiddlePos)
        {
            // 图标向右移动
#if OPNEN_ANIM == 1
            int32_t dx = lv_obj_get_x(ui_Button[curMiddlePos]);
            Obj2ObjAnimX(&a0_x, ui_Button[curMiddlePos], btnPosXY[1][0]);
            Obj2ObjAnimX(&a1_x, ui_Button[lastMiddlePos], btnPosXY[2][0]);
            Obj2ObjAnimX(&a2_x, ui_Button[(lastMiddlePos + UI_OBJ_NUM + 1) % UI_OBJ_NUM], btnPosXY[0][0]);
#else
            lv_obj_set_pos(ui_Button[curMiddlePos]->mainButtonUI, btnPosXY[1][0], btnPosXY[1][1]);
            lv_obj_set_pos(ui_Button[lastMiddlePos]->mainButtonUI, btnPosXY[2][0], btnPosXY[2][1]);
            lv_obj_set_pos(ui_Button[(lastMiddlePos + UI_OBJ_NUM + 1) % UI_OBJ_NUM]->mainButtonUI, btnPosXY[0][0], btnPosXY[0][1]);
            lv_scr_load(desktop_screen);
#endif
            // Obj2ObjAnimY(&a0_y, ui_Button[curMiddlePos], btnPosXY[1][1]);
            // Obj2ObjAnimY(&a1_y, ui_Button[lastMiddlePos], btnPosXY[2][1]);
            // Obj2ObjAnimY(&a2_y, ui_Button[(lastMiddlePos + UI_OBJ_NUM + 1) % UI_OBJ_NUM], btnPosXY[0][1]);
        }
        btn_anim_lock = false; // 解锁
                               // Serial0.println("btn_anim_lock = false");

#if OPNEN_ANIM == 1
        /* true（默认）：立即应用开始值，false：延迟设置动画后再应用开始值。真正开始。 */
        lv_anim_set_early_apply(&a0_x, true);
        lv_anim_set_early_apply(&a1_x, true);
        lv_anim_set_early_apply(&a2_x, true);

        // lv_anim_set_early_apply(&a0_y, true);
        // lv_anim_set_early_apply(&a1_y, true);
        // lv_anim_set_early_apply(&a2_y, true);

        /* 应用动画效果
         *------------------*/
        lv_anim_start(&a0_x); /* 应用动画效果 */
        lv_anim_start(&a1_x); /* 应用动画效果 */
        lv_anim_start(&a2_x); /* 应用动画效果 */

        // lv_anim_start(&a0_y); /* 应用动画效果 */
        // lv_anim_start(&a1_y); /* 应用动画效果 */
        // lv_anim_start(&a2_y); /* 应用动画效果 */

        // ANIEND
#endif
    }
    lastMiddlePos = curMiddlePos;
}

void ui_event_Button(lv_event_t *e)
{
    // return;
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    uint8_t curMiddlePos = 0;
    for (curMiddlePos = 0; curMiddlePos < UI_OBJ_NUM; curMiddlePos++)
    {
        // todo 有可能找不到或者寻找错误
        if (target == ui_Button[curMiddlePos]->mainButtonUI)
        {
            break;
        }
    }

    if (event_code == LV_EVENT_FOCUSED)
    {
        (e);
        if (true == btn_anim_lock)
        {
            return;
        }
        btn_anim_lock = true; // 加锁
        initUIObj(curMiddlePos);

        btn_anim_lock = false; // 解锁
    }
    lastMiddlePos = curMiddlePos;
}

void ui_Button_move_center(int middleIndex)
{
    if (main_btn_group == NULL || knobs_indev->group != main_btn_group)
    {
        // 不做切换
        return;
    }

    lastMiddlePos = middleIndex;
    // initUIObj(index);

    lv_obj_set_pos(ui_Button[(middleIndex + UI_OBJ_NUM - 1) % UI_OBJ_NUM]->mainButtonUI,
                   btnPosXY[0][0], btnPosXY[0][1]);

    lv_obj_set_pos(ui_Button[middleIndex]->mainButtonUI,
                   btnPosXY[1][0], btnPosXY[1][1]);

    lv_obj_set_pos(ui_Button[(middleIndex + 1) % UI_OBJ_NUM]->mainButtonUI,
                   btnPosXY[2][0], btnPosXY[2][1]);

    // 页面居中
    lv_group_focus_obj(ui_Button[middleIndex]->mainButtonUI);
    lv_scr_load(desktop_screen);
}

///////////////////// SCREENS ////////////////////
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
    lv_style_set_radius(&focused_style, 6);                            // 设置控件圆角半径

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

    ui_Button[0] = &solderUIObj;
    ui_Button[1] = &airhotUIObj;
    ui_Button[2] = &hpUIObj;
    ui_Button[3] = &adjPowerUIObj;
    ui_Button[4] = &sysInfoUIObj;

    main_btn_group = NULL;

    // 删除操作组
    if (NULL != main_btn_group)
    {
        lv_group_del(main_btn_group);
    }
    main_btn_group = lv_group_create();
    lv_indev_set_group(knobs_indev, main_btn_group);

    initUIObj(lastMiddlePos);
}

bool initUIObj(uint8_t midIndex)
{
    int startInd = (midIndex + UI_OBJ_NUM - 1) % UI_OBJ_NUM;

    // 删除两项不用的信息窗
    // for (int cnt = 0; cnt < UI_OBJ_NUM - WIN_INFO_SIZE; ++cnt)
    // {
    //     Serial0.printf("[delete] ind = %d\n", (midIndex + 2 + cnt) % UI_OBJ_NUM);
    //     ui_Button[(midIndex + 2 + cnt) % UI_OBJ_NUM]->ui_release();
    // }

    // 初始化需要显示的信息窗
    for (int cnt = 0; cnt < UI_OBJ_NUM; ++cnt)
    // for (int cnt = UI_OBJ_NUM - 1; cnt >= 0; --cnt)
    {
        int ind = (startInd + cnt + UI_OBJ_NUM) % UI_OBJ_NUM;
        bool ret_code = ui_Button[ind]->ui_init(ui_PanelMain); // 先初始化才是能进行以下操作
        if (ret_code)
        {
            // Serial0.printf("Init ind = %d, ret_code = %d\n", ind, ret_code);
            lv_obj_t *buttonUI = ui_Button[ind]->mainButtonUI;
            lv_group_add_obj(main_btn_group, buttonUI);
            lv_obj_add_event_cb(buttonUI, ui_event_Button, LV_EVENT_FOCUSED, NULL);
            lv_obj_add_event_cb(buttonUI, ui_Button[ind]->pressed_event_cb, LV_EVENT_PRESSED, NULL);
        }
        else
        {
            // Serial0.printf("[Already Init] Init already before, ind = %d\n", ind);
        }
    }
    lv_group_focus_obj(ui_Button[midIndex]->mainButtonUI);

    int curMiddlePos = midIndex;

    lv_obj_set_pos(ui_Button[(curMiddlePos + UI_OBJ_NUM - 1) % UI_OBJ_NUM]->mainButtonUI,
                   btnPosXY[0][0], btnPosXY[0][1]);

    lv_obj_set_pos(ui_Button[curMiddlePos]->mainButtonUI,
                   btnPosXY[1][0], btnPosXY[1][1]);

    lv_obj_set_pos(ui_Button[(curMiddlePos + 1) % UI_OBJ_NUM]->mainButtonUI,
                   btnPosXY[2][0], btnPosXY[2][1]);

    lv_obj_move_foreground(ui_Button[(curMiddlePos + UI_OBJ_NUM - 1) % UI_OBJ_NUM]->mainButtonUI);
    lv_obj_move_foreground(ui_Button[curMiddlePos]->mainButtonUI);
    lv_obj_move_foreground(ui_Button[(curMiddlePos + 1) % UI_OBJ_NUM]->mainButtonUI);

    lv_scr_load(desktop_screen);

    return 0;
}

// bool initNextUIobj(int index)
// {
//     int delInd =
//     // 删除两项不用的信息窗
//     ui_Button[(midIndex + UI_OBJ_NUM + 2) % UI_OBJ_NUM]->ui_release();
// }

void ui_main_pressed(lv_event_t *e)
{
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