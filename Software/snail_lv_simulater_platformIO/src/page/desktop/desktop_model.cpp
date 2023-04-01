
#include "desktop_model.h"

struct SolderModel solderModel = {SOLDER_STATE_DEEP_SLEEP, SOLDER_TYPE_T12,
                                  SOLDER_SHAKE_TYPE_CHANGE, 1, 270, 360, 400, 300,
                                  0, 3, 0, 0};

struct AirhotModel airhotModel = {HOTAIR_STATE_SLEEP, 0,
                                  250, 350, 400, 350,
                                  0, 0, 30, 0};

struct HeatplatformModel heatplatformModel = {HP_STATE_SLEEP, ENABLE_STATE_CLOSE, 1,
                                              180, 245, 270, 230,
                                              0, 0, 30, 0};

struct AdjPowerModel adjPowerModel = {ENABLE_STATE_CLOSE, 0, 0,
                                      0, 0, 0, 0};

struct StopWelderModel stopWelderModel = {SPOTWELDER_STATE_WAIT, SPOTWELDER_MODE_DOUBLE,
                                          5, 1000, 2,
                                          2700, 2000, 2700};

struct SysInfoModel sysInfoModel = {"", VERSION_INFO_SRCEEN_V20,
                                    VERSION_INFO_CORE_V20, VERSION_INFO_OUT_BOARD_V20,
                                    "", KNOBS_DIR_POS};

/*
 *   电烙铁
 *
 */
int setSolderInfo(SolderModel *model)
{
    solderModel = *model;
    return 0;
}

int setSolderCurTempAndPowerDuty(int curTemp, uint16_t powerRatio)
{
    solderModel.curTemp = curTemp;
    solderModel.powerRatio = powerRatio;
    return 0;
}

int setSolderWorkState(unsigned char workState)
{
    solderModel.workState = workState;
    return 0;
}

/*
 *   热风枪
 *
 */
int setAirhotInfo(AirhotModel *model)
{
    airhotModel = *model;
    return 0;
}

int setAirhotCurTempAndPowerDuty(int curTemp, uint16_t powerRatio)
{
    airhotModel.curTemp = curTemp;
    airhotModel.powerRatio = powerRatio;
    return 0;
}

int setAirhotWorkState(unsigned char workState)
{
    airhotModel.workState = workState;
    return 0;
}

/*
 *   加热台
 *
 */
int setHeatplatformInfo(HeatplatformModel *model)
{
    heatplatformModel = *model;
    return 0;
}

int setHeatplatformCurTempAndPowerDuty(int curTemp, uint16_t powerRatio)
{
    heatplatformModel.curTemp = curTemp;
    heatplatformModel.powerRatio = powerRatio;
    return 0;
}

int setHeatplatformEnable(unsigned char enable)
{
    heatplatformModel.enable = enable;
    return 0;
}

/*
 *   可调电源
 *
 */
int setAdjPowerInfo(AdjPowerModel *model)
{
    adjPowerModel = *model;
    return 0;
}

int setAdjPowerVoltage(int32_t voltage)
{
    adjPowerModel.voltage = voltage;
    return 0;
}

int setAdjPowerCurrent(int32_t current)
{
    adjPowerModel.current = current;
    return 0;
}

int setAdjPowerCapacity(int32_t capacity)
{
    adjPowerModel.capacity = capacity;
    return 0;
}

int setAdjPowerWorkState(unsigned char workState)
{
    adjPowerModel.workState = workState;
    return 0;
}

/*
 *   点焊机设置
 *
 */
int setStopWelderInfo(StopWelderModel mode)
{
    stopWelderModel = mode;
    return 0;
}

int setStopWelderVoltage(uint16_t voltage)
{
    stopWelderModel.voltage = voltage;
    return 0;
}

int setStopWelderWorkState(unsigned char workState)
{
    stopWelderModel.workState = workState;
    return 0;
}

/*
 *   系统设置
 *
 */
int setSysInfo(SysInfoModel *model)
{
    sysInfoModel = *model;
    return 0;
}

int setKnobsDir(KNOBS_DIR knobDir)
{
    sysInfoModel.knobDir = knobDir;
    return 0;
}