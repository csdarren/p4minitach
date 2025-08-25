#pragma once
#ifndef INFODISPLAY_HPP
#define INFODISPLAY_HPP
#include "lvgl.h"
#include "ParentDisplay.hpp"

class InfoDisplay : ParentDisplay {
  private:
    lv_obj_t *dash_bg = lv_img_create(parentDisplay);
};

#endif
