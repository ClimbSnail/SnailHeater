#include <WString.h>
#include "common.h"
#include "config.h"

void config_read(Config *cfg)
{
    prefs.begin("Config"); // 打开命名空间 namespace
    cfg->encoder_direction = prefs.getInt("encoder_direction", 0);
    cfg->pid_param_num = prefs.getInt("pid_param_num", 0);
    prefs.end(); // 关闭当前命名空间

    int cnt = 0;

    // 读取用户设置的加热对象信息
    for (cnt = 0; cnt < HEAT_OBJ_MAX_NUM; ++cnt)
    {
        heat_setting_read(&cfg->heat_obj[cnt], cnt);
    }

    // 读取用户设置的pid信息
    for (cnt = 0; cnt < cfg->pid_param_num; ++cnt)
    {
        pid_param_read(&cfg->pid_param[cnt], cnt);
    }

    // 读取用户设置的拓展应用的信息
    extern_info_read(&cfg->extern_info);
}

void config_save(Config *cfg)
{
    prefs.begin("Config"); // 打开命名空间 namespace
    prefs.putInt("encoder_direction", cfg->encoder_direction);
    prefs.putInt("pid_param_num", cfg->pid_param_num);
    prefs.end(); // 关闭当前命名空间

    // 这里先不操作保存其他数据
}

void heat_setting_read(HeatSetting *heat_cfg, int num)
{
    char prefs_namespace[30]; // 将要打开的名字看空间
    snprintf(prefs_namespace, 30, "HeatSetting%d", num);
    prefs.begin(prefs_namespace); // 打开命名空间namespace
    heat_cfg->mode = prefs.getInt("mode", 0);
    heat_cfg->save_temperature = prefs.getInt("save_temperature", 30);
    heat_cfg->err_temperature = prefs.getInt("err_temperature", 0);
    heat_cfg->overflow_temperature = prefs.getInt("overflow_temperature", 30);
    heat_cfg->pid_id = prefs.getInt("pid_id", 0);
    prefs.end(); // 关闭当前命名空间
}

void heat_setting_save(HeatSetting *heat_cfg, int num)
{
    char prefs_namespace[30]; // 将要打开的名字看空间
    snprintf(prefs_namespace, 30, "HeatSetting%d", num);
    prefs.begin(prefs_namespace); // 打开命名空间namespace
    prefs.putInt("mode", heat_cfg->mode);
    prefs.putInt("save_temperature", heat_cfg->save_temperature);
    prefs.putInt("err_temperature", heat_cfg->err_temperature);
    prefs.putInt("overflow_temperature", heat_cfg->overflow_temperature);
    prefs.putInt("pid_id", heat_cfg->pid_id);
    prefs.end(); // 关闭当前命名空间
}

void pid_param_read(PidParam *pid_cfg, int num)
{
    char prefs_namespace[30]; // 将要打开的名字看空间
    snprintf(prefs_namespace, 30, "PidParam%d", num);
    prefs.begin(prefs_namespace); // 打开命名空间namespace
    pid_cfg->kp = prefs.getDouble("kp", 4.5);
    pid_cfg->ki = prefs.getDouble("ki", 0.15);
    pid_cfg->kd = prefs.getDouble("kd", 0.2);
    pid_cfg->kt = prefs.getDouble("kt", 0.2);
    pid_cfg->pid_threshold_value = prefs.getInt("pid_threshold_value", 10);
    prefs.end(); // 关闭当前命名空间
}

void pid_param_save(PidParam *pid_cfg, int num)
{
    char prefs_namespace[30]; // 将要打开的名字看空间
    snprintf(prefs_namespace, 30, "PidParam%d", num);
    prefs.begin(prefs_namespace); // 打开命名空间namespace
    prefs.putDouble("kp", pid_cfg->kp);
    prefs.putDouble("ki", pid_cfg->ki);
    prefs.putDouble("kd", pid_cfg->kd);
    prefs.putDouble("kt", pid_cfg->kt);
    prefs.putInt("pid_threshold_value", pid_cfg->pid_threshold_value);
    prefs.end(); // 关闭当前命名空间
}

void extern_info_read(ExternConfig *extern_cfg)
{
    prefs.begin("ExternConfig"); // 打开命名空间 namespace
    extern_cfg->ssid = prefs.getString("ssid", "");
    extern_cfg->password = prefs.getString("password", "");
    extern_cfg->cityname = prefs.getString("cityname", "BeiJing");
    extern_cfg->language = prefs.getString("language", "zh-Hans");
    extern_cfg->weather_key = prefs.getString("weather_key", "");
    extern_cfg->backLight = prefs.getUChar("backLight", 100);
    extern_cfg->rotation = prefs.getUChar("rotation", 0);
    prefs.end(); // 关闭当前命名空间
}

void extern_info_save(ExternConfig *extern_cfg)
{
    prefs.begin("ExternConfig"); // 打开命名空间 namespace
    prefs.putString("ssid", extern_cfg->ssid);
    prefs.putString("password", extern_cfg->password);
    prefs.putString("cityname", extern_cfg->cityname);
    prefs.putString("language", extern_cfg->language);
    prefs.putString("weather_key", extern_cfg->weather_key);
    prefs.putUChar("backLight", extern_cfg->backLight);
    prefs.putUChar("rotation", extern_cfg->rotation);
    prefs.end(); // 关闭当前命名空间

    extern_info_read(&g_cfg.extern_info);
    // 立即更改屏幕方向
    screen.setBackLight(extern_cfg->backLight / 100.0);
    tft->setRotation(extern_cfg->rotation);
}