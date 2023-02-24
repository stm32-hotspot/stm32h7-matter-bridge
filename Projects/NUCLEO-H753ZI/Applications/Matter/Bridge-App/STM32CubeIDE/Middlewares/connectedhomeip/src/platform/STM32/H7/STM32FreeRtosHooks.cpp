/* See Project CHIP LICENSE file for licensing information. */

#include "FreeRTOS.h"
#include "STM32FreeRtosHooks.h"
#include "mbedtls/memory_buffer_alloc.h"

#if !defined( MBEDTLS_CONFIG_FILE )
    #include "mbedtls/config.h"
#else
    #include MBEDTLS_CONFIG_FILE
#endif

#include "mbedtls/platform.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha1.h"
#include "mbedtls/pk.h"
#include "mbedtls/x509_crt.h"

#include "mbedtls/threading.h"
#include "threading_alt.h"

#include <string.h>

static uint8_t mbedtls_heap[15000];
static void freertos_mbedtls_heap_init(void);

static inline void mutex_init(mbedtls_threading_mutex_t * mutex)
{
    mutex->mutex = xSemaphoreCreateMutex();

    if( mutex->mutex != NULL )
    {
        mutex->is_valid = 1;
    }
    else
    {
        mutex->is_valid = 0;
        //vLoggingPrintf( "mbedTLS mutex initialization failed !\r\n" );
    }
}

static inline void mutex_free(mbedtls_threading_mutex_t * mutex)
{
    if( mutex->is_valid == 1 )
    {
        vSemaphoreDelete( mutex->mutex );
        mutex->is_valid = 0;
    }
}

static inline int mutex_lock(mbedtls_threading_mutex_t * mutex)
{
    int ret = MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;

    if( mutex->is_valid == 1 )
    {
        if( xSemaphoreTake( mutex->mutex, portMAX_DELAY ) )
        {
            ret = 0;
        }
        else
        {
            ret = MBEDTLS_ERR_THREADING_MUTEX_ERROR;
            //vLoggingPrintf( "Failed to obtain mutex from Mbedtls !\r\n" );
        }
    }

    return ret;
}

static inline int mutex_unlock(mbedtls_threading_mutex_t * mutex)
{
	int ret = MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;

    if( mutex->is_valid == 1 )
    {
        if( xSemaphoreGive( mutex->mutex ) )
        {
            ret = 0;
        }
        else
        {
            ret = MBEDTLS_ERR_THREADING_MUTEX_ERROR;
            //vLoggingPrintf( "Failed to unlock mutex from Mbedtls !\r\n");
        }
    }

    return ret;
}

void freertos_mbedtls_mutex_init( void )
{
    // Configure mbedtls to use mutexes from FreeRTOS
    mbedtls_threading_set_alt( mutex_init,
                               mutex_free,
                               mutex_lock,
                               mutex_unlock );	//if def MBEDTLS_THREADING_C
}

static void freertos_mbedtls_heap_init(void)
{
	// mbedtls_memory_buffer_alloc_init(mbedtls_heap,sizeof(mbedtls_heap));	//Comment if def MBEDTLS_THREADING_C
}

void freertos_mbedtls_init( void )
{
    freertos_mbedtls_mutex_init();
    freertos_mbedtls_heap_init();
}

