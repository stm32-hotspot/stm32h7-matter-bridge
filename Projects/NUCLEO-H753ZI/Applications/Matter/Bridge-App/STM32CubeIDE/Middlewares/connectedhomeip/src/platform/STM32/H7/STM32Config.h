/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          Utilities for interacting with the the STM32 key-value store.
 */

#pragma once

#include "STM32CHIPConfig.h"
#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides functions and definitions for accessing device configuration information on the STM32.
 *
 * This class is designed to be mixed-in to concrete implementation classes as a means to
 * provide access to configuration information to generic base classes.
 */
class STM32Config
{
public:
    struct Key;

    // Maximum length of an Non-Volatile Storage key name.
    static constexpr size_t kMaxConfigKeyNameLength = STM32STORE_MAX_KEY_SIZE;

    // NVS namespaces used to store device configuration information.
	static const char kConfigNamespace_ChipFactory[];
	static const char kConfigNamespace_ChipConfig[];
	static const char kConfigNamespace_ChipCounters[];

	// Key definitions for well-known keys.
	static const Key kConfigKey_SerialNum;
	static const Key kConfigKey_MfrDeviceId;
	static const Key kConfigKey_MfrDeviceCert;
	static const Key kConfigKey_MfrDeviceICACerts;
	static const Key kConfigKey_MfrDevicePrivateKey;
	static const Key kConfigKey_HardwareVersion;
	static const Key kConfigKey_HardwareVersionString;
	static const Key kConfigKey_ManufacturingDate;
	static const Key kConfigKey_SetupPinCode;
	static const Key kConfigKey_SetupDiscriminator;
	static const Key kConfigKey_Spake2pIterationCount;
	static const Key kConfigKey_Spake2pSalt;
	static const Key kConfigKey_Spake2pVerifier;
	static const Key kConfigKey_DACCert;
	static const Key kConfigKey_DACPrivateKey;
	static const Key kConfigKey_DACPublicKey;
	static const Key kConfigKey_PAICert;
	static const Key kConfigKey_CertDeclaration;
	static const Key kConfigKey_VendorId;
	static const Key kConfigKey_VendorName;
	static const Key kConfigKey_ProductId;
	static const Key kConfigKey_ProductName;
	static const Key kConfigKey_ProductLabel;
	static const Key kConfigKey_ProductURL;
	static const Key kConfigKey_SupportedCalTypes;
	static const Key kConfigKey_SupportedLocaleSize;
	static const Key kConfigKey_RotatingDevIdUniqueId;

	// CHIP Config keys
	static const Key kConfigKey_ServiceConfig;
	static const Key kConfigKey_PairedAccountId;
	static const Key kConfigKey_ServiceId;
	static const Key kConfigKey_LastUsedEpochKeyId;
	static const Key kConfigKey_FailSafeArmed;
	static const Key kConfigKey_WiFiStationSecType;
	static const Key kConfigKey_RegulatoryLocation;
	static const Key kConfigKey_CountryCode;
	static const Key kConfigKey_UniqueId;

	// CHIP Counter keys
	static const Key kCounterKey_RebootCount;
	static const Key kCounterKey_UpTime;
	static const Key kCounterKey_TotalOperationalHours;

	// Config value accessors.
	static CHIP_ERROR ReadConfigValue(Key key, bool & val);
	static CHIP_ERROR ReadConfigValue(Key key, uint32_t & val);
	static CHIP_ERROR ReadConfigValue(Key key, uint64_t & val);

	// If buf is NULL then outLen is set to the required length to fit the string/blob
	static CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
	static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);

	static CHIP_ERROR WriteConfigValue(Key key, bool val);
	static CHIP_ERROR WriteConfigValue(Key key, uint32_t val);
	static CHIP_ERROR WriteConfigValue(Key key, uint64_t val);
	static CHIP_ERROR WriteConfigValueStr(Key key, const char * str);
	static CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
	static CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
	static CHIP_ERROR ClearConfigValue(Key key);
	static bool ConfigValueExists(Key key);

	// NVS Namespace helper functions.
	static CHIP_ERROR EnsureNamespace(const char * ns);
	static CHIP_ERROR ClearNamespace(const char * ns);

	static void RunConfigUnitTest(void);

private:
    static const char * GetPartitionLabelByNamespace(const char * ns);
};

struct STM32Config::Key
{
    const char * Namespace;
    const char * Name;

    CHIP_ERROR to_str(char * buf, size_t buf_size) const;
    size_t len() const;

    bool operator==(const Key & other) const;

    template <typename T, typename std::enable_if_t<std::is_convertible<T, const char *>::value, int> = 0>
    Key(const char * aNamespace, T aName) : Namespace(aNamespace), Name(aName)
    {}

    template <size_t N>
    Key(const char * aNamespace, const char (&aName)[N]) : Namespace(aNamespace), Name(aName)
    {
        // Note: N includes null-terminator.
        static_assert(N <= STM32Config::kMaxConfigKeyNameLength + 1, "Key too long");
    }
};


inline CHIP_ERROR STM32Config::Key::to_str(char * buf, size_t buf_size) const
{
    if (buf_size < len() + 1)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    strcpy(buf, Namespace);
    strcat(buf, ";");
    strcat(buf, Name);

    return CHIP_NO_ERROR;
}

// Length of key str (not including terminating null char)
inline size_t STM32Config::Key::len() const
{
    // + 1 for separating ';'
    size_t out_size = strlen(Namespace) + strlen(Name) + 1;
    return out_size;
}



inline bool STM32Config::Key::operator==(const Key & other) const
{
    return strcmp(Namespace, other.Namespace) == 0 && strcmp(Name, other.Name) == 0;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
