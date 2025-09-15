#pragma once
#ifndef PARENTDISPLAY_HPP
#define PARENTDISPLAY_HPP
#include "esp_log.h"
#include "lvgl.h"

class ParentDisplay {
  protected:
    static void hideObjectCallback(lv_event_t *event) {
        lv_event_code_t code = lv_event_get_code(event);

        auto *objToHide = static_cast<lv_obj_t *>(lv_event_get_user_data(event));
        if (code == LV_EVENT_CLICKED) {
            ESP_LOGI("CLICKED EVENT", "Toggling Object");
            bool hidden = lv_obj_has_flag(objToHide, LV_OBJ_FLAG_HIDDEN);
            if (hidden) {
                lv_obj_remove_flag(objToHide, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(objToHide, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }

    lv_obj_t *parentDisplay{};

    ParentDisplay() : parentDisplay(lv_obj_create(lv_screen_active())) {
        lv_obj_remove_style_all(parentDisplay);
        lv_obj_set_style_bg_opa(parentDisplay, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_size(parentDisplay, 720, 720);
        lv_obj_remove_flag(parentDisplay, LV_OBJ_FLAG_SCROLLABLE);
    }
};

#endif
