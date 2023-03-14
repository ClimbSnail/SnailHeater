/**
 ******************************************************************************
 * @file    main.c
 * @author  Ac6
 * @version V1.0
 * @date    01-December-2013
 * @brief   Default main function.
 ******************************************************************************
 */

#include "lvgl.h"
#include "app_hal.h"

#include "ctrl_common.h"
#include "driver/knobs.h"
#include "page/desktop/desktop_model.h"
#include "page/desktop/ui.h"
#include "page/startup/startup.h"

#include <SDL2/SDL_timer.h>
#include "demos/lv_demos.h"

#define SNAILHEATER_VERSION "1.3.7"

void SnailHeater_UI()
{
    // 开机动画
    startupInitScreen();
    startupUI(SNAILHEATER_VERSION);
    SDL_Delay(1000);
    startupDel();

    // 设置必要的预显示参数
    setAirhotInfo(300, 250,
                  20, 0,
                  50);
    setHeatplatformInfo(230, 220,
                        true, 20);
    setSolderInfo(SOLDER_TYPE_T12,
                  SOLDER_SHAKE_TYPE_CHANGE, 320,
                  300, SOLDER_STATE_NORMAL,
                  50);
    setAdjPowerInfo(3099, 5000, ADJ_POWER_MODE_CV,
                    0, 0,
                    0, ADJ_POWER_OPEN_STATE_CLOSE);
    setStopWelderInfo(SPOTWELDER_MODE_DOUBLE, 5, 1000,
                      2, 2700, 2000,
                      2700, SPOTWELDER_STATE_WAIT);

    setSysInfo("E3F6FAB", "V2.0",
               "V2.0", "V2.0",
               SNAILHEATER_VERSION, KNOBS_DIR_POS);

    // 绘制页面UI
    ui_init(indev_drv_p);
}

int main(void)
{
    lv_init();

    hal_setup();

    // lv_demo_widgets();
    SnailHeater_UI();

    hal_loop();
}
