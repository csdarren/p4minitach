#pragma once
#include "esp_log.h"
#include "misc/lv_event.h"
#include "stdlib/lv_sprintf.h"
#include "time.h"
#include "widgets/arc/lv_arc.h"
#include "widgets/label/lv_label.h"
#include <cstdint>
#ifndef MAINDISPLAY_HPP
#define MAINDISPLAY_HPP
#include <string>
#include "gaugeMath.hpp"
#include "hexCodes.hpp"
#include "lvgl.h"
#include "ParentDisplay.hpp"

LV_IMG_DECLARE(MiniDash_v1_2);

class MainDisplay : ParentDisplay {
  private:
    lv_obj_t *dash_bg = lv_img_create(parentDisplay);

    enum class ArcType : uint8_t {
        uNULL = 0,
        RPM = 1,
        SPEED = 2,
        TEMP = 3,
        FUEL = 4

    };

    struct arc_config {
        ArcType identifier = ArcType::uNULL;
        int32_t size = 0;
        int32_t rotation = 0;
        int32_t angle = 0;
        int32_t width = ARC_WIDTH;
        int32_t color = GAUGE_COLOR;
        int32_t min = 0;
        int32_t max = 100;
        int32_t ticks = 0;
        int32_t ticks_major = 1;
    };
    struct label_config {
        uint16_t value = 0;
        std::string prefix;
        int32_t label_offset = 0;
    };

    auto static setArcData(void *obj, int32_t value) -> void {
        auto *arc = static_cast<lv_obj_t *>(obj);
        lv_arc_set_value(arc, value);
    }

    auto static debugArcAnim(arc_config *config, lv_obj_t *arc) -> void {
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_var(&anim, arc);

        lv_anim_set_exec_cb(&anim, setArcData);

        lv_anim_set_duration(
            &anim, ANIM_DUR); // Set duration of the animation, this is for some
                              // reason linked to the clockwise rotation
        lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_playback_duration(
            &anim, ANIM_DUR); // Set duration of the animation, this is for some
                              // reason linked to the counter-clockwise rotation
                              // lv_anim_set_repeat_delay(&anim, ANIM_DELAY);
        lv_anim_set_values(
            &anim, config->min,
            config->max); // Set the begin and end values for the animation, this should
                          // be set to the begin and end values for the arc
        lv_anim_start(&anim);
    }

    static void arc_event_cb(lv_event_t *event) {
        lv_obj_t *arc = (lv_obj_t *)lv_event_get_target(event);
        lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(event);

        static char buf[8];
        int32_t value = lv_arc_get_value(arc);
        lv_snprintf(buf, sizeof(buf), "%d", value);
        lv_label_set_text(label, buf);
    }

    static void arcSetup(lv_obj_t *arc, arc_config *config) {
        lv_obj_t *scale = lv_scale_create(arc);
        // Scale Object alignment
        lv_obj_center(scale);
        lv_obj_set_size(scale, config->size, config->size);
        lv_scale_set_rotation(scale, config->rotation);
        lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_INNER);
        lv_scale_set_range(scale, config->min, config->max);
        lv_scale_set_angle_range(scale, config->angle);
        lv_obj_set_style_text_color(scale, lv_color_hex(0xFFFFFF), 0);
        lv_scale_set_total_tick_count(scale, config->ticks);
        lv_scale_set_major_tick_every(scale, config->ticks_major);

        // Object alignment and size adjustment
        lv_obj_center(arc);
        lv_obj_set_size(arc, config->size, config->size);
        lv_arc_set_rotation(arc, config->rotation);
        lv_arc_set_bg_angles(arc, 0, config->angle);
        lv_arc_set_value(arc, 0);

        // Object style changes
        lv_obj_set_style_arc_width(arc, config->width, LV_PART_INDICATOR);
        lv_obj_remove_style(arc, nullptr, LV_PART_KNOB);
        lv_obj_set_style_arc_color(arc, lv_color_hex(config->color),
                                   LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_arc_rounded(arc, false, LV_PART_INDICATOR);

        // Make it unclickable and give it an animation for testing
        lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);

        //Arc specific settings:
        if (config->identifier == ArcType::FUEL) {
            lv_arc_set_range(arc, config->max, config->min);
            lv_arc_set_mode(arc, LV_ARC_MODE_REVERSE); // Set mode to reverse so the
        } else {
            lv_arc_set_range(arc, config->min, config->max);
        }

        debugArcAnim(config, arc);
    }

    static auto labelSetup(lv_obj_t *label, label_config &config) -> void {
        lv_obj_align(label, LV_ALIGN_CENTER, LABEL_OFFSET_X, config.label_offset);
        // lv_label_set_text(label, config.prefix.c_str());
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    }

    auto RpmArc() -> void {
        lv_obj_t *arc = lv_arc_create(dash_bg);
        static arc_config arc_config;
        arc_config.identifier = ArcType::RPM;
        arc_config.rotation = RPM_ARC_ROTATION;
        arc_config.size = RPM_ARC_SIZE;
        arc_config.angle = RPM_ARC_ANGLE;
        arc_config.min = RPM_ARC_MIN;
        arc_config.max = RPM_ARC_MAX;
        arc_config.ticks = RPM_TICKS;
        arcSetup(arc, &arc_config);

        lv_obj_t *label = lv_label_create(dash_bg);
        static label_config label_config;
        label_config.prefix = "RPM: ";
        label_config.label_offset = RPM_LABEL_OFFSET_Y;
        label_config.value = lv_arc_get_value(arc);
        labelSetup(label, label_config);
    }

    auto SpeedArc() -> void {
        lv_obj_t *label = lv_label_create(dash_bg);
        static label_config label_config;
        label_config.prefix = "SPEED: ";
        label_config.label_offset = SPEEDO_LABEL_OFFSET_Y;
        labelSetup(label, label_config);

        lv_obj_t *arc = lv_arc_create(dash_bg);
        static arc_config arc_config;
        arc_config.identifier = ArcType::SPEED;
        arc_config.rotation = SPEEDO_ARC_ROTATION;
        arc_config.size = SPEEDO_ARC_SIZE;
        arc_config.angle = SPEEDO_ARC_ANGLE;
        arc_config.min = SPEED_ARC_MIN;
        arc_config.max = SPEED_ARC_MAX;
        arc_config.ticks = SPEED_TICKS;
        arcSetup(arc, &arc_config);
    }

    auto FuelArc() -> void {
        lv_obj_t *label = lv_label_create(dash_bg);
        static label_config label_config;
        label_config.label_offset = FUEL_LABEL_OFFSET_Y;
        label_config.prefix = "FUEL: ";
        labelSetup(label, label_config);

        lv_obj_t *arc = lv_arc_create(dash_bg);
        static arc_config arc_config;
        arc_config.identifier = ArcType::FUEL;
        arc_config.rotation = FUEL_ARC_ROTATION;
        arc_config.size = FUEL_ARC_SIZE;
        arc_config.angle = FUEL_ARC_ANGLE;
        arc_config.min = FUEL_ARC_MIN;
        arc_config.max = FUEL_ARC_MAX;
        arc_config.ticks = FUEL_TICKS;
        arcSetup(arc, &arc_config);
    }

    auto TempArc() -> void {
        lv_obj_t *label = lv_label_create(dash_bg);
        static label_config label_config;
        label_config.label_offset = TEMP_LABEL_OFFSET_Y;
        label_config.prefix = "TEMP: ";
        labelSetup(label, label_config);

        lv_obj_t *arc = lv_arc_create(dash_bg);
        static arc_config arc_config;
        arc_config.identifier = ArcType::TEMP;
        arc_config.rotation = TEMP_ARC_ROTATION;
        arc_config.size = TEMP_ARC_SIZE;
        arc_config.angle = TEMP_ARC_ANGLE;
        arc_config.ticks = TEMP_TICKS;
        arcSetup(arc, &arc_config);
    }

    auto ParentSetup() -> void {
        lv_obj_set_size(parentDisplay, 720, 720);

        lv_obj_remove_flag(parentDisplay, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_set_style_bg_opa(parentDisplay, LV_OPA_TRANSP, 0);
    }

    auto ImageSetup() -> void {
        lv_img_set_src(dash_bg, &MiniDash_v1_2);

        lv_obj_center(dash_bg);

        lv_img_set_zoom(dash_bg, LV_ZOOM_NONE);
    }

  public:
    MainDisplay() {
        ParentSetup();
        ImageSetup();

        RpmArc();
        FuelArc();
        SpeedArc();
        TempArc();
    }
    auto getMainDisplay() -> lv_obj_t * {
        return parentDisplay;
    }
};

#endif
