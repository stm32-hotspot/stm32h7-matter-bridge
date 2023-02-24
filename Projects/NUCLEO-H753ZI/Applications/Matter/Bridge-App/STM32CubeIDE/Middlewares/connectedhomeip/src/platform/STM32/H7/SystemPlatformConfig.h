/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP System
 *          Layer on STM32 platforms.
 *
 */

#pragma once

#include <stdint.h>

namespace chip {
namespace DeviceLayer {
struct ChipDeviceEvent;
} // namespace DeviceLayer
} // namespace chip

// ==================== Platform Adaptations ====================
#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 1		//Notes : had to be 0 or implement SystemTimeSupport.cpp
#define CHIP_SYSTEM_CONFIG_EVENT_OBJECT_TYPE const struct ::chip::DeviceLayer::ChipDeviceEvent *

// ========== Platform-specific Configuration Overrides =========

#ifndef CHIP_SYSTEM_CONFIG_NUM_TIMERS
#define CHIP_SYSTEM_CONFIG_NUM_TIMERS 16
#endif // CHIP_SYSTEM_CONFIG_NUM_TIMERS
