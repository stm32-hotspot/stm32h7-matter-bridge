/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          Platform-specific key value storage implementation for STM32
 */

#include <platform/KeyValueStoreManager.h>
#include <flash.h>

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

/** Singleton instance of the KeyValueStoreManager implementation object.
 */
KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset)
{

	void* ValuePt = NULL;

	uint8_t n = 0;

	n = memcmp ( key, "g/s/", sizeof("g/s/") );
	     if (n == 0){
	    	 ChipLogProgress(NotSpecified," TEST get Key '%s' TOTO ",key);
	    	}

	n = memcmp ( key, "g/fs/c", sizeof("g/fs/c") );
	if (n == 0){
		//	return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
	    // 	ChipLogProgress(NotSpecified, "Get Key '%s' VALUE NOT FOUND !!",key);
		ChipLogProgress(NotSpecified, "TEST get key '%s' --------- ",key);
		}


	n = memcmp ( key, "g/sri", sizeof("g/sri") );
	if (n == 0){
		//	return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
		//	ChipLogProgress(NotSpecified, "Get Key '%s' !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ",key);
		ChipLogProgress(NotSpecified, "TEST get key :: '%s' :: --------- ",key);
		}

	ValuePt = GetKeyValue(value, key, NULL, (uint32_t)value_size, read_bytes_size);
	if(ValuePt != NULL)
	{
		//ChipLogProgress(NotSpecified, "KeyValueStoreManagerImpl.Get : KEY:%s ; VALUE:%d ; VALUE_SIZE:%d, READ_BYTES_SIZE:%d, OFFSET:%d",key,*(uint32_t*)value,value_size,read_bytes_size,offset);
		ChipLogProgress(NotSpecified, "Get Key '%s' with value size %d and read bytes size %d",key,value_size,*read_bytes_size);
		return CHIP_NO_ERROR;
	} else {
		if(value == 0)
		{
			// check if value exist
			ChipLogProgress(NotSpecified, "Get Key '%s' if value exist !",key);
		}

		//ChipLogProgress(NotSpecified, "KeyValueStoreManagerImpl.Get : KEY:%s  ---- NOT FOUND ---",key);
		ChipLogProgress(NotSpecified, "Get Key '%s' --> NOT FOUND",key);
		return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
	}

	//had to change NVS (Non-volatile Storage) to KMS+NVS in uint32_t format.
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
	void* ValuePt = NULL;

	uint8_t n = 0;

	n = memcmp ( key, "g/sri", sizeof("g/sri") );
	if (n == 0){
			ChipLogProgress(NotSpecified, " TEST put KEY :: Key '%s' :: " ,key);
		}

	ValuePt = SetKeyValue((void*)value, (const char*)key, (uint32_t)value_size);
 	if(ValuePt != NULL)
	{
		//ChipLogProgress(NotSpecified, "KeyValueStoreManagerImpl.Put : KEY:%s ; VALUE:%d ; VALUE_SIZE:%d",key,value,value_size);
 		ChipLogProgress(NotSpecified, "Put Key '%s' with value size %d",key,value_size);
		return CHIP_NO_ERROR;
	} else {
		//ChipLogProgress(NotSpecified, "KeyValueStoreManagerImpl.Put : KEY:%s  ---- ERROR --- ",key);
		ChipLogProgress(NotSpecified, "Put Key '%s' --> ERROR !",key);
		return CHIP_ERROR_NOT_IMPLEMENTED;
	}

 }

 CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
//	 void* ValuePt = NULL;
//
//	 ValuePt = DeleteKey((const char*)key);
//	 if(ValuePt != NULL)
//	 	{
//	  		ChipLogProgress(NotSpecified, "Key '%s' deleted",key);
//	 		return CHIP_NO_ERROR;
//	 	} else {
//	 		ChipLogProgress(NotSpecified, "failed to deleting",key);
//	 		return CHIP_ERROR_NOT_IMPLEMENTED;
//	 	}

	 //ChipLogProgress(NotSpecified, "KeyValueStoreManagerImpl.Delete : KEY:%s",key);
	 //ChipLogProgress(NotSpecified, "Deleting the Key '%s'",key);

	 return CHIP_NO_ERROR;
	 //return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR KeyValueStoreManagerImpl::EraseAll(void)
{
	ChipLogProgress(NotSpecified, "EraseAll NVS not Implemented !! ");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
