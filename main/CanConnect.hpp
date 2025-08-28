#ifndef CANCONNECT_HPP
#define CANCONNECT_HPP
#include <cmath>
#include <optional>
#include "driver/twai.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/projdefs.h"

#define TORQ3 0x0AA
#define SPEED 0x1A0

static constexpr gpio_num_t RX0 = GPIO_NUM_22;
static constexpr gpio_num_t TX0 = GPIO_NUM_21;
static constexpr gpio_num_t RX1 = GPIO_NUM_27;
static constexpr gpio_num_t TX1 = GPIO_NUM_47;

class CanConnect {
  private:
    twai_handle_t h0{};
    twai_handle_t h1{};

    auto TwaiConfig() -> void {
        twai_general_config_t twai01 = TWAI_GENERAL_CONFIG_DEFAULT(TX0, RX0, TWAI_MODE_NORMAL);
        twai01.controller_id = 0;
        twai_general_config_t twai02 = TWAI_GENERAL_CONFIG_DEFAULT(TX1, RX1, TWAI_MODE_NORMAL);
        twai02.controller_id = 1;

        twai_timing_config_t twai_timing = TWAI_TIMING_CONFIG_500KBITS();
        twai_filter_config_t twai_filter = TWAI_FILTER_CONFIG_ACCEPT_ALL();

        ESP_ERROR_CHECK(twai_driver_install_v2(&twai01, &twai_timing, &twai_filter, &h0));
        ESP_ERROR_CHECK(twai_driver_install_v2(&twai02, &twai_timing, &twai_filter, &h1));

        ESP_ERROR_CHECK(twai_start_v2(h0));
        ESP_ERROR_CHECK(twai_start_v2(h1));
    }

  public:
    // can_frame is where you store the frames,
    // delay is how long to wait for a frame before timing out
    auto ReceiveFrame(twai_message_t can_frame, TickType_t delay) -> bool {
        return twai_receive_v2(h0, &can_frame, pdMS_TO_TICKS(delay)) != ESP_OK;
    }

    // auto TransmitFrame(twai_message_t can_frame, TickType_t delay) -> std::optional<twai_message_t> {
    // use h1 for transmitting frames?
    // }

    CanConnect() {
        TwaiConfig();
    }
    static auto HandleRPM(twai_message_t can_frame) -> uint32_t {
        if (can_frame.identifier != TORQ3) {
            return 0;
        }
        uint32_t rpm_value = 0;
        uint32_t modifier = 255;
        uint8_t scale = 4;
        uint32_t increment_value = can_frame.data[6];
        uint32_t raw_value = increment_value * modifier;
        uint32_t active_raw_value = can_frame.data[5];
        rpm_value = ((raw_value / 2) / scale) + active_raw_value;
        return rpm_value;
    }
    static auto HandleSpeed(twai_message_t can_frame) -> uint32_t {
        if (can_frame.identifier != SPEED) {
            return 0;
        }
        uint32_t speed_value = 0;
        uint32_t modifier = 255;
        uint8_t increment_value = can_frame.data[1] & 0x0F; // extracts only the right most hex digit, AKA first 4 bits
        uint32_t raw_value = increment_value * modifier;
        uint32_t active_raw_value = can_frame.data[0];

        uint32_t scaled = (raw_value * 621 + 5000) / 1000;
        speed_value = scaled + active_raw_value;
        return speed_value;
    }
};

#endif
