/**
  ******************************************************************************
  * @file           : main.cpp
  * @author 		: MCD Application Team
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  */

/* USER CODE END Header */
#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include "string.h"   /* Console output */
#include <ctype.h>
#include <inttypes.h>
#include <string.h>

#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/opt.h"
#include "lwipopts.h"
#include "ethernetif.h"
#include "app_ethernet.h"

/* USER CODE END Includes */
#ifdef __cplusplus
}
#endif

/* ------------------------ Includes C++ MATTER ------------------------ */

#include "FreeRTOS.h"
#include "task.h"
#include "portable.h"
//#include "mbedtls_user_config.h"

void print_stacks_highwatermarks( void );

#include "STM32FreeRtosHooks.h"
#include "CHIPDevicePlatformConfig.h"
#include "AppConfig.h"
#include "Device.h"
#include "DeviceCallbacks.h"
#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <common/Stm32AppServer.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/ZclString.h>

#include <app/InteractionModelEngine.h>
#include <app/server/Server.h>
#include <platform/STM32/H7/NetworkCommissioningDriver.h>

#include <app-common/zap-generated/callback.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <app/server/OnboardingCodesUtil.h>

/* ------------------------ Includes C++ MATTER ------------------------ */

#ifdef __cplusplus
extern "C" {
#endif
extern void xPortSysTickHandler( void );
/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* The SPI driver polls at a high priority. The logging task's priority must also
 * be high to be not be starved of CPU time. */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRYP_HandleTypeDef hcryp;
__ALIGN_BEGIN static const uint32_t pKeyCRYP[4] __ALIGN_END = {
		0x00000000,0x00000000,0x00000000,0x00000000};
__ALIGN_BEGIN static const uint32_t HeaderCRYP[4] __ALIGN_END = {
		0x00000000,0x00000000,0x00000000,0x00000000};
__ALIGN_BEGIN static const uint32_t B0CRYP[4] __ALIGN_END = {
		0x00000000,0x00000000,0x00000000,0x00000000};

HASH_HandleTypeDef hhash;

RNG_HandleTypeDef hrng;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart3;

UART_HandleTypeDef hlpuart1;

//		struct netif gnetif; /* network interface structure */

/* USER CODE BEGIN PV */
int errno;                                  /* Required by LwIP */
static __IO uint8_t button_flags = 0;       /* Counts the button interrupts */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);
static void MX_CRYP_Init(void);
static void MX_HASH_Init(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_RNG_Init(void);
static void MX_RTC_Init(void);
static void MX_USART3_UART_Init(void);

static void MX_LPUART1_UART_Init(void);



void vApplicationDaemonTaskStartupHook( void );
void Led_SetState(bool on);
void Led_Blink(int period, int duty, int count);

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
/* MATTER BRIDGE APPLICATION -------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#define mainLOGGING_TASK_PRIORITY                       ( configMAX_PRIORITIES - 1 )
#define mainLOGGING_TASK_STACK_SIZE                     ( 1000 )
#define mainLOGGING_MESSAGE_QUEUE_LENGTH                ( 15 )


/* USER CODE BEGIN PFP */
/**
 * @brief Initializes the FreeRTOS heap.
 *
 * Heap_5 is being used because the RAM is not contiguous, therefore the heap
 * needs to be initialized.  See http://www.freertos.org/a00111.html
 */
static void prvInitializeHeap( void );


#if CONFIG_ENABLE_STM32_FACTORY_DATA_PROVIDER
#include <platform/STM32/H7/STM32FactoryDataProvider.h>
//#else
//#include <platform/TestOnlyCommissionableDataProvider.h>
#endif // CONFIG_ENABLE_STM32_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_STM32_DEVICE_INFO_PROVIDER
#include <platform/STM32/H7/STM32DeviceInfoProvider.h>
#else
#include <DeviceInfoProviderImpl.h>
#endif // CONFIG_ENABLE_STM32_DEVICE_INFO_PROVIDER

namespace {
#if CONFIG_ENABLE_STM32_FACTORY_DATA_PROVIDER
chip::DeviceLayer::STM32FactoryDataProvider sFactoryDataProvider;
//#else
//chip::DeviceLayer::TestOnlyCommissionableDataProvider gTestOnlyCommissionableDataProvider;
#endif // CONFIG_ENABLE_STM32_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_STM32_DEVICE_INFO_PROVIDER
chip::DeviceLayer::STM32DeviceInfoProvider gExampleDeviceInfoProvider;
#else
chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
#endif // CONFIG_ENABLE_STM32_DEVICE_INFO_PROVIDER
} // namespace

struct netif gnetif; /* network interface structure */

const char * TAG = "bridge-app";

using namespace ::chip;
using namespace ::chip::DeviceManager;
using namespace ::chip::Platform;
using namespace ::chip::Credentials;
using namespace ::chip::app::Clusters;

static AppDeviceCallbacks AppCallback;

static const int kNodeLabelSize = 32;
// Current ZCL implementation of Struct uses a max-size array of 254 bytes
static const int kDescriptorAttributeArraySize = 254;

static EndpointId gCurrentEndpointId;
static EndpointId gFirstDynamicEndpointId;
static Device * gDevices[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT]; // number of dynamic endpoints count

// 8 Bridged devices
static Device gLight1("Light 1", "Kitchen");		// Kitchen
static Device gLight2("Light 2", "Bedroom");		// Light bulb in Bedroom
static Device gLight3("Light 3", "Garden");			// Garden Tree
static Device gLight4("Light 4", "Street");			// Street light
static Device gLight5("Light 5", "LivingRoom");		// TV in Living Room
static Device gLight6("Light 6", "Bathroom");		// Bathroom
static Device gLight7("Light 7", "Office");			// Office
static Device gLight8("Light 8", "Bedroom");		// Bedroom

// (taken from chip-devices.xml)
#define DEVICE_TYPE_BRIDGED_NODE 0x0013
// (taken from lo-devices.xml)
#define DEVICE_TYPE_LO_ON_OFF_LIGHT 0x0100

// (taken from chip-devices.xml)
#define DEVICE_TYPE_ROOT_NODE 0x0016
// (taken from chip-devices.xml)
#define DEVICE_TYPE_BRIDGE 0x000e

// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

/* BRIDGED DEVICE ENDPOINT: contains the following clusters:
   - On/Off
   - Descriptor
   - Bridged Device Basic
 */

// Declare On/Off cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(onOffAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_ON_OFF_ATTRIBUTE_ID, BOOLEAN, 1, 0), /* on/off */
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Descriptor cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(descriptorAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_DEVICE_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0),     /* device list */
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_SERVER_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* server list */
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_CLIENT_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* client list */
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_PARTS_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0),  /* parts list */
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Bridged Device Basic information cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(bridgedDeviceBasicAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_NODE_LABEL_ATTRIBUTE_ID, CHAR_STRING, kNodeLabelSize, 0), /* NodeLabel */
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_REACHABLE_ATTRIBUTE_ID, BOOLEAN, 1, 0),               /* Reachable */
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Cluster List for Bridged Light endpoint
// It's not clear whether it would be better to get the command lists from
// the ZAP config on our last fixed endpoint instead.
constexpr CommandId onOffIncomingCommands[] = {
		app::Clusters::OnOff::Commands::Off::Id,
		app::Clusters::OnOff::Commands::On::Id,
		app::Clusters::OnOff::Commands::Toggle::Id,
		app::Clusters::OnOff::Commands::OffWithEffect::Id,
		app::Clusters::OnOff::Commands::OnWithRecallGlobalScene::Id,
		app::Clusters::OnOff::Commands::OnWithTimedOff::Id,
		kInvalidCommandId,
};

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedLightClusters)
DECLARE_DYNAMIC_CLUSTER(ZCL_ON_OFF_CLUSTER_ID, onOffAttrs, onOffIncomingCommands, nullptr),
DECLARE_DYNAMIC_CLUSTER(ZCL_DESCRIPTOR_CLUSTER_ID, descriptorAttrs, nullptr, nullptr),
DECLARE_DYNAMIC_CLUSTER(ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID, bridgedDeviceBasicAttrs, nullptr,
		nullptr)
		DECLARE_DYNAMIC_CLUSTER_LIST_END;

// Declare Bridged Light endpoint
DECLARE_DYNAMIC_ENDPOINT(bridgedLightEndpoint, bridgedLightClusters);

DataVersion gLight1DataVersions[ArraySize(bridgedLightClusters)];
DataVersion gLight2DataVersions[ArraySize(bridgedLightClusters)];
DataVersion gLight3DataVersions[ArraySize(bridgedLightClusters)];
DataVersion gLight4DataVersions[ArraySize(bridgedLightClusters)];
DataVersion gLight5DataVersions[ArraySize(bridgedLightClusters)];
DataVersion gLight6DataVersions[ArraySize(bridgedLightClusters)];
DataVersion gLight7DataVersions[ArraySize(bridgedLightClusters)];
DataVersion gLight8DataVersions[ArraySize(bridgedLightClusters)];

/* REVISION definitions:
 */

#define ZCL_DESCRIPTOR_CLUSTER_REVISION (1u)
#define ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_REVISION (1u)
#define ZCL_FIXED_LABEL_CLUSTER_REVISION (1u)
#define ZCL_ON_OFF_CLUSTER_REVISION (4u)

/* MATTER BRIDGE APPLICATION -------------------------------------------------------------*/
uint8_t previousState = 0;
uint8_t currentState = 0;
/* USER CODE END 0 */

/* Private user code ---------------------------------------------------------*/


/* ------------------------ Matter function prototypes C++ ------------------------ */
int AddDeviceEndpoint(Device * dev, EmberAfEndpointType * ep, const Span<const EmberAfDeviceType> & deviceTypeList, const Span<DataVersion> & dataVersionStorage);
CHIP_ERROR RemoveDeviceEndpoint(Device * dev);

EmberAfStatus HandleReadBridgedDeviceBasicAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength);
EmberAfStatus HandleReadOnOffAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength);
EmberAfStatus HandleWriteOnOffAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer);

EmberAfStatus emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
		const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
		uint16_t maxReadLength);

EmberAfStatus emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
		const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer);

void HandleDeviceStatusChanged(Device * dev, Device::Changed_t itemChangedMask);

/* ------------------------ Matter function prototypes C++ ------------------------ */

int AddDeviceEndpoint(Device * dev, EmberAfEndpointType * ep, const Span<const EmberAfDeviceType> & deviceTypeList,
		const Span<DataVersion> & dataVersionStorage)
{
	uint8_t index = 0;
	while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
	{
		if (NULL == gDevices[index])
		{
			gDevices[index] = dev;
			EmberAfStatus ret;
			while (1)
			{
				dev->SetEndpointId(gCurrentEndpointId);
				ret = emberAfSetDynamicEndpoint(index, gCurrentEndpointId, ep, dataVersionStorage, deviceTypeList);
				if (ret == EMBER_ZCL_STATUS_SUCCESS)
				{
					ChipLogProgress(DeviceLayer, "Added device %s to dynamic endpoint %d (index=%d)", dev->GetName(),
							gCurrentEndpointId, index);
					return index;
				}
				else if (ret != EMBER_ZCL_STATUS_DUPLICATE_EXISTS)
				{
					return -1;
				}
				// Handle wrap condition
				if (++gCurrentEndpointId < gFirstDynamicEndpointId)
				{
					gCurrentEndpointId = gFirstDynamicEndpointId;
				}
			}
		}
		index++;
	}
	ChipLogProgress(DeviceLayer, "Failed to add dynamic endpoint: No endpoints available!");
	return -1;
}

CHIP_ERROR RemoveDeviceEndpoint(Device * dev)
{
	for (uint8_t index = 0; index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; index++)
	{
		if (gDevices[index] == dev)
		{
			EndpointId ep   = emberAfClearDynamicEndpoint(index);
			gDevices[index] = NULL;
			ChipLogProgress(DeviceLayer, "Removed device %s from dynamic endpoint %d (index=%d)", dev->GetName(), ep, index);
			// Silence complaints about unused ep when progress logging
			// disabled.
			UNUSED_VAR(ep);
			return CHIP_NO_ERROR;
		}
	}
	return CHIP_ERROR_INTERNAL;
}

EmberAfStatus HandleReadBridgedDeviceBasicAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer,
		uint16_t maxReadLength)
{
	ChipLogProgress(DeviceLayer, "HandleReadBridgedDeviceBasicAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

	if ((attributeId == ZCL_REACHABLE_ATTRIBUTE_ID) && (maxReadLength == 1))
	{
		*buffer = dev->IsReachable() ? 1 : 0;
	}
	else if ((attributeId == ZCL_NODE_LABEL_ATTRIBUTE_ID) && (maxReadLength == 32))
	{
		MutableByteSpan zclNameSpan(buffer, maxReadLength);
		MakeZclCharString(zclNameSpan, dev->GetName());
	}
	else if ((attributeId == ZCL_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID) && (maxReadLength == 2))
	{
		*buffer = (uint16_t) ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_REVISION;
	}
	else
	{
		return EMBER_ZCL_STATUS_FAILURE;
	}

	return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleReadOnOffAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
	ChipLogProgress(DeviceLayer, "HandleReadOnOffAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

	if ((attributeId == ZCL_ON_OFF_ATTRIBUTE_ID) && (maxReadLength == 1))
	{
		*buffer = dev->IsOn() ? 1 : 0;
	}
	else if ((attributeId == ZCL_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID) && (maxReadLength == 2))
	{
		*buffer = (uint16_t) ZCL_ON_OFF_CLUSTER_REVISION;
	}
	else
	{
		return EMBER_ZCL_STATUS_FAILURE;
	}

	return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleWriteOnOffAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer)
{
	ChipLogProgress(DeviceLayer, "HandleWriteOnOffAttribute: attrId=%d", attributeId);

	ReturnErrorCodeIf((attributeId != ZCL_ON_OFF_ATTRIBUTE_ID) || (!dev->IsReachable()), EMBER_ZCL_STATUS_FAILURE);

	dev->SetOnOff(*buffer == 1);

	return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
		const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
		uint16_t maxReadLength)
{
	uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

	if ((endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT) && (gDevices[endpointIndex] != NULL))
	{
		Device * dev = gDevices[endpointIndex];

		if (clusterId == ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID)
		{
			return HandleReadBridgedDeviceBasicAttribute(dev, attributeMetadata->attributeId, buffer, maxReadLength);
		}
		else if (clusterId == ZCL_ON_OFF_CLUSTER_ID)
		{
			return HandleReadOnOffAttribute(dev, attributeMetadata->attributeId, buffer, maxReadLength);
		}
	}

	return EMBER_ZCL_STATUS_FAILURE;
}

EmberAfStatus emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
		const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
	uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

	if (endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
	{
		Device * dev = gDevices[endpointIndex];

		if ((dev->IsReachable()) && (clusterId == ZCL_ON_OFF_CLUSTER_ID))
		{
			return HandleWriteOnOffAttribute(dev, attributeMetadata->attributeId, buffer);
		}
	}

	return EMBER_ZCL_STATUS_FAILURE;
}

namespace {
void CallReportingCallback(intptr_t closure)
{
	auto path = reinterpret_cast<app::ConcreteAttributePath *>(closure);
	MatterReportingAttributeChangeCallback(*path);
	Platform::Delete(path);
}

void ScheduleReportingCallback(Device * dev, ClusterId cluster, AttributeId attribute)
{
	auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(), cluster, attribute);
	DeviceLayer::PlatformMgr().ScheduleWork(CallReportingCallback, reinterpret_cast<intptr_t>(path));
}
} // anonymous namespace

void HandleDeviceStatusChanged(Device * dev, Device::Changed_t itemChangedMask)
{
	if (itemChangedMask & Device::kChanged_Reachable)
	{
		ScheduleReportingCallback(dev, BridgedDeviceBasic::Id, BridgedDeviceBasic::Attributes::Reachable::Id);
	}

	if (itemChangedMask & Device::kChanged_State)
	{
		ScheduleReportingCallback(dev, OnOff::Id, OnOff::Attributes::OnOff::Id);
	}

	if (itemChangedMask & Device::kChanged_Name)
	{
		ScheduleReportingCallback(dev, BridgedDeviceBasic::Id, BridgedDeviceBasic::Attributes::NodeLabel::Id);
	}
}

bool emberAfActionsClusterInstantActionCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
		const Actions::Commands::InstantAction::DecodableType & commandData)
{
	// No actions are implemented, just return status NotFound.
	commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::NotFound);
	return true;
}

const EmberAfDeviceType gBridgedRootDeviceTypes[] = { { DEVICE_TYPE_ROOT_NODE, DEVICE_VERSION_DEFAULT },
		{ DEVICE_TYPE_BRIDGE, DEVICE_VERSION_DEFAULT } };

const EmberAfDeviceType gBridgedOnOffDeviceTypes[] = { { DEVICE_TYPE_LO_ON_OFF_LIGHT, DEVICE_VERSION_DEFAULT },
		{ DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT } };
#ifdef __cplusplus
extern "C" {
#endif

static void InitServer(intptr_t context)
{

	tcpip_init(NULL, NULL);

	/* Initialize the LwIP stack */
	Netif_Config();

	Stm32AppServer::Init(); // Init ZCL Data Model and CHIP App Server AND Initialize device attestation config

	// Print QR Code URL
	PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kOnNetwork));

	// Set starting endpoint id where dynamic endpoints will be assigned, which
	// will be the next consecutive endpoint id after the last fixed endpoint.
	gFirstDynamicEndpointId = static_cast<chip::EndpointId>(
			static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
	gCurrentEndpointId = gFirstDynamicEndpointId;

	// Disable last fixed endpoint, which is used as a placeholder for all of the
	// supported clusters so that ZAP will generated the requisite code.
	emberAfEndpointEnableDisable(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1)), false);

	// Add lights 1..3 --> will be mapped to ZCL endpoints 2, 3, 4
	AddDeviceEndpoint(&gLight1, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
			Span<DataVersion>(gLight1DataVersions));
	AddDeviceEndpoint(&gLight2, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
			Span<DataVersion>(gLight2DataVersions));
	AddDeviceEndpoint(&gLight3, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
			Span<DataVersion>(gLight3DataVersions));

	// Add lights 4..8 --> will be mapped to ZCL endpoints 5, 6, 7, 8, 9
	AddDeviceEndpoint(&gLight4, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
			Span<DataVersion>(gLight4DataVersions));
	AddDeviceEndpoint(&gLight5, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
			Span<DataVersion>(gLight5DataVersions));
	AddDeviceEndpoint(&gLight6, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
			Span<DataVersion>(gLight6DataVersions));
	AddDeviceEndpoint(&gLight7, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
			Span<DataVersion>(gLight7DataVersions));
	AddDeviceEndpoint(&gLight8, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
			Span<DataVersion>(gLight8DataVersions));

	chip::DeviceLayer::ConfigurationMgr().LogDeviceConfig();
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* USER CODE BEGIN 1 */
	/* Perform first any hardware initialization that does not require the RTOS to be
	 * running.  */
	/* USER CODE END 1 */

	/* MPU Configuration--------------------------------------------------------*/
	MPU_Config();

	/* Enable D-Cache---------------------------------------------------------*/
	//SCB_EnableDCache();			/* NEVER UNCOMMENT THIS LINE OR THE PROJECT WILL BUG EVERYWHERE */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */
	/* Enable D2 domain SRAM3 Clock (0x30040000 AXI)*/
	__HAL_RCC_D2SRAM3_CLK_ENABLE();

	/* Heap_5 is being used because the RAM is not contiguous in memory, so the
	 * heap must be initialized. */
	prvInitializeHeap();

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_CRYP_Init();
	MX_HASH_Init();
	MX_RNG_Init();
	MX_RTC_Init();
	MX_LPUART1_UART_Init();
	MX_USART3_UART_Init();

	/* USER CODE BEGIN 2 */
	CHIP_ERROR err = CHIP_NO_ERROR;
	/* USER CODE END 2 */

	/* Create the thread(s) */
	/* USER CODE BEGIN RTOS_THREADS */

	freertos_mbedtls_init();

	/* USER CODE END RTOS_THREADS */

	// bridge will have own database named gDevices.
	// Clear database
	memset(gDevices, 0, sizeof(gDevices));

	gLight1.SetReachable(true);
	gLight2.SetReachable(true);
	gLight3.SetReachable(true);
	gLight4.SetReachable(true);
	gLight5.SetReachable(true);
	gLight6.SetReachable(true);
	gLight7.SetReachable(true);
	gLight8.SetReachable(true);

	// Whenever bridged device changes its state
	gLight1.SetChangeCallback(&HandleDeviceStatusChanged);
	gLight2.SetChangeCallback(&HandleDeviceStatusChanged);
	gLight3.SetChangeCallback(&HandleDeviceStatusChanged);
	gLight4.SetChangeCallback(&HandleDeviceStatusChanged);
	gLight5.SetChangeCallback(&HandleDeviceStatusChanged);
	gLight6.SetChangeCallback(&HandleDeviceStatusChanged);
	gLight7.SetChangeCallback(&HandleDeviceStatusChanged);
	gLight8.SetChangeCallback(&HandleDeviceStatusChanged);

	DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

	CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

	err = deviceMgr.Init(&AppCallback);

#if CONFIG_ENABLE_STM32_FACTORY_DATA_PROVIDER
	SetCommissionableDataProvider(&sFactoryDataProvider);
	SetDeviceAttestationCredentialsProvider(&sFactoryDataProvider);
#if CONFIG_ENABLE_STM32_DEVICE_INSTANCE_INFO_PROVIDER
	SetDeviceInstanceInfoProvider(&sFactoryDataProvider);
#endif
#else
	//chip::DeviceLayer::SetCommissionableDataProvider(&gTestOnlyCommissionableDataProvider);
	SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif // CONFIG_ENABLE_STM32_FACTORY_DATA_PROVIDER

	chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));

	print_stacks_highwatermarks();

	vTaskStartScheduler();
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)		// 480 MHz
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Supply configuration update enable
	 */
	HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

	/** Configure the main internal regulator output voltage
	 */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

	while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 60;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLQ = 10;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
			|RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief CRYP Initialization Function
 * @param None
 * @retval None
 */
static void MX_CRYP_Init(void)
{

	/* USER CODE BEGIN CRYP_Init 0 */

	/* USER CODE END CRYP_Init 0 */

	/* USER CODE BEGIN CRYP_Init 1 */

	/* USER CODE END CRYP_Init 1 */
	hcryp.Instance = CRYP;
	hcryp.Init.DataType = CRYP_DATATYPE_32B;
	hcryp.Init.KeySize = CRYP_KEYSIZE_128B;
	hcryp.Init.pKey = (uint32_t *)pKeyCRYP;
	hcryp.Init.Algorithm = CRYP_AES_CCM;
	hcryp.Init.Header = (uint32_t *)HeaderCRYP;
	hcryp.Init.HeaderSize = 4;
	hcryp.Init.B0 = (uint32_t *)B0CRYP;
	if (HAL_CRYP_Init(&hcryp) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN CRYP_Init 2 */

	/* USER CODE END CRYP_Init 2 */

}

/**
 * @brief HASH Initialization Function
 * @param None
 * @retval None
 */
static void MX_HASH_Init(void)
{

	/* USER CODE BEGIN HASH_Init 0 */

	/* USER CODE END HASH_Init 0 */

	/* USER CODE BEGIN HASH_Init 1 */

	/* USER CODE END HASH_Init 1 */
	hhash.Init.DataType = HASH_DATATYPE_32B;
	if (HAL_HASH_Init(&hhash) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN HASH_Init 2 */

	/* USER CODE END HASH_Init 2 */

}

/**
 * @brief RNG Initialization Function
 * @param None
 * @retval None
 */
static void MX_RNG_Init(void)
{

	/* USER CODE BEGIN RNG_Init 0 */

	/* USER CODE END RNG_Init 0 */

	/* USER CODE BEGIN RNG_Init 1 */

	/* USER CODE END RNG_Init 1 */
	hrng.Instance = RNG;
	hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
	if (HAL_RNG_Init(&hrng) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN RNG_Init 2 */

	/* USER CODE END RNG_Init 2 */

}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
static void MX_RTC_Init(void)
{

	/* USER CODE BEGIN RTC_Init 0 */
	RTC_TimeTypeDef xsTime;
	RTC_DateTypeDef xsDate;
	/* USER CODE END RTC_Init 0 */

	/* USER CODE BEGIN RTC_Init 1 */

	/* USER CODE END RTC_Init 1 */
	/** Initialize RTC Only
	 */
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
	if (HAL_RTC_Init(&hrtc) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN RTC_Init 2 */
	/* Initialize RTC and set the Time and Date. */
	xsTime.Hours = 0x12;
	xsTime.Minutes = 0x0;
	xsTime.Seconds = 0x0;
	xsTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	xsTime.StoreOperation = RTC_STOREOPERATION_RESET;

	if( HAL_RTC_SetTime( &hrtc, &xsTime, RTC_FORMAT_BCD ) != HAL_OK )
	{
		Error_Handler();
	}

	xsDate.WeekDay = RTC_WEEKDAY_FRIDAY;
	xsDate.Month = RTC_MONTH_JANUARY;
	xsDate.Date = 0x03;
	xsDate.Year = 0x20;

	if( HAL_RTC_SetDate( &hrtc, &xsDate, RTC_FORMAT_BCD ) != HAL_OK )
	{
		Error_Handler();
	}
	/* USER CODE END RTC_Init 2 */

}

/**
 * @brief LPUART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_LPUART1_UART_Init(void)
{

	/* USER CODE BEGIN LPUART1_Init 0 */

	/* USER CODE END LPUART1_Init 0 */

	/* USER CODE BEGIN LPUART1_Init 1 */

	/* USER CODE END LPUART1_Init 1 */
	hlpuart1.Instance = LPUART1;
	hlpuart1.Init.BaudRate = 115200;
	hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
	hlpuart1.Init.StopBits = UART_STOPBITS_1;
	hlpuart1.Init.Parity = UART_PARITY_NONE;
	hlpuart1.Init.Mode = UART_MODE_TX_RX;
	hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	hlpuart1.FifoMode = UART_FIFOMODE_DISABLE;
	if (HAL_UART_Init(&hlpuart1) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&hlpuart1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN LPUART1_Init 2 */

	/* USER CODE END LPUART1_Init 2 */

}

/**
 * @brief USART3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART3_UART_Init(void)
{

	/* USER CODE BEGIN USART3_Init 0 */

	/* USER CODE END USART3_Init 0 */

	/* USER CODE BEGIN USART3_Init 1 */

	/* USER CODE END USART3_Init 1 */
	huart3.Instance = USART3;
	huart3.Init.BaudRate = 115200;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart3) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART3_Init 2 */

	/* USER CODE END USART3_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, USER_LED1_Pin|USER_LED3_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(USER_LED2_GPIO_Port, USER_LED2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : USER_BUTTON_Pin */
	GPIO_InitStruct.Pin = USER_BUTTON_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : USER_LED1_Pin USER_LED3_Pin */
	GPIO_InitStruct.Pin = USER_LED1_Pin|USER_LED3_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_PowerSwitchOn_Pin */
	GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_OverCurrent_Pin */
	GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : USB_SOF_Pin USB_ID_Pin USB_DM_Pin USB_DP_Pin */
	GPIO_InitStruct.Pin = USB_SOF_Pin|USB_ID_Pin|USB_DM_Pin|USB_DP_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : USER_LED2_Pin */
	GPIO_InitStruct.Pin = USER_LED2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(USER_LED2_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = USER_A0_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(USER_A0_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = USER_A1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(USER_A1_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = USER_A2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(USER_A2_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = USER_A3_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(USER_A3_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = USER_A4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(USER_A4_GPIO_Port, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

void vApplicationIdleHook( void )
{
	static TickType_t xLastPrint = 0;
	TickType_t xTimeNow;
	const TickType_t xPrintFrequency = pdMS_TO_TICKS( 2000 );

	xTimeNow = xTaskGetTickCount();

	if( ( xTimeNow - xLastPrint ) > xPrintFrequency )
	{
		// configPRINT( "." );
		xLastPrint = xTimeNow;
	}

	if (button_flags > 0)
	{
		button_flags = 0;
		Led_Blink(100, 50, 5);
	}
}


#ifndef __GNUC__
void * malloc( size_t xSize )
{
	configASSERT( xSize == ~0 );

	return NULL;
}

void * calloc( size_t xNum, size_t xSize )
{
	configASSERT( xSize == ~0 );

	return NULL;
}

void free( void * addr )
{
	configASSERT( true );
}
#endif /* ifndef __GNUC__ */

#if (defined(__GNUC__) && !defined(__CC_ARM))
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar(void)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif /* __GNUC__ */

/**
 * @brief  Retargets the C library printf function to the USART.
 * @param  None
 * @retval None
 */
PUTCHAR_PROTOTYPE
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART and Loop until the end of transmission */

	while (HAL_OK != HAL_UART_Transmit(&huart3, (uint8_t *) &ch, 1, 30000))
	{
		;
	}
	return ch;

}

/**
 * @brief  Retargets the C library scanf function to the USART.
 * @param  None
 * @retval None
 */
GETCHAR_PROTOTYPE
{
	/* Place your implementation of fgetc here */
	/* e.g. read a character on USART and loop until the end of read */
	uint8_t ch = 0;
	while (HAL_OK != HAL_UART_Receive(&huart3, (uint8_t *)&ch, 1, 30000))
	{
		;
	}
	return ch;
}

void vMainUARTPrintString( const char * pcString )
{
	const uint32_t ulTimeout = 3000UL;
	HAL_UART_Transmit( &huart3,
			( uint8_t * ) pcString,
			strlen( pcString ),
			ulTimeout );
}

void vApplicationDaemonTaskStartupHook( void )
{

}

#ifdef __cplusplus
}
#endif

static void prvInitializeHeap( void )
{
	static uint8_t ucHeap1[ configTOTAL_HEAP_SIZE ];

	HeapRegion_t xHeapRegions[] =
	{
			{ ( unsigned char * ) ucHeap1, sizeof( ucHeap1 ) },
			{ NULL,                                        0 }
	};

	vPortDefineHeapRegions( xHeapRegions );
}

/**
 * @brief  Initializes the lwIP stack
 * @param  None
 * @retval None
 */
void Netif_Config(void)
{
	//IPv6 only
	ip_addr_t ipaddr;
	ip_addr_t netmask;
	ip_addr_t gw;

	ip_addr_set_zero_ip4(&ipaddr);
	ip_addr_set_zero_ip4(&netmask);
	ip_addr_set_zero_ip4(&gw);

	/* add the network interface */
	netif_add(&gnetif,  (ip_2_ip4(&ipaddr)), (ip_2_ip4(&netmask)), (ip_2_ip4(&gw)), NULL, &ethernetif_init, &tcpip_input);

	gnetif.name[0] = 'A';
	gnetif.name[1] = 'R';

	/* Create IPv6 local address */
	gnetif.ip6_autoconfig_enabled = 1;
	netif_create_ip6_linklocal_address(&gnetif, 1);
	//netif_ip6_addr_set_state(&gnetif, 0, IP6_ADDR_VALID);
	STM32_LOG("ip6 linklocal address: %s\n", ip6addr_ntoa(netif_ip6_addr(&gnetif, 0)));

	gnetif.flags |= NETIF_FLAG_MLD6;

	/*  Registers the default network interface. */
	netif_set_default(&gnetif);

	ethernet_link_status_updated(&gnetif);

#if LWIP_NETIF_LINK_CALLBACK
	netif_set_link_callback(&gnetif, ethernet_link_status_updated);

	xTaskCreate( ethernet_link_thread,
			"EthLink",
			configMINIMAL_STACK_SIZE *2,
			&gnetif,
			3, /* Normal task priority */
			NULL );
#endif
}

#if ( ( configUSE_TRACE_FACILITY == 1 ) && (INCLUDE_uxTaskGetStackHighWaterMark == 1) )
void print_stacks_highwatermarks( void )
{
	UBaseType_t task_nr =	uxTaskGetNumberOfTasks();
	TaskStatus_t *p_task_status = (TaskStatus_t *) pvPortMalloc(task_nr * sizeof(TaskStatus_t));
	if (p_task_status != NULL)
	{
		if (task_nr == uxTaskGetSystemState(p_task_status, task_nr, NULL))
		{
			//	  for (int i = 0; i < task_nr; i++)
			//	  {
			//		vLoggingPrintf("StackHighWaterMark: %s=%lu\n", p_task_status[i].pcTaskName, uxTaskGetStackHighWaterMark(p_task_status[i].xHandle));
			//	  }
		}
		vPortFree(p_task_status);
	}
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin)
	{
	case (USER_BUTTON_Pin):
		{
		button_flags++;
		break;
		}
	default:
	{
		break;
	}
	}
}

void Led_SetState(bool on)
{
	HAL_GPIO_WritePin(USER_LED2_GPIO_Port, USER_LED2_Pin, (on == true) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief Blink LED for 'count' cycles of 'period' period and 'duty' ON duration.
 * duty < 0 tells to start with an OFF state.
 */
void Led_Blink(int period, int duty, int count)
{
	if ( (duty > 0) && (period >= duty) )
	{
		/*  Shape:   ____
				  on |_off__ */
		do
		{
			Led_SetState(true);
			HAL_Delay(duty);
			Led_SetState(false);
			HAL_Delay(period - duty);
		} while (count--);
	}
	if ( (duty < 0) && (period >= -duty) )
	{
		/*  Shape:         ____
				__off_| on   */
		do
		{
			Led_SetState(false);
			HAL_Delay(period + duty);
			Led_SetState(true);
			HAL_Delay(-duty);
		} while (count--);
	}
}

/* USER CODE END 4 */
/* MPU Configuration */

static void MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct = {0};

	/* Disables the MPU */
	HAL_MPU_Disable();

	/* Configure the MPU attributes as Device not cacheable
	 for ETH DMA descriptors */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.BaseAddress = 0x30040000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_256B;
	MPU_InitStruct.SubRegionDisable = 0x0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes as Normal Non Cacheable
	 for LwIP RAM heap which contains the Tx buffers */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER1;
	MPU_InitStruct.BaseAddress = 0x30044000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
	MPU_InitStruct.SubRegionDisable = 0x0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	/* Enables the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}
/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM6 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM6) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* Prevent the tick to call FreeRTOS handler before it is started. */
	if (htim->Instance == TIM6) {
		if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
		{
			xPortSysTickHandler( );
		}
	}

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	while( 1 )
	{
		Led_Blink(200, 100, 1);
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(char *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	 tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
#ifdef __cplusplus
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
