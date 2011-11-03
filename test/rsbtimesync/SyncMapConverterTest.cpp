/* ============================================================
 *
 * This file is a part of the RSBTimeSync project.
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

#include <stdexcept>

#include <boost/thread.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "rsbtimesync/SyncMapConverter.h"

using namespace std;
using namespace testing;
using namespace rsb;
using namespace rsbtimesync;

EventPtr createRandomEvent(const Scope &scope) {

    EventPtr event(new Event);
    event->setScope(scope);
    if (rand() > RAND_MAX / 2) {
        event->setType(rsc::runtime::typeName<boost::uint64_t>());
        event->setData(
                boost::shared_ptr<boost::uint64_t>(
                        new boost::uint64_t(rand())));
    } else {
        event->setType(rsc::runtime::typeName<string>());
        event->setData(
                boost::shared_ptr<string>(
                        new string(rsc::misc::randAlnumStr(30))));
    }

    return event;

}

TEST(SyncMapConverterTest, testRoundtrip) {

    boost::shared_ptr<SyncMapConverter::DataMap> message(
            new SyncMapConverter::DataMap);
    const unsigned int numEvents = 5;
    for (unsigned int eventNum = 0; eventNum < numEvents; ++eventNum) {

    }

//    (*message)[primaryEvent->getScope()].push_back(primaryEvent);
//    resultEvent->addCause(primaryEvent->getEventId());
//
//    // select the subsidiary events
//    {
//        boost::mutex::scoped_lock lock(subEventMutex);
//        for (std::multimap<boost::uint64_t, rsb::EventPtr>::iterator it =
//                subEventsByTime.lower_bound(
//                        primaryEvent->getMetaData().getCreateTime()
//                                - timeFrameMus);
//                it
//                        != subEventsByTime.upper_bound(
//                                primaryEvent->getMetaData().getCreateTime()
//                                        + timeFrameMus); ++it) {
//            (*message)[it->second->getScope()].push_back(it->second);
//            resultEvent->addCause(it->second->getEventId());
//        }
//    }
//
//    // finally emit the event
//    resultEvent->setData(message);

}
