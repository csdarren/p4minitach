#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"
#include "core/lv_obj.h"
#include "display/lv_display.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_memory_utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_demos.h"
#include "lvgl.h"
#include "misc/lv_color.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "MainDisplay.hpp"

static void lvgl_full_overlay_cb(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);

    auto *dashboard = static_cast<lv_obj_t *>(lv_event_get_user_data(event));
    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI("CLICKED EVENT", "Hiding Object");
        bool hidden = lv_obj_has_flag(dashboard, LV_OBJ_FLAG_HIDDEN);
        if (hidden) {
            lv_obj_remove_flag(dashboard, LV_OBJ_FLAG_HIDDEN);
            bsp_display_backlight_on();
        } else {
            lv_obj_add_flag(dashboard, LV_OBJ_FLAG_HIDDEN);
            bsp_display_backlight_off();
        }
    }
}

static void touchToHideObj(lv_obj_t *dashboard) {
    lv_obj_t *invis_overlay = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(invis_overlay);
    lv_obj_set_size(invis_overlay, LV_PCT(100), LV_PCT(100));
    lv_obj_center(invis_overlay);
    lv_obj_set_style_opa(invis_overlay, LV_OPA_TRANSP, 0);
    lv_obj_add_event_cb(invis_overlay, lvgl_full_overlay_cb, LV_EVENT_ALL, dashboard);
}

static void lvglInit() {
    bsp_display_cfg_t cfg = {.lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
                             .buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
                             .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
                             .flags = {
                                 .buff_dma = true,
                                 .buff_spiram = false,
                                 .sw_rotate = false,
                             }};
    bsp_display_start_with_config(&cfg);
    bsp_display_backlight_on();
    bsp_display_brightness_set(100);
    lv_obj_remove_flag(lv_screen_active(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(lv_screen_active(), 720, 720);
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x000000), 0);
}

extern "C" void app_main(void) {
    lvglInit();

    bsp_display_lock(0);
    MainDisplay dashboard;
    touchToHideObj(dashboard.getMainDisplay());
    bsp_display_unlock();
}
