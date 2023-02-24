/* See Project CHIP LICENSE file for licensing information. */

#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>

#include "DeviceNetworkProvisioningDelegateImpl.h"
#include "NetworkCommissioningDriver.h"

namespace chip {
namespace DeviceLayer {
	
CHIP_ERROR DeviceNetworkProvisioningDelegateImpl::_ProvisionWiFiNetwork(const char * ssid, const char * key)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(NetworkProvisioning, "STM32NetworkProvisioningDelegate: SSID: %s", ssid);

	if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network: %s", chip::ErrorStr(err));
    }
    return err;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}
	
CHIP_ERROR DeviceNetworkProvisioningDelegateImpl::_ProvisionThreadNetwork(ByteSpan threadData)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    CHIP_ERROR err = CHIP_NO_ERROR;

exit:
    return err;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

} // namespace DeviceLayer
} // namespace chip
