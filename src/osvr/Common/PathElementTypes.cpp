/** @file
    @brief Implementation

    @date 2014

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2014 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
#include <osvr/Common/PathElementTypes.h>

// Library/third-party includes
// - none

// Standard includes
// - none

namespace osvr {
namespace common {
    namespace elements {
        AliasElement::AliasElement(std::string const &source)
            : m_source(source) {}
        void AliasElement::setSource(std::string const &source) {
            /// @todo validation?
            m_source = source;
        }

        std::string const &AliasElement::getSource() const { return m_source; }

        DeviceElement
        DeviceElement::createVRPNDeviceElement(std::string const &deviceName,
                                               std::string const &server) {
            DeviceElement ret;
            ret.m_devName = deviceName;
            ret.m_server = server;
            return ret;
        }

        std::string const &DeviceElement::getDeviceName() const {
            return m_devName;
        }
        std::string const &DeviceElement::getServer() const { return m_server; }
        std::string DeviceElement::getFullDeviceName() const {
            return getDeviceName() + "@" + getServer();
        }
    } // namespace elements
} // namespace common
} // namespace osvr
