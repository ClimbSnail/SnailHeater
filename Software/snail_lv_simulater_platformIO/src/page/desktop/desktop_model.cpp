
#include "desktop_model.h"

struct SolderModel solderModel = {SOLDER_TYPE_T12, 0, 0, SOLDER_STATE_DEEP_SLEEP, 0};
struct AirhotModel airhotModel = {0, 0, 20, HOTAIR_STATE_SLEEP, 0};
struct HeatplatformModel heatplatformModel = {0, 0, HOTAIR_STATE_SLEEP, 0};
struct AdjPowerModel adjPowerModel = {0, 0, 0, 0, ADJ_POWER_OPEN_STATE_CLOSE};
struct SysInfoModel sysInfoModel = {"", "", "", "", "", KNOBS_DIR_POS};

/*
 *   电烙铁
 *
 */
int setSolderInfo(unsigned char type, unsigned char wakeType,
                  int targetTemp, int curTemp,
                  unsigned char workState, uint16_t powerRatio)
{
    solderModel.type = type;
    solderModel.wakeType = wakeType;
    solderModel.targetTemp = targetTemp;
    solderModel.curTemp = curTemp;
    solderModel.workState = workState;
    solderModel.powerRatio = powerRatio;
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
int setAirhotInfo(int targetTemp, int curTemp,
                  unsigned int workAirSpeed, unsigned char workState,
                  uint16_t powerRatio)
{
    airhotModel.targetTemp = targetTemp;
    airhotModel.curTemp = curTemp;
    airhotModel.workAirSpeed = workAirSpeed;
    airhotModel.workState = workState;
    airhotModel.powerRatio = powerRatio;
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
int setHeatplatformInfo(int targetTemp, int curTemp,
                        unsigned char enable, uint16_t powerRatio)
{
    heatplatformModel.targetTemp = targetTemp;
    heatplatformModel.curTemp = curTemp;
    heatplatformModel.enable = enable;
    heatplatformModel.powerRatio = powerRatio;
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
int setAdjPowerInfo(int adcValue, int32_t voltage,
                    int32_t current, int32_t capacity,
                    unsigned char workState)
{
    adjPowerModel.adcValue = adcValue;
    adjPowerModel.voltage = voltage;
    adjPowerModel.current = current;
    adjPowerModel.capacity = capacity;
    adjPowerModel.workState = workState;
    return 0;
}

int setVoltage(int32_t voltage)
{
    adjPowerModel.voltage = voltage;
    return 0;
}

int setCurrent(int32_t current)
{
    adjPowerModel.current = current;
    return 0;
}

int setCapacity(int32_t capacity)
{
    adjPowerModel.capacity = capacity;
    return 0;
}

int setWorkState(unsigned char workState)
{
    adjPowerModel.workState = workState;
    return 0;
}

/*
 *   系统设置
 *
 */
int setSysInfo(const char *sn, const char *srceenVersion,
               const char *coreVersion, const char *outBoardVersion,
               const char *softwareVersion, KNOBS_DIR knobDir)
{
    snprintf(sysInfoModel.sn, 32, "%s", sn);
    snprintf(sysInfoModel.srceenVersion, 16, "%s", srceenVersion);
    snprintf(sysInfoModel.coreVersion, 16, "%s", coreVersion);
    snprintf(sysInfoModel.outBoardVersion, 16, "%s", outBoardVersion);
    snprintf(sysInfoModel.softwareVersion, 16, "%s", softwareVersion);
    sysInfoModel.knobDir = knobDir;

    return 0;
}

int setKnobsDir(KNOBS_DIR knobDir)
{
    sysInfoModel.knobDir = knobDir;
    return 0;
}