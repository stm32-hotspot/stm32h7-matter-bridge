/* See Project CHIP LICENSE file for licensing information. */

#pragma once

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

class KeyValueStoreManagerImpl : public KeyValueStoreManager
{
	// Allow the KeyValueStoreManager interface class to delegate method calls to
	// the implementation methods provided by this class.
	friend class KeyValueStoreManager;

public:
    CHIP_ERROR _Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset);

    CHIP_ERROR _Delete(const char * key);

    CHIP_ERROR _Put(const char * key, const void * value, size_t value_size);

    CHIP_ERROR EraseAll(void);
	
private:
	const char * kNamespace = "CHIP_KVS";

    // ===== Members for internal use by the following friends.
    friend KeyValueStoreManager & KeyValueStoreMgr();
    friend KeyValueStoreManagerImpl & KeyValueStoreMgrImpl();

    static KeyValueStoreManagerImpl sInstance;
};

/**
 * Returns the public interface of the KeyValueStoreManager singleton object.
 *
 * Chip applications should use this to access features of the KeyValueStoreManager object
 * that are common to all platforms.
 */
inline KeyValueStoreManager & KeyValueStoreMgr(void)
{
    return KeyValueStoreManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the KeyValueStoreManager singleton object.
 *
 * Chip applications can use this to gain access to features of the KeyValueStoreManager
 * that are specific to the STM32 platform.
 */
inline KeyValueStoreManagerImpl & KeyValueStoreMgrImpl(void)
{
    return KeyValueStoreManagerImpl::sInstance;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
