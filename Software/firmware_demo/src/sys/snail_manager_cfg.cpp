
#include "common.h"
#include "snail_manager.h"

// 持久化配置
#define SNAIL_MANAGER_CONFIG_PATH "/snail_manager_v2.cfg"

void SnailManager::write_config(SysConfig *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;

    w_data = w_data + cfg->sn + "\n";

    w_data = w_data + cfg->srceenVersion + "\n";

    w_data = w_data + cfg->coreVersion + "\n";

    w_data = w_data + cfg->outBoardVersion + "\n";

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->backLight);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->pilotLampLight);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->pilotLampSpeed);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->isStartupAnim);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->isStartupBell);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->lockScreenTime);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->adcVrefVoltage);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->sysVoltage);
    w_data += tmp;

    g_flashCfg.writeFile(SNAIL_MANAGER_CONFIG_PATH, w_data.c_str());
}

void SnailManager::read_config(SysConfig *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[256] = {0};
    uint16_t size = 0;
    size = g_flashCfg.readFile(SNAIL_MANAGER_CONFIG_PATH, (uint8_t *)info);

    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->sn = "";
        cfg->srceenVersion = "v2.0";
        cfg->coreVersion = "v2.0";
        cfg->outBoardVersion = "v2.0";
        cfg->backLight = 100;
        cfg->pilotLampLight = 100;
        cfg->pilotLampSpeed = 50;
        cfg->knobOrder = 0;
        cfg->isStartupAnim = 1;
        cfg->isStartupBell = 1;
        cfg->lockScreenTime = 30;
        cfg->adcVrefVoltage = 2516;
        cfg->sysVoltage = 3300;
        write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[13] = {0};
        analyseParam(info, 13, param);
        cfg->sn = param[0];
        cfg->srceenVersion = param[1];
        cfg->coreVersion = param[2];
        cfg->outBoardVersion = param[3];

        cfg->backLight = atoi(param[4]);
        cfg->pilotLampLight = atoi(param[5]);
        cfg->pilotLampSpeed = atoi(param[6]);

        cfg->knobOrder = atoi(param[7]);
        cfg->isStartupAnim = atoi(param[8]);
        cfg->isStartupBell = atoi(param[9]);
        cfg->lockScreenTime = atoi(param[10]);

        cfg->adcVrefVoltage = atoi(param[11]);
        cfg->sysVoltage = atoi(param[12]);
    }
}