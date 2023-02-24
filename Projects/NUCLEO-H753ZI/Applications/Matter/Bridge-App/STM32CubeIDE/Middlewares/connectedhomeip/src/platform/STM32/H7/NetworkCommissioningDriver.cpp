/* See Project CHIP LICENSE file for licensing information. */

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/STM32/H7/STM32Utils.h>
#include <platform/STM32/H7/NetworkCommissioningDriver.h>

#include <limits>
#include <string>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;
namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {
	
namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";
static uint8_t WiFiSSIDStr[DeviceLayer::Internal::kMaxWiFiSSIDLength];
} // namespace


} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
