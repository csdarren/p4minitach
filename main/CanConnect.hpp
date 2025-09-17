#pragma once
#ifndef CANCONNECT_HPP
#define CANCONNECT_HPP

#include "driver/twai.h"
#include "esp_log.h"
#include <cmath>

#define TORQ3 0x0AA
#define SPEED 0x1A0
#define ENGDATA 0x1D0
#define FUELMLS 0x330

#define RX0 GPIO_NUM_22
#define TX0 GPIO_NUM_21
#define RX1 GPIO_NUM_27
#define TX1 GPIO_NUM_47

using can_data_t = struct {
    uint16_t rpm_value;
    uint8_t speed_value;
    uint8_t fuel_value;
    uint16_t temp_value;
};

// These operator definitions allow direct comparison of entire struct
inline auto operator==(const can_data_t &lhs, const can_data_t &rhs) -> bool {
    return lhs.rpm_value == rhs.rpm_value &&
           lhs.speed_value == rhs.speed_value &&
           lhs.fuel_value == rhs.fuel_value &&
           lhs.temp_value == rhs.temp_value;
}

inline auto operator!=(const can_data_t &lhs, const can_data_t &rhs) -> bool {
    return !(lhs == rhs);
}
//////////////////

class CanConnect {
  private:
    twai_handle_t h0{};
    twai_handle_t h1{};
    twai_message_t can_frame{};
    static constexpr TickType_t timeout_in_ms = 3000;

    auto TwaiConfig(bool transmit) -> void {
        twai_timing_config_t twai_timing = TWAI_TIMING_CONFIG_500KBITS();
        twai_filter_config_t twai_filter = TWAI_FILTER_CONFIG_ACCEPT_ALL();

        twai_general_config_t twai0 = TWAI_GENERAL_CONFIG_DEFAULT(TX0, RX0, TWAI_MODE_NORMAL);
        twai0.controller_id = 0;

        if (esp_err_t err = twai_driver_install_v2(&twai0, &twai_timing, &twai_filter, &h0) != ESP_OK) {
            ESP_LOGE("CAN FATAL", "Failed to install twai driver for handle 0 (receive CAN) ERR: %s", esp_err_to_name(err));
        }
        if (esp_err_t err = twai_start_v2(h0) != ESP_OK) {
            ESP_LOGE("CAN FATAL", "Failed to start twai driver for handle 0 (receive CAN) ERR: %s", esp_err_to_name(err));
        };

        if (transmit) {
            twai_general_config_t twai1 = TWAI_GENERAL_CONFIG_DEFAULT(TX1, RX1, TWAI_MODE_NORMAL);
            twai1.controller_id = 1;
            if (esp_err_t err = twai_driver_install_v2(&twai1, &twai_timing, &twai_filter, &h1) != ESP_OK) {
                ESP_LOGE("CAN FATAL", "Failed to install twai driver for handle 1 (transmit CAN) ERR: %s", esp_err_to_name(err));
            }
            if (esp_err_t err = twai_start_v2(h1) != ESP_OK) {
                ESP_LOGE("CAN FATAL", "Failed to start twai driver for handle 1 (transmit CAN) ERR: %s", esp_err_to_name(err));
            }
        };
    }

    auto TransmitFrame(const twai_message_t &can_frame) -> void {
        if (esp_err_t err = twai_transmit_v2(h1, &can_frame, pdMS_TO_TICKS(timeout_in_ms)) != ESP_OK) {
            ESP_LOGE("CAN FATAL", "Not transmitting any CAN Data, %s", esp_err_to_name(err));
        }
    }

  public:
    CanConnect(bool transmit = false) {
        TwaiConfig(transmit);
    }
    auto ReceiveFrame() -> bool {
        if (esp_err_t err = twai_receive_v2(h0, &can_frame, pdMS_TO_TICKS(timeout_in_ms)) != ESP_OK) {
            ESP_LOGE("CAN FATAL", "Not receiving any CAN Data, %s", esp_err_to_name(err));
            return false;
        }
        return true;
    }

    auto HandleRPM(bool transmit = false) -> uint16_t {
        if (can_frame.identifier != TORQ3) {
            return 0;
        }
        uint16_t rpm_value = 0;
        uint8_t modifier = 255;
        float scale = 0.25;
        uint8_t increment_value = can_frame.data[5];
        uint16_t raw_value = increment_value * modifier;
        uint8_t active_raw_value = can_frame.data[4];
        rpm_value = std::round(((static_cast<float>(raw_value) / 2) * scale) + static_cast<float>(active_raw_value));

        if (transmit) {
            TransmitFrame(can_frame);
        }
        return rpm_value;
    }
    auto HandleSpeed(bool transmit = false) -> uint8_t {
        if (can_frame.identifier != SPEED) {
            return 0;
        }
        uint8_t speed_value = 0;
        uint8_t modifier = 255;
        float scale = 0.621;
        uint32_t scaled = 0;
        uint8_t increment_value = can_frame.data[1] & 0x0F; // extracts only the right most hex digit, AKA first 4 bits
        uint16_t raw_value = increment_value * modifier;
        uint8_t active_raw_value = can_frame.data[0];

        scaled = std::round(static_cast<float>(raw_value) * scale);
        speed_value = scaled + active_raw_value;

        if (transmit) {
            TransmitFrame(can_frame);
        }
        return speed_value;
    }

    auto HandleFuel(bool transmit = false) -> uint8_t {
        if (can_frame.identifier != ENGDATA) {
            return 0;
        }

        if (transmit) {
            TransmitFrame(can_frame);
        }
        return (can_frame.data[3] * 100) / 255; // full byte, turned into percentage for fuel
    }

    auto HandleTemp(bool transmit = false) -> uint8_t {
        if (can_frame.identifier != ENGDATA) {
            return 0;
        }
        if (transmit) {
            TransmitFrame(can_frame);
        }
        // coolant temp is held as one full byte (0-255), so should return the full byte and be fine
        return can_frame.data[0] - 48; // -48 for bias/offset
    }

    auto HandleODO(bool transmit = false) -> uint32_t {
        if (can_frame.identifier != FUELMLS) {
            return 0;
        }
        uint32_t odometer_value = 0;

        if (transmit) {
            TransmitFrame(can_frame);
        }
        return odometer_value;
    }

    auto HandleFUELRNG(bool transmit = false) -> uint16_t {
        if (can_frame.identifier != FUELMLS) {
            return 0;
        }
        uint16_t fuelRange_value = 0;

        if (transmit) {
            TransmitFrame(can_frame);
        }
        return fuelRange_value;
    }
};

#endif
