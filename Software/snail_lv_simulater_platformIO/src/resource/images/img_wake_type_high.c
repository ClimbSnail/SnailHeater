#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_IMG_WAKE_TYPE_HIGH
#define LV_ATTRIBUTE_IMG_IMG_WAKE_TYPE_HIGH
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_IMG_WAKE_TYPE_HIGH uint8_t img_wake_type_high_map[] = {
#if LV_COLOR_DEPTH == 1 || LV_COLOR_DEPTH == 8
  /*Pixel format: Alpha 8 bit, Red: 3 bit, Green: 3 bit, Blue: 2 bit*/
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x10, 0x25, 0x40, 0x25, 0x40, 0x25, 0x40, 0x25, 0x40, 0x25, 0x40, 0x25, 0x40, 0x25, 0x40, 0x24, 0x30, 0x00, 0x00, 
  0x00, 0x00, 0x25, 0x40, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0x92, 0xc0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x10, 0x25, 0x40, 0x25, 0x40, 0x25, 0x40, 0xb7, 0xff, 0x49, 0x70, 0x25, 0x40, 0x25, 0x40, 0x24, 0x30, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb7, 0xff, 0x25, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb7, 0xff, 0x25, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x24, 0x20, 0x6d, 0x80, 0x6d, 0x80, 0x6d, 0x80, 0xb7, 0xff, 0x6e, 0xa0, 0x6d, 0x80, 0x6d, 0x80, 0x49, 0x60, 0x00, 0x00, 
  0x00, 0x00, 0x25, 0x40, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0x92, 0xc0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x25, 0x40, 0x24, 0x20, 0x00, 0x00, 0x49, 0x60, 0x6d, 0x80, 0x6d, 0x80, 0x6d, 0x80, 0x24, 0x20, 0x00, 0x00, 
  0x00, 0x00, 0x25, 0x40, 0xb7, 0xff, 0x49, 0x60, 0x00, 0x00, 0x92, 0xc0, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0x49, 0x70, 0x00, 0x00, 
  0x00, 0x00, 0x49, 0x70, 0xb7, 0xff, 0x24, 0x20, 0x00, 0x00, 0x92, 0xc0, 0x6d, 0x80, 0x00, 0x00, 0x6e, 0xb0, 0x6e, 0xb0, 0x00, 0x00, 
  0x00, 0x00, 0x6d, 0x80, 0xb7, 0xff, 0x24, 0x20, 0x00, 0x00, 0x49, 0x60, 0x25, 0x40, 0x00, 0x00, 0x6e, 0xb0, 0x92, 0xc0, 0x00, 0x00, 
  0x00, 0x00, 0x24, 0x30, 0xb7, 0xff, 0x6d, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x20, 0xb7, 0xff, 0x6e, 0xb0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x92, 0xc0, 0xb7, 0xff, 0x92, 0xe0, 0x6d, 0x80, 0x6e, 0xa0, 0xb6, 0xf0, 0xb7, 0xff, 0x25, 0x40, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x6e, 0xa0, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb6, 0xf0, 0x49, 0x60, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x40, 0x24, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x24, 0x20, 0x6d, 0x80, 0x6d, 0x80, 0x6d, 0x80, 0x6d, 0x80, 0x6d, 0x80, 0x6d, 0x80, 0x6d, 0x80, 0x49, 0x60, 0x00, 0x00, 
  0x00, 0x00, 0x25, 0x40, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0x6e, 0xb0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x24, 0x20, 0x6d, 0x80, 0x6d, 0x80, 0x6d, 0x80, 0x6d, 0x80, 0x6d, 0x80, 0x6d, 0x80, 0x6d, 0x80, 0x49, 0x50, 0x00, 0x00, 
  0x00, 0x00, 0x25, 0x40, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0x92, 0xc0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb7, 0xff, 0x25, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb7, 0xff, 0x25, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb7, 0xff, 0x25, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x24, 0x20, 0x6d, 0x80, 0x6d, 0x80, 0x6d, 0x80, 0xb7, 0xff, 0x6e, 0xa0, 0x6d, 0x80, 0x6d, 0x80, 0x49, 0x60, 0x00, 0x00, 
  0x00, 0x00, 0x25, 0x40, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0xb7, 0xff, 0x92, 0xc0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
#endif
#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0
  /*Pixel format: Alpha 8 bit, Red: 5 bit, Green: 6 bit, Blue: 5 bit*/
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x82, 0x10, 0x10, 0x65, 0x29, 0x40, 0x65, 0x29, 0x40, 0x65, 0x29, 0x40, 0x65, 0x29, 0x40, 0x65, 0x29, 0x40, 0x65, 0x29, 0x40, 0x65, 0x29, 0x40, 0x04, 0x21, 0x30, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x65, 0x29, 0x40, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0xcf, 0x7b, 0xc0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x82, 0x10, 0x10, 0x65, 0x29, 0x40, 0x65, 0x29, 0x40, 0x65, 0x29, 0x40, 0x14, 0xa5, 0xff, 0x49, 0x4a, 0x70, 0x65, 0x29, 0x40, 0x65, 0x29, 0x40, 0x04, 0x21, 0x30, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xa5, 0xff, 0x65, 0x29, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xa5, 0xff, 0x65, 0x29, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xc3, 0x18, 0x20, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0x14, 0xa5, 0xff, 0x2d, 0x6b, 0xa0, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0xe8, 0x41, 0x60, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x65, 0x29, 0x40, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0xcf, 0x7b, 0xc0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x65, 0x29, 0x40, 0xc3, 0x18, 0x20, 0x00, 0x00, 0x00, 0xe8, 0x41, 0x60, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0xc3, 0x18, 0x20, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x65, 0x29, 0x40, 0x14, 0xa5, 0xff, 0xe8, 0x41, 0x60, 0x00, 0x00, 0x00, 0xcf, 0x7b, 0xc0, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x49, 0x4a, 0x70, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x49, 0x4a, 0x70, 0x14, 0xa5, 0xff, 0xc3, 0x18, 0x20, 0x00, 0x00, 0x00, 0xcf, 0x7b, 0xc0, 0x8a, 0x52, 0x80, 0x00, 0x00, 0x00, 0x8e, 0x73, 0xb0, 0x8e, 0x73, 0xb0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x8a, 0x52, 0x80, 0x14, 0xa5, 0xff, 0xc3, 0x18, 0x20, 0x00, 0x00, 0x00, 0xe8, 0x41, 0x60, 0x65, 0x29, 0x40, 0x00, 0x00, 0x00, 0x8e, 0x73, 0xb0, 0xcf, 0x7b, 0xc0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x04, 0x21, 0x30, 0x14, 0xa5, 0xff, 0xeb, 0x5a, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0x18, 0x20, 0x14, 0xa5, 0xff, 0x8e, 0x73, 0xb0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcf, 0x7b, 0xc0, 0x14, 0xa5, 0xff, 0x72, 0x94, 0xe0, 0x8a, 0x52, 0x80, 0x2d, 0x6b, 0xa0, 0xd3, 0x9c, 0xf0, 0x14, 0xa5, 0xff, 0x65, 0x29, 0x40, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x10, 0x10, 0x2d, 0x6b, 0xa0, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0xd3, 0x9c, 0xf0, 0xe8, 0x41, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x65, 0x29, 0x40, 0x04, 0x21, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xc3, 0x18, 0x20, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0xe8, 0x41, 0x60, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x65, 0x29, 0x40, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x8e, 0x73, 0xb0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xc3, 0x18, 0x20, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0xa6, 0x31, 0x50, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x65, 0x29, 0x40, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0xcf, 0x7b, 0xc0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xa5, 0xff, 0x65, 0x29, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xa5, 0xff, 0x65, 0x29, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xa5, 0xff, 0x65, 0x29, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xc3, 0x18, 0x20, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0x14, 0xa5, 0xff, 0x2d, 0x6b, 0xa0, 0x8a, 0x52, 0x80, 0x8a, 0x52, 0x80, 0xe8, 0x41, 0x60, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x65, 0x29, 0x40, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0x14, 0xa5, 0xff, 0xcf, 0x7b, 0xc0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
#endif
#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP != 0
  /*Pixel format: Alpha 8 bit, Red: 5 bit, Green: 6 bit, Blue: 5 bit  BUT the 2  color bytes are swapped*/
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x10, 0x82, 0x10, 0x29, 0x65, 0x40, 0x29, 0x65, 0x40, 0x29, 0x65, 0x40, 0x29, 0x65, 0x40, 0x29, 0x65, 0x40, 0x29, 0x65, 0x40, 0x29, 0x65, 0x40, 0x21, 0x04, 0x30, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x29, 0x65, 0x40, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0x7b, 0xcf, 0xc0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x10, 0x82, 0x10, 0x29, 0x65, 0x40, 0x29, 0x65, 0x40, 0x29, 0x65, 0x40, 0xa5, 0x14, 0xff, 0x4a, 0x49, 0x70, 0x29, 0x65, 0x40, 0x29, 0x65, 0x40, 0x21, 0x04, 0x30, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa5, 0x14, 0xff, 0x29, 0x65, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa5, 0x14, 0xff, 0x29, 0x65, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x18, 0xc3, 0x20, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0xa5, 0x14, 0xff, 0x6b, 0x2d, 0xa0, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x41, 0xe8, 0x60, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x29, 0x65, 0x40, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0x7b, 0xcf, 0xc0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0x65, 0x40, 0x18, 0xc3, 0x20, 0x00, 0x00, 0x00, 0x41, 0xe8, 0x60, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x18, 0xc3, 0x20, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x29, 0x65, 0x40, 0xa5, 0x14, 0xff, 0x41, 0xe8, 0x60, 0x00, 0x00, 0x00, 0x7b, 0xcf, 0xc0, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0x4a, 0x49, 0x70, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x4a, 0x49, 0x70, 0xa5, 0x14, 0xff, 0x18, 0xc3, 0x20, 0x00, 0x00, 0x00, 0x7b, 0xcf, 0xc0, 0x52, 0x8a, 0x80, 0x00, 0x00, 0x00, 0x73, 0x8e, 0xb0, 0x73, 0x8e, 0xb0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x52, 0x8a, 0x80, 0xa5, 0x14, 0xff, 0x18, 0xc3, 0x20, 0x00, 0x00, 0x00, 0x41, 0xe8, 0x60, 0x29, 0x65, 0x40, 0x00, 0x00, 0x00, 0x73, 0x8e, 0xb0, 0x7b, 0xcf, 0xc0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x21, 0x04, 0x30, 0xa5, 0x14, 0xff, 0x5a, 0xeb, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xc3, 0x20, 0xa5, 0x14, 0xff, 0x73, 0x8e, 0xb0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7b, 0xcf, 0xc0, 0xa5, 0x14, 0xff, 0x94, 0x72, 0xe0, 0x52, 0x8a, 0x80, 0x6b, 0x2d, 0xa0, 0x9c, 0xd3, 0xf0, 0xa5, 0x14, 0xff, 0x29, 0x65, 0x40, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x82, 0x10, 0x6b, 0x2d, 0xa0, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0x9c, 0xd3, 0xf0, 0x41, 0xe8, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0x65, 0x40, 0x21, 0x04, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x18, 0xc3, 0x20, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x41, 0xe8, 0x60, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x29, 0x65, 0x40, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0x73, 0x8e, 0xb0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x18, 0xc3, 0x20, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x31, 0xa6, 0x50, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x29, 0x65, 0x40, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0x7b, 0xcf, 0xc0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa5, 0x14, 0xff, 0x29, 0x65, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa5, 0x14, 0xff, 0x29, 0x65, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa5, 0x14, 0xff, 0x29, 0x65, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x18, 0xc3, 0x20, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0xa5, 0x14, 0xff, 0x6b, 0x2d, 0xa0, 0x52, 0x8a, 0x80, 0x52, 0x8a, 0x80, 0x41, 0xe8, 0x60, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x29, 0x65, 0x40, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0xa5, 0x14, 0xff, 0x7b, 0xcf, 0xc0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
#endif
#if LV_COLOR_DEPTH == 32
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x10, 0x2b, 0x2b, 0x2b, 0x40, 0x2b, 0x2b, 0x2b, 0x40, 0x2b, 0x2b, 0x2b, 0x40, 0x2b, 0x2b, 0x2b, 0x40, 0x2b, 0x2b, 0x2b, 0x40, 0x2b, 0x2b, 0x2b, 0x40, 0x2b, 0x2b, 0x2b, 0x40, 0x1f, 0x1f, 0x1f, 0x30, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x2b, 0x2b, 0x2b, 0x40, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0x78, 0x78, 0x78, 0xc0, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x10, 0x2b, 0x2b, 0x2b, 0x40, 0x2b, 0x2b, 0x2b, 0x40, 0x2b, 0x2b, 0x2b, 0x40, 0xa0, 0xa0, 0xa0, 0xff, 0x46, 0x46, 0x46, 0x70, 0x2b, 0x2b, 0x2b, 0x40, 0x2b, 0x2b, 0x2b, 0x40, 0x1f, 0x1f, 0x1f, 0x30, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0xa0, 0xa0, 0xff, 0x2b, 0x2b, 0x2b, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0xa0, 0xa0, 0xff, 0x2b, 0x2b, 0x2b, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x17, 0x17, 0x17, 0x20, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0xa0, 0xa0, 0xa0, 0xff, 0x64, 0x64, 0x64, 0xa0, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x3d, 0x3d, 0x3d, 0x60, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x2b, 0x2b, 0x2b, 0x40, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0x78, 0x78, 0x78, 0xc0, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2b, 0x2b, 0x2b, 0x40, 0x17, 0x17, 0x17, 0x20, 0x00, 0x00, 0x00, 0x00, 0x3d, 0x3d, 0x3d, 0x60, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x17, 0x17, 0x17, 0x20, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x2b, 0x2b, 0x2b, 0x40, 0xa0, 0xa0, 0xa0, 0xff, 0x3d, 0x3d, 0x3d, 0x60, 0x00, 0x00, 0x00, 0x00, 0x78, 0x78, 0x78, 0xc0, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0x46, 0x46, 0x46, 0x70, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x46, 0x46, 0x46, 0x70, 0xa0, 0xa0, 0xa0, 0xff, 0x17, 0x17, 0x17, 0x20, 0x00, 0x00, 0x00, 0x00, 0x78, 0x78, 0x78, 0xc0, 0x51, 0x51, 0x51, 0x80, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x6e, 0x6e, 0xb0, 0x6e, 0x6e, 0x6e, 0xb0, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x51, 0x51, 0x51, 0x80, 0xa0, 0xa0, 0xa0, 0xff, 0x17, 0x17, 0x17, 0x20, 0x00, 0x00, 0x00, 0x00, 0x3d, 0x3d, 0x3d, 0x60, 0x2b, 0x2b, 0x2b, 0x40, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x6e, 0x6e, 0xb0, 0x78, 0x78, 0x78, 0xc0, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x1f, 0x30, 0xa0, 0xa0, 0xa0, 0xff, 0x5a, 0x5a, 0x5a, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x17, 0x17, 0x20, 0xa0, 0xa0, 0xa0, 0xff, 0x6e, 0x6e, 0x6e, 0xb0, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x78, 0x78, 0xc0, 0xa0, 0xa0, 0xa0, 0xff, 0x8d, 0x8d, 0x8d, 0xe0, 0x51, 0x51, 0x51, 0x80, 0x64, 0x64, 0x64, 0xa0, 0x97, 0x97, 0x97, 0xf0, 0xa0, 0xa0, 0xa0, 0xff, 0x2b, 0x2b, 0x2b, 0x40, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x10, 0x64, 0x64, 0x64, 0xa0, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0x97, 0x97, 0x97, 0xf0, 0x3d, 0x3d, 0x3d, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2b, 0x2b, 0x2b, 0x40, 0x1f, 0x1f, 0x1f, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x17, 0x17, 0x17, 0x20, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x3d, 0x3d, 0x3d, 0x60, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x2b, 0x2b, 0x2b, 0x40, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0x6e, 0x6e, 0x6e, 0xb0, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x17, 0x17, 0x17, 0x20, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x33, 0x33, 0x33, 0x50, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x2b, 0x2b, 0x2b, 0x40, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0x78, 0x78, 0x78, 0xc0, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0xa0, 0xa0, 0xff, 0x2b, 0x2b, 0x2b, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0xa0, 0xa0, 0xff, 0x2b, 0x2b, 0x2b, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0xa0, 0xa0, 0xff, 0x2b, 0x2b, 0x2b, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x17, 0x17, 0x17, 0x20, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0xa0, 0xa0, 0xa0, 0xff, 0x64, 0x64, 0x64, 0xa0, 0x51, 0x51, 0x51, 0x80, 0x51, 0x51, 0x51, 0x80, 0x3d, 0x3d, 0x3d, 0x60, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x2b, 0x2b, 0x2b, 0x40, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0xa0, 0xa0, 0xa0, 0xff, 0x78, 0x78, 0x78, 0xc0, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
#endif
};

const lv_img_dsc_t img_wake_type_high = {
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 11,
  .header.h = 31,
  .data_size = 341 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .data = img_wake_type_high_map,
};