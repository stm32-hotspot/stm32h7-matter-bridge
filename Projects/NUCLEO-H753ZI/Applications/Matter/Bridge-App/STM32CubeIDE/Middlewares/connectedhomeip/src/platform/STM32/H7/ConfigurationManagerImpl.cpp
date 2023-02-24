/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          Provides the implementation of the Device Layer ConfigurationManager object
 *          for the STM32.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/core/CHIPKeyIds.h>
#include <lib/support/CodeUtils.h>
#include <platform/ConfigurationManager.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/STM32/H7/STM32Config.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#if defined(STM32H753xx)
  #include "stm32h7xx.h"
  #include "stm32h7xx_hal.h"
  #include "stm32h7xx_hal_def.h"
  #include "stm32h7xx_hal_rcc.h"
#endif

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

namespace {

enum
{
    kChipProduct_Connect = 0x0016
};

} // unnamed namespace

ConfigurationManagerImpl & ConfigurationManagerImpl::GetDefaultInstance()
{
    static ConfigurationManagerImpl sInstance;
    return sInstance;
}

CHIP_ERROR ConfigurationManagerImpl::Init()
{
    CHIP_ERROR err;
    uint32_t rebootCount;
	
	#ifdef CONFIG_NVS_ENCRYPTION
	// Securely initialize the nvs partitions, have to add it !
	
	#endif
	
	 // Force initialization of NVS namespaces if they doesn't already exist.
    err = STM32Config::EnsureNamespace(STM32Config::kConfigNamespace_ChipFactory);
    SuccessOrExit(err);
    err = STM32Config::EnsureNamespace(STM32Config::kConfigNamespace_ChipConfig);
    SuccessOrExit(err);
    err = STM32Config::EnsureNamespace(STM32Config::kConfigNamespace_ChipCounters);
    SuccessOrExit(err);
	
	 if (STM32Config::ConfigValueExists(STM32Config::kCounterKey_RebootCount))
    {
        err = GetRebootCount(rebootCount);
        SuccessOrExit(err);

        err = StoreRebootCount(rebootCount + 1);
        SuccessOrExit(err);
    }
    else
    {
        // The first boot after factory reset of the Node.
        err = StoreRebootCount(1);
        SuccessOrExit(err);
    }
	
	if (!STM32Config::ConfigValueExists(STM32Config::kCounterKey_TotalOperationalHours))
    {
        err = StoreTotalOperationalHours(0);
        SuccessOrExit(err);
    }

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<STM32Config>::Init();
    SuccessOrExit(err);

    //Initialize the global GroupKeyStore object here (#1266)

    err = CHIP_NO_ERROR;

exit:
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::GetRebootCount(uint32_t & rebootCount)
{
    return ReadConfigValue(STM32Config::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::StoreRebootCount(uint32_t rebootCount)
{
    return WriteConfigValue(STM32Config::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    return ReadConfigValue(STM32Config::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::StoreTotalOperationalHours(uint32_t totalOperationalHours)
{
    return WriteConfigValue(STM32Config::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI

#else
    return CHIP_NO_ERROR;	//return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

bool ConfigurationManagerImpl::CanFactoryReset()
{
    // query the application to determine if factory reset is allowed.
    return true;
}

void ConfigurationManagerImpl::InitiateFactoryReset()
{
    PlatformMgr().ScheduleWork(DoFactoryReset);
}

CHIP_ERROR ConfigurationManagerImpl::GetBootReason(uint32_t & bootReason)
{
    // rebootCause is obtained at bootup.
    BootReasonType matterBootCause;

#if defined(STM32H753xx)

    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWR1RST))
        {
            /* RESET_CAUSE_LOW_POWER_RESET */
            matterBootCause = BootReasonType::kUnspecified;
        }
        else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDG1RST))
        {
            /* RESET_CAUSE_WINDOW_WATCHDOG_RESET */
            matterBootCause = BootReasonType::kSoftwareWatchdogReset;
        }
        else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDG1RST))
        {
            /* RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET */
            matterBootCause = BootReasonType::kSoftwareWatchdogReset;
        }
        else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
        {
            /*
             * This reset is induced by calling the ARM CMSIS
             * `NVIC_SystemReset()` function!
             */
            /*  RESET_CAUSE_SOFTWARE_RESET */
            matterBootCause = BootReasonType::kSoftwareReset;
        }
        else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
        {
            /* RESET_CAUSE_POWER_ON_POWER_DOWN_RESET */
            matterBootCause = BootReasonType::kPowerOnReboot;
        }
        else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
        {
            /* RESET_CAUSE_EXTERNAL_RESET_PIN_RESET */
            matterBootCause = BootReasonType::kPowerOnReboot;
        }
        /*
         * Needs to come *after* checking the `RCC_FLAG_PORRST` flag in order to
         * ensure first that the reset cause is NOT a POR/PDR reset. See note
         * below.
         */
        else if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST))
        {
            /* RESET_CAUSE_BROWNOUT_RESET */
            matterBootCause = BootReasonType::kBrownOutReset;
        }
        else
        {
            /* RESET_CAUSE_UNKNOWN */
            matterBootCause = BootReasonType::kUnspecified;
        }

        // Clear all the reset flags or else they will remain set during future
        // resets until system power is fully removed.
        __HAL_RCC_CLEAR_RESET_FLAGS();

#else
    matterBootCause = BootReasonType::kUnspecified;
#endif

    bootReason = to_underlying(matterBootCause);

    ChipLogProgress(DeviceLayer, "The system reset cause is \"%d\"\n", bootReason);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigurationManagerImpl::ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    STM32Config::Key configKey{ STM32Config::kConfigNamespace_ChipCounters, key };

    CHIP_ERROR err = ReadConfigValue(configKey, value);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    STM32Config::Key configKey{ STM32Config::kConfigNamespace_ChipCounters, key };
    return WriteConfigValue(configKey, value);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool & val)
{
    return STM32Config::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t & val)
{
    return STM32Config::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t & val)
{
    return STM32Config::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return STM32Config::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return STM32Config::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val)
{
    return STM32Config::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val)
{
    return STM32Config::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val)
{
    return STM32Config::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str)
{
    return STM32Config::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return STM32Config::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return STM32Config::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest(void)
{
    STM32Config::RunConfigUnitTest();
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    CHIP_ERROR err;

    ChipLogProgress(DeviceLayer, "Performing factory reset");

    // Erase all values in the chip-config NVS namespace.
    err = STM32Config::ClearNamespace(STM32Config::kConfigNamespace_ChipConfig);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ClearNamespace(ChipConfig) failed: %s", chip::ErrorStr(err));
    }

    // Restore WiFi persistent settings to default values.
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    //have to add wifi setting restore paramaters impl..
#elif CHIP_DEVICE_CONFIG_ENABLE_THREAD
    //have to add Thread setting restore paramaters impl..
#endif

    // Restart the system.
    ChipLogProgress(DeviceLayer, "System restarting");
	
	// NOT IMPLEMENTED 

}

ConfigurationManager & ConfigurationMgrImpl()
{
    return ConfigurationManagerImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
