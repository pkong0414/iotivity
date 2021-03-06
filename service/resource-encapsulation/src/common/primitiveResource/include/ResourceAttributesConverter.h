/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#ifndef COMMON_RESOURCEATTRIBUTESCONVERTER_H
#define COMMON_RESOURCEATTRIBUTESCONVERTER_H

#include <RCSResourceAttributes.h>

#include <OCRepresentation.h>

/** OIC namespace */
namespace OIC
{
    /** service namespace */
    namespace Service
    {
        /** detail namespace */
        namespace Detail
        {
            template< int >
            struct Int2Type {};

            template < typename T >
            struct TypeDef
            {
                typedef T type;
            };

            template< OC::AttributeType TYPE >
            struct OCBaseType;

            /** Integer type attribute */
            template< >
            struct OCBaseType< OC::AttributeType::Integer > : TypeDef< int >{ };

            /** double type attribute */
            template< >
            struct OCBaseType< OC::AttributeType::Double > : TypeDef< double > { };

            /** boolean type attribute */
            template< >
            struct OCBaseType< OC::AttributeType::Boolean > : TypeDef< bool > { };

            /** string type attribute */
            template< >
            struct OCBaseType< OC::AttributeType::String > : TypeDef< std::string > { };

            /** binary type attribute */
            template< >
            struct OCBaseType< OC::AttributeType::Binary > : TypeDef< RCSByteString::DataType > { };

            /** string type attribute */
            template< >
            struct OCBaseType< OC::AttributeType::OCByteString > : TypeDef< OCByteString > { };

            template< >
            struct OCBaseType< OC::AttributeType::OCRepresentation >
                : TypeDef< OC::OCRepresentation >
            {};

            template< int DEPTH, typename BASE_TYPE >
            struct SeqType
            {
                typedef std::vector< typename SeqType< DEPTH - 1, BASE_TYPE >::type > type;
            };

            template< typename BASE_TYPE >
            struct SeqType< 0, BASE_TYPE >
            {
                typedef BASE_TYPE type;
            };

            template< int DEPTH, OC::AttributeType BASE_TYPE >
            struct OCItemType
            {
                typedef typename SeqType< DEPTH,
                                        typename OCBaseType< BASE_TYPE >::type >::type type;
            };

            template< typename T >
            struct TypeInfo
            {
                typedef T type;
                typedef T base_type;
                constexpr static size_t depth = 0;
            };

            template< typename T >
            struct TypeInfo< std::vector< T > >
            {
                typedef T type;
                typedef typename TypeInfo< T >::base_type base_type;
                constexpr static size_t depth = 1 + TypeInfo< T >::depth;
            };
        }

        class ResourceAttributesConverter
        {
        private:
            ResourceAttributesConverter() = delete;

            class ResourceAttributesBuilder
            {
            private:
                template < int DEPTH >
                void insertItem(Detail::Int2Type< DEPTH >,
                        const OC::OCRepresentation::AttributeItem& item)
                {
                    switch (item.base_type())
                    {
                        case OC::AttributeType::Null:
                            return putValue(item.attrname(), nullptr);

                        case OC::AttributeType::Integer:
                            return insertItem< DEPTH, OC::AttributeType::Integer >(item);

                        case OC::AttributeType::Double:
                            return insertItem< DEPTH, OC::AttributeType::Double >(item);

                        case OC::AttributeType::Boolean:
                            return insertItem< DEPTH, OC::AttributeType::Boolean >(item);

                        case OC::AttributeType::String:
                            return insertItem< DEPTH, OC::AttributeType::String >(item);

                        case OC::AttributeType::Binary:
                            // OCRep support only 0-depth for binary type.
                            // If RI changed, this line should be changed to DEPTH.
                            return insertOcBinary< OC::AttributeType::Binary >
                            (Detail::Int2Type< 0 >{ }, item);

                        case OC::AttributeType::OCByteString:
                            return insertOcBinary< OC::AttributeType::OCByteString >
                            (Detail::Int2Type< DEPTH >{ }, item);

                        case OC::AttributeType::OCRepresentation:
                            return insertOcRep(Detail::Int2Type< DEPTH >{ }, item);

                        default:
                            assert("There must be no another base type!");
                    }
                }

                template< int DEPTH, OC::AttributeType BASE_TYPE >
                void insertItem(const OC::OCRepresentation::AttributeItem& item)
                {
                    typedef typename Detail::OCItemType< DEPTH, BASE_TYPE >::type ItemType;
                    putValue(item.attrname(), item.getValue< ItemType >());
                }

                RCSResourceAttributes insertOcRep(Detail::Int2Type< 0 >,
                        const OC::OCRepresentation& ocRep)
                {
                    return ResourceAttributesConverter::fromOCRepresentation(ocRep);
                }

                template< int DEPTH, typename OCREPS,
                    typename ATTRS = typename Detail::SeqType< DEPTH, RCSResourceAttributes >::type >
                ATTRS insertOcRep(Detail::Int2Type< DEPTH >, const OCREPS& ocRepVec)
                {
                    ATTRS result;

                    for (const auto& nested : ocRepVec)
                    {
                        result.push_back(insertOcRep(Detail::Int2Type< DEPTH - 1 >{ }, nested));
                    }

                    return result;
                }

                template< int DEPTH >
                void insertOcRep(Detail::Int2Type< DEPTH >,
                        const OC::OCRepresentation::AttributeItem& item)
                {
                    typedef typename Detail::OCItemType< DEPTH,
                            OC::AttributeType::OCRepresentation >::type ItemType;

                    putValue(item.attrname(),
                            insertOcRep(Detail::Int2Type< DEPTH >{ }, item.getValue< ItemType >()));
                }

                template< typename OCREP >
                RCSByteString insertOcBinary(Detail::Int2Type< 0 >, const OCREP& ocBinary)
                {
                    return RCSByteString(ocBinary);
                }

                template< int DEPTH, typename OCREPS,
                    typename ATTRS = typename Detail::SeqType< DEPTH, RCSByteString >::type >
                ATTRS insertOcBinary(Detail::Int2Type< DEPTH >, const OCREPS& ocBinaryVec)
                {
                    ATTRS result;

                    for (const auto& nested : ocBinaryVec)
                    {
                        result.push_back(insertOcBinary(Detail::Int2Type< DEPTH - 1 >{ }, nested));
                    }

                    return result;
                }

                template< OC::AttributeType BASE_TYPE, int DEPTH >
                void insertOcBinary(Detail::Int2Type< DEPTH >,
                        const OC::OCRepresentation::AttributeItem& item)
                {
                    typedef typename Detail::OCItemType< DEPTH, BASE_TYPE >::type ItemType;

                    putValue(item.attrname(),
                             insertOcBinary(Detail::Int2Type< DEPTH >{ },
                                            item.getValue< ItemType >()));
                }

            public:
                ResourceAttributesBuilder() = default;

                /** insert items */
                void insertItem(const OC::OCRepresentation::AttributeItem& item)
                {
                    switch (item.depth())
                    {
                        case 0:
                            return insertItem(Detail::Int2Type< 0 >{ }, item);
                        case 1:
                            return insertItem(Detail::Int2Type< 1 >{ }, item);
                        case 2:
                            return insertItem(Detail::Int2Type< 2 >{ }, item);
                        case 3:
                            return insertItem(Detail::Int2Type< 3 >{ }, item);

                        default:
                            assert("There must be no another depth!");
                    }
                }

                /**
                 * extract the atribute value
                 * @return resource attribute value
                 */
                RCSResourceAttributes&& extract()
                {
                    return std::move(m_target);
                }

            private:
                template< typename T >
                void putValue(const std::string& key, T&& value)
                {
                    m_target[key] = std::forward< T >(value);
                }

            private:
                RCSResourceAttributes m_target;
            };

            class OCRepresentationBuilder
            {
            public:
                /** constructor */
                OCRepresentationBuilder() = default;

                template< typename T, typename B = typename Detail::TypeInfo< T >::base_type >
                typename std::enable_if< (
                !std::is_same< B, RCSResourceAttributes >::value &&
                !std::is_same< B, RCSByteString >::value
                )>::type
                operator()(const std::string& key, const T& value)
                {
                    m_target[key] = value;
                }

                template< typename T, typename I = Detail::TypeInfo< T > >
                typename std::enable_if< std::is_same< typename I::base_type,
                                                RCSResourceAttributes >::value >::type
                operator()(const std::string& key, const T& value)
                {
                    m_target[key] = convertAttributes(Detail::Int2Type< I::depth >{ }, value);
                }

                template< typename T, typename I = Detail::TypeInfo< T > >
                typename std::enable_if< std::is_same< typename I::base_type,
                                                RCSByteString >::value >::type
                operator()(const std::string& key, const T& value)
                {
                    m_target[key] = convertByteString(Detail::Int2Type< I::depth >{ }, value);
                }

                void operator()(const std::string& key, const std::nullptr_t&)
                {
                    m_target.setNULL(key);
                }

                /**
                 * convert the attribute to the oc representation formate
                 * @param attrs   resource attributes
                 * @return converted attributes value
                 */
                OC::OCRepresentation convertAttributes(Detail::Int2Type< 0 >,
                        const RCSResourceAttributes& attrs)
                {
                    return ResourceAttributesConverter::toOCRepresentation(attrs);
                }

                template< int DEPTH, typename ATTRS, typename OCREPS = typename Detail::SeqType<
                        DEPTH, OC::OCRepresentation >::type >
                OCREPS convertAttributes(Detail::Int2Type< DEPTH >, const ATTRS& attrs)
                {
                    OCREPS result;

                    for (const auto& nested : attrs)
                    {
                        result.push_back(
                                convertAttributes(Detail::Int2Type< DEPTH - 1 >{ }, nested));
                    }

                    return result;
                }

                /**
                 * convert to byte string value
                 * @param bytestring    byte string value
                 * @return converted bytestring
                 */
                OCByteString convertByteString(Detail::Int2Type< 0 >,
                        const RCSByteString& byteString)
                {
                    OCByteString blob;
                    blob.len = byteString.size();
                    blob.bytes = new uint8_t[blob.len];
                    for (size_t i = 0; i < blob.len; ++i)
                    {
                        blob.bytes[i] = byteString[i];
                    }

                    return blob;
                }

                template< int DEPTH, typename ATTRS, typename OCREPS = typename Detail::SeqType<
                        DEPTH, OCByteString >::type >
                OCREPS convertByteString(Detail::Int2Type< DEPTH >, const ATTRS& byteStringVec)
                {
                    OCREPS result;

                    for (const auto& nested : byteStringVec)
                    {
                        result.push_back(
                                convertByteString(Detail::Int2Type< DEPTH - 1 >{ }, nested));
                    }

                    return result;
                }

                /**
                 * This method is to extract the value
                 * @return extracted value
                 */
                OC::OCRepresentation&& extract()
                {
                    return std::move(m_target);
                }

            private:
                OC::OCRepresentation m_target;
            };

        public:
            static RCSResourceAttributes fromOCRepresentation(
                    const OC::OCRepresentation& ocRepresentation)
            {
                ResourceAttributesBuilder builder;

                for (const auto& item : ocRepresentation)
                {
                    builder.insertItem(item);
                }

                return builder.extract();
            }

            static OC::OCRepresentation toOCRepresentation(
                    const RCSResourceAttributes& resourceAttributes)
            {
                OCRepresentationBuilder builder;

                resourceAttributes.visit(builder);

                return builder.extract();
            }
        };

    }
}

#endif // COMMON_RESOURCEATTRIBUTESCONVERTER_H
