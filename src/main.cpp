#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "bsp/esp32_p4_wifi6_touch_lcd_xc.h"
#include "bsp_board_extra.h"
#include "core/lv_obj.h"
#include "display/lv_display.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_memory_utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "lv_demos.h"
#include "lvgl.h"
#include "misc/lv_color.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/twai.h"
#include "driver/gpio.h"

#include "MainDisplay.hpp"
#include "CanConnect.hpp"

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

    CanConnect CAN;
    static twai_message_t can_frame;
    static uint16_t rpm_value = 3000;
    static uint8_t speed_value = 69;
    static uint8_t fuel_value = 75;
    static uint16_t temp_value = 103;

    bsp_display_lock(0);
    MainDisplay dashboard;
    dashboard.SetupRpmArc();
    dashboard.SetupSpeedArc();
    dashboard.SetupFuelArc();
    dashboard.SetupTempArc();
    dashboard.RunArcAnimation();
    bsp_display_unlock();

    while (true) {
        bsp_display_lock(0);

        dashboard.HideOnTouch();
        dashboard.SetRPMValue(rpm_value);
        dashboard.SetSpeedValue(speed_value);
        dashboard.SetFuelValue(fuel_value);
        dashboard.SetTempValue(temp_value);

        bsp_display_unlock();

        if (esp_err_t error = CAN.ReceiveFrame(can_frame) != ESP_OK) {
            ESP_LOGE("FATAL", "Not receiving any CAN Data, %s", error);
            continue;
        }

        if (auto val = CanConnect::HandleRPM(can_frame)) {
            rpm_value = val;
        }
        if (auto value = CanConnect::HandleSPD(can_frame)) {
            speed_value = value;
        }
        if (auto value = CanConnect::HandleFUEL(can_frame)) {
            fuel_value = value;
        }
        if (auto value = CanConnect::HandleTEMP(can_frame)) {
            temp_value = value;
        }
    }
}
