#include "bsp/esp32_p4_wifi6_touch_lcd_xc.h"
#include "lvgl.h"

#include "MainDisplay.hpp"
#include "CanConnect.hpp"

static void lvglInit() {
    lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 5,
        .task_stack = 7168,
        .task_affinity = 1,
        .task_max_sleep_ms = 500,
        .timer_period_ms = 5,
    };
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = lvgl_cfg,
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

extern "C" void can_task(void * /*task_param*/) {
    CanConnect CAN;
    // struct defined in CanConnect.hpp
    can_data_t can_data{0, 0, 0, 0};
    can_data_t can_data_last{0, 0, 0, 0};
    static char statBuffer[1024];

    while (true) {
        if (!CAN.ReceiveFrame()) {
            vTaskGetRunTimeStats(statBuffer);
            ESP_LOGI("Stats", "\n%s", statBuffer);
            continue; // Cancel the rest
        }

        if (uint16_t val = CAN.HandleRPM(); can_data.rpm_value != val) {
            can_data.rpm_value = val;
        }
        if (uint8_t val = CAN.HandleSpeed(); can_data.speed_value != val) {
            can_data.speed_value = val;
        }
        if (uint8_t val = CAN.HandleFuel(); can_data.fuel_value != val) {
            can_data.fuel_value = val;
        }
        if (uint8_t val = CAN.HandleTemp(); can_data.temp_value != val) {
            can_data.temp_value = val;
        }
        if (can_data_last != can_data) {
            xQueueOverwrite(can_queue, &can_data);
            can_data_last = can_data;
        }

        //Every x ms the queue is overwritten with new data

        // vTaskGetRunTimeStats(statBuffer);
        // ESP_LOGI("Stats", "\n%s", statBuffer);
        vTaskDelay(pdMS_TO_TICKS(500)); // x ms delay keeps scheduler happy
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
        if (xQueueReceive(can_queue, &can_data, pdMS_TO_TICKS(3000)) == pdTRUE) {
            bsp_display_lock(1);

            dashboard.SetRPMValue(can_data.rpm_value);
            dashboard.SetSpeedValue(can_data.speed_value);
            dashboard.SetFuelValue(can_data.fuel_value);
            dashboard.SetTempValue(can_data.temp_value);

            bsp_display_unlock();
            vTaskDelay(pdMS_TO_TICKS(10));
        } else {
            ESP_LOGE("UI FATAL", "Queue is empty, no data being received from can_task");
            continue;
        }
    }
}

extern "C" void app_main(void) {
    can_queue = xQueueCreate(1, sizeof(can_data_t));
    xTaskCreatePinnedToCore(can_task, "CAN TASK", 4096, nullptr, 5, nullptr, 0);
    xTaskCreatePinnedToCore(ui_task, "UI/LVGL TASK", 8192, nullptr, 4, nullptr, 1);
}
