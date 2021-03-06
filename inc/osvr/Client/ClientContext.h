/** @file
    @brief Header

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

#ifndef INCLUDED_ContextImpl_h_GUID_9000C62E_3693_4888_83A2_0D26F4591B6A
#define INCLUDED_ContextImpl_h_GUID_9000C62E_3693_4888_83A2_0D26F4591B6A

// Internal Includes
#include <osvr/Client/Export.h>
#include <osvr/Client/ClientContext_fwd.h>
#include <osvr/Client/ClientInterfacePtr.h>
#include <osvr/Common/RouteContainer.h>
#include <osvr/Util/KeyedOwnershipContainer.h>

// Library/third-party includes
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>

// Standard includes
#include <string>
#include <vector>
#include <map>

struct OSVR_ClientContextObject : boost::noncopyable {
  public:
    typedef std::vector<osvr::client::ClientInterfacePtr> InterfaceList;
    /// @brief Destructor
    virtual ~OSVR_ClientContextObject();

    /// @brief System-wide update method.
    OSVR_CLIENT_EXPORT void update();

    /// @brief Accessor for app ID
    std::string const &getAppId() const;

    /// @brief Creates an interface object for the given path. The context
    /// retains shared ownership.
    ///
    /// @param path Path to a resource. Should be absolute.
    OSVR_CLIENT_EXPORT::osvr::client::ClientInterfacePtr
    getInterface(const char path[]);

    /// @brief Searches through this context to determine if the passed
    /// interface object has been retained, and if so, ownership is transferred
    /// to the caller.
    ///
    /// @param iface raw interface pointer (from C, usually)
    ///
    /// @returns Pointer owning the submitted interface object, or an empty
    /// pointer if NULL passed or not found.
    OSVR_CLIENT_EXPORT::osvr::client::ClientInterfacePtr
    releaseInterface(::osvr::client::ClientInterface *iface);

    InterfaceList const &getInterfaces() const { return m_interfaces; }

    /// @brief Sends a JSON route/transform object to the server.
    OSVR_CLIENT_EXPORT void sendRoute(std::string const &route);

    /// @brief Gets routing directives.
    OSVR_CLIENT_EXPORT osvr::common::RouteContainer const &getRoutes() const;

    /// @brief Gets a string parameter value.
    OSVR_CLIENT_EXPORT std::string
    getStringParameter(std::string const &path) const;

    /// @brief Sets a string parameter value.
    void setParameter(std::string const &path, std::string const &value);

    /// @brief Pass (smart-pointer) ownership of some object to the client
    /// context.
    template <typename T> void *acquireObject(T obj) {
        return m_ownedObjects.acquire(obj);
    }

    /// @brief Frees some object whose lifetime is controlled by the client
    /// context.
    ///
    /// @returns true if the object was found and released.
    OSVR_CLIENT_EXPORT bool releaseObject(void *obj);

  protected:
    /// @brief Constructor for derived class use only.
    OSVR_ClientContextObject(const char appId[]);

    osvr::common::RouteContainer m_routingDirectives;

  private:
    virtual void m_update() = 0;
    virtual void m_sendRoute(std::string const &route) = 0;
    std::string const m_appId;
    InterfaceList m_interfaces;
    std::map<std::string, std::string> m_params;

    osvr::util::KeyedOwnershipContainer m_ownedObjects;
};

#endif // INCLUDED_ContextImpl_h_GUID_9000C62E_3693_4888_83A2_0D26F4591B6A
