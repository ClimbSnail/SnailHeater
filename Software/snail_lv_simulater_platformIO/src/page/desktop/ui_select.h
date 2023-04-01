#ifndef _UI_SELECT_H
#define _UI_SELECT_H

#include <stdint.h>

// UI选择文件 只能选用一个
// 若添加新的UI方案 需要在只需要在 snail_ui.h 中添加文件路径

// #define MULTI_UI
#define FULL_UI
// #define NEW_UI_EXAMPLE

// 触摸事件处理
void touch_event(unsigned int value);

#endif
