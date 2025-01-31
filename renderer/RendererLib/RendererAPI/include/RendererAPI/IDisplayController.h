//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IDISPLAYCONTROLLER_H
#define RAMSES_IDISPLAYCONTROLLER_H

#include "RendererAPI/Types.h"
#include "RendererAPI/SceneRenderExecutionIterator.h"

namespace ramses_internal
{
    class IRenderBackend;
    class IEmbeddedCompositingManager;
    struct RenderingContext;
    class RendererCachedScene;
    class Matrix44f;
    class Vector3;
    class Vector4;
    class WarpingMeshData;
    class ProjectionParams;
    class FrameTimer;

    class IDisplayController
    {
    public:
        virtual ~IDisplayController() {};

        virtual void                    handleWindowEvents() = 0;
        virtual Bool                    canRenderNewFrame() const = 0;
        virtual void                    enableContext() = 0;
        virtual void                    swapBuffers() = 0;
        virtual SceneRenderExecutionIterator renderScene(const RendererCachedScene& scene, RenderingContext& renderContext, const FrameTimer* frameTimer = nullptr) = 0;
        virtual void                    executePostProcessing() = 0;
        virtual void                    clearBuffer(DeviceResourceHandle buffer, uint32_t clearFlags, const Vector4& clearColor) = 0;

        virtual DeviceResourceHandle    getDisplayBuffer() const = 0;
        virtual IRenderBackend&         getRenderBackend() const = 0;
        virtual IEmbeddedCompositingManager& getEmbeddedCompositingManager() = 0;
        virtual UInt32                  getDisplayWidth() const = 0;
        virtual UInt32                  getDisplayHeight() const = 0;

        virtual void                    readPixels(DeviceResourceHandle renderTargetHandle, UInt32 x, UInt32 y, UInt32 width, UInt32 height, std::vector<UInt8>& dataOut) = 0;
        virtual Bool                    isWarpingEnabled() const = 0;
        virtual void                    setWarpingMeshData(const WarpingMeshData& warpingMeshData) = 0;

        virtual void                    validateRenderingStatusHealthy() const = 0;
    };
}

#endif
