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

 #include "Stm32AppServer.h"
 #include "CHIPDeviceManager.h"
 #include <app/clusters/network-commissioning/network-commissioning.h>
 #include <app/server/Dnssd.h>
 #include <app/server/Server.h>

 using namespace chip;
 using namespace chip::Credentials;
 using namespace chip::DeviceLayer;

 namespace {
 #if CHIP_DEVICE_CONFIG_ENABLE_WIFI
 #endif

 } // namespace

 void Stm32AppServer::Init(AppDelegate * sAppDelegate)
 {
     // Init ZCL Data Model and CHIP App Server
     static chip::CommonCaseDeviceServerInitParams initParams;
     (void) initParams.InitializeStaticResourcesBeforeServerInit();
     if (sAppDelegate != nullptr)
     {
         initParams.appDelegate = sAppDelegate;
     }
     chip::Server::GetInstance().Init(initParams);

 }
