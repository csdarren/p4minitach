#pragma once
#include <stdint.h>
#ifndef GAUGEMATH_HPP
#define GAUGEMATH_HPP

static constexpr int32_t ARC_WIDTH = 35;
static constexpr int32_t ANIM_DUR = 1000;
static constexpr int32_t ANIM_DELAY = 200;

static constexpr int32_t RPM_ARC_SIZE = 542;
static constexpr int32_t RPM_ARC_ROTATION = 200;
static constexpr int32_t RPM_ARC_ANGLE = 140;
static constexpr int32_t RPM_ARC_MIN = 0;
static constexpr int32_t RPM_ARC_MAX = 7000;
static constexpr int32_t RPM_TICKS = 8;

static constexpr int32_t SPEED_ARC_SIZE = 672;
static constexpr int32_t SPEED_ARC_ROTATION = 190;
static constexpr int32_t SPEED_ARC_ANGLE = 160;
static constexpr int32_t SPEED_ARC_MIN = 0;
static constexpr int32_t SPEED_ARC_MAX = 130;
static constexpr int32_t SPEED_TICKS = 14;

static constexpr int32_t FUEL_ARC_SIZE = 672;
static constexpr int32_t FUEL_ARC_ROTATION = 10;
static constexpr int32_t FUEL_ARC_ANGLE = 45;
static constexpr int32_t FUEL_ARC_MIN = 100;
static constexpr int32_t FUEL_ARC_MAX = 0;
static constexpr int32_t FUEL_TICKS = 5;

static constexpr int32_t TEMP_ARC_SIZE = 672;
static constexpr int32_t TEMP_ARC_ROTATION = 125;
static constexpr int32_t TEMP_ARC_ANGLE = 45;
static constexpr int32_t TEMP_ARC_MIN = 0;
static constexpr int32_t TEMP_ARC_MAX = 200;
static constexpr int32_t TEMP_TICKS = 5;

static constexpr int32_t RPM_LABEL_OFFSET_Y = 180;
static constexpr int32_t SPEEDO_LABEL_OFFSET_Y = 195;
static constexpr int32_t FUEL_LABEL_OFFSET_Y = 210;
static constexpr int32_t TEMP_LABEL_OFFSET_Y = 225;
static constexpr int32_t LABEL_OFFSET_X = 0;

#endif
