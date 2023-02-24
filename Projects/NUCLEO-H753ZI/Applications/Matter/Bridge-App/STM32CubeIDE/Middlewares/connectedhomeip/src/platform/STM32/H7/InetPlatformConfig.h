/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP Inet
 *          Layer on the STM32 platform.
 *
 */

#pragma once

// ==================== Platform Adaptations ====================

// ==================== General Configuration Overrides ====================

#ifndef INET_CONFIG_NUM_TCP_ENDPOINTS
#define INET_CONFIG_NUM_TCP_ENDPOINTS 10
#endif // INET_CONFIG_NUM_TCP_ENDPOINTS

#ifndef INET_CONFIG_NUM_UDP_ENDPOINTS
#define INET_CONFIG_NUM_UDP_ENDPOINTS 10
#endif // INET_CONFIG_NUM_UDP_ENDPOINTS
