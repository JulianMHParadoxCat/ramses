//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererConfigUtils.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/DisplayConfig.h"
#include "Utils/LogMacros.h"
#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "Utils/StringUtils.h"

namespace ramses_internal
{
    struct RendererCommandLineArguments
    {
        explicit RendererCommandLineArguments(const RendererConfig& config)
            : waylandSocketEmbedded("wse", "wayland-socket-embedded", config.getWaylandSocketEmbedded(), "set socket name clients use to connect to the compositor embedded in the renderer")
            , waylandSocketEmbeddedGroup("wsegn", "wayland-socket-embedded-groupname", config.getWaylandSocketEmbeddedGroup(), "groupname for embedded compositing socket")
            , waylandSocketEmbeddedPermissions("wsep", "wayland-socket-embedded-permissions", config.getWaylandSocketEmbeddedPermissions(), "permissions for embedded compositing socket")
            , systemCompositorControllerEnabled("scc", "enable-system-compositor-controller", "enable system compositor controller")
            , kpiFilename("kpi", "kpioutputfile", config.getKPIFileName(), "KPI filename")
        {
        }

        ArgumentString waylandSocketEmbedded;
        ArgumentString waylandSocketEmbeddedGroup;
        ArgumentUInt32 waylandSocketEmbeddedPermissions;
        ArgumentBool   systemCompositorControllerEnabled;
        ArgumentString kpiFilename;

        void print()
        {
            LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos) {
                        sos << "\nRenderer arguments:\n";

                        sos << waylandSocketEmbedded.getHelpString();
                        sos << waylandSocketEmbeddedGroup.getHelpString();
                        sos << waylandSocketEmbeddedPermissions.getHelpString();
                        sos << kpiFilename.getHelpString();
                        sos << systemCompositorControllerEnabled.getHelpString();
                    }));

        }
    };

    struct DisplayCommandLineArguments
    {
        explicit DisplayCommandLineArguments(const DisplayConfig& config)
            : windowPositionX("x", "position-x", config.getWindowPositionX(), "set x position of window")
            , windowPositionY("y", "position-y", config.getWindowPositionY(), "set y position of window")
            , windowWidth("w", "width", config.getDesiredWindowWidth(), "set window width")
            , windowHeight("h", "height", config.getDesiredWindowHeight(), "set window height")
            , fullscreen("f", "fullscreen", "enable fullscreen mode")
            , borderless("bl", "borderless", "disable window borders")
            , enableWarping("warp", "enable-warping", "enable warping")
            , deleteEffects("de", "delete-effects", "do not keep effects uploaded")
            , antialiasingMethod("aa", "antialiasing-method", "", "set antialiasing method (options: MSAA)")
            , antialiasingSampleCount("as", "aa-samples", config.getAntialiasingSampleCount(), "set antialiasing sample count")
            , waylandIviLayerId("lid", "waylandIviLayerId", config.getWaylandIviLayerID().getValue(), "set id of IVI layer the display surface will be added to")
            , waylandIviSurfaceID("sid", "waylandIviSurfaceID", config.getWaylandIviSurfaceID().getValue(), "set id of IVI surface the display will be composited on")
            , integrityRGLDeviceUnit("rglDeviceUnit", "integrityRGLDeviceUnit", config.getIntegrityRGLDeviceUnit().getValue(), "set id of the device unit to use on Integrity")
            , startVisible("startVisible", "startVisible", "set IVI surface visible when created")
            , resizable("resizableWindow", "resizable window", "enables resizable renderer window")
            , clearColorR("ccr", "clearColorR", config.getClearColor().r, "set r component of clear color")
            , clearColorG("ccg", "clearColorG", config.getClearColor().g, "set g component of clear color")
            , clearColorB("ccb", "clearColorB", config.getClearColor().b, "set b component of clear color")
            , clearColorA("cca", "clearColorA", config.getClearColor().a, "set a component of clear color")
        {
        }

        ArgumentInt32 windowPositionX;
        ArgumentInt32 windowPositionY;
        ArgumentInt32 windowWidth;
        ArgumentInt32 windowHeight;
        ArgumentBool fullscreen;
        ArgumentBool borderless;

        ArgumentBool enableWarping;
        ArgumentBool deleteEffects;
        ArgumentString antialiasingMethod;
        ArgumentUInt32 antialiasingSampleCount;
        ArgumentUInt32 waylandIviLayerId;
        ArgumentUInt32 waylandIviSurfaceID;
        ArgumentUInt32 integrityRGLDeviceUnit;
        ArgumentBool startVisible;
        ArgumentBool resizable;
        ArgumentFloat clearColorR;
        ArgumentFloat clearColorG;
        ArgumentFloat clearColorB;
        ArgumentFloat clearColorA;

        void print(Bool onlyExposedArgs = false)
        {
            LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos) {
                        sos << "\nDisplay arguments:\n";
                        sos << windowPositionX.getHelpString();
                        sos << windowPositionY.getHelpString();
                        sos << windowWidth.getHelpString();
                        sos << windowHeight.getHelpString();
                        sos << fullscreen.getHelpString();
                        sos << borderless.getHelpString();
                        sos << waylandIviLayerId.getHelpString();
                        sos << waylandIviSurfaceID.getHelpString();
                        sos << integrityRGLDeviceUnit.getHelpString();
                        sos << startVisible.getHelpString();
                        sos << clearColorR.getHelpString();
                        sos << clearColorG.getHelpString();
                        sos << clearColorB.getHelpString();
                        sos << clearColorA.getHelpString();

                        if (!onlyExposedArgs)
                        {
                            sos << enableWarping.getHelpString();
                            sos << deleteEffects.getHelpString();
                            sos << antialiasingMethod.getHelpString();
                            sos << antialiasingSampleCount.getHelpString();
                        }
                    }));
        }
    };

    void RendererConfigUtils::ApplyValuesFromCommandLine(const CommandLineParser& parser, RendererConfig& config)
    {
        RendererCommandLineArguments rendererArgs(config);
        config.setWaylandEmbeddedCompositingSocketName(rendererArgs.waylandSocketEmbedded.parseValueFromCmdLine(parser));
        config.setWaylandEmbeddedCompositingSocketGroup(rendererArgs.waylandSocketEmbeddedGroup.parseValueFromCmdLine(parser));
        config.setWaylandEmbeddedCompositingSocketPermissions(rendererArgs.waylandSocketEmbeddedPermissions.parseValueFromCmdLine(parser));
        config.setKPIFileName(rendererArgs.kpiFilename.parseValueFromCmdLine(parser));

        if(rendererArgs.systemCompositorControllerEnabled.parseFromCmdLine(parser))
        {
            config.enableSystemCompositorControl();
        }
    }

    void RendererConfigUtils::ApplyValuesFromCommandLine(const CommandLineParser& parser, DisplayConfig& config)
    {
        DisplayCommandLineArguments rendererArgs(config);

        config.setFullscreenState(rendererArgs.fullscreen.parseFromCmdLine(parser));
        config.setBorderlessState(rendererArgs.borderless.parseFromCmdLine(parser));
        config.setWarpingEnabled(rendererArgs.enableWarping.parseFromCmdLine(parser));
        config.setKeepEffectsUploaded(!rendererArgs.deleteEffects.parseFromCmdLine(parser));
        config.setDesiredWindowWidth(rendererArgs.windowWidth.parseValueFromCmdLine(parser));
        config.setDesiredWindowHeight(rendererArgs.windowHeight.parseValueFromCmdLine(parser));
        config.setWindowPositionX(rendererArgs.windowPositionX.parseValueFromCmdLine(parser));
        config.setWindowPositionY(rendererArgs.windowPositionY.parseValueFromCmdLine(parser));

        // obsolete way of setting MSAA via string, use 2 as default sample count
        const String antialiasingMethodName = rendererArgs.antialiasingMethod.parseValueFromCmdLine(parser);
        if (antialiasingMethodName == "MSAA")
            config.setAntialiasingSampleCount(2u);
        const UInt8 sampleCount = static_cast<UInt8>(rendererArgs.antialiasingSampleCount.parseValueFromCmdLine(parser));
        if (rendererArgs.antialiasingSampleCount.wasDefined())
        {
            if (!ramses_internal::contains_c<uint32_t>({ 1u, 2u, 4u, 8u }, sampleCount))
            {
                LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "Invalid MSAA sample count command line argument, sample count must be 1, 2, 4 or 8!");
            }
            else
                config.setAntialiasingSampleCount(sampleCount);
        }

        config.setWaylandIviLayerID(WaylandIviLayerId(rendererArgs.waylandIviLayerId.parseValueFromCmdLine(parser)));
        config.setWaylandIviSurfaceID(WaylandIviSurfaceId(rendererArgs.waylandIviSurfaceID.parseValueFromCmdLine(parser)));
        config.setIntegrityRGLDeviceUnit(IntegrityRGLDeviceUnit(rendererArgs.integrityRGLDeviceUnit.parseValueFromCmdLine(parser)));
        config.setStartVisibleIvi(rendererArgs.startVisible.parseFromCmdLine(parser));
        config.setResizable(rendererArgs.resizable.parseFromCmdLine(parser));

        const Vector4 clearColor = Vector4(
            rendererArgs.clearColorR.parseValueFromCmdLine(parser),
            rendererArgs.clearColorG.parseValueFromCmdLine(parser),
            rendererArgs.clearColorB.parseValueFromCmdLine(parser),
            rendererArgs.clearColorA.parseValueFromCmdLine(parser));
        config.setClearColor(clearColor);
    }

    void RendererConfigUtils::PrintCommandLineOptions()
    {
        const RendererConfig defaultConfig;
        RendererCommandLineArguments rendererArgs(defaultConfig);
        rendererArgs.print();

        const DisplayConfig defaultDisplayConfig;
        DisplayCommandLineArguments displayArgs(defaultDisplayConfig);
        displayArgs.print(true);
    }
}
