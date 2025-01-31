//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOW_BASE_H
#define RAMSES_WINDOW_BASE_H

#include "RendererAPI/IWindow.h"

namespace ramses_internal
{
    class DisplayConfig;
    class IWindowEventHandler;

    class Window_Base : public IWindow
    {
    public:
        Window_Base(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler, UInt32 id);

        Bool canRenderNewFrame() const override;
        void frameRendered() override;
        Int32 getPosX() const override final;
        Int32 getPosY() const override final;

        UInt32 getWidth() const override final;
        UInt32 getHeight() const override final;
        Float  getAspectRatio() const override final;

        const String& getTitle() const override final;
        void setTitle(const String& title) override;

        UInt32 getMSAASampleCount() const;

        virtual IntegrityRGLDeviceUnit getIntegrityRGLDeviceUnit() const final;
        virtual WaylandIviSurfaceId getWaylandIviSurfaceID() const override final;

        virtual bool setExternallyOwnedWindowSize(uint32_t width, uint32_t height) override;

    protected:
        String m_windowName;
        IWindowEventHandler& m_eventHandler;
        const Bool m_fullscreen;
        const Bool m_borderless;
        UInt32 m_msaaSampleCount;

        UInt32 m_width;
        UInt32 m_height;
        Int32 m_posX;
        Int32 m_posY;

        const IntegrityRGLDeviceUnit m_integrityRGLDeviceUnit;
        const WaylandIviSurfaceId m_waylandIviSurfaceID;
        const Bool m_resizable;
    };
}

#endif
