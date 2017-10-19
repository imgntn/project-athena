//
//  RayPickScriptingInterface.cpp
//  interface/src/raypick
//
//  Created by Sam Gondelman 8/15/2017
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RayPickScriptingInterface.h"

#include <QVariant>
#include "GLMHelpers.h"
#include "Application.h"

#include "StaticRayPick.h"
#include "JointRayPick.h"
#include "MouseRayPick.h"

QUuid RayPickScriptingInterface::createRayPick(const QVariant& properties) {
    QVariantMap propMap = properties.toMap();

    bool enabled = false;
    if (propMap["enabled"].isValid()) {
        enabled = propMap["enabled"].toBool();
    }

    PickFilter filter = PickFilter();
    if (propMap["filter"].isValid()) {
        filter = PickFilter(propMap["filter"].toUInt());
    }

    float maxDistance = 0.0f;
    if (propMap["maxDistance"].isValid()) {
        maxDistance = propMap["maxDistance"].toFloat();
    }

    if (propMap["joint"].isValid()) {
        std::string jointName = propMap["joint"].toString().toStdString();

        if (jointName != "Mouse") {
            // x = upward, y = forward, z = lateral
            glm::vec3 posOffset = Vectors::ZERO;
            if (propMap["posOffset"].isValid()) {
                posOffset = vec3FromVariant(propMap["posOffset"]);
            }

            glm::vec3 dirOffset = Vectors::UP;
            if (propMap["dirOffset"].isValid()) {
                dirOffset = vec3FromVariant(propMap["dirOffset"]);
            }

            return qApp->getPickManager().addPick(RAY, std::make_shared<JointRayPick>(jointName, posOffset, dirOffset, filter, maxDistance, enabled));
           
        } else {
            return qApp->getPickManager().addPick(RAY, std::make_shared<MouseRayPick>(filter, maxDistance, enabled));
        }
    } else if (propMap["position"].isValid()) {
        glm::vec3 position = vec3FromVariant(propMap["position"]);

        glm::vec3 direction = -Vectors::UP;
        if (propMap["direction"].isValid()) {
            direction = vec3FromVariant(propMap["direction"]);
        }

        return qApp->getPickManager().addPick(RAY, std::make_shared<StaticRayPick>(position, direction, filter, maxDistance, enabled));
    }

    return QUuid();
}

void RayPickScriptingInterface::enableRayPick(const QUuid& uid) {
    qApp->getPickManager().enablePick(uid);
}

void RayPickScriptingInterface::disableRayPick(const QUuid& uid) {
    qApp->getPickManager().disablePick(uid);
}

void RayPickScriptingInterface::removeRayPick(const QUuid& uid) {
    qApp->getPickManager().removePick(uid);
}

QVariantMap RayPickScriptingInterface::getPrevRayPickResult(const QUuid& uid) {
    return qApp->getPickManager().getPrevPickResult(uid);
}

void RayPickScriptingInterface::setPrecisionPicking(const QUuid& uid, const bool precisionPicking) {
    qApp->getPickManager().setPrecisionPicking(uid, precisionPicking);
}

void RayPickScriptingInterface::setIgnoreItems(const QUuid& uid, const QScriptValue& ignoreItems) {
    qApp->getPickManager().setIgnoreItems(uid, qVectorQUuidFromScriptValue(ignoreItems));
}

void RayPickScriptingInterface::setIncludeItems(const QUuid& uid, const QScriptValue& includeItems) {
    qApp->getPickManager().setIncludeItems(uid, qVectorQUuidFromScriptValue(includeItems));
}
