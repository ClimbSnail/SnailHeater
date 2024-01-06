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
#include "page/verification/verification.h"

#include <SDL2/SDL_timer.h>
#include "demos/lv_demos.h"

#define SNAILHEATER_VERSION "1.9.9"

int saveSettingAPI(void *parameter)
{
    snprintf(sysInfoModel.sysInfo, 128, "%s", "设置已保存");
    sysInfoModel.sysInfoDispTime = 1000;
    return 0;
}

bool setIntellectRateFlag(bool flag)
{
    // intellectRateFlag = flag;
    return 0;
}

void SnailHeater_UI()
{
    // 开机动画
    startupInitScreen();
    startupUI(SNAILHEATER_VERSION,
              version_info[SH_HARDWARE_VER * 3]);
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
        model.utilConfig.targetTemp = 300;
        model.utilConfig.fastPID = ENABLE_STATE::ENABLE_STATE_CLOSE;
        model.curTemp = 33;
        model.powerRatio = 0;

        model.coreConfig.kp = 40;
        model.coreConfig.ki = 1.25;
        model.coreConfig.kd = 20;
        model.coreConfig.kt = 50;
        for (int i = DISPLAY_TEMP_0; i < DISPLAY_TEMP_MAX; i++)
        {
            model.coreConfig.realTemp[i] = i * DISPLAY_TEMP_STEP;
        }
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
        model.utilConfig.targetTemp = 230;
        model.curTemp = 100;
        model.utilConfig.workAirSpeed = 30;
        model.utilConfig.fastPID = ENABLE_STATE::ENABLE_STATE_CLOSE;
        model.powerRatio = 0;
        model.manageCurveAction = INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_CURVE_IDLE;
        model.manageCoreAction = INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_HP_IDLE;

        model.coreConfig.kp = 50;
        model.coreConfig.ki = 30.00;
        model.coreConfig.kd = 15;
        model.coreConfig.kt = 50;
        for (int i = DISPLAY_TEMP_0; i < DISPLAY_TEMP_MAX; i++)
        {
            model.coreConfig.realTemp[i] = i * DISPLAY_TEMP_STEP;
        }

        model.curveConfig.id = 0;
        model.curveConfig.stageNum = 6;
        int data[MAX_STAGE_NUM] = {0, 150, 160, 250, 250, 100};
        int time[MAX_STAGE_NUM] = {2, 60, 150, 180, 210, 270};
        model.curveAllTime = time[model.curveConfig.stageNum - 1] + 20;
        model.curveRunTime = 0;
        for (int ind = 0; ind < MAX_STAGE_NUM; ++ind)
        {
            model.curveConfig.temperatur[ind] = (uint16_t)data[ind];
            model.curveConfig.time[ind] = time[ind];
        }
        setHeatplatformInfo(&model);
    }

    if (true)
    {
        SolderModel model;
        model.workState = SOLDER_STATE_DEEP_SLEEP;
        model.utilConfig.curCoreID = 0;
        model.coreNameList = "0_NUKNOWN\n1_T12\n2_C210\n3_C245";
        model.coreConfig.solderType = SOLDER_TYPE_T12;
        model.coreConfig.wakeSwitchType = SOLDER_SHAKE_TYPE_CHANGE;
        // model.coreConfig.wakeSwitchType = SOLDER_SHAKE_TYPE_NONE;
        model.coreConfig.kp = 150;
        model.coreConfig.ki = 4.0;
        model.coreConfig.kd = 120;
        model.coreConfig.kt = 50;
        for (int i = DISPLAY_TEMP_0; i < DISPLAY_TEMP_MAX; i++)
        {
            model.coreConfig.realTemp[i] = i * DISPLAY_TEMP_STEP;
        }
        model.tempEnable.allValue = 0x01;
        model.utilConfig.quickSetupTemp_0 = 270;
        model.utilConfig.quickSetupTemp_1 = 360;
        model.utilConfig.quickSetupTemp_2 = 400;
        model.fineAdjTemp = 300;
        model.utilConfig.targetTemp = 300;
        model.utilConfig.easySleepTemp = 150;
        model.utilConfig.autoTypeSwitch = ENABLE_STATE::ENABLE_STATE_OPEN;
        model.utilConfig.fastPID = ENABLE_STATE::ENABLE_STATE_CLOSE;
        model.curTemp = 30;
        model.powerRatio = 0;
        model.manageCoreAction = INFO_MANAGE_ACTION::INFO_MANAGE_ACTION_SOLDER_IDLE;
        setSolderInfo(&model);
    }

    if (true)
    {
        AdjPowerModel model;
        model.workState = ENABLE_STATE_CLOSE;

        model.utilConfig.quickSetupVoltage_0 = 5000;
        model.utilConfig.quickSetupVoltage_1 = 12000;
        model.utilConfig.quickSetupVoltage_2 = 20000;

        model.utilConfig.volDacValue = 0;
        model.utilConfig.curDacValue = 0;
        model.utilConfig.settingVoltage = model.utilConfig.quickSetupVoltage_0;
        model.utilConfig.settingCurrent = 1000;
        model.utilConfig.mode = 0;
        model.voltage = 20000;
        model.current = 3200;
        model.capacity = model.capacity * model.current;
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
        model.coreConfig.srceenVersion = VERSION_INFO_SRCEEN_V25;
        model.coreConfig.coreVersion = VERSION_INFO_CORE_V25;
        model.coreConfig.outBoardVersion = VERSION_INFO_OUT_BOARD_V25;
        snprintf(model.softwareVersion, 16, "%s", SNAILHEATER_VERSION);
        model.utilConfig.knobDir = KNOBS_DIR_POS;
        model.utilConfig.touchFlag = ENABLE_STATE_OPEN;
        model.utilConfig.sysTone = ENABLE_STATE_OPEN;
        model.utilConfig.knobTone = ENABLE_STATE_OPEN;
        model.utilConfig.isStartupAnim = ENABLE_STATE_OPEN;
        model.utilConfig.isStartupBell = ENABLE_STATE_OPEN;
        model.utilConfig.isRunAnim = ENABLE_STATE_OPEN;
        model.utilConfig.enableWallpaper = ENABLE_STATE_OPEN;
        model.utilConfig.enableStaticWallpaper = ENABLE_STATE_OPEN;
        model.utilConfig.enableDynamicWallpaper = ENABLE_STATE_OPEN;
        model.utilConfig.pilotLampLight = 128;
        model.utilConfig.pilotLampSpeed = 80;
        model.utilConfig.lockScreenDelayTime = 10;
        model.utilConfig.wallpaperSwitchTime = 5;
        model.utilConfig.uiParam.uiGlobalParam = {0, 0, 1, 0, 1};
        model.lockScreenFlag = false;
        // model.lockScreenFlag = true;
        model.saveConfAPI = saveSettingAPI;
        model.setIntellectRateFlag = setIntellectRateFlag;

        setSysInfo(&model);
    }

    // 绘制页面UI
    ui_init(indev_drv_p);

    // verification_init_1(indev_drv_p, "7731");
}

int main(void)
{
    lv_init();

    hal_setup();

    // lv_demo_widgets();
    SnailHeater_UI();

    hal_loop();
}
