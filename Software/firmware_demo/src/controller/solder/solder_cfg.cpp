
#include "common.h"
#include "solder.h"

// 持久化配置
#define SOLDER_CONFIG_PATH "/solder_v2.cfg"

void Solder::write_config(SolderConfig *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->wakeSensitivity);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->settingTemp);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->easySleepTemp);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->enterEasySleepTime);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->enterDeepSleepTime);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->alarmValue);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->heaterCnt);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->curID);
    w_data += tmp;

    g_flashCfg.writeFile(SOLDER_CONFIG_PATH, w_data.c_str());
}

void Solder::read_config(SolderConfig *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[256] = {0};
    uint16_t size = 0;
    size = g_flashCfg.readFile(SOLDER_CONFIG_PATH, (uint8_t *)info);

    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->wakeSensitivity = 0;
        cfg->settingTemp = 300;
        cfg->easySleepTemp = 150;
        cfg->enterEasySleepTime = 180000;
        cfg->enterDeepSleepTime = 1800000;
        // cfg->enterEasySleepTime = 30000;
        // cfg->enterDeepSleepTime = 60000;
        cfg->alarmValue = 50;
        cfg->heaterCnt = 50;
        cfg->curID = 0;
        write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[8] = {0};
        analyseParam(info, 8, param);
        cfg->wakeSensitivity = atoi(param[0]);
        cfg->settingTemp = atoi(param[1]);
        cfg->easySleepTemp = atoi(param[2]);
        cfg->enterEasySleepTime = atoi(param[3]);
        cfg->enterDeepSleepTime = atoi(param[4]);
        cfg->alarmValue = atoi(param[5]);
        cfg->heaterCnt = atoi(param[6]);
        cfg->curID = atoi(param[7]);
    }
}