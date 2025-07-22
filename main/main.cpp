#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "bsp_board_extra.h"
#include "display/lv_display.h"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_memory_utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_demos.h"
#include "lvgl.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "arcWidget.hpp"
#include "images.hpp"
#include "infoDisplay.hpp"

extern "C" void app_main(void) {
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

  bsp_display_lock(0);

  // lv_demo_music();
  // lv_demo_benchmark();
  // lv_demo_widgets();

  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(BG_COLOR),
                            LV_PART_MAIN);
  lv_obj_clear_flag(lv_screen_active(), LV_OBJ_FLAG_SCROLLABLE);
  RpmArc();
  SpeedArc();
  TempArc();
  FuelArc();
  LoadImages();
  InfoDisplay();

  bsp_display_unlock();
}
