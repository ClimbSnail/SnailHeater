#include "verification.h"

static lv_obj_t *veriScreen;
static lv_obj_t *ui_Keyboard;
static lv_obj_t *ui_userIdLabel;
static lv_obj_t *ui_userIdTextArea;
static lv_obj_t *ui_keyLabel;
static lv_obj_t *ui_keyTextArea;

lv_group_t *key_group;

static void ui_key_pressed(lv_event_t *e);

void verification_init(lv_indev_t *indev, const char *userId)
{
    veriScreen = lv_obj_create(NULL);
    lv_obj_clear_flag(veriScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_grad_color(veriScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Keyboard = lv_keyboard_create(veriScreen);
    lv_obj_set_size(ui_Keyboard, 280, 150);
    lv_obj_set_pos(ui_Keyboard, 0, 50);
    lv_obj_set_align(ui_Keyboard, LV_ALIGN_CENTER);

    ui_userIdLabel = lv_label_create(veriScreen);
    lv_obj_set_size(ui_userIdLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_userIdLabel, -110, -90);
    lv_obj_set_align(ui_userIdLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_userIdLabel, "UserID");

    ui_userIdTextArea = lv_textarea_create(veriScreen);
    lv_obj_set_size(ui_userIdTextArea, 220, 36);
    lv_obj_set_pos(ui_userIdTextArea, 30, -90);
    lv_obj_set_align(ui_userIdTextArea, LV_ALIGN_CENTER);
    lv_textarea_set_text(ui_userIdTextArea, userId);

    ui_keyLabel = lv_label_create(veriScreen);
    lv_obj_set_size(ui_keyLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_keyLabel, -110, -50);
    lv_obj_set_align(ui_keyLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_keyLabel, "Key");

    ui_keyTextArea = lv_textarea_create(veriScreen);
    lv_obj_set_size(ui_keyTextArea, 220, 36);
    lv_obj_set_pos(ui_keyTextArea, 30, -50);
    lv_obj_set_align(ui_keyTextArea, LV_ALIGN_CENTER);

    lv_keyboard_set_textarea(ui_Keyboard, ui_keyTextArea);
    lv_obj_add_event_cb(ui_Keyboard, ui_key_pressed, LV_EVENT_KEY, NULL);

    lv_disp_load_scr(veriScreen);
    key_group = lv_group_create();
    lv_group_add_obj(key_group, ui_Keyboard);
    lv_indev_set_group(indev, key_group);
}

static void ui_key_pressed(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (LV_EVENT_KEY == event_code)
    {
        uint16_t key_code = lv_keyboard_get_selected_btn(ui_Keyboard);
        LV_LOG_USER("key_code = %u", key_code);
    }
}

void verification_release(void)
{
    if (veriScreen)
    {
        lv_obj_clean(veriScreen);
        veriScreen = NULL;
        ui_Keyboard = NULL;
        ui_userIdLabel = NULL;
        ui_userIdTextArea = NULL;
        ui_keyLabel = NULL;
        ui_keyTextArea = NULL;
    }
}