
#include "common.h"
#include "hot_air.h"

// 持久化配置
#define HOT_AIR_ONE_CONFIG_PATH "/hot_air_one_v1.cfg"
#define HOT_AIR_SECOND_CONFIG_PATH "/hot_air_second_v1.cfg"

void HotAir::write_config(HA_Config *cfg, int num)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->id);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->settingTemp);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->workAirSpeed);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->coolingAirSpeed);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->coolingFinishTemp);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->kp);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->ki);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->kd);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->kt);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->pid_start_low_value);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->pid_start_high_value);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->calibrationError_100);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->calibrationError_135);
    w_data += tmp;

    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d\n", cfg->calibrationError_170);
    w_data += tmp;

    if (num == 0)
    {
        g_flashCfg.writeFile(HOT_AIR_ONE_CONFIG_PATH, w_data.c_str());
    }
    else
    {
        g_flashCfg.writeFile(HOT_AIR_SECOND_CONFIG_PATH, w_data.c_str());
    }
}

void HotAir::read_config(HA_Config *cfg, int num)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[256] = {0};
    uint16_t size = 0;
    if (num == 0)
    {
        size = g_flashCfg.readFile(HOT_AIR_ONE_CONFIG_PATH, (uint8_t *)info);
    }
    else
    {
        size = g_flashCfg.readFile(HOT_AIR_SECOND_CONFIG_PATH, (uint8_t *)info);
    }

    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->id = 0;
        cfg->settingTemp = 350;
        cfg->workAirSpeed = 50;
        cfg->coolingAirSpeed = 100;
        cfg->coolingFinishTemp = 40;
        cfg->kp = 0;
        cfg->ki = 0;
        cfg->kd = 0;
        cfg->kt = 0;
        cfg->pid_start_low_value = 30;
        cfg->pid_start_high_value = 1;
        cfg->calibrationError_100 = 0;
        cfg->calibrationError_135 = 0;
        cfg->calibrationError_170 = 0;
        write_config(cfg, num);
    }
    else
    {
        // 解析数据
        char *param[14] = {0};
        analyseParam(info, 14, param);
        cfg->id = atoi(param[0]);
        cfg->settingTemp = atoi(param[1]);
        cfg->workAirSpeed = atoi(param[2]);
        cfg->coolingAirSpeed = atoi(param[3]);
        cfg->coolingFinishTemp = atoi(param[4]);
        cfg->kp = atoi(param[5]);
        cfg->ki = atoi(param[6]);
        cfg->kd = atoi(param[7]);
        cfg->kt = atoi(param[8]);
        cfg->pid_start_low_value = atoi(param[9]);
        cfg->pid_start_high_value = atoi(param[10]);
        cfg->calibrationError_100 = atoi(param[11]);
        cfg->calibrationError_135 = atoi(param[12]);
        cfg->calibrationError_170 = atoi(param[13]);
    }
}