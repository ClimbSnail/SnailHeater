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
#include "page/desktop/snail_ui.h"
#include "page/startup/startup.h"

#include <SDL2/SDL_timer.h>
#include "demos/lv_demos.h"

#define SNAILHEATER_VERSION "1.6.12"

int saveSettingAPI(void *parameter)
{
    snprintf(sysInfoModel.sysInfo, 128, "%s", "设置已保存");
    sysInfoModel.sysInfoDispTime = 1000;
    return 0;
}

void SnailHeater_UI()
{
    // 开机动画
    startupInitScreen();
    startupUI(SNAILHEATER_VERSION);
    SDL_Delay(5000);
    startupDel();

    // 设置必要的预显示参数
    if (true)
    {
        AirhotModel model;
        model.workState = HOTAIR_STATE_SLEEP;
        model.utilConfig.quickSetupTemp_0 = 270;
        model.utilConfig.quickSetupTemp_1 = 360;
        model.utilConfig.quickSetupTemp_2 = 400;
        model.utilConfig.workAirSpeed = 30;
        model.utilConfig.targetTemp = 0;
        model.curTemp = 0;
        model.powerRatio = 0;
        model.powerRatio = 0;
        setAirhotInfo(&model);
    }

    if (true)
    {
        HeatplatformModel model;
        // model.workState = ;
        model.enable = HP_STATE_SLEEP;
        model.utilConfig.quickSetupTemp_0 = 270;
        model.utilConfig.quickSetupTemp_1 = 360;
        model.utilConfig.quickSetupTemp_2 = 400;
        model.utilConfig.targetTemp = 0;
        model.curTemp = 0;
        model.powerRatio = 0;
        setHeatplatformInfo(&model);
    }

    if (true)
    {
        SolderModel model;
        model.workState = SOLDER_STATE_DEEP_SLEEP;
        model.utilConfig.curCoreID = 0;
        model.coreNameList = "0_T12\n1_C210\n2_C245";
        model.coreConfig.solderType = SOLDER_TYPE_T12;
        model.coreConfig.wakeSwitchType = SOLDER_SHAKE_TYPE_CHANGE;
        model.tempEnable.allValue = 0x01;
        model.utilConfig.quickSetupTemp_0 = 270;
        model.utilConfig.quickSetupTemp_1 = 360;
        model.utilConfig.quickSetupTemp_2 = 400;
        model.fineAdjTemp = 300;
        model.utilConfig.targetTemp = 0;
        model.curTemp = 0;
        model.powerRatio = 0;
        setSolderInfo(&model);
    }

    if (true)
    {
        AdjPowerModel model;
        model.workState = ENABLE_STATE_CLOSE;
        model.utilConfig.volDacValue = 0;
        model.utilConfig.curDacValue = 0;
        model.utilConfig.mode = 0;
        model.voltage = 0;
        model.current = 0;
        model.capacity = 0;
        model.curToZeroFlag = 0;
        setAdjPowerInfo(&model);
    }

    // setStopWelderInfo(SPOTWELDER_MODE_DOUBLE, 5, 1000,
    //                   2, 2700, 2000,
    //                   2700, SPOTWELDER_STATE_WAIT);

    if (true)
    {
        // 系统信息
        SysInfoModel model;
        snprintf(model.sn, 32, "%s", "");
        model.srceenVersion = VERSION_INFO_SRCEEN_V20;
        model.coreVersion = VERSION_INFO_CORE_V20;
        model.outBoardVersion = VERSION_INFO_OUT_BOARD_V20;
        snprintf(model.softwareVersion, 16, "%s", SNAILHEATER_VERSION);
        model.knobDir = KNOBS_DIR_POS;
        model.lockScreenDelayTime = 10;
        model.wallpaperSwitchTime = 5;
        model.lockScreenFlag = false;
        // model.lockScreenFlag = true;
        model.saveConfAPI = saveSettingAPI;
        model.uiGlobalParam = {0, 0, 1, 0, 0};

        setSysInfo(&model);
    }

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
