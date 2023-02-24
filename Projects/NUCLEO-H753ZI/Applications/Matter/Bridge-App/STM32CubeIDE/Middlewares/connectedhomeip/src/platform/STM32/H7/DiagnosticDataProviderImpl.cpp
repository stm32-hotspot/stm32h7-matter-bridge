/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          Provides an implementation of the DiagnosticDataProvider object
 *          for STM32 platform.
 */
 
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <app-common/zap-generated/enums.h>
#include <crypto/CHIPCryptoPAL.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/STM32/H7/DiagnosticDataProviderImpl.h>
#include <platform/STM32/H7/STM32Utils.h>

// ADD HERE stm32event, stm32logs, stm32netif, stm32wifi libraries ..

using namespace ::chip;
using namespace ::chip::TLV;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::app::Clusters::GeneralDiagnostics;

namespace {

InterfaceType GetInterfaceType(const char * if_desc)
{
    if (strncmp(if_desc, "ap", strnlen(if_desc, 2)) == 0 || strncmp(if_desc, "sta", strnlen(if_desc, 3)) == 0)
        return InterfaceType::EMBER_ZCL_INTERFACE_TYPE_WI_FI;
    if (strncmp(if_desc, "openthread", strnlen(if_desc, 10)) == 0)
        return InterfaceType::EMBER_ZCL_INTERFACE_TYPE_THREAD;
    if (strncmp(if_desc, "eth", strnlen(if_desc, 3)) == 0)
        return InterfaceType::EMBER_ZCL_INTERFACE_TYPE_ETHERNET;
    return InterfaceType::EMBER_ZCL_INTERFACE_TYPE_UNSPECIFIED;
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

} // namespace

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{

	return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{

	return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetRebootCount(uint16_t & rebootCount)
{
    uint32_t count = 0;

    CHIP_ERROR err = ConfigurationMgr().GetRebootCount(count);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(count <= UINT16_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        rebootCount = static_cast<uint16_t>(count);
    }

    return err;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetUpTime(uint64_t & upTime)
{
    System::Clock::Timestamp currentTime = System::SystemClock().GetMonotonicTimestamp();
    System::Clock::Timestamp startTime   = PlatformMgrImpl().GetStartTime();

    if (currentTime >= startTime)
    {
        upTime = std::chrono::duration_cast<System::Clock::Seconds64>(currentTime - startTime).count();
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INVALID_TIME;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    uint64_t upTime = 0;

    if (GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalHours = 0;
        if (ConfigurationMgr().GetTotalOperationalHours(totalHours) == CHIP_NO_ERROR)
        {
            VerifyOrReturnError(upTime / 3600 <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
            totalOperationalHours = totalHours + static_cast<uint32_t>(upTime / 3600);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_INVALID_TIME;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(BootReasonType & bootReason)
{
    bootReason = BootReasonType::kUnspecified;
    uint8_t reason;
	
	//HAVE TO ADD bootreason for STM32 !
	bootReason = BootReasonType::kSoftwareReset; //DEFAULT
	
	return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
	return CHIP_ERROR_NOT_IMPLEMENTED;

}
	
void DiagnosticDataProviderImpl::ReleaseNetworkInterfaces(NetworkInterface * netifp)
{

}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
/* 

*/
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
