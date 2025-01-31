//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMPONENT_IDCSMCOMPONENT_H
#define RAMSES_COMPONENT_IDCSMCOMPONENT_H

#include "Components/DcsmTypes.h"
#include "Components/CategoryInfo.h"
#include "Components/IDcsmProviderEventHandler.h"
#include "Components/DcsmMetadata.h"
#include "ramses-framework-api/IDcsmConsumerEventHandler.h"
#include <chrono>

namespace ramses
{
    class DcsmStatusMessageImpl;
}

namespace ramses_internal
{
    class IDcsmComponent
    {
    public:
        virtual ~IDcsmComponent() = default;

        virtual bool setLocalConsumerAvailability(bool available) = 0;
        virtual bool setLocalProviderAvailability(bool available) = 0;

        virtual bool sendOfferContent(ContentID contentID, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName, bool localOnly) = 0;
        virtual bool sendContentDescription(ContentID contentID, TechnicalContentDescriptor technicalContentDescriptor) = 0;
        virtual bool sendContentReady(ContentID contentID) = 0;
        virtual bool sendContentEnableFocusRequest(ContentID contentID, int32_t focusRequest) = 0;
        virtual bool sendContentDisableFocusRequest(ContentID contentID, int32_t focusRequest) = 0;
        virtual bool sendRequestStopOfferContent(ContentID contentID) = 0;
        virtual bool sendForceStopOfferContent(ContentID contentID) = 0;

        virtual bool sendUpdateContentMetadata(ContentID contentID, const DcsmMetadata& metadata) = 0;

        virtual bool sendCanvasSizeChange(ContentID contentID, const CategoryInfo& categoryInfo, AnimationInformation ai) = 0;
        virtual bool sendContentStateChange(ContentID contentID, EDcsmState status, const CategoryInfo& categoryInfo, AnimationInformation ai) = 0;
        virtual bool sendContentStatus(ContentID contentID, ramses::DcsmStatusMessageImpl const& message) = 0;

        virtual bool dispatchProviderEvents(IDcsmProviderEventHandler& handler, std::chrono::milliseconds timeout) = 0;
        virtual bool dispatchProviderEvents(IDcsmProviderEventHandler& handler) = 0;
        virtual bool dispatchConsumerEvents(ramses::IDcsmConsumerEventHandler& handler, std::chrono::milliseconds timeout) = 0;
        virtual bool dispatchConsumerEvents(ramses::IDcsmConsumerEventHandler& handler) = 0;
    };
}

#endif
