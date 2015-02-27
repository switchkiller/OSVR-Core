/** @file
    @brief Header

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2015 Sensics, Inc.
//
// All rights reserved.
//
// (Final version intended to be licensed under
// the Apache License, Version 2.0)

#ifndef INCLUDED_ImagingComponent_h_GUID_BA26C922_01FD_43B3_8EB7_A9AB2777CEBC
#define INCLUDED_ImagingComponent_h_GUID_BA26C922_01FD_43B3_8EB7_A9AB2777CEBC

// Internal Includes
#include <osvr/Common/Export.h>
#include <osvr/Common/DeviceComponent.h>
#include <osvr/Common/SerializationTags.h>
#include <osvr/Util/ChannelCountC.h>
#include <osvr/Util/ImagingReportTypesC.h>

// Library/third-party includes
#include <vrpn_BaseClass.h>

// Standard includes
// - none

namespace osvr {
namespace common {
    namespace messages {
        class ImageRegion : public MessageRegistration<ImageRegion> {
          public:
            class MessageSerialization;
            typedef void (*MessageHandler)(void *userdata,
                                           std::string const &message);

            static const char *identifier();
        };

    } // namespace messages

    /// @brief BaseDevice component
    class ImagingComponent : public DeviceComponent {
      public:
        /// @brief Factory method
        ///
        /// Required to ensure that allocation and deallocation stay on the same
        /// side of a DLL line.
        static OSVR_COMMON_EXPORT shared_ptr<ImagingComponent>
        create(OSVR_ChannelCount numSensor);

        /// @brief Message from server to client, containing some image data.
        messages::ImageRegion imageRegion;

        OSVR_COMMON_EXPORT void sendImageData(
            OSVR_ImagingMetadata metadata, OSVR_ImageBufferElement *imageData,
            OSVR_ChannelCount sensor, OSVR_TimeValue const &timestamp);
        OSVR_COMMON_EXPORT void
        registerImageRegionHandler(vrpn_MESSAGEHANDLER handler, void *userdata);

      private:
        ImagingComponent(OSVR_ChannelCount numChan);
        virtual void m_parentSet();

        static int VRPN_CALLBACK
        handleImageRegion(void *userdata, vrpn_HANDLERPARAM p);

        OSVR_ChannelCount m_numSensor;
    };
} // namespace common
} // namespace osvr

#endif // INCLUDED_ImagingComponent_h_GUID_BA26C922_01FD_43B3_8EB7_A9AB2777CEBC