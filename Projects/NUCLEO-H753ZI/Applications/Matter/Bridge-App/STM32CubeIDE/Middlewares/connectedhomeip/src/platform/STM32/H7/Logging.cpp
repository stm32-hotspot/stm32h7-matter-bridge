/* See Project CHIP LICENSE file for licensing information. */
#include <platform/logging/LogV.h>

#include <lib/core/CHIPConfig.h>
#include <platform/CHIPDeviceConfig.h>

#include <lib/support/SafeString.h>
#include <lib/support/logging/CHIPLogging.h>

// FreeRTOS libraries
#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <queue.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <task.h>

#define STM32_LOG_ENABLED 		(1)
#define LOG_COLOR_ENABLED 		(1)
#define HARD_FAULT_LOG_ENABLE 	(0)

#define LOG_ERROR "<error > "
#define LOG_WARN "<warn  > "
#define LOG_INFO "<info  > "
#define LOG_DETAIL "<detail> "
#define LOG_STM32 "<stm32 > "
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#define LOG_LWIP "<lwip  > "
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

// If a new category string "LOG_.." is created, add it in the MaxStringLength arguments below
#if CHIP_SYSTEM_CONFIG_USE_LWIP
static constexpr size_t kMaxCategoryStrLen = chip::MaxStringLength(LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DETAIL, LOG_LWIP, LOG_STM32);
#else
static constexpr size_t kMaxCategoryStrLen = chip::MaxStringLength(LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DETAIL, LOG_STM32);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

/* ANSI escape codes (bright edition) */
#define LOG_COLOR_CODE_DEFAULT "\x1b[0m"
#define LOG_COLOR_CODE_RED     "\x1b[0;91m"
#define LOG_COLOR_CODE_GREEN   "\x1b[0;92m"
#define LOG_COLOR_CODE_YELLOW  "\x1b[0;93m"
#define LOG_COLOR_CODE_CYAN    "\x1b[0;96m"
#define LOG_COLOR_CODE_VIOLET  "\x1b[0;94m"
#define LOG_COLOR_CODE_PURPLE  "\x1b[0;95m"
#define LOG_COLOR_CODE_GREY    "\x1b[0;90m"
#define LOG_COLOR_CODE_WHITE   "\x1b[0;89m"
#define LOG_COLOR_CODE_BLACK   "\x1b[0;90m"

#if LOG_COLOR_ENABLED
#define LOG_COLOR_ERROR		LOG_COLOR_CODE_RED
#define LOG_COLOR_WARN		LOG_COLOR_CODE_YELLOW
#define LOG_COLOR_INFO		LOG_COLOR_CODE_GREEN
#define LOG_COLOR_DETAIL	LOG_COLOR_CODE_YELLOW
#define LOG_COLOR_STM32		LOG_COLOR_CODE_VIOLET
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#define LOG_COLOR_LWIP		LOG_COLOR_CODE_WHITE
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
#endif // LOG_COLOR_ENABLED

#if LOG_COLOR_ENABLED
#if CHIP_SYSTEM_CONFIG_USE_LWIP
static constexpr size_t kMaxColorStrLen = chip::MaxStringLength(LOG_COLOR_ERROR, LOG_COLOR_WARN, LOG_COLOR_INFO, LOG_COLOR_DETAIL, LOG_COLOR_LWIP, LOG_COLOR_STM32);
#else
static constexpr size_t kMaxColorStrLen = chip::MaxStringLength(LOG_COLOR_ERROR, LOG_COLOR_WARN, LOG_COLOR_INFO, LOG_COLOR_DETAIL, LOG_COLOR_STM32);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
#else
static constexpr size_t kMaxColorStrLen = NULL;
#endif // LOG_COLOR_ENABLED

//#if STM32_LOG_ENABLED
//static bool sLogInitialized = false;
//#endif // STM32_LOG_ENABLED

/**
 * Print a log message to console using vMainUARTPrintString
 */
#if STM32_LOG_ENABLED
//static void PrintLog(const char * msg, ...)
static void PrintLog(const char * msg)
{
	// (add printf implementation on main.c)

	size_t sz;
	sz = strlen(msg);

	char logString[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE + 3U];

	strcpy(logString, msg);

    logString[sz++] = '\r';
    logString[sz++] = '\n';
    logString[sz++] = 0;

	vMainUARTPrintString( logString );

//	va_list v;
//	va_start(v, msg);
//	vprintf(msg, v);
//	va_end(v);
}
#endif // STM32_LOG_ENABLED

/**
 * STM32 General-purpose logging function
 */
extern "C" void stm32Log(const char * aFormat, ...)
{
    va_list v;

    va_start(v, aFormat);

#if STM32_LOG_ENABLED
    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    size_t formattedMsgLen;
    static_assert(sizeof(formattedMsg) > kMaxCategoryStrLen); // Greater than to at least accommodate a ending Null Character

#if LOG_COLOR_ENABLED
        	// Form the log color
        	strcpy(formattedMsg, LOG_COLOR_STM32);
        	// Form the log category
        	formattedMsgLen = strlen(formattedMsg);
        	snprintf(formattedMsg + formattedMsgLen, sizeof(formattedMsg) - formattedMsgLen, LOG_STM32);
#else
    strcpy(formattedMsg, LOG_STM32);
#endif // LOG_COLOR_ENABLED
    size_t prefixLen = strlen(formattedMsg);
    size_t len       = vsnprintf(formattedMsg + prefixLen, sizeof formattedMsg - prefixLen, aFormat, v);

    if (len >= sizeof formattedMsg - prefixLen)
    {
        formattedMsg[sizeof formattedMsg - 1] = '\0';
    }

    PrintLog(formattedMsg);
#endif // STM32_LOG_ENABLED

    va_end(v);
}

namespace chip {
namespace DeviceLayer {

/**
 * Called whenever a log message is emitted by Chip or LwIP.
 *
 * This function is intended be overridden by the application to, e.g.,
 * schedule output of queued log entries.
 */
void __attribute__((weak)) OnLogOutput(void) {}

} // namespace DeviceLayer
} // namespace chip

namespace chip {
namespace Logging {
namespace Platform {

/**
 * CHIP log output functions.
 */
void LogV(const char * module, uint8_t category, const char * aFormat, va_list v)
{
#if STM32_LOG_ENABLED && _CHIP_USE_LOGGING
    if (IsCategoryEnabled(category))
    {
        char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
        size_t formattedMsgLen;

#if LOG_COLOR_ENABLED
        // len for Color string + Category string + "[" + Module name + "] " (Brackets and space =3)
        constexpr size_t maxPrefixLen = kMaxColorStrLen + kMaxCategoryStrLen + chip::Logging::kMaxModuleNameLen + 3;
#else
        // len for Category string + "[" + Module name + "] " (Brackets and space =3)
        constexpr size_t maxPrefixLen = kMaxCategoryStrLen + chip::Logging::kMaxModuleNameLen + 3;
#endif
        static_assert(sizeof(formattedMsg) > maxPrefixLen); // Greater than to at least accommodate a ending Null Character

        switch (category)
        {
        case kLogCategory_Error:
#if LOG_COLOR_ENABLED
        	// Form the log color
        	strcpy(formattedMsg, LOG_COLOR_ERROR);
        	// Form the log category
        	formattedMsgLen = strlen(formattedMsg);
        	snprintf(formattedMsg + formattedMsgLen, sizeof(formattedMsg) - formattedMsgLen, LOG_ERROR);
#else
            strcpy(formattedMsg, LOG_ERROR);
#endif
            break;
        case kLogCategory_Progress:
        default:
#if LOG_COLOR_ENABLED
        	// Form the log color
        	strcpy(formattedMsg, LOG_COLOR_INFO);
        	// Form the log category
			formattedMsgLen = strlen(formattedMsg);
			snprintf(formattedMsg + formattedMsgLen, sizeof(formattedMsg) - formattedMsgLen, LOG_INFO);

#else
            strcpy(formattedMsg, LOG_INFO);
#endif
            break;
        case kLogCategory_Detail:
#if LOG_COLOR_ENABLED
        	// Form the log color
			strcpy(formattedMsg, LOG_COLOR_DETAIL);
			// Form the log category
			formattedMsgLen = strlen(formattedMsg);
			snprintf(formattedMsg + formattedMsgLen, sizeof(formattedMsg) - formattedMsgLen, LOG_DETAIL);

#else
            strcpy(formattedMsg, LOG_DETAIL);
#endif
            break;
        }

        formattedMsgLen = strlen(formattedMsg);

        // Form the log prefix, e.g. "[DL] "
        snprintf(formattedMsg + formattedMsgLen, sizeof(formattedMsg) - formattedMsgLen, "[%s] ", module);
        formattedMsg[sizeof(formattedMsg) - 1] = 0;
        formattedMsgLen                        = strlen(formattedMsg);

        size_t len = vsnprintf(formattedMsg + formattedMsgLen, sizeof formattedMsg - formattedMsgLen, aFormat, v);

        if (len >= sizeof formattedMsg - formattedMsgLen)
        {
            formattedMsg[sizeof formattedMsg - 1] = '\0';
        }

        PrintLog(formattedMsg);
    }

    // Let the application know that a log message has been emitted.
    chip::DeviceLayer::OnLogOutput();
#endif // STM32_LOG_ENABLED && _CHIP_USE_LOGGING
}

} // namespace Platform
} // namespace Logging
} // namespace chip

//#if CHIP_SYSTEM_CONFIG_USE_LWIP
//
///**
// * LwIP log output function.
// */
//extern "C" void LwIPLog(const char * aFormat, ...)
//{
//    va_list v;
//
//    va_start(v, aFormat);
//#if STM32_LOG_ENABLED
//    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
//	  size_t formattedMsgLen;
//
//#if LOG_COLOR_ENABLED
//        	// Form the log color
//        	strcpy(formattedMsg, LOG_COLOR_LWIP);
//        	// Form the log category
//        	formattedMsgLen = strlen(formattedMsg);
//        	snprintf(formattedMsg + formattedMsgLen, sizeof(formattedMsg) - formattedMsgLen, LOG_LWIP);
//#else
//    strcpy(formattedMsg, LOG_LWIP);
//#endif // LOG_COLOR_ENABLED
//    size_t prefixLen = strlen(formattedMsg);
//    size_t len       = vsnprintf(formattedMsg + prefixLen, sizeof formattedMsg - prefixLen, aFormat, v);
//
//    if (len >= sizeof formattedMsg - prefixLen)
//    {
//        formattedMsg[sizeof formattedMsg - 1] = '\0';
//    }
//
//    PrintLog(formattedMsg);
//
//#if configCHECK_FOR_STACK_OVERFLOW
//    // Force a stack overflow check.
//    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
//        taskYIELD();
//#endif
//
//    // Let the application know that a log message has been emitted.
//    chip::DeviceLayer::OnLogOutput();
//#endif // STM32_LOG_ENABLED
//    va_end(v);
//}
//#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if HARD_FAULT_LOG_ENABLE && STM32_LOG_ENABLED

enum { r0, r1, r2, r3, r12, lr, pc, psr};

/**
 * Log register contents to UART when a hard fault occurs.
 */
extern "C" void debugHardfault(uint32_t * stack)
{
    char formattedMsg[30];

//    if (sLogInitialized == false)
//        {
//            stm32InitLog();
//        }

    snprintf(formattedMsg, sizeof formattedMsg, LOG_COLOR_ERROR LOG_ERROR "HardFault:");		PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, LOG_ERROR "r0  = 0x%08x", stack[r0]);			PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, LOG_ERROR "r1  = 0x%08x", stack[r1]);			PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, LOG_ERROR "r2  = 0x%08x", stack[r2]);			PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, LOG_ERROR "r3  = 0x%08x", stack[r3]);			PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, LOG_ERROR "r12 = 0x%08x", stack[r12]);			PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, LOG_ERROR "lr  = 0x%08x", stack[lr]);			PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, LOG_ERROR "pc  = 0x%08x", stack[pc]);			PrintLog(formattedMsg);
    snprintf(formattedMsg, sizeof formattedMsg, LOG_ERROR "psr = 0x%08x", stack[psr]);			PrintLog(formattedMsg);

    __asm("BKPT #0\n") ; // Break into the debugger
}

/**
 * Override default hard-fault handler
 */
extern "C" __attribute__((naked)) void HardFault_Handler(void)
{
    __asm volatile("tst lr, #4                                    \n" // Test for msp or psp
                   "ite eq                                        \n"
                   "mrseq r0, msp                                 \n"
                   "mrsne r0, psp                                 \n"
                   "ldr r1, debugHardfault_address                \n"
                   "bx r1                                         \n"
                   "debugHardfault_address: .word debugHardfault  \n");
}

#endif // HARD_FAULT_LOG_ENABLE && STM32_LOG_ENABLED
