/**
 * @file lv_numberbtn.h  v0.5
 *
 */

#ifndef LV_NUMBERBTN_H
#define LV_NUMBERBTN_H

#ifdef __cplusplus
extern "C"
{
#endif

    /*********************
     *      INCLUDES
     *********************/
#include "lvgl.h"

    typedef struct
    {
        lv_obj_t obj;
        lv_obj_t *text_obj;
        char format[16];
        double value;     /*Current value of the numberbtn*/
        double step;      // 步进
        double min_value; /*Minimum value of the numberbtn*/
        double max_value; /*Maximum value of the numberbtn*/
        int8_t dragging : 2;
        int8_t is_set_text : 6;
        lv_align_t align; // 文本对齐方式
    } lv_numberbtn_t;

    extern const lv_obj_class_t lv_numberbtn_class;

    /**********************
     * GLOBAL PROTOTYPES
     **********************/

    /**
     * Create an numberbtn object
     * @param parent pointer to an object, it will be the parent of the new numberbtn
     * @return pointer to the created numberbtn
     */
    lv_obj_t *lv_numberbtn_create(lv_obj_t *parent);

    /*======================
     * Add/remove functions
     *=====================*/

    /*=====================
     * Setter functions
     *====================*/
    void lv_numberbtn_set_align(lv_obj_t *obj, lv_align_t align);
    void lv_numberbtn_set_range(lv_obj_t *obj, double min, double max);
    // void lv_numberbtn_set_value(lv_obj_t *obj, int16_t value);
    void lv_numberbtn_set_value(lv_obj_t *obj, double value);
    double lv_numberbtn_get_value(const lv_obj_t *obj);
    void lv_numberbtn_set_label_and_format(const lv_obj_t *obj,
                                           lv_obj_t *obj_to_add,
                                           const char *format, double step);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_ARCMENU_H*/
