//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "WaylandHandler.h"
#include "TestWaylandApplication.h"
#include "TestSignalHandler.h"
#include "SHMTriangleDrawer.h"
#include "TestScenes/MultipleTrianglesScene.h"
#include "WaylandUtilities/WaylandEnvironmentUtils.h"

#include "TestScenesAndRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"

#include "DisplayConfigImpl.h"
#include "PlatformAbstraction/PlatformSignal.h"
#include "RendererLib/DisplayConfig.h"
#include "Utils/BinaryInputStream.h"

namespace ramses_internal
{
    TestWaylandApplication::TestWaylandApplication(const String& testToWaylandClientPipeName, const String& waylandClientToTestPipeName)
        : m_testToWaylandClientPipe(testToWaylandClientPipeName, false)
        , m_waylandClientToTestPipe(waylandClientToTestPipeName, false)
        , m_triangleColor(ETriangleColor::Red)
    {
        setTriangleColor(ETriangleColor::Red);
        TestSignalHandler::RegisterSignalHandlersForCurrentProcess("TestWaylandApplication");

        m_testToWaylandClientPipe.open();
        m_waylandClientToTestPipe.open();
    }

    TestWaylandApplication::~TestWaylandApplication()
    {
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication exiting");
    }

    Bool TestWaylandApplication::run()
    {
        while(dispatchIncomingMessage())
        {
        }

        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication done handling incoming messages");

        stopWayland();
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication closing application");
        return true;
    }

    bool TestWaylandApplication::readFromTestFramework(void* data, UInt32 numberOfBytes)
    {
        const EReadFromPipeStatus readingStatus = m_testToWaylandClientPipe.read(data, numberOfBytes);
        switch (readingStatus)
        {
        case EReadFromPipeStatus_Failure:
            LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication::readFromTestFramework(): can not read from pipe");
            return false;
        case EReadFromPipeStatus_Closed:
            LOG_WARN(CONTEXT_RENDERER, "TestWaylandApplication::readFromTestFramework(): pipe closed");
            return false;
        case EReadFromPipeStatus_Empty:
            LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication::readFromTestFramework(): pipe empty");
            return false;
        default:
            return true;
        }
    }

    //TODO Mohamed: remove!

    template <typename T>
    bool TestWaylandApplication::readFromTestFramework(T& value)
    {
        return readFromTestFramework(static_cast<void*>(&value), sizeof(value));
    }

    bool TestWaylandApplication::readBufferFromTestFramework()
    {
        UInt32 bufferSize;
        if (!readFromTestFramework(bufferSize))
            return false;

        m_readingBuffer.reserve(bufferSize);
        if (!readFromTestFramework(m_readingBuffer.data(), bufferSize))
            return false;

        return true;
    }

    Bool TestWaylandApplication::dispatchIncomingMessage()
    {
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): waiting for message...");

        if (!readBufferFromTestFramework())
            return false;
        BinaryInputStream bis(m_readingBuffer.data());

        ETestWaylandApplicationMessage message = ETestWaylandApplicationMessage::AttachBuffer;
        bis >> message;

        switch(message)
        {
        case ETestWaylandApplicationMessage::InitializeTestApplication:
        {
            String displayName;
            bool useSocketFD;

            bis >> displayName >> useSocketFD;

            if (!initializeWayland(displayName.c_str(), useSocketFD))
            {
                LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication::dispatchIncomingMessage(): failed initializing wayland");
                return false;
            }
            return true;
        }
        case ETestWaylandApplicationMessage::StopApplication:
        {
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message stop application");
            return false;
        }
        case ETestWaylandApplicationMessage::CreateSurface:
        {
            TestApplicationSurfaceId surfaceId;
            UInt32 width;
            UInt32 height;
            UInt32 swapInterval;
            Bool useEGL;

            bis >> surfaceId.getReference() >> width >> height >> swapInterval >> useEGL;

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message create surface size " << width << "/" << height << " swap interval " << swapInterval);

            bool bSuccess = m_waylandHandler.createWindow(surfaceId, width, height, swapInterval, useEGL);
            if (!bSuccess)
            {
                LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication: Could not setup Wayland window");
            }

            return true;
        }
        case ETestWaylandApplicationMessage::CreateShellSurface:
        {
            TestApplicationSurfaceId surfaceId;
            TestApplicationShellSurfaceId shellSurfaceId;

            bis >> surfaceId.getReference() >> shellSurfaceId.getReference();

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message create shell surface for surface with id " << surfaceId.getValue());
            m_waylandHandler.createShellSurface(surfaceId, shellSurfaceId);
            return true;
        }
        case ETestWaylandApplicationMessage::DestroyShellSurface:
        {
            TestApplicationShellSurfaceId shellSurfaceId;

            bis >> shellSurfaceId.getReference();

            LOG_INFO(CONTEXT_RENDERER,
                     "TestWaylandApplication::handleIncomingMessages(): received message destroy shell surface with id "
                         << shellSurfaceId.getValue());
            m_waylandHandler.destroyShellSurface(shellSurfaceId);
            return true;
        }
        case ETestWaylandApplicationMessage::SetShellSurfaceTitle:
        {
            TestApplicationShellSurfaceId shellSurfaceId;
            String title;

            bis >> shellSurfaceId.getReference() >> title;

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message set shell surface title for shell surface with id " << shellSurfaceId.getValue() << " title: " << title);
            m_waylandHandler.setShellSurfaceTitle(shellSurfaceId, title);
            return true;
        }
        case ETestWaylandApplicationMessage::SetShellSurfaceDummyValues:
        {
            TestApplicationSurfaceId      surfaceId;
            TestApplicationShellSurfaceId shellSurfaceId;
            bis >> surfaceId.getReference() >> shellSurfaceId.getReference();

            LOG_INFO(CONTEXT_RENDERER,
                     "TestWaylandApplication::handleIncomingMessages(): received message set shell surface dummy values "
                     "surface with id "
                         << surfaceId.getValue());
            m_waylandHandler.setShellSurfaceDummyValues(surfaceId, shellSurfaceId);
            return true;
        }
        case ETestWaylandApplicationMessage::DestroySurface:
        {
            TestApplicationSurfaceId surfaceId;

            bis >> surfaceId.getReference();

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message destroy surface with id " << surfaceId.getValue());
            m_waylandHandler.destroyWindow(surfaceId);
            return true;
        }
        case ETestWaylandApplicationMessage::DestroyIVISurface:
        {
            TestApplicationSurfaceId surfaceId;

            bis >> surfaceId.getReference();

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message destroy ivi surface for surface with id " << surfaceId.getValue());
            m_waylandHandler.destroyIVISurface(surfaceId);
            return true;
        }
        case ETestWaylandApplicationMessage::CreateIVISurface:
        {
            TestApplicationSurfaceId surfaceId;
            UInt32 surfaceIviId;

            bis >> surfaceId.getReference() >> surfaceIviId;

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message create ivi surface for surface with id " << surfaceId.getValue() << " ivi-id: " << surfaceIviId);
            m_waylandHandler.createIVISurface(surfaceId, surfaceIviId);
            return true;
        }
        case ETestWaylandApplicationMessage::RenderOneFrame_ToEGLBuffer:
        {
            TestApplicationSurfaceId surfaceId;
            bool useCallback;

            bis >> surfaceId.getReference() >> useCallback;

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message render one frame to egl buffer on surface with id " << surfaceId.getValue());
            renderFrameToEGLBuffer(surfaceId, useCallback);
            return true;
        }
        case ETestWaylandApplicationMessage::RenderOneFrame_ToSharedMemoryBuffer:
        {
            TestApplicationSurfaceId surfaceId;
            bool useCallback;

            bis >> surfaceId.getReference() >> useCallback;

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message render one frame to shraed memory buffer on surface with id " << surfaceId.getValue());
            renderFrameToSharedMemoryBuffer(surfaceId, useCallback);
            return true;
        }
        case ETestWaylandApplicationMessage::AttachBuffer:
        {
            TestApplicationSurfaceId surfaceId;
            bool commit;

            bis >> surfaceId.getReference() >> commit;

            LOG_INFO(CONTEXT_RENDERER,
                     "TestWaylandApplication::handleIncomingMessages(): received message attach buffer "
                     "with id "
                         << surfaceId.getValue());
            attachBuffer(surfaceId, commit);
            return true;
        }
        case ETestWaylandApplicationMessage::DestroyBuffers:
        {
            LOG_INFO(CONTEXT_RENDERER,
                     "TestWaylandApplication::handleIncomingMessages(): received message destroy buffers");
            m_waylandHandler.deleteSHMBuffers();
            return true;
        }
        case ETestWaylandApplicationMessage::SetSurfaceSize:
        {
            TestApplicationSurfaceId surfaceId;
            UInt32 width;
            UInt32 height;

            bis >> surfaceId.getReference() >> width >> height;

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message change size for surface with id  " << surfaceId.getValue() << " to " << width << "*" << height);
            m_waylandHandler.resizeWindow(surfaceId, width, height);
            return true;
        }
        case ETestWaylandApplicationMessage::SetTriangleColor:
        {
            uint32_t triangleColor;

            bis >> triangleColor;

            setTriangleColor(static_cast<ETriangleColor>(triangleColor));

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message set triangle color " <<  static_cast<uint32_t>(triangleColor));
            return true;
        }
        case ETestWaylandApplicationMessage::AdditionalConnectToEmbeddedCompositor:
        {
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message additional connect to embedded compositor");
            m_waylandHandler2.initWithSharedDisplayConnection(m_waylandHandler);
            return true;
        }

        case ETestWaylandApplicationMessage::DetachBufferFromSurface:
        {
            TestApplicationSurfaceId surfaceId;

            bis >> surfaceId.getReference();

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message detach buffer from surface with id " << surfaceId.getValue());
            detachBufferFromSurface(surfaceId);
            return true;
        }

        case ETestWaylandApplicationMessage::GetNumberOfAllocatedSHMBuffer:
        {
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message get number of allocated shm buffer");
            const UInt32 numberOfAllocatedSHMBuffer = m_waylandHandler.getNumberOfAllocatedSHMBuffer();
            sendAnswerToTestFramework(numberOfAllocatedSHMBuffer);
            return true;
        }
        case ETestWaylandApplicationMessage::RenderOneFrameToTwoSurfaces:
        {
            TestApplicationSurfaceId surfaceId1;
            TestApplicationSurfaceId surfaceId2;
            bool useCallback;

            bis >> surfaceId1.getReference() >> surfaceId2.getReference() >> useCallback;

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message render one frame to two surfaces with ivi id's " << surfaceId1.getValue() << ", " << surfaceId2.getValue());
            renderFrameToTwoSurfaces(surfaceId1, surfaceId2, useCallback);
            return true;
        }
        case ETestWaylandApplicationMessage::GetIsBufferFree:
        {
            UInt32 buffer;

            bis >> buffer;

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message get is buffer free");

            const bool isBufferFree = m_waylandHandler.getIsSHMBufferFree(buffer);
            sendAnswerToTestFramework(isBufferFree);
            if (buffer >= m_waylandHandler.getNumberOfAllocatedSHMBuffer())
            {
                LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "WaylandHandler::getIsSHMBufferFree buffer: " << buffer << " does not exist !");
                return false;
            }

            return true;
        }
        case ETestWaylandApplicationMessage::GetWaylandOutputParams:
        {
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message get wayland output params");

            bool errorsFound = false;
            WaylandOutputTestParams waylandOutputParams = {};

            m_waylandHandler.getWaylandOutputTestParams(errorsFound, waylandOutputParams);
            sendAnswerToTestFramework(errorsFound);
            sendAnswerToTestFramework(waylandOutputParams);

            return true;
        }
        case ETestWaylandApplicationMessage::SetRequiredWaylandOutputVersion:
        {
            uint32_t protocolVersion = 0u;
            bis >> protocolVersion;
            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message set required wayland output version :" << protocolVersion);

            m_waylandHandler.setRequiredWaylandOutputVersion(protocolVersion);

            return true;
        }

        case ETestWaylandApplicationMessage::StartRamsesRendererAndRunRenderingTest:
        {
            UInt32 waylandIviLayerId;
            UInt32 iviSurfaceOffset;

            bis >> waylandIviLayerId >> iviSurfaceOffset;

            LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::handleIncomingMessages(): received message start RAMSES renderer and run rendering test");

            const bool testResult = startRamsesRendererAndRunRenderingTest(WaylandIviLayerId{waylandIviLayerId}, iviSurfaceOffset);
            sendAnswerToTestFramework(testResult);
            return true;
        }

        }

        return false;
    }

    bool TestWaylandApplication::initializeWayland(const String& displayName, bool connectUsingFD)
    {
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication started");

        int socketFD = -1;
        if(connectUsingFD)
        {
            m_socket = std::make_unique<ramses_internal::UnixDomainSocket>(displayName, WaylandEnvironmentUtils::GetVariable(WaylandEnvironmentVariable::XDGRuntimeDir));
            socketFD = m_socket->createConnectedFileDescriptor(false);

            if(socketFD == -1)
            {
                LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication: Could not create file desciptor for display socket");
                return false;
            }
        }

        bool bSuccess = m_waylandHandler.init(displayName, socketFD);
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication m_waylandHandler initialized");
        if (!bSuccess)
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication: Could not setup Wayland");
        }
        return bSuccess;
    }

    void TestWaylandApplication::stopWayland()
    {
        m_waylandHandler2.deinit();
        m_waylandHandler.deinit();
    }

    void TestWaylandApplication::renderFrameToEGLBuffer(TestApplicationSurfaceId surfaceId, bool useCallback)
    {
        m_waylandHandler.enableContextForSurface(surfaceId);
        OpenGLTriangleDrawer triangleDrawer(m_triangleColor);
        triangleDrawer.draw();
        m_waylandHandler.swapBuffersAndProcessEvents(surfaceId, useCallback);
        m_waylandHandler.disableContextForSurface();
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::renderFrameToEGLBuffer: buffers swapped");
    }

    void TestWaylandApplication::renderFrameToSharedMemoryBuffer(TestApplicationSurfaceId surfaceId, bool useCallback)
    {
        uint32_t width;
        uint32_t height;
        m_waylandHandler.getWindowSize(surfaceId, width, height);
        SHMBuffer* buffer = m_waylandHandler.getFreeSHMBuffer(width, height);
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::renderFrameToSharedMemoryBuffer render to SHMBuffer with id  " << buffer->getId());
        SHMTriangleDrawer triangleDrawer(m_triangleColor);

        triangleDrawer.draw(buffer);

        m_waylandHandler.swapBuffersAndProcessEvents(surfaceId, *buffer, useCallback);
        if (useCallback)
        {
            m_waylandHandler.waitOnFrameCallback(surfaceId);
        }
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::renderFrameToSharedMemoryBuffer shm buffers swapped");
    }

    void TestWaylandApplication::attachBuffer(TestApplicationSurfaceId surfaceId, bool commit)
    {
        uint32_t width;
        uint32_t height;
        m_waylandHandler.getWindowSize(surfaceId, width, height);
        SHMBuffer* buffer = m_waylandHandler.getFreeSHMBuffer(width, height);
        LOG_INFO(CONTEXT_RENDERER,
                 "TestWaylandApplication::attachBuffer: attaching buffer" << buffer->getId());

        m_waylandHandler.attachBuffer(surfaceId, *buffer, commit);
    }

    void TestWaylandApplication::renderFrameToTwoSurfaces(TestApplicationSurfaceId surfaceId1, TestApplicationSurfaceId surfaceId2, bool useCallback)
    {
        uint32_t width1;
        uint32_t height1;
        m_waylandHandler.getWindowSize(surfaceId1, width1, height1);

        uint32_t width2;
        uint32_t height2;
        m_waylandHandler.getWindowSize(surfaceId2, width2, height2);

        if (width1 != width2 || height1 != height2)
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication::renderFrameToTwoSurfaces Surfaces must have same size!");
            assert(false);
            return;
        }

        SHMBuffer* buffer = m_waylandHandler.getFreeSHMBuffer(width1, height2);
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::renderFrame render to SHMBuffer with id  " << buffer->getId());
        SHMTriangleDrawer triangleDrawer(m_triangleColor);
        triangleDrawer.draw(buffer);

        m_waylandHandler.swapBuffersAndProcessEvents(surfaceId1, *buffer, useCallback);
        m_waylandHandler.swapBuffersAndProcessEvents(surfaceId2, *buffer, useCallback);
        if (useCallback)
        {
            m_waylandHandler.waitOnFrameCallback(surfaceId1);
            m_waylandHandler.waitOnFrameCallback(surfaceId2);
        }
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::renderFrame shm buffers swapped");
    }

    void TestWaylandApplication::setTriangleColor(ETriangleColor color)
    {
        m_triangleColor = color;
    }

    void TestWaylandApplication::detachBufferFromSurface(TestApplicationSurfaceId surfaceId)
    {
        m_waylandHandler.detachBuffer(surfaceId);
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::detachBufferFromSurface buffer detached");
    }

    bool TestWaylandApplication::startRamsesRendererAndRunRenderingTest(WaylandIviLayerId waylandIviLayerId, uint32_t iviSurfaceIdOffset)
    {
        const uint32_t windowWidth = 128u;
        const uint32_t windowHeight = 64u;

        const char* systemCompositorDisplay = "wayland-0";
        //start renderer with two displays that show content on the system compositor (not the RAMSES renderer's EC)
        ramses::RamsesFrameworkConfig config;
        TestScenesAndRenderer testScenesAndRenderer(config);
        TestRenderer& testRenderer = testScenesAndRenderer.getTestRenderer();
        RendererTestUtils::SetWaylandDisplayForSystemCompositorControllerForAllTests(systemCompositorDisplay);
        testScenesAndRenderer.initializeRenderer();

        ramses::DisplayConfig displayConfig1 = RendererTestUtils::CreateTestDisplayConfig(iviSurfaceIdOffset);
        displayConfig1.setWindowRectangle(0, 0, windowWidth, windowHeight);
        displayConfig1.impl.setWaylandDisplay(systemCompositorDisplay);
        displayConfig1.setWaylandIviLayerID(ramses::waylandIviLayerId_t(waylandIviLayerId.getValue()));
        displayConfig1.setWaylandEmbeddedCompositingSocketName("");

        ramses::DisplayConfig displayConfig2 = RendererTestUtils::CreateTestDisplayConfig(iviSurfaceIdOffset + 1);
        displayConfig2.setWindowRectangle(windowWidth, 0, windowWidth, windowHeight);
        displayConfig2.impl.setWaylandDisplay(systemCompositorDisplay);
        displayConfig2.setWaylandIviLayerID(ramses::waylandIviLayerId_t(waylandIviLayerId.getValue()));
        displayConfig2.setWaylandEmbeddedCompositingSocketName("");

        const auto displayHandle1 = testRenderer.createDisplay(displayConfig1);
        const auto displayHandle2 = testRenderer.createDisplay(displayConfig2);

        //create two scenes and map a scene to each display
        const ramses::sceneId_t sceneId1 = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f), windowWidth, windowHeight);

        testScenesAndRenderer.publish(sceneId1);
        testScenesAndRenderer.flush(sceneId1);
        testRenderer.setSceneMapping(sceneId1, displayHandle1);
        testRenderer.getSceneToState(sceneId1, ramses::RendererSceneState::Rendered);

        const ramses::sceneId_t sceneId2 = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED, ramses_internal::Vector3(0.0f, 0.0f, 5.0f), windowWidth, windowHeight);
        testScenesAndRenderer.publish(sceneId2);
        testScenesAndRenderer.flush(sceneId2);
        testRenderer.setSceneMapping(sceneId2, displayHandle2);
        testRenderer.getSceneToState(sceneId2, ramses::RendererSceneState::Rendered);

        //take screenshots and perform check to make sure that the renderer created here does render the scenes on the system compositor's surfaces
        bool testResult = testRenderer.performScreenshotCheck(displayHandle1, {}, 0u, 0u, windowWidth, windowHeight, "ARendererInstance_Three_Triangles");
        testResult &= testRenderer.performScreenshotCheck(displayHandle2, {}, 0u, 0u, windowWidth, windowHeight, "ARendererInstance_Triangles_reordered");

        //cleanup
        testScenesAndRenderer.unpublish(sceneId1);
        testScenesAndRenderer.unpublish(sceneId2);
        testRenderer.destroyDisplay(displayHandle1);
        testRenderer.destroyDisplay(displayHandle2);
        testScenesAndRenderer.destroyRenderer();

        return testResult;
    }

    template <typename T>
    void TestWaylandApplication::sendAnswerToTestFramework(const T& value)
    {
        LOG_INFO(CONTEXT_RENDERER, "TestWaylandApplication::sendAnswerToTestFramework message");
        if (!m_waylandClientToTestPipe.write(&value, sizeof(value)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestWaylandApplication::sendAnswerToTestFramework failed to write data to pipe!");
        }
    }
}
