#include "bsp/esp32_p4_wifi6_touch_lcd_xc.h"
#include "lvgl.h"

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

static QueueHandle_t can_queue;
using can_data_t = struct {
    uint16_t rpm_value;
    uint8_t speed_value;
    uint8_t fuel_value;
    uint16_t temp_value;
};

extern "C" void can_task(void * /*task_param*/) {
    CanConnect CAN;
    static can_data_t can_data;

    while (true) {
        if (!CAN.ReceiveFrame()) {
            continue; // Cancel the rest
        }

        if (auto val = CAN.HandleRPM()) {
            can_data.rpm_value = val;
        }
        if (auto value = CAN.HandleSpeed()) {
            can_data.speed_value = value;
        }
        if (auto value = CAN.HandleFuel()) {
            can_data.fuel_value = value;
        }
        if (auto value = CAN.HandleTemp()) {
            can_data.temp_value = value;
        }
        xQueueOverwrite(can_queue, &can_data);

        vTaskDelay(pdMS_TO_TICKS(1)); // 10ms delay keeps scheduler happy
    }
}

extern "C" void ui_task(void * /*task_param*/) {
    can_data_t can_data;
    lvglInit();

    bsp_display_lock(1);
    MainDisplay dashboard;
    dashboard.SetupRpmArc();
    dashboard.SetupSpeedArc();
    dashboard.SetupFuelArc();
    dashboard.SetupTempArc();
    dashboard.RunArcAnimation();
    bsp_display_unlock();

    while (true) {
        // dashboard.HideOnTouch();
        xQueueReceive(can_queue, &can_data, portMAX_DELAY);
        bsp_display_lock(0);

        dashboard.SetRPMValue(can_data.rpm_value);
        dashboard.SetSpeedValue(can_data.speed_value);
        dashboard.SetFuelValue(can_data.fuel_value);
        dashboard.SetTempValue(can_data.temp_value);

        bsp_display_unlock();
    }
}

extern "C" void app_main(void) {
    xTaskCreatePinnedToCore(can_task, "CAN TASK", 4096, nullptr, 5, nullptr, 0);
    xTaskCreatePinnedToCore(ui_task, "UI/LVGL TASK", 8192, nullptr, 4, nullptr, 1);
}
