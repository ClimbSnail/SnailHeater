#ifndef _SNAIL_UI_H
#define _SNAIL_UI_H

// 若更换UI方案 只需要在 ui_select.h 中开启响应的宏
#include "./ui_select.h"

#ifdef MULTI_UI
#include "multi/ui.h"
#endif

#ifdef FULL_UI
#include "full/ui.h"
#endif

#ifdef NEW_UI_EXAMPLE
#include "new_ui_example/ui.h"
#endif

#endif
