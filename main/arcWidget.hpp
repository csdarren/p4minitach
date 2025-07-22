#pragma once
#include "display/lv_display.h"
#include "widgets/arc/lv_arc.h"
#ifndef ARCWIDGET_HPP
#define ARCWIDGET_HPP
#include "gaugeMath.hpp"
#include "hexCodes.hpp"
#include "lvgl.h"

using lv_arc_data = struct lv_arc_data {
  lv_obj_t *src = lv_arc_create(lv_screen_active());
  lv_obj_t *label = lv_label_create(lv_screen_active());
  lv_obj_t *scale = lv_scale_create(lv_screen_active());
  const char *prefix{};
  int32_t size = 0;
  int32_t rotation = 0;
  int32_t angle = 0;
  int32_t width = ARC_WIDTH;
  int32_t color = GAUGE_COLOR;
  int32_t min = 0;
  int32_t max = 100;
  int32_t label_offset = 0;
};

static void setArcData(void *obj, int32_t value);

static void
debugArcAnim(lv_arc_data *arc); // Ensure this is before gaugeArcSetup, as it is
                                // used in that function
static void gaugeArcSetup(lv_arc_data *arc);

inline auto RpmArc() -> const lv_arc_data & {
  static lv_arc_data arc;

  arc.prefix = "RPM: ";
  arc.rotation = RPM_ARC_ROTATION;
  arc.size = RPM_ARC_SIZE;
  arc.angle = RPM_ARC_ANGLE;
  arc.min = RPM_ARC_MIN;
  arc.max = RPM_ARC_MAX;
  arc.label_offset = RPM_LABEL_OFFSET;

  gaugeArcSetup(&arc);
  return arc;
}

inline auto SpeedArc() -> const lv_arc_data & {
  static lv_arc_data arc;

  arc.prefix = "SPEED: ";
  arc.rotation = SPEEDO_ARC_ROTATION;
  arc.size = SPEEDO_ARC_SIZE;
  arc.angle = SPEEDO_ARC_ANGLE;
  arc.min = SPEED_ARC_MIN;
  arc.max = SPEED_ARC_MAX;
  arc.label_offset = SPEEDO_LABEL_OFFSET;

  gaugeArcSetup(&arc);
  return arc;
}
inline auto FuelArc() -> const lv_arc_data & {
  static lv_arc_data arc;

  arc.prefix = "FUEL: ";
  arc.rotation = FUEL_ARC_ROTATION;
  arc.size = FUEL_ARC_SIZE;
  arc.angle = FUEL_ARC_ANGLE;
  arc.label_offset = FUEL_LABEL_OFFSET;

  gaugeArcSetup(&arc);
  lv_arc_set_mode(arc.src, LV_ARC_MODE_REVERSE); // Set mode to reverse so the
                                                 // gauge goes the right way
  return arc;
}
inline auto TempArc() -> const lv_arc_data & {
  static lv_arc_data arc;

  arc.prefix = "TEMP: ";
  arc.rotation = TEMP_ARC_ROTATION;
  arc.size = TEMP_ARC_SIZE;
  arc.angle = TEMP_ARC_ANGLE;
  arc.label_offset = TEMP_LABEL_OFFSET;

  gaugeArcSetup(&arc);
  return arc;
}

static void gaugeArcSetup(lv_arc_data *arc) {
  // Label alignment
  lv_obj_align(arc->label, LV_ALIGN_BOTTOM_RIGHT, 0, arc->label_offset);

  // Scale Object alignment
  lv_obj_center(arc->scale);
  lv_obj_set_size(arc->scale, arc->size, arc->size);
  lv_scale_set_rotation(arc->scale, arc->rotation);
  lv_scale_set_mode(arc->scale, LV_SCALE_MODE_ROUND_INNER);
  lv_scale_set_range(arc->scale, arc->max, arc->min);
  lv_scale_set_angle_range(arc->scale, arc->angle);

  // Object alignment and size adjustment
  lv_obj_center(arc->src);
  lv_obj_set_size(arc->src, arc->size, arc->size);
  lv_arc_set_rotation(arc->src, arc->rotation);
  lv_arc_set_bg_angles(arc->src, 0, arc->angle);
  lv_arc_set_value(arc->src, 0);

  // Object style changes
  lv_obj_set_style_arc_width(arc->src, arc->width, LV_PART_INDICATOR);
  lv_obj_remove_style(arc->src, nullptr, LV_PART_KNOB);
  lv_obj_set_style_arc_color(arc->src, lv_color_hex(arc->color),
                             LV_PART_INDICATOR);
  lv_obj_set_style_arc_opa(arc->src, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_arc_rounded(arc->src, false, LV_PART_INDICATOR);

  // Make it unclickable and give it an animation for testing
  lv_obj_clear_flag(arc->src, LV_OBJ_FLAG_CLICKABLE);
  debugArcAnim(arc);

  // Set arc range (min/max values)
  lv_arc_set_range(arc->src, arc->min, arc->max);

  // Use these when actually changing values manually->
  // lv_obj_add_event_cb(arc->arc, rpmArcEventCB, LV_EVENT_VALUE_CHANGED,
  // arc->label); lv_obj_send_event(arc->arc, LV_EVENT_VALUE_CHANGED, nullptr);
}

static void setArcData(void *obj, int32_t value) {
  auto *arc = static_cast<lv_arc_data *>(obj);
  lv_arc_set_value(arc->src, value);

  lv_label_set_text_fmt(arc->label, "%s%d ", arc->prefix,
                        lv_arc_get_value(arc->src));
}

static void debugArcAnim(lv_arc_data *arc) {
  lv_anim_t anim;
  lv_anim_init(&anim);
  lv_anim_set_var(&anim,
                  arc); // This assigns which variable is sent to the callback
                        // function, which we set in the next function call
  lv_anim_set_exec_cb(
      &anim, setArcData); // Set the callback function that should already be
                          // defined to send variable info to.
  lv_anim_set_duration(
      &anim, ANIM_DUR); // Set duration of the animation, this is for some
                        // reason linked to the clockwise rotation
  lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
  lv_anim_set_playback_duration(
      &anim, ANIM_DUR); // Set duration of the animation, this is for some
                        // reason linked to the counter-clockwise rotation
  // lv_anim_set_repeat_delay(&anim, ANIM_DELAY);
  lv_anim_set_values(
      &anim, arc->min,
      arc->max); // Set the begin and end values for the animation, this should
                 // be set to the begin and end values for the arc
  lv_anim_start(&anim);
}

#endif
