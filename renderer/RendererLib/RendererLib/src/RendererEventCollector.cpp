//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererEventCollector.h"
#include "Utils/ThreadLocalLogForced.h"

namespace ramses_internal
{
    void RendererEventCollector::appendAndConsumePendingEvents(RendererEventVector& rendererEvents, RendererEventVector& sceneControlEvents)
    {
        rendererEvents.insert(rendererEvents.end(), m_rendererEvents.cbegin(), m_rendererEvents.cend());
        m_rendererEvents.clear();

        sceneControlEvents.insert(sceneControlEvents.end(), m_sceneControlEvents.cbegin(), m_sceneControlEvents.cend());
        m_sceneControlEvents.clear();
    }

    void RendererEventCollector::dispatchInternalSceneStateEvents(InternalSceneStateEvents& resultEvents)
    {
        m_internalSceneStateEvents.swap(resultEvents);
        m_internalSceneStateEvents.clear();
    }

    RendererEventVector RendererEventCollector::getRendererEvents() const
    {
        return m_rendererEvents;
    }

    RendererEventVector RendererEventCollector::getSceneControlEvents() const
    {
        return m_sceneControlEvents;
    }

    void RendererEventCollector::addDisplayEvent(ERendererEventType eventType, DisplayHandle displayHandle, const DisplayConfig& config)
    {
        LOG_INFO(CONTEXT_RENDERER, eventType << " display=" << displayHandle);

        RendererEvent event(eventType);
        event.displayHandle = displayHandle;
        event.displayConfig = config;
        pushToRendererEventQueue(std::move(event));
    }

    void RendererEventCollector::addReadPixelsEvent(ERendererEventType eventType, DisplayHandle displayHandle, OffscreenBufferHandle offscreenBufferHandle, UInt8Vector&& pixelData)
    {
        LOG_INFO(CONTEXT_RENDERER, eventType << " display=" << displayHandle);

        RendererEvent event(eventType);
        event.displayHandle = displayHandle;
        event.pixelData = std::move(pixelData);
        event.offscreenBuffer = offscreenBufferHandle;
        pushToRendererEventQueue(std::move(event));
    }

    void RendererEventCollector::addInternalSceneEvent(ERendererEventType eventType, SceneId sceneId)
    {
        pushToInternalSceneStateEventQueue({ eventType, sceneId });
    }

    void RendererEventCollector::addSceneEvent(ERendererEventType eventType, SceneId sceneId, RendererSceneState state)
    {
        RendererEvent event(eventType);
        event.sceneId = sceneId;
        event.state = state;
        pushToSceneControlEventQueue(std::move(event));
    }

    void RendererEventCollector::addSceneExpirationEvent(ERendererEventType eventType, SceneId sceneId)
    {
        LOG_INFO(CONTEXT_RENDERER, eventType << " sceneId=" << sceneId);

        RendererEvent event(eventType);
        event.sceneId = sceneId;
        pushToSceneControlEventQueue(std::move(event));
    }

    void RendererEventCollector::addDataLinkEvent(ERendererEventType eventType, SceneId providerSceneId, SceneId consumerSceneId, DataSlotId providerdataId, DataSlotId consumerdataId)
    {
        LOG_INFO(CONTEXT_RENDERER, eventType << " providerSceneId=" << providerSceneId << " providerDataId=" << providerdataId.getValue() << " consumerSceneId=" << consumerSceneId << " consumerDataId=" << consumerdataId.getValue());

        RendererEvent event(eventType);
        event.providerSceneId = providerSceneId;
        event.consumerSceneId = consumerSceneId;
        event.providerdataId = providerdataId;
        event.consumerdataId = consumerdataId;
        pushToSceneControlEventQueue(std::move(event));
    }

    void RendererEventCollector::addBufferEvent(ERendererEventType eventType, OffscreenBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerdataId)
    {
        LOG_INFO(CONTEXT_RENDERER, eventType << " consumerSceneId=" << consumerSceneId.getValue() << " consumerDataId=" << consumerdataId.getValue() << " offscreenBufferHandle=" << providerBuffer);

        RendererEvent event(eventType);
        event.offscreenBuffer = providerBuffer;
        event.consumerSceneId = consumerSceneId;
        event.consumerdataId = consumerdataId;
        pushToSceneControlEventQueue(std::move(event));
    }

    void RendererEventCollector::addBufferEvent(ERendererEventType eventType, StreamBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerdataId)
    {
        LOG_INFO(CONTEXT_RENDERER, eventType << " consumerSceneId=" << consumerSceneId.getValue() << " consumerDataId=" << consumerdataId.getValue() << " streamBufferHandle=" << providerBuffer);

        RendererEvent event(eventType);
        event.streamBuffer = providerBuffer;
        event.consumerSceneId = consumerSceneId;
        event.consumerdataId = consumerdataId;
        pushToSceneControlEventQueue(std::move(event));
    }

    void RendererEventCollector::addBufferEvent(ERendererEventType eventType, ExternalBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerdataId)
    {
        LOG_INFO(CONTEXT_RENDERER, eventType << " consumerSceneId=" << consumerSceneId.getValue() << " consumerDataId=" << consumerdataId.getValue() << " externalBufferHandle=" << providerBuffer);

        RendererEvent event(eventType);
        event.externalBuffer = providerBuffer;
        event.consumerSceneId = consumerSceneId;
        event.consumerdataId = consumerdataId;
        pushToSceneControlEventQueue(std::move(event));
    }

    void RendererEventCollector::addExternalBufferEvent(ERendererEventType eventType, DisplayHandle display, ExternalBufferHandle externalBufferHandle, uint32_t glTexId)
    {
        RendererEvent event(eventType);
        event.displayHandle = display;
        event.externalBuffer = externalBufferHandle;
        event.textureGlId = glTexId;

        pushToRendererEventQueue(std::move(event));
    }

    void RendererEventCollector::addOBEvent(ERendererEventType eventType, OffscreenBufferHandle buffer, DisplayHandle display, int dmaBufferFD, uint32_t dmaBufferStride)
    {
        LOG_INFO(CONTEXT_RENDERER, eventType << " display=" << display << " bufferHandle=" << buffer << " dmaBufferFD=" << dmaBufferFD << " dmaBufferStride=" << dmaBufferStride);

        RendererEvent event(eventType);
        event.offscreenBuffer = buffer;
        event.displayHandle = display;
        event.dmaBufferFD = dmaBufferFD;
        event.dmaBufferStride = dmaBufferStride;
        pushToRendererEventQueue(std::move(event));
    }

    void RendererEventCollector::addSceneFlushEvent(ERendererEventType eventType, SceneId sceneId, SceneVersionTag sceneVersionTag)
    {
        LOG_TRACE(CONTEXT_RENDERER, eventType << " sceneId=" << sceneId.getValue() << " sceneVersionTag=" << sceneVersionTag.getValue());

        RendererEvent event(eventType);
        event.sceneId = sceneId;
        event.sceneVersionTag = sceneVersionTag;
        pushToSceneControlEventQueue(std::move(event));
    }

    void RendererEventCollector::addWindowEvent(ERendererEventType eventType, DisplayHandle display, MouseEvent mouseEvent)
    {
        RendererEvent event(eventType);
        event.displayHandle = display;
        event.mouseEvent = mouseEvent;
        pushToRendererEventQueue(std::move(event));
    }

    void RendererEventCollector::addWindowEvent(ERendererEventType eventType, DisplayHandle display, KeyEvent keyEvent)
    {
        RendererEvent event(eventType);
        event.displayHandle = display;
        event.keyEvent = keyEvent;
        pushToRendererEventQueue(std::move(event));
    }

    void RendererEventCollector::addWindowEvent(ERendererEventType eventType, DisplayHandle display, ResizeEvent resizeEvent)
    {
        RendererEvent event(eventType);
        event.displayHandle = display;
        event.resizeEvent = resizeEvent;
        pushToRendererEventQueue(std::move(event));
    }

    void RendererEventCollector::addWindowEvent(ERendererEventType eventType, DisplayHandle display, WindowMoveEvent moveEvent)
    {
        RendererEvent event(eventType);
        event.displayHandle = display;
        event.moveEvent = moveEvent;
        pushToRendererEventQueue(std::move(event));
    }

    void RendererEventCollector::addStreamSourceEvent(ERendererEventType eventType, WaylandIviSurfaceId streamSourceId)
    {
        LOG_INFO(CONTEXT_RENDERER, eventType << " streamSourceId=" << streamSourceId);

        RendererEvent event(eventType);
        event.streamSourceId = streamSourceId;
        pushToSceneControlEventQueue(std::move(event));
    }

    void RendererEventCollector::addPickedEvent(ERendererEventType eventType, const SceneId& sceneId, PickableObjectIds&& pickedObjectIds)
    {
        RendererEvent event(eventType);
        event.sceneId = sceneId;
        event.pickedObjectIds = std::move(pickedObjectIds);
        pushToSceneControlEventQueue(std::move(event));
    }

    void RendererEventCollector::addFrameTimingReport(DisplayHandle display, bool isFirstDisplay, std::chrono::microseconds maxLoopTime, std::chrono::microseconds avgLooptime)
    {
        RendererEvent event{ ERendererEventType::FrameTimingReport };
        event.frameTimings.maximumLoopTimeWithinPeriod = maxLoopTime;
        event.frameTimings.averageLoopTimeWithinPeriod = avgLooptime;
        event.displayHandle = display;
        event.isFirstDisplay = isFirstDisplay;
        pushToRendererEventQueue(std::move(event));
    }

    void RendererEventCollector::pushToRendererEventQueue(RendererEvent&& newEvent)
    {
        m_rendererEvents.push_back(std::move(newEvent));
    }

    void RendererEventCollector::pushToSceneControlEventQueue(RendererEvent&& newEvent)
    {
        m_sceneControlEvents.push_back(std::move(newEvent));
    }

    void RendererEventCollector::pushToInternalSceneStateEventQueue(InternalSceneStateEvent&& newEvent)
    {
        m_internalSceneStateEvents.push_back(std::move(newEvent));
    }
}
