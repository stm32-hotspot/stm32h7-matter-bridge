/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          General utility methods for the STM32 platform.
 */

 /* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/STM32/H7/STM32Utils.h>

using namespace ::chip::DeviceLayer::Internal;
using chip::DeviceLayer::Internal::DeviceNetworkInfo;

CHIP_ERROR STM32Utils::IsAPEnabled(bool & apEnabled)
{
	return CHIP_ERROR_NOT_IMPLEMENTED;
	// ADD TO IMPL WIFI ACCESS POINT
}

CHIP_ERROR STM32Utils::IsStationEnabled(bool & staEnabled)
{
	return CHIP_ERROR_NOT_IMPLEMENTED;
	// ADD TO IMPL WIFI ACCESS POINT
}

bool STM32Utils::IsStationProvisioned(void)
{
	// NOT_IMPLEMENTED;
}

CHIP_ERROR STM32Utils::IsStationConnected(bool & connected)
{
	return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR STM32Utils::StartWiFiLayer(void)
{
	return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR STM32Utils::EnableStationMode(void)
{
	return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR STM32Utils::SetAPMode(bool enabled)
{
	return CHIP_ERROR_NOT_IMPLEMENTED;
}

int STM32Utils::OrderScanResultsByRSSI(const void * _res1, const void * _res2)
{
	// NOT_IMPLEMENTED;
}

struct netif * STM32Utils::GetStationNetif(void)
{
	// NOT_IMPLEMENTED;
}

struct netif * STM32Utils::GetNetif(const char * ifKey)
{
	// NOT_IMPLEMENTED;
}

bool STM32Utils::IsInterfaceUp(const char * ifKey)
{
	// NOT_IMPLEMENTED;
}

bool STM32Utils::HasIPv6LinkLocalAddress(const char * ifKey)
{
	// NOT_IMPLEMENTED;
}

CHIP_ERROR STM32Utils::GetWiFiStationProvision(Internal::DeviceNetworkInfo & netInfo, bool includeCredentials)
{
	return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR STM32Utils::SetWiFiStationProvision(const Internal::DeviceNetworkInfo & netInfo)
{
	return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR STM32Utils::ClearWiFiStationProvision(void)
{
	return CHIP_ERROR_NOT_IMPLEMENTED;
}

