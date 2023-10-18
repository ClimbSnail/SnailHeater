#ifndef _UI_SELECT_H
#define _UI_SELECT_H

#include <stdint.h>

// UI选择文件 只能选用一个
// 若添加新的UI方案 需要在只需要在 snail_ui.h 中添加文件路径

// #define MULTI_UI
#define FULL_UI
// #define NEW_UI_EXAMPLE

#if SH_HARDWARE_VER == SH_ESP32S2_WROOM_V26
#define SH_SCREEN_WIDTH 320
#define SH_SCREEN_HEIGHT 240
#elif SH_HARDWARE_VER == SH_ESP32S3_FN8_V27
#define SH_SCREEN_WIDTH 320
#define SH_SCREEN_HEIGHT 240
#else
#define SH_SCREEN_WIDTH 280
#define SH_SCREEN_HEIGHT 240
#endif

// 触摸事件处理
void touch_event(unsigned int value = 0);

#endif
