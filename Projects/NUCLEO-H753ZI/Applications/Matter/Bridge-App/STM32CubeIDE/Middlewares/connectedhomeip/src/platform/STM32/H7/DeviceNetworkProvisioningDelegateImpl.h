/* See Project CHIP LICENSE file for licensing information. */

#pragma once

#include <platform/internal/GenericDeviceNetworkProvisioningDelegateImpl.h>

namespace chip {
namespace DeviceLayer {

namespace Internal {

template <class ImplClass>
class GenericDeviceNetworkProvisioningDelegateImpl;

} // namespace Internal

class DeviceNetworkProvisioningDelegateImpl final
    : public Internal::GenericDeviceNetworkProvisioningDelegateImpl<DeviceNetworkProvisioningDelegateImpl>
{
private:
    friend class GenericDeviceNetworkProvisioningDelegateImpl<DeviceNetworkProvisioningDelegateImpl>;

    CHIP_ERROR _ProvisionWiFiNetwork(const char * ssid, const char * passwd);
    CHIP_ERROR _ProvisionThreadNetwork(ByteSpan threadData);
};

} // namespace DeviceLayer
} // namespace chip
