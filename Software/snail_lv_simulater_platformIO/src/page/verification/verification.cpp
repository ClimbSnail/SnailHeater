#include "verification.h"

static lv_obj_t *veriScreen = NULL;
static lv_obj_t *ui_Keyboard;
static lv_obj_t *ui_userIdLabel;
static lv_obj_t *ui_userIdTextArea;
static lv_obj_t *ui_keyLabel;
static lv_obj_t *ui_keyTextArea;

static lv_group_t *key_group;

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
    lv_keyboard_set_mode(ui_Keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);

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

    if (LV_EVENT_VALUE_CHANGED == event_code)
    {
        uint16_t key_code = lv_keyboard_get_selected_btn(ui_Keyboard);
        LV_LOG_USER("key_code = %u", key_code);
    }
}

// static void ui_key_pressed_1(lv_event_t *e);

// void verification_init_1(lv_indev_t *indev, const char *userId)
// {
//     veriScreen = lv_obj_create(NULL);
//     lv_obj_clear_flag(veriScreen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
//     lv_obj_set_style_bg_grad_color(veriScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);

//     ui_Keyboard = lv_keyboard_create(veriScreen);
//     lv_obj_set_size(ui_Keyboard, 200, 110);
//     lv_obj_set_pos(ui_Keyboard, 0, 50);
//     lv_obj_set_align(ui_Keyboard, LV_ALIGN_CENTER);
//     lv_keyboard_set_mode(ui_Keyboard, LV_KEYBOARD_MODE_NUMBER);

//     ui_keyLabel = lv_label_create(veriScreen);
//     lv_obj_set_size(ui_keyLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
//     lv_obj_set_pos(ui_keyLabel, -110, -50);
//     lv_obj_set_align(ui_keyLabel, LV_ALIGN_CENTER);
//     lv_label_set_text(ui_keyLabel, "Key");

//     ui_keyTextArea = lv_textarea_create(veriScreen);
//     lv_obj_set_size(ui_keyTextArea, 220, 36);
//     lv_obj_set_pos(ui_keyTextArea, 30, -50);
//     lv_obj_set_align(ui_keyTextArea, LV_ALIGN_CENTER);

//     lv_keyboard_set_textarea(ui_Keyboard, ui_keyTextArea);
//     lv_obj_add_event_cb(ui_Keyboard, ui_key_pressed_1, LV_EVENT_ALL, ui_Keyboard);

//     lv_disp_load_scr(veriScreen);
//     key_group = lv_group_create();
//     lv_group_add_obj(key_group, ui_Keyboard);
//     lv_indev_set_group(indev, key_group);
// }

// static void ui_key_pressed_1(lv_event_t *e)
// {
//     lv_event_code_t event_code = lv_event_get_code(e);
//     lv_obj_t *ta = lv_event_get_target(e);
//     lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);

//     if (LV_EVENT_VALUE_CHANGED == event_code)
//     {
//         uint16_t key_code = lv_keyboard_get_selected_btn(ui_Keyboard);
//         LV_LOG_USER("key_code = %u", key_code);
//         if (7 == key_code)
//         {
//             lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
//             return ;
//         }
//         // lv_textarea_add_char(ta, key_code + 48);
//     }

//     // if (event_code == LV_EVENT_FOCUSED)
//     // {
//     //     lv_keyboard_set_textarea(kb, ta);
//     //     lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
//     // }

//     // if (event_code == LV_EVENT_DEFOCUSED)
//     // {
//     //     lv_keyboard_set_textarea(kb, NULL);
//     //     lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
//     // }
// }

void verification_release(void)
{
    if (NULL != veriScreen)
    {
        lv_obj_del(veriScreen);
        veriScreen = NULL;
        ui_Keyboard = NULL;
        ui_userIdLabel = NULL;
        ui_userIdTextArea = NULL;
        ui_keyLabel = NULL;
        ui_keyTextArea = NULL;
        key_group = NULL;
    }
}