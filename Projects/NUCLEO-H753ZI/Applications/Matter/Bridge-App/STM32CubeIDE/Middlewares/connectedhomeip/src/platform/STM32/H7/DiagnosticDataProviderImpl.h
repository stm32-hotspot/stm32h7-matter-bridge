/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          Provides an implementation of the DiagnosticDataProvider object.
 */

#pragma once

#include <memory>

#include <platform/DiagnosticDataProvider.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for STM32 platforms.
 */
class DiagnosticDataProviderImpl : public DiagnosticDataProvider
{
public:
    static DiagnosticDataProviderImpl & GetDefaultInstance();

    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR GetCurrentHeapFree(uint64_t & currentHeapFree) override;
    CHIP_ERROR GetCurrentHeapUsed(uint64_t & currentHeapUsed) override;
    CHIP_ERROR GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark) override;

    CHIP_ERROR GetRebootCount(uint16_t & rebootCount) override;
    CHIP_ERROR GetUpTime(uint64_t & upTime) override;
    CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) override;
    CHIP_ERROR GetBootReason(BootReasonType & bootReason) override;

    CHIP_ERROR GetNetworkInterfaces(NetworkInterface ** netifpp) override;
    void ReleaseNetworkInterfaces(NetworkInterface * netifp) override;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
/* 
		DiagnosticDataProviderImpl::GetWiFiBssId,
		DiagnosticDataProviderImpl::GetWiFiSecurityType
		DiagnosticDataProviderImpl::GetWiFiVersion
		DiagnosticDataProviderImpl::GetWiFiChannelNumber
		DiagnosticDataProviderImpl::GetWiFiRssi
		DiagnosticDataProviderImpl::GetWiFiBeaconLostCount
		DiagnosticDataProviderImpl::GetWiFiCurrentMaxRate
		DiagnosticDataProviderImpl::GetWiFiPacketMulticastRxCount
		DiagnosticDataProviderImpl::GetWiFiPacketMulticastTxCount
		DiagnosticDataProviderImpl::GetWiFiPacketUnicastRxCount
		DiagnosticDataProviderImpl::GetWiFiPacketUnicastTxCount
		DiagnosticDataProviderImpl::GetWiFiOverrunCount
		DiagnosticDataProviderImpl::ResetWiFiNetworkDiagnosticsCounts
*/
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
};

/**
 * Returns the platform-specific implementation of the DiagnosticDataProvider singleton object.
 *
 * Applications can use this to gain access to features of the DiagnosticDataProvider
 * that are specific to the selected platform.
 */
DiagnosticDataProvider & GetDiagnosticDataProviderImpl();

} // namespace DeviceLayer
} // namespace chip
