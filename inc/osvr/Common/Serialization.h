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

#ifndef INCLUDED_Serialization_h_GUID_339B4851_CA90_41A6_E1CC_4F94A18614AC
#define INCLUDED_Serialization_h_GUID_339B4851_CA90_41A6_E1CC_4F94A18614AC

// Internal Includes
#include <osvr/Common/Buffer.h>
#include <osvr/Common/Endianness.h>
#include <osvr/Util/StdInt.h>

// Library/third-party includes
#include <boost/call_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>

// Standard includes
#include <string>

namespace osvr {
namespace common {

    namespace serialization {
        template <typename T, typename BufferType>
        inline typename boost::enable_if<boost::is_integral<T> >::type
        serialize(BufferType &buf, T const val) {
            /// integer - handle endianness.
            bufferAppend(buf, hton(val));
        }
        template <typename T, typename BufferType>
        inline typename boost::enable_if<boost::is_floating_point<T> >::type
        serialize(BufferType &buf, T const val) {
            /// float - handle endianness.
            bufferAppend(buf, hton(val));
        }

        template <typename T, typename BufferType>
        inline typename boost::enable_if<boost::is_integral<T> >::type
        deserialize(T &val, BufferType const &buf,
                    typename BufferType::const_iterator &it) {
            /// Integer - handle endianness.
        }

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
        /// The dummy template parameter exists for usage of `enable_if`.
        template <typename Tag, typename Dummy = void>
        struct SerializationTraits;

        template <typename T, size_t Alignment>
        struct DefaultSerializationTraits {
            typedef typename boost::call_traits<T>::param_type param_type;
            typedef typename boost::call_traits<T>::value_type value_type;
            typedef typename boost::call_traits<T>::reference reference;

            BOOST_STATIC_ASSERT(Alignment > 1);

            static size_t paddingRequired(size_t existingBufferLength) {
                auto leftover = existingBufferLength % Alignment;
                return (leftover == 0) ? 0 : (Alignment - leftover);
            }
            static size_t spaceRequired(size_t existingBufferLength) {
                return paddingRequired(existingBufferLength) + sizeof(T);
            }
            /// @brief Buffers an object of this type.
            template <typename BufferWrapperType>
            static void buffer(BufferWrapperType &buf, param_type val) {
                buf.appendPadding(paddingRequired(buf.size()));
                buf.append(hton(val));
            }

            /// @brief Reads an object of this type from a buffer
            template <typename BufferReaderType>
            static void unbuffer(BufferReaderType &buf, reference val) {
                buf.skipPadding(paddingRequired(buf.bytesRead()));
                buf.read(val);
                val = ntoh(val);
            }
        };
        template <typename T>
        struct SerializationTraits<
            DefaultSerializationTag<T>,
            typename boost::enable_if<boost::is_integral<T> >::type> {};

        class SerializeFunctor : boost::noncopyable {
          public:
            /// @brief Constructor, taking the buffer
            SerializeFunctor(BufferWrapper<> &buf) : m_buf(buf) {}

            /// @brief Main function call operator method, optionally taking a
            /// "tag type" to specify serialization-related behavior.
            ///
            /// @param v The value to process - in this case, to add to the
            /// buffer.
            template <typename T, typename Tag = DefaultSerializationTag<T> >
            void operator()(typename boost::call_traits<T>::param_type v,
                            Tag const & = Tag()) {
                SerializationTraits<Tag>::buffer(m_buf, v);
            }

          private:
            BufferWrapper<> &m_buf;
        };

        class DeserializeFunctor : boost::noncopyable {
          public:
            typedef BufferWrapper<>::Reader Reader;
            /// @brief Constructor, taking the buffer reader
            DeserializeFunctor(Reader &reader) : m_reader(reader) {}

            /// @brief Main function call operator method, optionally taking a
            /// "tag type" to specify serialization-related behavior.
            ///
            /// @param v The value to process - in this case, to add to the
            /// buffer.
            template <typename T, typename Tag = DefaultSerializationTag<T> >
            void operator()(typename boost::call_traits<T>::reference v,
                            Tag const & = Tag()) {
                SerializationTraits<Tag>::unbuffer(m_reader, v);
            }

          private:
            Reader &m_reader;
        };

    } // namespace serialization
    /// @brief Template class, using the CRTP to produce message
    /// serialization/deserialization code. Your class should inherit from this
    /// class (with your class as the template parameter), and implement a
    /// method `template<typename T> void processMessage(T & process)` that
    /// calls the function call operator with each member and an optional tag
    /// type.
    template <typename Base> class MessageSerializationBase {
      public:
        /// @brief Serialize to buffer.
        void serialize(BufferWrapper<> &buf) {
            serialization::SerializeFunctor functor(buf);
            static_cast<Base *>(this)->processMessage(functor);
        }
    };
} // namespace common
} // namespace osvr

#endif // INCLUDED_Serialization_h_GUID_339B4851_CA90_41A6_E1CC_4F94A18614AC