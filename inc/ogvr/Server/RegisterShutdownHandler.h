/** @file
    @brief Header

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

#ifndef INCLUDED_RegisterShutdownHandler_h_GUID_3663ABF6_8788_4BB7_CA9F_3609E2EC7009
#define INCLUDED_RegisterShutdownHandler_h_GUID_3663ABF6_8788_4BB7_CA9F_3609E2EC7009

// Internal Includes
// - none

// Library/third-party includes
// - none

// Standard includes
// - none

namespace ogvr {
namespace server {
    /// @brief The type of a shutdown handler callback.
    typedef void (*ShutdownHandler)();
} // namespace server
} // namespace ogvr

#if !defined(OGVR_USE_WIN32_SHUTDOWN_HANDLER) &&                               \
    !defined(OGVR_USE_POSIX_SIGNAL_SHUTDOWN_HANDLER)

#ifdef _WIN32
#define OGVR_USE_WIN32_SHUTDOWN_HANDLER
#else
/// @todo the world isn't just Win32 and POSIX signal()
#define OGVR_USE_POSIX_SIGNAL_SHUTDOWN_HANDLER
#endif

#endif // !defined any config flag

#if defined(OGVR_USE_WIN32_SHUTDOWN_HANDLER)
#include <ogvr/Server/RegisterShutdownHandlerWin32.h>

#elif defined(OGVR_USE_POSIX_SIGNAL_SHUTDOWN_HANDLER)
#include <ogvr/Server/RegisterShutdownHandlerPOSIXSignal.h>
#endif

#endif // INCLUDED_RegisterShutdownHandler_h_GUID_3663ABF6_8788_4BB7_CA9F_3609E2EC7009
