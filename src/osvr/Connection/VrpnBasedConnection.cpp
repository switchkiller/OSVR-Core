/** @file
    @brief Implementation

    @date 2014

    @author
    Ryan Pavlik
    <ryan@sensics.com>
    <http://sensics.com>
*/

// Copyright 2014 Sensics, Inc.
//
// All rights reserved.
//
// (Final version intended to be licensed under
// the Apache License, Version 2.0)

// Internal Includes
#include "VrpnBasedConnection.h"
#include "VrpnMessageType.h"
#include "VrpnConnectionDevice.h"
#include "VrpnConnectionKind.h"
#include <osvr/Util/Verbosity.h>

// Library/third-party includes
// - none

// Standard includes
#include <memory>
#include <functional>

namespace osvr {
namespace connection {
    namespace messageid {
        /// @brief Return the string identifying VRPN ping messages
        const char *vrpnPing() { return "vrpn_Base ping_message"; }
    } // namespace messageid

    VrpnBasedConnection::VrpnBasedConnection(ConnectionType type) {
        switch (type) {
        case VRPN_LOCAL_ONLY: {
            m_initConnection("localhost");
            break;
        }
        case VRPN_SHARED: {
            m_initConnection();
            break;
        }
        }
    }

    VrpnBasedConnection::VrpnBasedConnection(
        boost::optional<std::string const &> iface, boost::optional<int> port) {
        int myPort = port.get_value_or(0);
        if (iface && !(iface->empty())) {
            m_initConnection(iface->c_str(), myPort);
        } else {
            m_initConnection(nullptr, myPort);
        }
    }

    void VrpnBasedConnection::m_initConnection(const char iface[], int port) {
        if (0 == port) {
            port = vrpn_DEFAULT_LISTEN_PORT_NO;
        }
        m_vrpnConnection = vrpn_ConnectionPtr::create_server_connection(
            port, nullptr, nullptr, iface);
    }

    MessageTypePtr
    VrpnBasedConnection::m_registerMessageType(std::string const &messageId) {
        MessageTypePtr ret(new VrpnMessageType(messageId, m_vrpnConnection));
        return ret;
    }

    ConnectionDevicePtr
    VrpnBasedConnection::m_registerDevice(std::string const &deviceName) {
        ConnectionDevicePtr ret =
            make_shared<VrpnConnectionDevice>(deviceName, m_vrpnConnection);
        return ret;
    }

    void VrpnBasedConnection::m_registerConnectionHandler(
        std::function<void()> handler) {
        if (m_connectionHandlers.empty()) {
            /// got connection handler
            m_vrpnConnection->register_handler(
                m_vrpnConnection->register_message_type(vrpn_got_connection),
                &m_connectionHandler, static_cast<void *>(this),
                vrpn_ANY_SENDER);
/// Ping handler
#if 0
            m_vrpnConnection->register_handler(
                m_vrpnConnection->register_message_type(messageid::vrpnPing()),
                &m_connectionHandler, static_cast<void *>(this),
                vrpn_ANY_SENDER);
#endif
        }
        m_connectionHandlers.push_back(handler);
    }
    void VrpnBasedConnection::m_registerMessageHandler(
        GeneralMessageHandler const &handler, std::string const &device,
        std::string const &messageType) {

        boost::optional<vrpn_int32> devID;
        if (!device.empty()) {
            devID = m_vrpnConnection->register_sender(device.c_str());
        }
        boost::optional<vrpn_int32> msgType;
        if (!messageType.empty()) {
            msgType = m_vrpnConnection->register_message_type(device.c_str());
        }
        unique_ptr<HandlerRecord> record(new HandlerRecord(handler, *this));

        m_vrpnConnection->register_handler(
            msgType.get_value_or(vrpn_ANY_SENDER),
            &VrpnBasedConnection::m_messageHandler, record.get(),
            devID.get_value_or(vrpn_ANY_SENDER));
        m_generalMessageHandlers.emplace_back(record.release());
    }

    int VrpnBasedConnection::m_messageHandler(void *userdata,
                                              vrpn_HANDLERPARAM param) {
        HandlerRecord *record = static_cast<HandlerRecord *>(userdata);
        std::string msgType(
            record->self->m_vrpnConnection->message_type_name(param.type));
        std::string sender(
            record->self->m_vrpnConnection->sender_name(param.sender));
        OSVR_TimeValue timestamp;
        osvrStructTimevalToTimeValue(&timestamp, &(param.msg_time));
        std::string msg(param.buffer, param.payload_len);
        record->handler(sender, msgType, timestamp, msg);
        return 0;
    }

    int VrpnBasedConnection::m_connectionHandler(void *userdata,
                                                 vrpn_HANDLERPARAM) {
        VrpnBasedConnection *conn =
            static_cast<VrpnBasedConnection *>(userdata);
        for (auto const &f : conn->m_connectionHandlers) {
            f();
        }
        return 0;
    }
    void VrpnBasedConnection::m_process() { m_vrpnConnection->mainloop(); }

    VrpnBasedConnection::~VrpnBasedConnection() {
        /// @todo wait until all async threads are done
    }

    void *VrpnBasedConnection::getUnderlyingObject() {
        return static_cast<void *>(m_vrpnConnection.get());
    }

    const char *VrpnBasedConnection::getConnectionKindID() {
        return getVRPNConnectionKindID();
    }

} // namespace connection
} // namespace osvr