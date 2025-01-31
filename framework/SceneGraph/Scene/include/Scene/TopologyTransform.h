//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TOPOLOGYTRANSFORM_H
#define RAMSES_TOPOLOGYTRANSFORM_H

#include "SceneAPI/Handles.h"
#include "SceneAPI/ERotationConvention.h"
#include "Math3d/Vector3.h"
#include "Utils/AssertMovable.h"

namespace ramses_internal
{
    struct TopologyTransform
    {
        Vector3 translation = { 0.f, 0.f, 0.f };
        Vector3 rotation    = { 0.f, 0.f, 0.f };
        Vector3 scaling     = { 1.f, 1.f, 1.f };
        ERotationConvention rotationConvention = ERotationConvention::Legacy_ZYX;

        NodeHandle node;
    };

    ASSERT_MOVABLE(TopologyTransform)
}

#endif
