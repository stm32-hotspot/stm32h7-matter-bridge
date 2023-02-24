/* See Project CHIP LICENSE file for licensing information. */

#pragma once

#include <platform/NetworkCommissioning.h>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {
	
namespace {
constexpr uint8_t kMaxWiFiNetworks                  = 1;
constexpr uint8_t kWiFiScanNetworksTimeOutSeconds   = 10;
constexpr uint8_t kWiFiConnectNetworkTimeoutSeconds = 30;
} // namespace

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
