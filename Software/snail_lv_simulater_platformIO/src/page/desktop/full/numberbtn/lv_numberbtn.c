

/*********************
 *      INCLUDES
 *********************/
#include "lv_numberbtn.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lv_numberbtn_class

#define VALUE_UNSET INT16_MIN
#define VALUE_DEFAULT_MIN 0
#define VALUE_DEFAULT_MAX 100

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void lv_numberbtn_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
static void lv_numberbtn_event(const lv_obj_class_t *class_p, lv_event_t *e);
static void draw_main(lv_event_t *e);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_numberbtn_class = {
    .constructor_cb = lv_numberbtn_constructor,
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .event_cb = lv_numberbtn_event,
    .instance_size = sizeof(lv_numberbtn_t),
    .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
    .base_class = &lv_obj_class};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t *lv_numberbtn_create(lv_obj_t *parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t *obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_numberbtn_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_res_t res;
    lv_numberbtn_t *numBnt = (lv_numberbtn_t *)obj;

    /*Initialize the allocated 'ext'*/
    numBnt->text_obj = NULL;
    lv_memset_00(numBnt->format, 16);
    lv_memcpy(numBnt->format, "%d", 3);
    numBnt->value = VALUE_UNSET;
    numBnt->step = 1; // 默认步进为1
    numBnt->min_value = VALUE_DEFAULT_MIN;
    numBnt->max_value = VALUE_DEFAULT_MAX;
    numBnt->dragging = false;
    numBnt->is_set_text = false;
    numBnt->align = LV_ALIGN_CENTER;

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    // lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    // lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_CHAIN | LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_ext_click_area(obj, LV_DPI_DEF / 10);

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_numberbtn_event(const lv_obj_class_t *class_p, lv_event_t *e)
{
    LV_UNUSED(class_p);

    lv_res_t res;

    /*Call the ancestor's event handler*/
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_DRAW_MAIN && code != LV_EVENT_DRAW_MAIN_END)
    {
        res = lv_obj_event_base(MY_CLASS, e);
        if (res != LV_RES_OK)
            return;
    }

    lv_obj_t *obj = lv_event_get_target(e);
    lv_numberbtn_t *numBnt = (lv_numberbtn_t *)obj;
    if (code == LV_EVENT_DRAW_MAIN)
    {
        lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);
        lv_draw_rect_dsc_t draw_dsc;
        lv_draw_rect_dsc_init(&draw_dsc);
        /*If the border is drawn later disable loading its properties*/
        if (lv_obj_get_style_border_post(obj, LV_PART_MAIN))
        {
            draw_dsc.border_post = 1;
        }

        lv_obj_init_draw_rect_dsc(obj, LV_PART_MAIN, &draw_dsc);
        lv_coord_t w = lv_obj_get_style_transform_width(obj, LV_PART_MAIN);
        lv_coord_t h = lv_obj_get_style_transform_height(obj, LV_PART_MAIN);
        lv_area_t coords;
        lv_area_copy(&coords, &obj->coords);
        coords.x1 -= w;
        coords.x2 += w;
        coords.y1 -= h;
        coords.y2 += h;

        lv_obj_draw_part_dsc_t part_dsc;
        lv_obj_draw_dsc_init(&part_dsc, draw_ctx);
        part_dsc.class_p = MY_CLASS;
        part_dsc.type = LV_OBJ_DRAW_PART_RECTANGLE;
        part_dsc.rect_dsc = &draw_dsc;
        part_dsc.draw_area = &coords;
        part_dsc.part = LV_PART_MAIN;
        lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_dsc);

#if LV_DRAW_COMPLEX
        /*With clip corner enabled draw the bg img separately to make it clipped*/
        bool clip_corner = (lv_obj_get_style_clip_corner(obj, LV_PART_MAIN) && draw_dsc.radius != 0) ? true : false;
        const void *bg_img_src = draw_dsc.bg_img_src;
        if (clip_corner)
        {
            draw_dsc.bg_img_src = NULL;
        }
#endif

        lv_draw_rect(draw_ctx, &draw_dsc, &coords);

#if LV_DRAW_COMPLEX
        if (clip_corner)
        {
            lv_draw_mask_radius_param_t *mp = lv_mem_buf_get(sizeof(lv_draw_mask_radius_param_t));
            lv_draw_mask_radius_init(mp, &obj->coords, draw_dsc.radius, false);
            /*Add the mask and use `obj+8` as custom id. Don't use `obj` directly because it might be used by the user*/
            lv_draw_mask_add(mp, obj + 8);

            if (bg_img_src)
            {
                draw_dsc.bg_opa = LV_OPA_TRANSP;
                draw_dsc.border_opa = LV_OPA_TRANSP;
                draw_dsc.outline_opa = LV_OPA_TRANSP;
                draw_dsc.shadow_opa = LV_OPA_TRANSP;
                draw_dsc.bg_img_src = bg_img_src;
                lv_draw_rect(draw_ctx, &draw_dsc, &coords);
            }
        }
#endif
        lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_dsc);
    }
    else if (code == LV_EVENT_PRESSING)
    {
        lv_indev_t *indev = lv_indev_get_act();
        if (indev == NULL)
            return;

        /*Handle only pointers here*/
        lv_indev_type_t indev_type = lv_indev_get_type(indev);
        if (indev_type != LV_INDEV_TYPE_POINTER)
            return;

        lv_point_t p;
        lv_indev_get_point(indev, &p);

        /*Make point relative to the arc's center*/
        lv_point_t center;
        lv_coord_t r;
        // get_center(obj, &center, &r);

        // p.x -= center.x;
        // p.y -= center.y;

        // /*Enter dragging mode if pressed out of the knob*/
        // if (numBnt->dragging == false)
        // {
        //     lv_coord_t indic_width = lv_obj_get_style_arc_width(obj, LV_PART_INDICATOR);
        //     r -= indic_width;
        //     /*Add some more sensitive area if there is no advanced git testing.
        //      * (Advanced hit testing is more precise)*/
        //     if (lv_obj_has_flag(obj, LV_OBJ_FLAG_ADV_HITTEST))
        //     {
        //         r -= indic_width;
        //     }
        //     else
        //     {
        //         r -= LV_MAX(r / 4, indic_width);
        //     }
        //     if (r < 1)
        //         r = 1;

        //     if (p.x * p.x + p.y * p.y > r * r)
        //     {
        //         numBnt->dragging = true;
        //         // arc->last_tick = lv_tick_get(); /*Capture timestamp at dragging start*/
        //     }
        // }
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        numBnt->dragging = false;

        /*Leave edit mode if released. (No need to wait for LONG_PRESS)*/
        lv_group_t *g = lv_obj_get_group(obj);
        bool editing = lv_group_get_editing(g);
        lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_get_act());
        if (indev_type == LV_INDEV_TYPE_ENCODER)
        {
            if (editing)
                lv_group_set_editing(g, false);
        }
    }
    else if (code == LV_EVENT_KEY)
    {
        char c = *((char *)lv_event_get_param(e));
        if (c == LV_KEY_RIGHT || c == LV_KEY_UP)
        {
            lv_numberbtn_set_value(numBnt, numBnt->value + numBnt->step);
            res = lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
        }
        else if (c == LV_KEY_LEFT || c == LV_KEY_DOWN)
        {
            res = lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
            lv_numberbtn_set_value(numBnt, numBnt->value - numBnt->step);
            res = lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
        }
    }
    // else if (code == LV_EVENT_HIT_TEST)
    // {
    //     lv_hit_test_info_t *info = lv_event_get_param(e);

    //     lv_point_t p;
    //     lv_coord_t r;
    //     // get_center(obj, &p, &r);

    //     lv_coord_t ext_click_area = 0;
    //     if (obj->spec_attr)
    //         ext_click_area = obj->spec_attr->ext_click_pad;

    //     lv_coord_t w = lv_obj_get_style_arc_width(obj, LV_PART_MAIN);
    //     r -= w + ext_click_area;

    //     lv_area_t a;
    //     /*Invalid if clicked inside*/
    //     lv_area_set(&a, p.x - r, p.y - r, p.x + r, p.y + r);
    //     if (_lv_area_is_point_on(&a, info->point, LV_RADIUS_CIRCLE))
    //     {
    //         info->res = false;
    //         return;
    //     }

    //     /*Valid if no clicked outside*/
    //     lv_area_increase(&a, w + ext_click_area * 2, w + ext_click_area * 2);
    //     info->res = _lv_area_is_point_on(&a, info->point, LV_RADIUS_CIRCLE);
    // }
    else if (code == LV_EVENT_REFR_EXT_DRAW_SIZE)
    {
        lv_coord_t bg_left = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
        lv_coord_t bg_right = lv_obj_get_style_pad_right(obj, LV_PART_MAIN);
        lv_coord_t bg_top = lv_obj_get_style_pad_top(obj, LV_PART_MAIN);
        lv_coord_t bg_bottom = lv_obj_get_style_pad_bottom(obj, LV_PART_MAIN);
        lv_coord_t bg_pad = LV_MAX4(bg_left, bg_right, bg_top, bg_bottom);

        lv_coord_t knob_left = lv_obj_get_style_pad_left(obj, LV_PART_KNOB);
        lv_coord_t knob_right = lv_obj_get_style_pad_right(obj, LV_PART_KNOB);
        lv_coord_t knob_top = lv_obj_get_style_pad_top(obj, LV_PART_KNOB);
        lv_coord_t knob_bottom = lv_obj_get_style_pad_bottom(obj, LV_PART_KNOB);
        lv_coord_t knob_pad = LV_MAX4(knob_left, knob_right, knob_top, knob_bottom) + 2;

        lv_coord_t *s = lv_event_get_param(e);
        *s = LV_MAX(*s, knob_pad - bg_pad);
    }
    else if (code == LV_EVENT_DRAW_MAIN)
    {
        draw_main(e);
    }
}

static void draw_main(lv_event_t *e)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_numberbtn_t *numBnt = (lv_numberbtn_t *)obj;
    lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);

    lv_coord_t h = obj->coords.y2 - obj->coords.y1;
    lv_coord_t w = obj->coords.x2 - obj->coords.x1;

    lv_draw_label_dsc_t label_draw_dsc;
    lv_draw_label_dsc_init(&label_draw_dsc);
    lv_obj_init_draw_label_dsc(obj, LV_EVENT_DRAW_MAIN, &label_draw_dsc);

    lv_obj_draw_part_dsc_t part_draw_dst;
    lv_obj_draw_dsc_init(&part_draw_dst, draw_ctx);
    part_draw_dst.draw_area = &obj->coords;
    part_draw_dst.class_p = MY_CLASS;
    part_draw_dst.type = LV_LED_DRAW_PART_RECTANGLE;
    part_draw_dst.rect_dsc = &label_draw_dsc;
    part_draw_dst.part = LV_PART_MAIN;
    lv_event_send(obj, LV_EVENT_DRAW_PART_BEGIN, &part_draw_dst);

    lv_bidi_calculate_align(&label_draw_dsc.align, &label_draw_dsc.bidi_dir, "Look");

    lv_event_send(obj, LV_EVENT_DRAW_PART_END, &part_draw_dst);
}

void lv_numberbtn_set_label_and_format(const lv_obj_t *obj,
                                       lv_obj_t *obj_to_add,
                                       const char *format, double step)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(obj_to_add);

    lv_numberbtn_t *numBnt = (lv_numberbtn_t *)obj;
    numBnt->step = step;
    numBnt->text_obj = obj_to_add;
    numBnt->is_set_text = true;
    lv_memset_00(numBnt->format, 16);
    strcpy(numBnt->format, format);
    lv_label_set_text_fmt(obj_to_add, numBnt->format, numBnt->value);
    lv_obj_set_size(obj_to_add, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    // lv_obj_align_to(obj_to_add, obj, numBnt->align, 0, 0);
    lv_obj_align(obj_to_add, numBnt->align, 0, 0);
}

void lv_numberbtn_set_step(const lv_obj_t *obj, double step)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_numberbtn_t *numBnt = (lv_numberbtn_t *)obj;
    numBnt->step = step;
}

void lv_numberbtn_set_align(lv_obj_t *obj, lv_align_t align)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_numberbtn_t *numBnt = (lv_numberbtn_t *)obj;

    numBnt->align = align;
    lv_obj_align(numBnt->text_obj, numBnt->align, 0, 0);
}

void lv_numberbtn_set_range(lv_obj_t *obj, double min, double max)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_numberbtn_t *numBnt = (lv_numberbtn_t *)obj;

    numBnt->min_value = min;
    numBnt->max_value = max;
    // 重新检查范围
    numBnt->value = LV_CLAMP(numBnt->min_value, numBnt->value, numBnt->max_value);
}

// void lv_numberbtn_set_value(lv_obj_t *obj, int16_t value)
// {
//     LV_ASSERT_OBJ(obj, MY_CLASS);

//     lv_obj_update_layout(obj);
//     lv_numberbtn_t *numBnt = (lv_numberbtn_t *)obj;
//     if (value == numBnt->value)
//         return;
//     numBnt->value = LV_CLAMP(numBnt->min_value, value, numBnt->max_value);
//     if (true == numBnt->is_set_text)
//     {
//         lv_label_set_text_fmt(numBnt->text_obj, numBnt->format, numBnt->value);
//         // lv_obj_align_to(numBnt->text_obj, obj, numBnt->align, 0, 0);
//         lv_obj_align(numBnt->text_obj, numBnt->align, 0, 0);

//         lv_res_t res = lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
//         if (res != LV_RES_OK)
//             return;
//     }
// }

void lv_numberbtn_set_value(lv_obj_t *obj, double value)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_obj_update_layout(obj);
    lv_numberbtn_t *numBnt = (lv_numberbtn_t *)obj;

    numBnt->value = LV_CLAMP(numBnt->min_value, value, numBnt->max_value);
    if (true == numBnt->is_set_text)
    {
        bool found = false;
        for (int i = 0; numBnt->format[i] != 0; ++i)
        {
            if (numBnt->format[i] == 'f')
            {
                found = true;
                break;
            }
        }
        if (true == found)
        {
            lv_label_set_text_fmt(numBnt->text_obj, numBnt->format, numBnt->value);
        }
        else
        {
            lv_label_set_text_fmt(numBnt->text_obj, numBnt->format, (int)numBnt->value);
        }
        // lv_obj_align_to(numBnt->text_obj, obj, numBnt->align, 0, 0);
        lv_obj_align(numBnt->text_obj, numBnt->align, 0, 0);

        lv_res_t res = lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
        if (res != LV_RES_OK)
            return;
    }
}

double lv_numberbtn_get_value(const lv_obj_t *obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_numberbtn_t *numBnt = (lv_numberbtn_t *)obj;
    return numBnt->value;
}