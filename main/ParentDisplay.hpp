#pragma once
#ifndef PARENTDISPLAY_HPP
#define PARENTDISPLAY_HPP
#include "lvgl.h"

class ParentDisplay {
  protected:
    lv_obj_t *parentDisplay = lv_obj_create(lv_screen_active());
};

#endif
