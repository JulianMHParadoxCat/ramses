//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENTSCENELOGICSHADOWCOPY_H
#define RAMSES_CLIENTSCENELOGICSHADOWCOPY_H

#include "Components/ClientSceneLogicBase.h"
#include "Components/FlushTimeInformation.h"
#include "Components/ManagedResource.h"

namespace ramses_internal
{
    class ClientSceneLogicShadowCopy final : public ClientSceneLogicBase
    {
    public:
        ClientSceneLogicShadowCopy(ISceneGraphSender& sceneGraphSender, ClientScene& scene, IResourceProviderComponent& res, const Guid& clientAddress);

        virtual bool flushSceneActions(const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag) override;

    private:
        virtual void postAddSubscriber() override;
        void sendShadowCopySceneToWaitingSubscribers();

        SceneWithExplicitMemory m_sceneShadowCopy;
        FlushTimeInformation m_flushTimeInfoOfLastFlush;
        FlushTime::Clock::time_point m_effectTimeSync{FlushTime::InvalidTimestamp};
        SceneVersionTag m_lastVersionTag;
        ManagedResourceVector m_lastFlushUsedResources;
        ramses_internal::FlushTime::Clock::time_point m_lastFlushedExpirationTimestamp{ ramses_internal::FlushTime::InvalidTimestamp };
    };
}

#endif
