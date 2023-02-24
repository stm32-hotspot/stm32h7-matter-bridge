/* See Project CHIP LICENSE file for licensing information. */

#pragma once

#include "platform/internal/DeviceNetworkInfo.h"
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class STM32Utils
{
public:
    static CHIP_ERROR IsAPEnabled(bool & apEnabled);
    static CHIP_ERROR IsStationEnabled(bool & staEnabled);
    static bool IsStationProvisioned(void);
    static CHIP_ERROR IsStationConnected(bool & connected);
    static CHIP_ERROR StartWiFiLayer(void);
    static CHIP_ERROR EnableStationMode(void);
    static CHIP_ERROR SetAPMode(bool enabled);
    static int OrderScanResultsByRSSI(const void * _res1, const void * _res2);
    static struct netif * GetNetif(const char * ifKey);
    static struct netif * GetStationNetif(void);
    static bool IsInterfaceUp(const char * ifKey);
    static bool HasIPv6LinkLocalAddress(const char * ifKey);

    static CHIP_ERROR GetWiFiStationProvision(Internal::DeviceNetworkInfo & netInfo, bool includeCredentials);
    static CHIP_ERROR SetWiFiStationProvision(const Internal::DeviceNetworkInfo & netInfo);
    static CHIP_ERROR ClearWiFiStationProvision(void);

};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
