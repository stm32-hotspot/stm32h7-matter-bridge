

/**
 *    @file
 *          Utilities for interacting with the the STM32 "NVS" key-value store.
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/KeyValueStoreManager.h>
#include <platform/STM32/H7/STM32Config.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// NVS namespaces used to store device configuration information.
const char STM32Config::kConfigNamespace_ChipFactory[]  = "chip-factory";
const char STM32Config::kConfigNamespace_ChipConfig[]   = "chip-config";
const char STM32Config::kConfigNamespace_ChipCounters[] = "chip-counters";

// Keys stored in the chip-factory namespace
const STM32Config::Key STM32Config::kConfigKey_SerialNum             = { kConfigNamespace_ChipFactory, "serial-num" };
const STM32Config::Key STM32Config::kConfigKey_MfrDeviceId           = { kConfigNamespace_ChipFactory, "device-id" };
const STM32Config::Key STM32Config::kConfigKey_MfrDeviceCert         = { kConfigNamespace_ChipFactory, "device-cert" };
const STM32Config::Key STM32Config::kConfigKey_MfrDeviceICACerts     = { kConfigNamespace_ChipFactory, "device-ca-certs" };
const STM32Config::Key STM32Config::kConfigKey_MfrDevicePrivateKey   = { kConfigNamespace_ChipFactory, "device-key" };
const STM32Config::Key STM32Config::kConfigKey_HardwareVersion       = { kConfigNamespace_ChipFactory, "hardware-ver" };
const STM32Config::Key STM32Config::kConfigKey_HardwareVersionString = { kConfigNamespace_ChipFactory, "hw-ver-str" };
const STM32Config::Key STM32Config::kConfigKey_ManufacturingDate     = { kConfigNamespace_ChipFactory, "mfg-date" };
const STM32Config::Key STM32Config::kConfigKey_SetupPinCode          = { kConfigNamespace_ChipFactory, "pin-code" };
const STM32Config::Key STM32Config::kConfigKey_SetupDiscriminator    = { kConfigNamespace_ChipFactory, "discriminator" };
const STM32Config::Key STM32Config::kConfigKey_Spake2pIterationCount = { kConfigNamespace_ChipFactory, "iteration-count" };
const STM32Config::Key STM32Config::kConfigKey_Spake2pSalt           = { kConfigNamespace_ChipFactory, "salt" };
const STM32Config::Key STM32Config::kConfigKey_Spake2pVerifier       = { kConfigNamespace_ChipFactory, "verifier" };
const STM32Config::Key STM32Config::kConfigKey_DACCert               = { kConfigNamespace_ChipFactory, "dac-cert" };
const STM32Config::Key STM32Config::kConfigKey_DACPrivateKey         = { kConfigNamespace_ChipFactory, "dac-key" };
const STM32Config::Key STM32Config::kConfigKey_DACPublicKey          = { kConfigNamespace_ChipFactory, "dac-pub-key" };
const STM32Config::Key STM32Config::kConfigKey_PAICert               = { kConfigNamespace_ChipFactory, "pai-cert" };
const STM32Config::Key STM32Config::kConfigKey_CertDeclaration       = { kConfigNamespace_ChipFactory, "cert-dclrn" };
const STM32Config::Key STM32Config::kConfigKey_VendorId              = { kConfigNamespace_ChipFactory, "vendor-id" };
const STM32Config::Key STM32Config::kConfigKey_VendorName            = { kConfigNamespace_ChipFactory, "vendor-name" };
const STM32Config::Key STM32Config::kConfigKey_ProductId             = { kConfigNamespace_ChipFactory, "product-id" };
const STM32Config::Key STM32Config::kConfigKey_ProductName           = { kConfigNamespace_ChipFactory, "product-name" };
const STM32Config::Key STM32Config::kConfigKey_UniqueId              = { kConfigNamespace_ChipFactory, "unique-id" };
const STM32Config::Key STM32Config::kConfigKey_SupportedCalTypes     = { kConfigNamespace_ChipFactory, "cal-types" };
const STM32Config::Key STM32Config::kConfigKey_SupportedLocaleSize   = { kConfigNamespace_ChipFactory, "locale-sz" };

// Keys stored in the chip-config namespace
const STM32Config::Key STM32Config::kConfigKey_ServiceConfig      = { kConfigNamespace_ChipConfig, "service-config" };
const STM32Config::Key STM32Config::kConfigKey_PairedAccountId    = { kConfigNamespace_ChipConfig, "account-id" };
const STM32Config::Key STM32Config::kConfigKey_ServiceId          = { kConfigNamespace_ChipConfig, "service-id" };
const STM32Config::Key STM32Config::kConfigKey_LastUsedEpochKeyId = { kConfigNamespace_ChipConfig, "last-ek-id" };
const STM32Config::Key STM32Config::kConfigKey_FailSafeArmed      = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const STM32Config::Key STM32Config::kConfigKey_WiFiStationSecType = { kConfigNamespace_ChipConfig, "sta-sec-type" };
const STM32Config::Key STM32Config::kConfigKey_RegulatoryLocation = { kConfigNamespace_ChipConfig, "reg-location" };
const STM32Config::Key STM32Config::kConfigKey_CountryCode        = { kConfigNamespace_ChipConfig, "country-code" };

// Keys stored in the Chip-counters namespace
const STM32Config::Key STM32Config::kCounterKey_RebootCount           = { kConfigNamespace_ChipCounters, "reboot-count" };
const STM32Config::Key STM32Config::kCounterKey_UpTime                = { kConfigNamespace_ChipCounters, "up-time" };
const STM32Config::Key STM32Config::kCounterKey_TotalOperationalHours = { kConfigNamespace_ChipCounters, "total-hours" };

const char * STM32Config::GetPartitionLabelByNamespace(const char * ns)
{
    if (strcmp(ns, kConfigNamespace_ChipFactory) == 0)
    {
        return CHIP_DEVICE_CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION;
    }
    else if (strcmp(ns, kConfigNamespace_ChipConfig) == 0)
    {
        return CHIP_DEVICE_CONFIG_CHIP_CONFIG_NAMESPACE_PARTITION;
    }
    else if (strcmp(ns, kConfigNamespace_ChipCounters))
    {
        return CHIP_DEVICE_CONFIG_CHIP_COUNTERS_NAMESPACE_PARTITION;
    }

    return "nvs";
}

CHIP_ERROR STM32Config::ReadConfigValue(Key key, bool & val)
{
    bool in;
    char key_str[STM32STORE_MAX_KEY_SIZE*2] = { 0 };
    key.to_str(key_str, STM32STORE_MAX_KEY_SIZE*2);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, static_cast<void *>(&in), sizeof(bool));
    val            = in;
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR STM32Config::ReadConfigValue(Key key, uint32_t & val)
{
    uint32_t in;
    char key_str[STM32STORE_MAX_KEY_SIZE*2] = { 0 };
    key.to_str(key_str, STM32STORE_MAX_KEY_SIZE*2);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, static_cast<void *>(&in), 4);
    val            = in;
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR STM32Config::ReadConfigValue(Key key, uint64_t & val)
{
    uint64_t in;
    char key_str[STM32STORE_MAX_KEY_SIZE*2] = { 0 };
    key.to_str(key_str, STM32STORE_MAX_KEY_SIZE*2);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, static_cast<void *>(&in), 8);
    val            = in;
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR STM32Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    char key_str[STM32STORE_MAX_KEY_SIZE*2] = { 0 };
    key.to_str(key_str, STM32STORE_MAX_KEY_SIZE*2);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, buf, bufSize, &outLen);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR STM32Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    char key_str[STM32STORE_MAX_KEY_SIZE*2] = { 0 };
    key.to_str(key_str, STM32STORE_MAX_KEY_SIZE*2);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, buf, bufSize, &outLen);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR STM32Config::WriteConfigValue(Key key, bool val)
{
    char key_str[STM32STORE_MAX_KEY_SIZE*2] = { 0 };
    key.to_str(key_str, STM32STORE_MAX_KEY_SIZE*2);

    ChipLogProgress(DeviceLayer, "STM32 set: %s/%s = %s", key.Namespace, key.Name, val ? "true" : "false");

    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&val), sizeof(bool));
}

CHIP_ERROR STM32Config::WriteConfigValue(Key key, uint32_t val)
{
    char key_str[STM32STORE_MAX_KEY_SIZE*2] = { 0 };
    key.to_str(key_str, STM32STORE_MAX_KEY_SIZE*2);

    ChipLogProgress(DeviceLayer, "STM32 set: %s/%s = %" PRIu32 " (0x%" PRIX32 ")", key.Namespace, key.Name, val, val);

    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&val), 4);
}

CHIP_ERROR STM32Config::WriteConfigValue(Key key, uint64_t val)
{
    char key_str[STM32STORE_MAX_KEY_SIZE*2] = { 0 };
    key.to_str(key_str, STM32STORE_MAX_KEY_SIZE*2);

    ChipLogProgress(DeviceLayer, "STM32 set: %s/%s = %" PRIu64 " (0x%" PRIX64 ")", key.Namespace, key.Name, val, val);

    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&val), 8);

}

CHIP_ERROR STM32Config::WriteConfigValueStr(Key key, const char * str)
{
    size_t size                            = strlen(str) + 1;
    char key_str[STM32STORE_MAX_KEY_SIZE*2] = { 0 };
    key.to_str(key_str, STM32STORE_MAX_KEY_SIZE*2);

    ChipLogProgress(DeviceLayer, "STM32 set: %s/%s = \"%s\"", key.Namespace, key.Name, str);

    return PersistedStorage::KeyValueStoreMgr().Put(key_str, str, size);

}

CHIP_ERROR STM32Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    char key_str[STM32STORE_MAX_KEY_SIZE*2] = { 0 };
    key.to_str(key_str, STM32STORE_MAX_KEY_SIZE*2);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, str, strLen);
}
CHIP_ERROR STM32Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    char key_str[STM32STORE_MAX_KEY_SIZE*2] = { 0 };
    key.to_str(key_str, STM32STORE_MAX_KEY_SIZE*2);

    ChipLogProgress(DeviceLayer, "STM32 set: %s/%s = (blob length %" PRId32 ")", key.Namespace, key.Name, dataLen);

    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&data), dataLen);

}

CHIP_ERROR STM32Config::ClearConfigValue(Key key)
{
    char key_str[STM32STORE_MAX_KEY_SIZE*2] = { 0 };
    key.to_str(key_str, STM32STORE_MAX_KEY_SIZE*2);
    return PersistedStorage::KeyValueStoreMgr().Delete(key_str);
}

bool STM32Config::ConfigValueExists(Key key)
{
    char key_str[STM32STORE_MAX_KEY_SIZE*2] = { 0 };
    key.to_str(key_str, STM32STORE_MAX_KEY_SIZE*2);
    if (PersistedStorage::KeyValueStoreMgr().Get(key_str, NULL, 0) == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return false;
    }

    return true;
}

CHIP_ERROR STM32Config::EnsureNamespace(const char * ns)
{
	return CHIP_NO_ERROR;
}

CHIP_ERROR STM32Config::ClearNamespace(const char * ns)
{
	return CHIP_NO_ERROR;
}

void STM32Config::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

