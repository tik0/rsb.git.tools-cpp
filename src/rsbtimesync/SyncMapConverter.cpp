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

#include "SyncMapConverter.h"

#include <map>
#include <vector>

#include <rsb/Event.h>
#include <rsb/Scope.h>
#include <rsb/converter/SerializationException.h>
#include <rsb/protocol/Notification.h>

using namespace std;

namespace rsbtimesync {

SyncMapConverter::SyncMapConverter(
        rsb::converter::Repository<std::string>::Ptr converterRepository) :
        rsb::converter::Converter<string>("SyncMap", "SyncMap", true), converterRepository(
                converterRepository) {

}

SyncMapConverter::~SyncMapConverter() {
}

string SyncMapConverter::getClassName() const {
    return "SyncMapConverter";
}

string SyncMapConverter::serialize(const rsb::converter::AnnotatedData &data,
        string &wire) {

    if (data.first != getDataType()) {
        throw rsb::converter::SerializationException(
                "Called with unsupported data type " + data.first);
    }

    boost::shared_ptr<DataMap> dataMap = boost::static_pointer_cast<DataMap>(
            data.second);

    boost::shared_ptr<SyncMap> map(new SyncMap);
    SyncMap &syncMap = *map;

    for (DataMap::const_iterator mapIt = dataMap->begin();
            mapIt != dataMap->end(); ++mapIt) {

        SyncMap::ScopeSet *scopeSet = syncMap.add_sets();
        scopeSet->set_scope(mapIt->first.toString());

        cout << "Processing scope " << mapIt->first << endl;

        for (vector<rsb::EventPtr>::const_iterator eventIt =
                mapIt->second.begin(); eventIt != mapIt->second.end();
                ++eventIt) {

            rsb::EventPtr event = *eventIt;
            cout << "  processing event " << event << endl;
            boost::shared_ptr<pair<string, boost::shared_ptr<string> > > annotatedData =
                    boost::static_pointer_cast<
                            pair<string, boost::shared_ptr<string> > >(
                            event->getData());

            rsb::protocol::Notification *notification =
                    scopeSet->add_notifications();
            rsb::protocol::fillNotificationId(*notification, event);
            rsb::protocol::fillNotificationHeader(*notification, event,
                    annotatedData->first);
            notification->set_data(*(annotatedData->second));

        }

    }

    cout << "build SyncMap: " << syncMap.DebugString() << endl;

    //syncMap.SerializeToString(&wire);
    return converter.serialize(
            make_pair(rsc::runtime::typeName<SyncMap>(), map), wire);

    //return getWireSchema();

}

rsb::converter::AnnotatedData SyncMapConverter::deserialize(
        const string &wireSchema, const string &wire) {

    if (wireSchema != getWireSchema()) {
        throw rsb::converter::SerializationException(
                "Unexpected wire schema " + wireSchema);
    }

    SyncMap syncMap;
    syncMap.ParseFromString(wire);

    boost::shared_ptr<DataMap> dataMap(new DataMap);

    for (unsigned int setCount = 0; setCount < syncMap.sets_size();
            ++setCount) {

        const SyncMap::ScopeSet &scopeSet = syncMap.sets(setCount);
        rsb::ScopePtr scope(new rsb::Scope(scopeSet.scope()));

        for (unsigned int notificationIndex = 0;
                notificationIndex < scopeSet.notifications_size();
                ++notificationIndex) {

            const rsb::protocol::Notification &notification =
                    scopeSet.notifications(notificationIndex);

            rsb::EventPtr event(new rsb::Event);
            event->setScopePtr(scope);
            rsb::converter::AnnotatedData annotatedData =
                    converterRepository->getConvertersForDeserialization()->getConverter(
                            notification.wire_schema())->deserialize(
                            notification.wire_schema(), notification.data());
            event->setType(annotatedData.first);
            event->setData(annotatedData.second);

            rsb::protocol::fillEvent(event, notification, annotatedData.second,
                    annotatedData.first);

            (*dataMap)[*scope].push_back(event);

        }

    }

    return make_pair(getWireSchema(), dataMap);

}

}
