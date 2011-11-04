/* ============================================================
 *
 * This file is a part of the RSB TimeSync project.
 *
 * Copyright (C) 2011 by Johannes Wienke <jwienke at techfak dot uni-bielefeld dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "EventsByScopeMapConverter.h"

#include <map>
#include <vector>

#include <rsb/Event.h>
#include <rsb/Scope.h>
#include <rsb/converter/Converter.h>
#include <rsb/converter/SerializationException.h>
#include <rsb/protocol/Notification.h>
#include <rsb/converter/ProtocolBufferConverter.h>

#include "SyncMap.pb.h"

#include "EventCollections.h"

using namespace std;

namespace rsbtimesync {

EventsByScopeMapConverter::EventsByScopeMapConverter(
        rsb::converter::ConverterSelectionStrategy<std::string>::Ptr serializationConverters,
        rsb::converter::ConverterSelectionStrategy<std::string>::Ptr deserializationConverters) :
        rsb::converter::Converter<string>("dummy", RSB_TYPE_TAG(EventsByScopeMap)), serializationConverters(
                serializationConverters), deserializationConverters(
                deserializationConverters), converter(
                new rsb::converter::ProtocolBufferConverter<SyncMap>) {

}

EventsByScopeMapConverter::~EventsByScopeMapConverter() {
}

string EventsByScopeMapConverter::getWireSchema() const {
    return converter->getWireSchema();
}

string EventsByScopeMapConverter::getClassName() const {
    return "EventsByScopeMapConverter";
}

string EventsByScopeMapConverter::serialize(const rsb::converter::AnnotatedData &data,
        string &wire) {

    if (data.first != getDataType()) {
        throw rsb::converter::SerializationException(
                "Called with unsupported data type " + data.first);
    }

    boost::shared_ptr<EventsByScopeMap> dataMap = boost::static_pointer_cast<EventsByScopeMap>(
            data.second);

    boost::shared_ptr<SyncMap> syncMap(new SyncMap);

    // iterate over all scopes
    for (EventsByScopeMap::const_iterator mapIt = dataMap->begin();
            mapIt != dataMap->end(); ++mapIt) {

        SyncMap::ScopeSet *scopeSet = syncMap->add_sets();
        scopeSet->set_scope(mapIt->first.toString());

        // iterate over all events in one scope
        for (vector<rsb::EventPtr>::const_iterator eventIt =
                mapIt->second.begin(); eventIt != mapIt->second.end();
                ++eventIt) {

            // convert event to notification
            rsb::EventPtr event = *eventIt;

            rsb::converter::Converter<string>::Ptr c = serializationConverters->getConverter(
                    event->getType());
            string wire;
            string wireSchema = c->serialize(
                    make_pair(event->getType(), event->getData()), wire);

            rsb::protocol::Notification *notification =
                    scopeSet->add_notifications();
            rsb::protocol::fillNotificationId(*notification, event);
            rsb::protocol::fillNotificationHeader(*notification, event,
                    wireSchema);
            notification->set_data(wire);

        }

    }

    return converter->serialize(
            make_pair(rsc::runtime::typeName<SyncMap>(), syncMap), wire);

}

rsb::converter::AnnotatedData EventsByScopeMapConverter::deserialize(
        const string &wireSchema, const string &wire) {

    if (wireSchema != getWireSchema()) {
        throw rsb::converter::SerializationException(
                "Unexpected wire schema " + wireSchema);
    }

    SyncMap syncMap;
    syncMap.ParseFromString(wire);

    boost::shared_ptr<EventsByScopeMap> dataMap(new EventsByScopeMap);

    // iterate over all scope sets
    for (unsigned int setCount = 0; setCount < syncMap.sets_size();
            ++setCount) {

        const SyncMap::ScopeSet &scopeSet = syncMap.sets(setCount);
        rsb::ScopePtr scope(new rsb::Scope(scopeSet.scope()));

        // iterate over all notifications in each scope set
        for (unsigned int notificationIndex = 0;
                notificationIndex < scopeSet.notifications_size();
                ++notificationIndex) {

            // convert the notification back to an event
            const rsb::protocol::Notification &notification =
                    scopeSet.notifications(notificationIndex);

            rsb::EventPtr event(new rsb::Event);
            event->setScopePtr(scope);
            rsb::converter::AnnotatedData annotatedData =
                    deserializationConverters->getConverter(
                            notification.wire_schema())->deserialize(
                            notification.wire_schema(), notification.data());
            event->setType(annotatedData.first);
            event->setData(annotatedData.second);

            rsb::protocol::fillEvent(event, notification, annotatedData.second,
                    annotatedData.first);

            (*dataMap)[*scope].push_back(event);

        }

    }

    return make_pair(getDataType(), dataMap);

}

}
