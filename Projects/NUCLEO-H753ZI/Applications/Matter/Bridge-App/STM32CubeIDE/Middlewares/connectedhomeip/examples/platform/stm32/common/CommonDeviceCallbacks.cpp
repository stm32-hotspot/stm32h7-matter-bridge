/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "CommonDeviceCallbacks.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/server/Dnssd.h>
#include <app/util/util.h>
#include <lib/support/CodeUtils.h>

#include "AppConfig.h"

#if CONFIG_ENABLE_OTA_REQUESTOR
#include <ota/OTAHelper.h>
#endif

using namespace chip;
using namespace chip::DeviceLayer;
using namespace chip::System;

DeviceCallbacksDelegate * appDelegate = nullptr;

void CommonDeviceCallbacks::DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kInternetConnectivityChange:
        OnInternetConnectivityChange(event);
        break;

    case DeviceEventType::kCHIPoBLEConnectionEstablished:
        STM32_LOG("CHIPoBLE connection established");
        break;

    case DeviceEventType::kCHIPoBLEConnectionClosed:
        STM32_LOG("CHIPoBLE disconnected");
        break;

    case DeviceEventType::kCommissioningComplete: {
        STM32_LOG("Commissioning complete");
    }
    break;

    case DeviceEventType::kInterfaceIpAddressChanged:
        if ((event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV4_Assigned) ||
            (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned))
        {
            // MDNS server restart on any ip assignment: if link local ipv6 is configured, that
            // will not trigger a 'internet connectivity change' as there is no internet
            // connectivity. MDNS still wants to refresh its listening interfaces to include the
            // newly selected address.
            chip::app::DnssdServer::Instance().StartServer();
        }
        if (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned)
        {

        }
        break;
    }
}

void CommonDeviceCallbacks::OnInternetConnectivityChange(const ChipDeviceEvent * event)
{
#if CONFIG_ENABLE_OTA_REQUESTOR
    static bool isOTAInitialized = false;
#endif
    appDelegate = DeviceCallbacksDelegate::Instance().GetAppDelegate();
    if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
    {
        STM32_LOG("IPv4 Server ready...");
        if (appDelegate != nullptr)
        {
            appDelegate->OnIPv4ConnectivityEstablished();
        }
        chip::app::DnssdServer::Instance().StartServer();
#if CONFIG_ENABLE_OTA_REQUESTOR
        if (!isOTAInitialized)
        {
            OTAHelpers::Instance().InitOTARequestor();
            isOTAInitialized = true;
        }
#endif
    }
    else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
    {
        STM32_LOG("Lost IPv4 connectivity...");
        if (appDelegate != nullptr)
        {
            appDelegate->OnIPv4ConnectivityLost();
        }
    }
    if (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
    {
        STM32_LOG("IPv6 Server ready...");
        chip::app::DnssdServer::Instance().StartServer();

#if CONFIG_ENABLE_OTA_REQUESTOR
        if (!isOTAInitialized)
        {
            OTAHelpers::Instance().InitOTARequestor();
            isOTAInitialized = true;
        }
#endif
    }
    else if (event->InternetConnectivityChange.IPv6 == kConnectivity_Lost)
    {
        STM32_LOG("Lost IPv6 connectivity...");
    }
}
