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

#ifndef INCLUDED_SerializationTraits_h_GUID_DF0CDFE0_097F_41B2_2DAE_7D4033D28D43
#define INCLUDED_SerializationTraits_h_GUID_DF0CDFE0_097F_41B2_2DAE_7D4033D28D43

// Internal Includes
// - none

// Library/third-party includes
// - none

// Standard includes
// - none

// Internal Includes
#include <osvr/Common/Endianness.h>

// Library/third-party includes
#include <boost/call_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <boost/noncopyable.hpp>

// Standard includes
#include <string>

namespace osvr {
namespace common {

    namespace serialization {
        /// @brief The default "type tag" for specifying serialization behavior.
        ///
        /// Usage of tag types and matching type traits allows us to serialize
        /// the same underlying type different ways when requested.
        template <typename T> struct DefaultSerializationTag {
            typedef T type;
        };

        /// @brief Traits class indicating how to serialize a type with a given
        /// tag. The default tag for a type `T` is `DefaultSerializationTag<T>`
        ///
        /// A traits class must implement:
        ///
        /// - `template <typename BufferType> static void
        /// buffer(BufferType &buf, typename Base::param_type val, Tag const&)`
        /// - `template <typename BufferReaderType> static void
        /// unbuffer(BufferReaderType &buf, typename Base::reference_type val,
        /// Tag const&)`
        /// - `static size_t spaceRequired(size_t existingBytes, typename
        /// Base::param_type val, Tag const&)`
        ///
        /// The tag parameter may be used to pass runtime arguments in (like a
        /// length), if required.
        ///
        /// A traits class should inherit from `BaseSerializationTraits<T>` to
        /// get useful typedefs.
        ///
        /// The dummy template parameter exists for usage of `enable_if`.
        template <typename Tag, typename Dummy = void>
        struct SerializationTraits;

        /// @brief Serialize a value to a buffer, with optional tag to specify
        /// non-default traits.
        template <typename T, typename BufferType,
                  typename Tag = DefaultSerializationTag<T> >
        inline void serializeRaw(BufferType &buf, T const &v,
                                 Tag const &tag = Tag()) {
            SerializationTraits<Tag>::buffer(buf, v, tag);
        }

        /// @brief Deserialize a value from a buffer, with optional tag to
        /// specify non-default traits.
        template <typename T, typename BufferReaderType,
                  typename Tag = DefaultSerializationTag<T> >
        inline void deserializeRaw(BufferReaderType &reader, T &v,
                                   Tag const &tag = Tag()) {
            SerializationTraits<Tag>::unbuffer(reader, v, tag);
        }

        /// @brief Get the size a value from a buffer, with optional tag to
        /// specify non-default traits.
        template <typename T, typename Tag = DefaultSerializationTag<T> >
        inline size_t getBufferSpaceRequiredRaw(size_t existingBufferSize,
                                                T const &v,
                                                Tag const &tag = Tag()) {
            return SerializationTraits<Tag>::spaceRequired(existingBufferSize,
                                                           v, tag);
        }

        /// @brief Base of serialization traits, containing useful typedefs.
        template <typename T> struct BaseSerializationTraits {
            typedef T type;
            typedef typename boost::call_traits<type>::param_type param_type;
            typedef typename boost::call_traits<type>::value_type value_type;
            typedef typename boost::call_traits<type>::reference reference_type;
        };

        /// @brief Serialization traits for a given arithmetic type (that is, a
        /// number type that has a network byte order) with a specified
        /// alignment
        template <typename T, size_t Alignment>
        struct ArithmeticSerializationTraits : BaseSerializationTraits<T> {
            typedef BaseSerializationTraits<T> Base;
            typedef T type;
            /// @brief Buffers an object of this type.
            template <typename BufferType, typename Tag>
            static void buffer(BufferType &buf, typename Base::param_type val,
                               Tag const &) {
                buf.appendAligned(hton(val), Alignment);
            }

            /// @brief Reads an object of this type from a buffer
            template <typename BufferReaderType, typename Tag>
            static void unbuffer(BufferReaderType &buf,
                                 typename Base::reference_type val,
                                 Tag const &) {
                buf.readAligned(val, Alignment);
                val = ntoh(val);
            }

            /// @brief Returns the number of bytes required for this type (and
            /// alignment padding if applicable) to be appended to a buffer of
            /// the supplied existing size.
            template <typename Tag>
            static size_t spaceRequired(size_t existingBytes,
                                        typename Base::param_type,
                                        Tag const &) {
                return computeAlignmentPadding(Alignment, existingBytes) +
                       sizeof(T);
            }
        };

        /// @brief Set up the default serialization traits for arithmetic types,
        /// aligning to their type size.
        template <typename T>
        struct SerializationTraits<
            DefaultSerializationTag<T>,
            typename boost::enable_if<boost::is_arithmetic<T> >::type>
            : ArithmeticSerializationTraits<T, sizeof(T)> {

            BOOST_STATIC_ASSERT(boost::alignment_of<T>::value == sizeof(T));
        };

        /// @brief String, length-prefixed.
        template <>
        struct SerializationTraits<DefaultSerializationTag<std::string>, void>
            : BaseSerializationTraits<std::string> {

            typedef BaseSerializationTraits<std::string> Base;
            typedef DefaultSerializationTag<std::string> tag_type;

            typedef uint32_t length_type;

            template <typename BufferType>
            static void buffer(BufferType &buf, Base::param_type val,
                               tag_type const &) {
                length_type len = val.length();
                serializeRaw(buf, len);
                buf.append(val.c_str(), len);
            }

            /// @brief Reads an object of this type from a buffer
            template <typename BufferReaderType>
            static void unbuffer(BufferReaderType &reader,
                                 Base::reference_type val, tag_type const &) {
                length_type len;
                deserializeRaw(reader, len);
                auto iter = reader.readBytes(len);
                val.assign(iter, iter + len);
            }

            /// @brief Returns the number of bytes required for this type (and
            /// alignment padding if applicable) to be appended to a buffer of
            /// the supplied existing size.
            static size_t spaceRequired(size_t existingBytes,
                                        Base::param_type val,
                                        tag_type const &) {
                return getBufferSpaceRequiredRaw(existingBytes, length_type()) +
                       val.length();
            }
        };

    } // namespace serialization

} // namespace common
} // namespace osvr
#endif // INCLUDED_SerializationTraits_h_GUID_DF0CDFE0_097F_41B2_2DAE_7D4033D28D43