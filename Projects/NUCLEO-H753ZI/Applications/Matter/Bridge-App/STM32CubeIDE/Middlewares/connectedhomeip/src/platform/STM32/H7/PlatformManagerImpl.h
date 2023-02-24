/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          Provides an implementation of the PlatformManager object
 *          for the STM32 platform.
 */

#pragma once

#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.h>
#include <system/SystemClock.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for the STM32 platform.
 */
class PlatformManagerImpl final : public PlatformManager, public Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>
{
    // Allow the PlatformManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend PlatformManager;

    // Allow the generic implementation base class to call helper methods on
    // this class.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>;
#endif

public:
    // ===== Platform-specific members that may be accessed directly by the application.

    CHIP_ERROR InitLwIPCoreLock(void);
   // static void HandleESPSystemEvent(void * arg, esp_event_base_t eventBase, int32_t eventId, void * eventData);
    System::Clock::Timestamp GetStartTime() { return mStartTime; }

private:
    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR _InitChipStack(void);
    void _Shutdown();
	
	// ===== Members for internal use by the following friends.
    friend PlatformManager & PlatformMgr(void);
    friend PlatformManagerImpl & PlatformMgrImpl(void);

    System::Clock::Timestamp mStartTime = System::Clock::kZero;

    static PlatformManagerImpl sInstance;
};

/**
 * Returns the public interface of the PlatformManager singleton object.
 *
 * Chip applications should use this to access features of the PlatformManager object
 * that are common to all platforms.
 */
inline PlatformManager & PlatformMgr(void)
{
    return PlatformManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the PlatformManager singleton object.
 *
 * Chip applications can use this to gain access to features of the PlatformManager
 * that are specific to the STM32 platform.
 */
inline PlatformManagerImpl & PlatformMgrImpl(void)
{
    return PlatformManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
