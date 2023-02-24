/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          Provides an implementation of the PlatformManager object
 *          for the STM32 platform.
 */

 /* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <app-common/zap-generated/enums.h>
#include <crypto/CHIPCryptoPAL.h>
#include <platform/STM32/H7/DiagnosticDataProviderImpl.h>
#include <platform/STM32/H7/STM32Utils.h>
#include <platform/STM32/H7/SystemTimeSupport.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>
 
namespace chip {
namespace DeviceLayer {

namespace Internal {
extern CHIP_ERROR InitLwIPCoreLock(void);
}
 
PlatformManagerImpl PlatformManagerImpl::sInstance;

static int app_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{
 /* ADD ENTROPY FUNCTION FOR STM32 */
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
	CHIP_ERROR err;

	// Initialize the configuration system.
	//err = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();
	SuccessOrExit(err);

	SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());
	SetDiagnosticDataProvider(&DiagnosticDataProviderImpl::GetDefaultInstance());

	// Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
	ReturnErrorOnFailure(Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack());
	
exit:
	return CHIP_NO_ERROR;
}

void PlatformManagerImpl::_Shutdown()
{
	uint64_t upTime = 0;

    if (GetDiagnosticDataProvider().GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalOperationalHours = 0;

        if (ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours) == CHIP_NO_ERROR)
        {
            ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours + static_cast<uint32_t>(upTime / 3600));
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get current uptime since the Node’s last reboot");
    }
	
 Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_Shutdown();
}

} // namespace DeviceLayer
} // namespace chip
