//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESFRAMEWORKTYPESIMPL_H
#define RAMSES_RAMSESFRAMEWORKTYPESIMPL_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/DcsmApiTypes.h"
#include "PlatformAbstraction/Hash.h"
#include "Collections/StringOutputStream.h"
#include "Common/StronglyTypedValue.h"
#include "Utils/StringOutputSpecialWrapper.h"
#include "Components/CategoryInfo.h"
#include "ramses-framework-api/CategoryInfoUpdate.h"
#include "CategoryInfoUpdateImpl.h"

MAKE_SPECIAL_STRONGLYTYPEDVALUE_PRINTABLE(ramses::ContentID, ramses::ContentID);
MAKE_SPECIAL_STRONGLYTYPEDVALUE_PRINTABLE(ramses::Category, ramses::Category);
MAKE_SPECIAL_STRONGLYTYPEDVALUE_PRINTABLE(ramses::sceneId_t, ramses::sceneId_t);
MAKE_SPECIAL_STRONGLYTYPEDVALUE_PRINTABLE(ramses::dataConsumerId_t, ramses::dataConsumerId_t);
MAKE_SPECIAL_STRONGLYTYPEDVALUE_PRINTABLE(ramses::dataProviderId_t, ramses::dataProviderId_t);

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::TechnicalContentDescriptor);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::pickableObjectId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::displayId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::displayBufferId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::resourceCacheFlag_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::binaryShaderFormatId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::waylandIviSurfaceId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::waylandIviLayerId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::streamBufferId_t);
MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::externalBufferId_t);

template <>
struct fmt::formatter<ramses::CategoryInfoUpdate> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses::CategoryInfoUpdate& categoryInfo, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}",categoryInfo.impl.getCategoryInfo());
    }
};

template <>
struct fmt::formatter<ramses::SizeInfo> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses::SizeInfo& sizeInfo, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}/{}", sizeInfo.width, sizeInfo.height);
    }
};

template <>
struct fmt::formatter<ramses::Rect> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses::Rect& rect, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}/{}/{}/{}", rect.x, rect.y, rect.width, rect.height);
    }
};

template <>
struct fmt::formatter<ramses::AnimationInformation> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses::AnimationInformation& ai, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "[{}; {}]", ai.startTime, ai.finishTime);
    }
};

template <>
struct fmt::formatter<ramses::resourceId_t> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses::resourceId_t& res, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "0x{:016X}:{:016X}", res.highPart, res.lowPart);
    }
};

namespace std
{
    template<>
    struct hash<::ramses::resourceId_t>
    {
        size_t operator()(const ::ramses::resourceId_t& rid) const
        {
            return ramses_internal::HashValue(rid.lowPart, rid.highPart);
        }
    };
}

#endif
