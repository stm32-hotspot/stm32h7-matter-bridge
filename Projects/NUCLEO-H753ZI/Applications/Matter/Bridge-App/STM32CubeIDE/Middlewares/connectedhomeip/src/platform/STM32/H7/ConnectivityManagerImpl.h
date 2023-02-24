/* See Project CHIP LICENSE file for licensing information. */

#pragma once

#include <platform/CHIPDeviceConfig.h>

#include <platform/ConnectivityManager.h>
#include <platform/internal/GenericConnectivityManagerImpl.h>
#include <platform/internal/GenericConnectivityManagerImpl_UDP.h>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoWiFi.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoThread.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoBLE.h>
#endif

#include <lib/support/BitFlags.h>

namespace chip {

namespace Inet {
class IPAddress;
} // namespace Inet

namespace DeviceLayer {

class PlatformManagerImpl;

/**
 * Concrete implementation of the ConnectivityManager singleton object for the STM32 platform.
 */
 class ConnectivityManagerImpl final : public ConnectivityManager,
                                      public Internal::GenericConnectivityManagerImpl<ConnectivityManagerImpl>,
                                      public Internal::GenericConnectivityManagerImpl_UDP<ConnectivityManagerImpl>,
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
                                      public Internal::GenericConnectivityManagerImpl_TCP<ConnectivityManagerImpl>,
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
                                      public Internal::GenericConnectivityManagerImpl_WiFi<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoWiFi<ConnectivityManagerImpl>,
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
                                      public Internal::GenericConnectivityManagerImpl_BLE<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoBLE<ConnectivityManagerImpl>,
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
                                      public Internal::GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>
#else
                                      public Internal::GenericConnectivityManagerImpl_NoThread<ConnectivityManagerImpl>
#endif
{
	// Allow the ConnectivityManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ConnectivityManager;

private:
    CHIP_ERROR _Init(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

    // ===== Members for internal use by the following friends.

    friend ConnectivityManager & ConnectivityMgr(void);
    friend ConnectivityManagerImpl & ConnectivityMgrImpl(void);

    static ConnectivityManagerImpl sInstance;
};




/**
 * Returns the public interface of the ConnectivityManager singleton object.
 *
 * Chip applications should use this to access features of the ConnectivityManager object
 * that are common to all platforms.
 */
inline ConnectivityManager & ConnectivityMgr(void)
{
    return ConnectivityManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the ConnectivityManager singleton object.
 *
 * Chip applications can use this to gain access to features of the ConnectivityManager
 * that are specific to the STM32 platform.
 */
inline ConnectivityManagerImpl & ConnectivityMgrImpl(void)
{
    return ConnectivityManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
