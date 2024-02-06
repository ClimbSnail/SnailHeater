#include "./ui.h"
#include "../desktop_model.h"

#ifdef FULL_UI

static lv_timer_t *signalTimer = NULL;
static lv_obj_t *signalPageUI = NULL;
static lv_obj_t *ui_enableSwitch;
static lv_obj_t *signalImg;
static lv_obj_t *earphone3Img;
static lv_obj_t *ui_signalTypeBtn;
static lv_obj_t *ui_freqSet;
static lv_obj_t *ui_dutyCycleLabel;
static lv_obj_t *ui_dutyCycleBtn;
static lv_obj_t *ui_pulseWidthLabel;
static lv_obj_t *ui_pulseWidth;
static lv_obj_t *ui_freqStepDropdown;

static lv_obj_t *ui_scaleLabel;
static lv_obj_t *ui_scaleDropdown;
static lv_obj_t *ui_phaseLabel;
static lv_obj_t *ui_phaseDropdown;

static lv_group_t *btn_group = NULL;

static void ui_set_obj_change(lv_event_t *e);
static void ui_enable_switch_pressed(lv_event_t *e);
static void ui_bnt_obj_pressed(lv_event_t *e);
void ui_updateSignalData(void);
void ui_updateSignalWorkState(void);
static void signalPageUI_release();

static void signalPageUI_focused(lv_event_t *e)
{
    lv_indev_set_group(knobs_indev, btn_group);
    if (signalGeneratorModel.workState == 1)
    {
        lv_group_focus_obj(ui_enableSwitch);
    }
    else
    {
        lv_group_focus_obj(ui_backBtn);
    }
}

static void signalTimer_timeout(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    ui_updateSignalData();
    ui_updateSignalWorkState();
}

static void ui_freq_step_change(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        uint16_t index = lv_dropdown_get_selected(ui_freqStepDropdown); // 获取索引
        int cnt;
        for (cnt = 0, signalGeneratorModel.freqStep = 1; cnt < index; ++cnt)
        {
            signalGeneratorModel.freqStep *= 10;
        }
        lv_numberbtn_set_step(ui_freqSet, signalGeneratorModel.freqStep);
        LV_LOG_USER("freqStep %d", signalGeneratorModel.freqStep);
    }
}

static void ui_scale_change(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        uint16_t index = lv_dropdown_get_selected(ui_scaleDropdown); // 获取索引
        signalGeneratorModel.utilConfig.scale = index;
    }
}

static void ui_phase_change(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        uint16_t index = lv_dropdown_get_selected(ui_phaseDropdown); // 获取索引
        if (index == 0)
        {
            signalGeneratorModel.utilConfig.phase = 0x2;
        }
        else if (index == 1)
        {
            signalGeneratorModel.utilConfig.phase = 0x3;
        }
    }
}

static void setSignalImg()
{
    if (SIGNAL_TYPE::SIGNAL_TYPE_SINE_WARE == signalGeneratorModel.utilConfig.signalType)
    {
        // 正弦波
        lv_img_set_src(signalImg, &signal_sine);
        lv_group_remove_obj(ui_enableSwitch);
        lv_group_remove_obj(ui_dutyCycleBtn);
        lv_group_add_obj(btn_group, ui_scaleDropdown);
        lv_group_add_obj(btn_group, ui_phaseDropdown);
        lv_group_add_obj(btn_group, ui_enableSwitch);
        // 显示控件
        lv_obj_set_style_text_opa(ui_scaleLabel, 255, LV_PART_MAIN);
        lv_obj_set_style_text_opa(ui_phaseLabel, 255, LV_PART_MAIN);
        lv_obj_set_style_text_opa(ui_scaleDropdown, 255, LV_PART_MAIN);
        lv_obj_set_style_text_opa(ui_phaseDropdown, 255, LV_PART_MAIN);

        lv_obj_set_style_border_width(ui_scaleDropdown, 2, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(ui_scaleDropdown, 255, LV_PART_MAIN);
        lv_obj_set_style_border_width(ui_phaseDropdown, 2, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(ui_phaseDropdown, 255, LV_PART_MAIN);
        // 隐藏控件
        lv_obj_set_style_text_opa(ui_dutyCycleLabel, 0, LV_PART_MAIN);
        lv_obj_set_style_text_opa(ui_dutyCycleBtn, 0, LV_PART_MAIN);
        lv_obj_set_style_text_opa(ui_pulseWidthLabel, 0, LV_PART_MAIN);
        lv_obj_set_style_text_opa(ui_pulseWidth, 0, LV_PART_MAIN);
        // 频率限制
        lv_numberbtn_set_range(ui_freqSet, 130, 55000);
        lv_numberbtn_set_value(ui_freqSet, signalGeneratorModel.utilConfig.freqDAC);
    }
    else
    {
        // 矩形波时
        lv_img_set_src(signalImg, &signal_square);
        lv_group_remove_obj(ui_enableSwitch);
        lv_group_remove_obj(ui_scaleDropdown);
        lv_group_remove_obj(ui_phaseDropdown);
        lv_group_add_obj(btn_group, ui_dutyCycleBtn);
        lv_group_add_obj(btn_group, ui_enableSwitch);
        // 隐藏控件
        lv_obj_set_style_text_opa(ui_scaleLabel, 0, LV_PART_MAIN);
        lv_obj_set_style_text_opa(ui_phaseLabel, 0, LV_PART_MAIN);
        lv_obj_set_style_text_opa(ui_scaleDropdown, 0, LV_PART_MAIN);
        lv_obj_set_style_text_opa(ui_phaseDropdown, 0, LV_PART_MAIN);

        lv_obj_set_style_border_width(ui_scaleDropdown, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(ui_scaleDropdown, 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(ui_phaseDropdown, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(ui_phaseDropdown, 0, LV_PART_MAIN);
        // 显示控件
        lv_obj_set_style_text_opa(ui_dutyCycleLabel, 255, LV_PART_MAIN);
        lv_obj_set_style_text_opa(ui_dutyCycleBtn, 255, LV_PART_MAIN);
        lv_obj_set_style_text_opa(ui_pulseWidthLabel, 255, LV_PART_MAIN);
        lv_obj_set_style_text_opa(ui_pulseWidth, 255, LV_PART_MAIN);
        // 频率限制
        lv_numberbtn_set_range(ui_freqSet, 1, 80000);
        lv_numberbtn_set_value(ui_freqSet, signalGeneratorModel.utilConfig.freqPwm);
    }
}

static void setFreqStep(lv_obj_t *obj, unsigned int step)
{
    uint8_t ind;

    switch (step)
    {
    case 1:
    {
        ind = 0;
    }
    break;
    case 10:
    {
        ind = 1;
    }
    break;
    case 100:
    {
        ind = 2;
    }
    break;
    case 1000:
    {
        ind = 3;
    }
    break;
    default:
    {
        ind = 0;
    }
    break;
    }

    lv_dropdown_set_selected(obj, ind);
}

static void countPulseWidth()
{
    double pulseWidth = 1000000.0 / signalGeneratorModel.utilConfig.freqPwm * (signalGeneratorModel.utilConfig.duty / 1000.0);
    if (pulseWidth < 1000)
    {
        lv_label_set_text_fmt(ui_pulseWidth, "%.1fus", pulseWidth);
    }
    else if (pulseWidth < 1000000)
    {
        lv_label_set_text_fmt(ui_pulseWidth, "%.1fms", pulseWidth / 1000.0);
    }
}

static bool signalPageUI_init(lv_obj_t *father)
{
    signalPageUI_release();

    top_layer_set_name();
    theme_color_init();

    signalPageUI = lv_btn_create(father);
    signalUIObj.mainButtonUI = signalPageUI;
    lv_obj_t *ui_ButtonTmp = signalPageUI;

    lv_obj_remove_style_all(ui_ButtonTmp);
    lv_obj_set_size(ui_ButtonTmp, EACH_PAGE_SIZE_X, EACH_PAGE_SIZE_Y);
    lv_obj_center(ui_ButtonTmp);
    lv_obj_add_flag(ui_ButtonTmp, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_ButtonTmp, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_style_bg_color(signalPageUI, IS_WHITE_THEME ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_ButtonTmp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 频率调节步进
    lv_obj_t *ui_freqSetStepLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align(ui_freqSetStepLabel, LV_ALIGN_TOP_RIGHT, -85, 20);
    lv_label_set_text_fmt(ui_freqSetStepLabel, "%s", TEXT_SIGNAL_FREQ_STEP);
    lv_obj_add_style(ui_freqSetStepLabel, &label_text_style, 0);

    ui_freqStepDropdown = lv_dropdown_create(ui_ButtonTmp);
    lv_dropdown_set_options(ui_freqStepDropdown, "1\n10\n100\n1K");
    // 初始已当前选择的烙铁芯为主
    setFreqStep(ui_freqStepDropdown, signalGeneratorModel.freqStep);
    lv_obj_set_size(ui_freqStepDropdown, 50, LV_SIZE_CONTENT);
    lv_obj_align_to(ui_freqStepDropdown, ui_freqSetStepLabel, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_add_flag(ui_freqStepDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_text_font(ui_freqStepDropdown, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_freqStepDropdown, SIGNAL_THEME_COLOR1,
                                   LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_freqStepDropdown, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_freqStepDropdown, 2, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 频率
    lv_obj_t *ui_freqSetLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(ui_freqSetLabel, ui_freqSetStepLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    lv_label_set_text_fmt(ui_freqSetLabel, "%s", TEXT_SIGNAL_FREQ);
    lv_obj_add_style(ui_freqSetLabel, &label_text_style, 0);

    ui_freqSet = lv_numberbtn_create(ui_ButtonTmp);
    lv_obj_t *ui_freqSetBtnLabel = lv_label_create(ui_freqSet);
    lv_numberbtn_set_label_and_format(ui_freqSet,
                                      ui_freqSetBtnLabel, "%d", signalGeneratorModel.freqStep);
    lv_numberbtn_set_range(ui_freqSet, 130, 55000);
    lv_numberbtn_set_value(ui_freqSet, signalGeneratorModel.utilConfig.freqDAC);
    lv_obj_remove_style_all(ui_freqSet);
    lv_obj_set_size(ui_freqSet, 70, 20);
    lv_obj_align_to(ui_freqSet, ui_freqSetLabel, LV_ALIGN_OUT_RIGHT_MID, 5, -2);
    lv_obj_add_flag(ui_freqSet, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_freqSet, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_freqSet, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_freqSet, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_freqSet, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_freqSet, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_freqSet, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_freqSet, &btn_type1_pressed_style, LV_STATE_EDITED);

    // 占空比
    ui_dutyCycleLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(ui_dutyCycleLabel, ui_freqSetLabel, LV_ALIGN_OUT_BOTTOM_MID, -3, 20);
    lv_label_set_text_fmt(ui_dutyCycleLabel, "%s", TEXT_SIGNAL_DUTY_CYCLE);
    lv_obj_add_style(ui_dutyCycleLabel, &label_text_style, 0);

    ui_dutyCycleBtn = lv_numberbtn_create(ui_ButtonTmp);
    lv_obj_t *dutyCycleBtnLabel = lv_label_create(ui_dutyCycleBtn);
    lv_numberbtn_set_label_and_format(ui_dutyCycleBtn,
                                      dutyCycleBtnLabel, "%.1f%%", 0.1);
    lv_numberbtn_set_range(ui_dutyCycleBtn, 0, 100);
    lv_numberbtn_set_value(ui_dutyCycleBtn, signalGeneratorModel.utilConfig.duty / 10.0);
    lv_obj_remove_style_all(ui_dutyCycleBtn);
    lv_obj_set_size(ui_dutyCycleBtn, 70, 20);
    lv_obj_align_to(ui_dutyCycleBtn, ui_dutyCycleLabel, LV_ALIGN_OUT_RIGHT_MID, 6, -2);
    lv_obj_add_flag(ui_dutyCycleBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_dutyCycleBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(ui_dutyCycleBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_dutyCycleBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_dutyCycleBtn, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_dutyCycleBtn, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_dutyCycleBtn, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_add_style(ui_dutyCycleBtn, &btn_type1_pressed_style, LV_STATE_EDITED);

    // 脉宽
    ui_pulseWidthLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(ui_pulseWidthLabel, ui_dutyCycleLabel, LV_ALIGN_OUT_BOTTOM_MID, 5, 20);
    lv_label_set_text_fmt(ui_pulseWidthLabel, "%s", TEXT_SIGNAL_PULSE_WIDTH);
    lv_obj_add_style(ui_pulseWidthLabel, &label_text_style, 0);

    ui_pulseWidth = lv_label_create(ui_ButtonTmp);
    // lv_obj_align_to(ui_pulseWidth, ui_pulseWidthLabel, LV_ALIGN_OUT_RIGHT_MID, 12, -3);
    // lv_obj_align_to(ui_pulseWidth, ui_freqSet, LV_ALIGN_OUT_BOTTOM_MID, -20, 65);
    lv_obj_align_to(ui_pulseWidth, ui_freqSet, LV_ALIGN_CENTER, -15, 80);
    countPulseWidth();
    lv_obj_add_style(ui_pulseWidth, &label_text_style, 0);
    lv_obj_set_style_text_color(ui_pulseWidth, lv_color_hex(0x989798), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_pulseWidth, &FontJost_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 振幅
    ui_scaleLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(ui_scaleLabel, ui_freqSetLabel, LV_ALIGN_OUT_BOTTOM_MID, 7, 20);
    lv_label_set_text_fmt(ui_scaleLabel, "%s", TEXT_SIGNAL_SCALE);
    lv_obj_add_style(ui_scaleLabel, &label_text_style, 0);

    ui_scaleDropdown = lv_dropdown_create(ui_ButtonTmp);
    lv_dropdown_set_options(ui_scaleDropdown, "1/1\n1/2\n1/4\n1/8");
    lv_dropdown_set_selected(ui_scaleDropdown, signalGeneratorModel.utilConfig.scale);
    lv_obj_set_size(ui_scaleDropdown, 50, LV_SIZE_CONTENT);
    lv_obj_align_to(ui_scaleDropdown, ui_scaleLabel, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_add_flag(ui_scaleDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_text_font(ui_scaleDropdown, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_scaleDropdown, SIGNAL_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_scaleDropdown, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_scaleDropdown, 2, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 相位
    ui_phaseLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_align_to(ui_phaseLabel, ui_scaleLabel, LV_ALIGN_OUT_BOTTOM_MID, 2, 20);
    lv_label_set_text_fmt(ui_phaseLabel, "%s", TEXT_SIGNAL_PHASE);
    lv_obj_add_style(ui_phaseLabel, &label_text_style, 0);

    ui_phaseDropdown = lv_dropdown_create(ui_ButtonTmp);
    lv_dropdown_set_options(ui_phaseDropdown, "0\n180");
    lv_dropdown_set_selected(ui_phaseDropdown, signalGeneratorModel.utilConfig.phase - 0x2);
    lv_obj_set_size(ui_phaseDropdown, 50, LV_SIZE_CONTENT);
    lv_obj_align_to(ui_phaseDropdown, ui_phaseLabel, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_add_flag(ui_phaseDropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_text_font(ui_phaseDropdown, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_color(ui_phaseDropdown, SIGNAL_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_opa(ui_phaseDropdown, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_phaseDropdown, 2, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 使能开关
    ui_enableSwitch = lv_switch_create(ui_ButtonTmp);
    lv_obj_add_flag(ui_enableSwitch, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_size(ui_enableSwitch, 60, 30);
    lv_obj_align_to(ui_enableSwitch, ui_freqSetLabel, LV_ALIGN_OUT_BOTTOM_MID, 40, 95);
    if (ENABLE_STATE_OPEN == signalGeneratorModel.workState)
    {
        lv_obj_add_state(ui_enableSwitch, LV_STATE_CHECKED); // 开
    }
    else
    {
        lv_obj_clear_state(ui_enableSwitch, LV_STATE_CHECKED); // 关
    }
    // 注意，这里触发的是3个状态 CHECKED 、LV_STATE_FOCUS 、 和 LV_STATE_FOCUS_KEY，必须设置成KEY才有效
    lv_obj_set_style_outline_color(ui_enableSwitch, SIGNAL_THEME_COLOR1, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_bg_color(ui_enableSwitch, SIGNAL_THEME_COLOR1, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_outline_opa(ui_enableSwitch, 255, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(ui_enableSwitch, 2, LV_PART_MAIN | LV_STATE_FOCUS_KEY);

    // 信号图片
    earphone3Img = lv_img_create(ui_ButtonTmp);
    lv_obj_remove_style_all(earphone3Img);
    lv_img_set_src(earphone3Img, &earphone3);
    lv_obj_align(earphone3Img, LV_ALIGN_TOP_LEFT, 10, 15);
    // lv_obj_set_style_img_recolor_opa(earphone3Img, LV_OPA_70, 0);
    // lv_obj_set_style_img_recolor(earphone3Img, SIGNAL_THEME_COLOR1, 0);

    lv_obj_t *pwmLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_remove_style_all(pwmLabel);
    lv_label_set_text(pwmLabel, "PWM");
    lv_obj_align_to(pwmLabel, earphone3Img, LV_ALIGN_OUT_RIGHT_MID, -12, -10);
    lv_obj_add_style(pwmLabel, &label_text_style, 0);
    lv_obj_set_style_text_font(pwmLabel, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *gndLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_remove_style_all(gndLabel);
    lv_label_set_text(gndLabel, "GND");
    lv_obj_align_to(gndLabel, earphone3Img, LV_ALIGN_OUT_RIGHT_MID, -50, -15);
    lv_obj_add_style(gndLabel, &label_text_style, 0);
    lv_obj_set_style_text_font(gndLabel, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *DACLabel = lv_label_create(ui_ButtonTmp);
    lv_obj_remove_style_all(DACLabel);
    lv_label_set_text(DACLabel, "DAC");
    lv_obj_align_to(DACLabel, earphone3Img, LV_ALIGN_OUT_RIGHT_MID, -30, 14);
    lv_obj_add_style(DACLabel, &label_text_style, 0);
    lv_obj_set_style_text_font(DACLabel, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);


    // 信号图片
    signalImg = lv_img_create(ui_ButtonTmp);
    setSignalImg();
    lv_obj_align_to(signalImg, earphone3Img, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_style_img_recolor_opa(signalImg, LV_OPA_70, 0);
    lv_obj_set_style_img_recolor(signalImg, SIGNAL_THEME_COLOR1, 0);

    // 波形切换按钮
    ui_signalTypeBtn = lv_btn_create(ui_ButtonTmp);
    lv_obj_remove_style_all(ui_signalTypeBtn);
    lv_obj_set_size(ui_signalTypeBtn, 60, 30);
    lv_obj_align_to(ui_signalTypeBtn, signalImg, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_obj_add_flag(ui_signalTypeBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(ui_signalTypeBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(ui_signalTypeBtn, &btn_type1_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(ui_signalTypeBtn, &btn_type1_focused_style, LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(ui_signalTypeBtn, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *signalTypeSwitchLabel = lv_label_create(ui_signalTypeBtn);
    lv_label_set_text(signalTypeSwitchLabel, TEXT_SIGNAL_TYPE);
    lv_obj_align(signalTypeSwitchLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(signalTypeSwitchLabel, &label_text_style, 0);

    lv_obj_add_event_cb(ui_signalTypeBtn, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_freqStepDropdown, ui_freq_step_change, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_freqSet, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_dutyCycleBtn, ui_bnt_obj_pressed, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(ui_scaleDropdown, ui_scale_change, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_phaseDropdown, ui_phase_change, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui_enableSwitch, ui_set_obj_change, LV_EVENT_VALUE_CHANGED, NULL);

    btn_group = lv_group_create();
    lv_group_add_obj(btn_group, ui_backBtn);
    lv_group_add_obj(btn_group, ui_signalTypeBtn);
    lv_group_add_obj(btn_group, ui_freqStepDropdown);
    lv_group_add_obj(btn_group, ui_freqSet);
    if (SIGNAL_TYPE::SIGNAL_TYPE_SINE_WARE == signalGeneratorModel.utilConfig.signalType)
    {
        lv_group_add_obj(btn_group, ui_scaleDropdown);
        lv_group_add_obj(btn_group, ui_phaseDropdown);
    }
    else
    {
        lv_group_add_obj(btn_group, ui_dutyCycleBtn);
    }
    lv_group_add_obj(btn_group, ui_enableSwitch);
    lv_indev_set_group(knobs_indev, btn_group);

    signalTimer = lv_timer_create(signalTimer_timeout, DATA_REFRESH_MS, NULL);
    lv_timer_set_repeat_count(signalTimer, -1);
    return true;
}

static void signalPageUI_release()
{
    if (NULL != signalTimer)
    {
        lv_timer_del(signalTimer);
        signalTimer = NULL;
    }

    if (NULL != btn_group)
    {
        lv_group_del(btn_group);
        btn_group = NULL;
    }

    if (NULL != signalPageUI)
    {
        lv_obj_del(signalPageUI);
        signalPageUI = NULL;
        signalUIObj.mainButtonUI = NULL;
    }
}

static void ui_enable_switch_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        bool isEnable = lv_obj_has_state(target, LV_STATE_CHECKED); // 返回 bool 类型， 开-1 ； 关-2
        signalGeneratorModel.workState = isEnable == true ? ENABLE_STATE_OPEN : ENABLE_STATE_CLOSE;
    }
}

void ui_updateSignalData(void)
{
    if (NULL == signalPageUI)
    {
        return;
    }

    // if (SIGNAL_TYPE::SIGNAL_TYPE_SINE_WARE == signalGeneratorModel.utilConfig.signalType)
    // {
    //     lv_numberbtn_set_value(ui_freqSet, signalGeneratorModel.utilConfig.freqDAC);
    // }
    // else if (SIGNAL_TYPE::SIGNAL_TYPE_SQUARE_WARE == signalGeneratorModel.utilConfig.signalType)
    // {
    //     lv_numberbtn_set_value(ui_freqSet, signalGeneratorModel.utilConfig.freqPwm);
    // }
}

void ui_updateSignalWorkState(void)
{
    if (NULL == signalPageUI)
    {
        return;
    }

    if (ENABLE_STATE_CLOSE == signalGeneratorModel.workState)
    {
        lv_obj_clear_state(ui_enableSwitch, LV_STATE_CHECKED); // 关
    }
    else
    {
        lv_obj_add_state(ui_enableSwitch, LV_STATE_CHECKED); // 开
    }
}

static void ui_bnt_obj_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (event_code == LV_EVENT_PRESSED)
    {
        if (target == ui_signalTypeBtn)
        {
            signalGeneratorModel.utilConfig.signalType =
                (SIGNAL_TYPE)((signalGeneratorModel.utilConfig.signalType + 1) % SIGNAL_TYPE_MAX);
            setSignalImg();
        }
        else if (target == ui_freqSet)
        {
            if (SIGNAL_TYPE::SIGNAL_TYPE_SINE_WARE == signalGeneratorModel.utilConfig.signalType)
            {
                signalGeneratorModel.utilConfig.freqDAC = lv_numberbtn_get_value(ui_freqSet);
            }
            else if (SIGNAL_TYPE::SIGNAL_TYPE_SQUARE_WARE == signalGeneratorModel.utilConfig.signalType)
            {
                signalGeneratorModel.utilConfig.freqPwm = lv_numberbtn_get_value(ui_freqSet);
            }
            countPulseWidth();
        }
        else if (target == ui_dutyCycleBtn)
        {
            signalGeneratorModel.utilConfig.duty = lv_numberbtn_get_value(ui_dutyCycleBtn) * 10;
            countPulseWidth();
        }
    }
}

static void ui_set_obj_change(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        if (target == ui_enableSwitch)
        {
            bool isEnable = lv_obj_has_state(target, LV_STATE_CHECKED); // 返回 bool 类型， 开-1 ； 关-2
            signalGeneratorModel.workState = isEnable == true ? ENABLE_STATE_OPEN : ENABLE_STATE_CLOSE;
        }
    }
}

FE_UI_OBJ signalUIObj = {signalPageUI, signalPageUI_init,
                         signalPageUI_release, signalPageUI_focused};

#endif