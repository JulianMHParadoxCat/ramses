//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// internal
#include "ResourceImpl.h"
#include "RamsesClientImpl.h"
#include "RamsesFrameworkTypesImpl.h"

#include "ramses-client-api/Resource.h"

// framework
#include "ClientApplicationLogic.h"
#include "Components/ManagedResource.h"

#include "Collections/StringOutputStream.h"
#include "Utils/StringUtils.h"
#include "Utils/BinaryOutputStream.h"
#include "RamsesObjectTypeUtils.h"
#include "city.h"
#include "Utils/LogMacros.h"

namespace ramses
{
    ResourceImpl::ResourceImpl(ERamsesObjectType type,
        ramses_internal::ResourceHashUsage hashUsage,
        SceneImpl& scene,
        const char* name)
        : SceneObjectImpl(scene, type, name)
        , m_hashUsage(std::move(hashUsage))
    {
        if (m_hashUsage.isValid())
        {
            updateResourceHash();
        }
    }

    ResourceImpl::~ResourceImpl()
    {
        LOG_DEBUG_P(CONTEXT_FRAMEWORK, "Destroy resource type {}, hl {}, ll {}", getType(), m_resourceId, getLowlevelResourceHash());
    }

    resourceId_t ResourceImpl::getResourceId() const
    {
        assert(m_resourceId.isValid());
        return m_resourceId;
    }

    resourceId_t ResourceImpl::CreateResourceHash(ramses_internal::ResourceContentHash llhash, ramses_internal::String const& name, ERamsesObjectType type)
    {
        resourceId_t hash;

        ramses_internal::BinaryOutputStream metaDataStream(1024);
        metaDataStream << llhash;
        metaDataStream << name;
        metaDataStream << static_cast<uint32_t>(type);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) external API expects char* to binary data
        const cityhash::uint128 cityHashMetadataAndBlob = cityhash::CityHash128(reinterpret_cast<const char*>(metaDataStream.getData()), metaDataStream.getSize());
        hash.highPart = cityhash::Uint128High64(cityHashMetadataAndBlob);
        hash.lowPart = cityhash::Uint128Low64(cityHashMetadataAndBlob);
        return hash;
    }

    void ResourceImpl::updateResourceHash()
    {
        const auto id = m_resourceId;
        m_resourceId = CreateResourceHash(getLowlevelResourceHash(), getName(), getType());

        if (id.isValid() && id != m_resourceId)
            getSceneImpl().updateResourceId(id, RamsesObjectTypeUtils::ConvertTo<Resource>(getRamsesObject()));
    }

    void ResourceImpl::deinitializeFrameworkData()
    {
    }

    ramses::status_t ResourceImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << getLowlevelResourceHash();
        outStream << m_resourceId.highPart;
        outStream << m_resourceId.lowPart;

        return StatusOK;
    }

    status_t ResourceImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        ramses_internal::ResourceContentHash llhash;
        inStream >> llhash;
        m_hashUsage = getClientImpl().getHashUsage_ThreadSafe(llhash);

        inStream >> m_resourceId.highPart;
        inStream >> m_resourceId.lowPart;

        return StatusOK;
    }

    ramses_internal::ResourceContentHash ResourceImpl::getLowlevelResourceHash() const
    {
        return m_hashUsage.getHash();
    }

    status_t ResourceImpl::validate() const
    {
        const status_t status = SceneObjectImpl::validate();
        ramses_internal::StringOutputStream stringStream;
        stringStream << "Resource ID: " << m_resourceId;
        stringStream << "  Resource Hash: " << m_hashUsage.getHash();
        addValidationMessage(EValidationSeverity_Info, ramses_internal::String{ stringStream.release() });
        return status;
    }

    status_t ResourceImpl::setName(RamsesObject& object, const char* name)
    {
        const status_t status = SceneObjectImpl::setName(object, name);

        // name is also included in resource hash
        updateResourceHash();

        return status;
    }
}
