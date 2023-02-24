
  @verbatim
  ******************************************************************************
  * @file    st_readme.txt
  * @author  MCD Application Team
  * @brief   This file lists the modification done by STMicroelectronics on
  *          Matter for integration with STM32Cube solution.
  ******************************************************************************
  * Copyright (c) 2020 Project CHIP Authors
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  ******************************************************************************
  @endverbatim

### 27-Jan-2023 ###
========================
  
  + Using Matter v1.0.0 SDK.
  
  Limitation:
  + All symbolic links of Matter and his dependances has been removed.
  
  + List of files removed from the original Matter directory:
    + in connectedhomeip/third_party/ directory:
      - connectedhomeip/third_party/ameba/
	  - connectedhomeip/third_party/android_deps/
	  - connectedhomeip/third_party/boringssl/
	  - connectedhomeip/third_party/bouffalolab/
	  - connectedhomeip/third_party/cirque/
	  - connectedhomeip/third_party/editline/
	  - connectedhomeip/third_party/infineon/
	  - connectedhomeip/third_party/inipp/
	  - connectedhomeip/third_party/jlink/
	  - connectedhomeip/third_party/jsoncpp/
	  - connectedhomeip/third_party/mbed-mcu-boot/
	  - connectedhomeip/third_party/mbed-os-cypress-capsense-button/
	  - connectedhomeip/third_party/mbed-os-posix-socket/
	  - connectedhomeip/third_party/mbed-os/
	  - connectedhomeip/third_party/mt793x_sdk/
	  - connectedhomeip/third_party/mynewt-core/
	  - connectedhomeip/third_party/nanopb/
	  - connectedhomeip/third_party/nxp/
	  - connectedhomeip/third_party/openthread/
	  - connectedhomeip/third_party/ot-br-posix/
	  - connectedhomeip/third_party/pigweed/
	  - connectedhomeip/third_party/pybind11/
	  - connectedhomeip/third_party/qpg_sdk/
	  - connectedhomeip/third_party/silabs/
	  - connectedhomeip/third_party/simw-top-mini/
	  - connectedhomeip/third_party/telink_sdk/
	  - connectedhomeip/third_party/ti_simplelink_sdk/
	  - connectedhomeip/third_party/tizen/
	  - connectedhomeip/third_party/zap/

    + in connectedhomeip/src/ directory:
	  - connectedhomeip/src/android
	  - connectedhomeip/src/darwin
 
    + In connectedhomeip directory, the following directories/files has been removed:
      - connectedhomeip/.devcontainer/
	  - connectedhomeip/.githooks/
	  - connectedhomeip/.github/
	  - connectedhomeip/.vscode/
	  - connectedhomeip/build/
	  - connectedhomeip/build_overrides/
	  - connectedhomeip/config/
	  - connectedhomeip/integrations/
	  - connectedhomeip/scripts/
	  - connectedhomeip/.clang-format
	  - connectedhomeip/.clang-tidy
	  - connectedhomeip/.default-version.min
	  - connectedhomeip/.dirs-locals.el
	  - connectedhomeip/.editorconfig
	  - connectedhomeip/.flake8
	  - connectedhomeip/.gitattributes
	  - connectedhomeip/.gitignore
	  - connectedhomeip/.gitmodules
	  - connectedhomeip/.gn
	  - connectedhomeip/.prettierrc.json
	  - connectedhomeip/.pullapprove.yml
	  - connectedhomeip/.restyled.yaml
	  - connectedhomeip/.shellcheck_tree
	  - connectedhomeip/.spellcheck.yml
	  - connectedhomeip/.gn_build.sh
	  - connectedhomeip/.lgtm.yml
	   
	+ To recover all deleted files, please follow these steps:
	  - download the Git for Windows software.
	  - save this file (st_readme.txt) in a location other than the connectedhomeip directory.
	  - delete the Middlewares/Third_Party/connectedhomeip directory.
	  - do the following command in the Middlewares/Third_Party directory, with a git for Windows shell terminal:
	  
	    git clone -b v1.0.0 https://github.com/project-chip/connectedhomeip.git --recurse-submodule
		
	  - you now have the complete Matter SDK v1.0.0.