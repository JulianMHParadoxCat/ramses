//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMUNICATION_TCPCONNECTIONSYSTEM_H
#define RAMSES_COMMUNICATION_TCPCONNECTIONSYSTEM_H

#include "TransportCommon/ICommunicationSystem.h"
#include "TransportCommon/ConnectionStatusUpdateNotifier.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "TransportTCP/NetworkParticipantAddress.h"
#include "TransportTCP/EMessageId.h"
#include "Utils/BinaryOutputStream.h"
#include "Collections/HashSet.h"
#include "Collections/HashMap.h"
#include "TransportTCP/AsioWrapper.h"
#include <deque>


namespace ramses_internal
{
    class StatisticCollectionFramework;
    class BinaryInputStream;

    class TCPConnectionSystem final : public Runnable, public ICommunicationSystem
    {
    public:
        TCPConnectionSystem(const NetworkParticipantAddress& participantAddress, UInt32 protocolVersion, const NetworkParticipantAddress& daemonAddress, bool pureDaemon,
                            PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection,
                            std::chrono::milliseconds aliveInterval, std::chrono::milliseconds aliveTimeout);
        virtual ~TCPConnectionSystem() override;

        static Guid GetDaemonId();

        virtual bool connectServices() override;
        virtual bool disconnectServices() override;

        virtual IConnectionStatusUpdateNotifier& getRamsesConnectionStatusUpdateNotifier() override;
        virtual IConnectionStatusUpdateNotifier& getDcsmConnectionStatusUpdateNotifier() override;

        // scene
        virtual bool broadcastNewScenesAvailable(const SceneInfoVector& newScenes) override;
        virtual bool broadcastScenesBecameUnavailable(const SceneInfoVector& unavailableScenes) override;
        virtual bool sendScenesAvailable(const Guid& to, const SceneInfoVector& availableScenes) override;

        virtual bool sendSubscribeScene(const Guid& to, const SceneId& sceneId) override;
        virtual bool sendUnsubscribeScene(const Guid& to, const SceneId& sceneId) override;

        virtual bool sendInitializeScene(const Guid& to, const SceneId& sceneId) override;
        virtual bool sendSceneUpdate(const Guid& to, const SceneId& sceneId, const ISceneUpdateSerializer& serializer) override;

        virtual bool sendRendererEvent(const Guid& to, const SceneId& sceneId, const std::vector<Byte>& data) override;

        // dcsm client -> renderer
        virtual bool sendDcsmBroadcastOfferContent(ContentID contentID, Category, ETechnicalContentType technicalContentType, const std::string& friendlyName) override;
        virtual bool sendDcsmOfferContent(const Guid& to, ContentID contentID, Category, ETechnicalContentType technicalContentType, const std::string& friendlyName) override;
        virtual bool sendDcsmContentDescription(const Guid& to, ContentID contentID, TechnicalContentDescriptor technicalContentDescriptor) override;
        virtual bool sendDcsmContentReady(const Guid& to, ContentID contentID) override;
        virtual bool sendDcsmContentEnableFocusRequest(const Guid& to, ContentID contentID, int32_t focusRequest) override;
        virtual bool sendDcsmContentDisableFocusRequest(const Guid& to, ContentID contentID, int32_t focusRequest) override;
        virtual bool sendDcsmBroadcastRequestStopOfferContent(ContentID contentID) override;
        virtual bool sendDcsmBroadcastForceStopOfferContent(ContentID contentID) override;
        virtual bool sendDcsmUpdateContentMetadata(const Guid& to, ContentID contentID, const DcsmMetadata& metadata) override;

        // dcsm renderer -> client
        virtual bool sendDcsmCanvasSizeChange(const Guid& to, ContentID contentID, const CategoryInfo& categoryInfo, AnimationInformation ai) override;
        virtual bool sendDcsmContentStateChange(const Guid& to, ContentID contentID, EDcsmState status, const CategoryInfo& categoryInfo, AnimationInformation ai) override;
        virtual bool sendDcsmContentStatus(const Guid& to, ContentID contentID, uint64_t messageID, std::vector<Byte> const& message) override;

        // set service handlers
        void setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler) override;
        void setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler) override;
        void setDcsmProviderServiceHandler(IDcsmProviderServiceHandler* handler) override;
        void setDcsmConsumerServiceHandler(IDcsmConsumerServiceHandler* handler) override;

        // log triggers
        virtual void logConnectionInfo() override;
        virtual void triggerLogMessageForPeriodicLog() override;

    private:
        enum class EParticipantState
        {
            Invalid,
            Connecting,
            WaitingForHello,
            Established,
        };

        enum class EParticipantType
        {
            Client,
            Daemon,
            PureDaemon
        };

        struct OutMessage
        {
            OutMessage(const Guid& to_, EMessageId messageType_)
                : OutMessage(std::vector<Guid>({to_}), messageType_)
            {
                assert(to_.isValid());
            }

            OutMessage(const std::vector<Guid>& to_, EMessageId messageType_)
                : to(to_)
                , messageType(messageType_)
            {
                stream << static_cast<uint32_t>(0)  // fill in size later
                       << static_cast<uint32_t>(0)  // fill in protocol version later
                       << static_cast<uint32_t>(messageType);
            }

            // TODO(tobias) make move only in c++14
            OutMessage(OutMessage&&) noexcept = default;
            OutMessage(const OutMessage&) = default;
            OutMessage& operator=(const OutMessage&) = default;

            std::vector<Guid> to;
            EMessageId messageType;
            BinaryOutputStream stream;
        };

        struct Participant
        {
            Participant(const NetworkParticipantAddress& address_, asio::io_service& io_,
                        EParticipantType type_, EParticipantState state_);
            ~Participant();

            NetworkParticipantAddress address;
            asio::ip::tcp::socket socket;
            asio::steady_timer connectTimer;

            std::deque<OutMessage> outQueue;
            std::vector<Byte> currentOutBuffer;

            uint32_t lengthReceiveBuffer;
            std::vector<Byte> receiveBuffer;

            std::chrono::steady_clock::time_point lastSent;
            asio::steady_timer sendAliveTimer;
            std::chrono::steady_clock::time_point lastReceived;
            asio::steady_timer checkReceivedAliveTimer;

            EParticipantType type;
            EParticipantState state;
        };
        using ParticipantPtr = std::shared_ptr<Participant>;

        struct RunState
        {
            RunState();

            asio::io_service        m_io;
            asio::ip::tcp::acceptor m_acceptor;
            asio::ip::tcp::socket   m_acceptorSocket;
        };

        virtual void run() override;

        void doConnect(const ParticipantPtr& pp);
        void sendConnectionDescriptionOnNewConnection(const ParticipantPtr& pp);
        void doSendQueuedMessage(const ParticipantPtr& pp);
        void doTrySendAliveMessage(const ParticipantPtr& pp);
        void doReadHeader(const ParticipantPtr& pp);
        void doReadContent(const ParticipantPtr& pp);

        bool openAcceptor();
        void doAcceptIncomingConnections();

        void sendMessageToParticipant(const ParticipantPtr& pp, OutMessage msg);
        void removeParticipant(const ParticipantPtr& pp, bool reconnectWithBackoff = false);
        void addNewParticipantByAddress(const NetworkParticipantAddress& address);
        void initializeNewlyConnectedParticipant(const ParticipantPtr& pp);
        void handleReceivedMessage(const ParticipantPtr& pp);
        bool postMessageForSending(OutMessage msg);
        void updateLastReceivedTime(const ParticipantPtr& pp);
        void sendConnectorAddressExchangeMessagesForNewParticipant(const ParticipantPtr& newPp);
        void triggerConnectionUpdateNotification(Guid participant, EConnectionStatus status);

        void handleConnectionDescriptionMessage(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleConnectorAddressExchange(const ParticipantPtr& pp, BinaryInputStream& stream);

        void handleSubscribeScene(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleUnsubscribeScene(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleCreateScene(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleSceneUpdate(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handlePublishScene(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleUnpublishScene(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleSceneNotAvailable(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleRendererEvent(const ParticipantPtr& pp, BinaryInputStream& stream);

        void handleDcsmCanvasSizeChange(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleDcsmContentStateChange(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleDcsmContentStatus(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleDcsmRegisterContent(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleDcsmContentDescription(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleDcsmContentAvailable(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleDcsmCategoryContentSwitchRequest(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleDcsmRequestUnregisterContent(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleDcsmForceStopOfferContent(const ParticipantPtr& pp, BinaryInputStream& stream);
        void handleDcsmUpdateContentMetadata(const ParticipantPtr& pp, BinaryInputStream& stream);

        static const char* EnumToString(EParticipantState e);
        static const char* EnumToString(EParticipantType e);

        const NetworkParticipantAddress m_participantAddress;
        const UInt32 m_protocolVersion;
        const NetworkParticipantAddress m_daemonAddress;
        const bool m_actAsDaemon;
        const bool m_hasOtherDaemon;
        const EParticipantType m_participantType;
        const std::chrono::milliseconds m_aliveInterval;
        const std::chrono::milliseconds m_aliveIntervalTimeout;

        PlatformLock& m_frameworkLock;
        PlatformThread m_thread;
        StatisticCollectionFramework& m_statisticCollection;

        ConnectionStatusUpdateNotifier m_ramsesConnectionStatusUpdateNotifier;
        ConnectionStatusUpdateNotifier m_dcsmConnectionStatusUpdateNotifier;
        std::vector<Guid> m_connectedParticipantsForBroadcasts;

        ISceneProviderServiceHandler* m_sceneProviderHandler;
        ISceneRendererServiceHandler* m_sceneRendererHandler;
        IDcsmProviderServiceHandler* m_dcsmProviderHandler;
        IDcsmConsumerServiceHandler* m_dcsmConsumerHandler;

        std::unique_ptr<RunState>     m_runState;
        HashSet<ParticipantPtr>       m_connectingParticipants;
        HashMap<Guid, ParticipantPtr> m_establishedParticipants;
    };
}

#endif
