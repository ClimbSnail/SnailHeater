
#include "common.h"
#include "heat_platform.h"

// 持久化配置
#define HEAT_PLATFORM_CONFIG_PATH "/heat_platform_v1.cfg"

void HeatPlatform::write_config(HPConfig *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->settingTemp);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->calibrationError_50);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->calibrationError_100);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->calibrationError_150);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->calibrationError_200);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->calibrationError_250);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->calibrationError_300);
    w_data += tmp;

    g_flashCfg.writeFile(HEAT_PLATFORM_CONFIG_PATH, w_data.c_str());
}

void HeatPlatform::read_config(HPConfig *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(HEAT_PLATFORM_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->settingTemp = 230;
        cfg->calibrationError_50 = 1;
        cfg->calibrationError_100 = 1;
        cfg->calibrationError_150 = 1;
        cfg->calibrationError_200 = 1;
        cfg->calibrationError_250 = 1;
        cfg->calibrationError_300 = 1;
        write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[7] = {0};
        analyseParam(info, 7, param);
        cfg->settingTemp = atol(param[0]);
        cfg->calibrationError_50 = atol(param[1]);
        cfg->calibrationError_100 = atol(param[2]);
        cfg->calibrationError_150 = atol(param[3]);
        cfg->calibrationError_200 = atol(param[4]);
        cfg->calibrationError_250 = atol(param[5]);
        cfg->calibrationError_300 = atol(param[6]);
    }
}