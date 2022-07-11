#ifndef CONFIG_H
#define CONFIG_H
#include <WString.h>

#define HEAT_OBJ_MAX_NUM 2
#define PID_PARAM_MAX_NUM 10

struct ExternConfig
{
    String ssid;        // wifi id
    String password;    // wifi密码
    String cityname;    // 显示的城市
    String language;    // 天气查询的地址编码
    String weather_key; // 知心天气api_key（秘钥）
    uint8_t backLight;  // 屏幕亮度（1-100）
    uint8_t rotation;   // 屏幕旋转方向
};

// PID的参数信息
struct PidParam
{
    double kp;
    double ki;
    double kd;
    double kt;
    int pid_threshold_value; // PID开始控制的阈值范围
};

struct HeatSetting
{
    int mode;                 // 模式
    int save_temperature;     // 保存的温度
    int err_temperature;      // 误差温度
    int overflow_temperature; // 超温度的报警温差
    int pid_id;               // PID参数配置编号 指向 PidParam的一个实体
};

struct Config
{
    int encoder_direction;                 // 编码器方向 0正 1反
    int pid_param_num;  //  保存用户设定的PID参数的数量
    HeatSetting heat_obj[HEAT_OBJ_MAX_NUM]; // 加热台的实体 这里是双路输出，就选定为两个
    PidParam pid_param[PID_PARAM_MAX_NUM]; // 保存pid的参数设置
    ExternConfig extern_info;          // 附加功能的参数
};

void config_read(Config *cfg);

void config_save(Config *cfg);

void heat_setting_read(HeatSetting *heat_cfg, int num);

void heat_setting_save(HeatSetting *heat_cfg, int num);

void pid_param_read(PidParam *pid_cfg, int num);

void pid_param_save(PidParam *pid_cfg, int num);

void extern_info_read(ExternConfig *extern_cfg);

void extern_info_save(ExternConfig *extern_cfg);

#endif