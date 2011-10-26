/* ============================================================
 *
 * This file is a part of RSB project
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

#include "rsbtimesync/ApproximateTimeStrategy.cpp"

using namespace std;
using namespace testing;
using namespace rsb;
using namespace rsbtimesync;

class StoringSyncDataHandler: public SyncDataHandler {
public:

    virtual rsb::EventPtr createEvent() {
        EventPtr event(new Event);
        event->setScope("/test/sync");
        event->setType("SyncMap");
        return event;
    }

    virtual void handle(EventPtr event) {
        events.push_back(event);
    }

    vector<EventPtr> &getEvents() {
        return events;
    }

private:

    vector<EventPtr> events;

};

class ApproximateTimeStrategyTest: public ::testing::Test {
public:

    boost::shared_ptr<ApproximateTimeStrategy> strategy;
    boost::shared_ptr<StoringSyncDataHandler> handler;
    set<Scope> scopes;

    void SetUp() {
        strategy.reset(new ApproximateTimeStrategy());
        handler.reset(new StoringSyncDataHandler);
        strategy->setSyncDataHandler(handler);
        scopes.clear();
        scopes.insert("/aaa");
        scopes.insert("/bbb");
        scopes.insert("/ccc");
        strategy->initializeChannels(*(scopes.begin()), scopes);
    }
};

TEST_F(ApproximateTimeStrategyTest, testEqualFrequencyAndTiming) {

    // very simple case, completely synchronized events
    unsigned int totalIterations = 5;
    for (unsigned int i = 0; i < totalIterations; ++i) {

        for (set<Scope>::const_iterator scopeIt = scopes.begin();
                scopeIt != scopes.end(); ++scopeIt) {

            EventPtr event(new Event);
            event->setEventId(rsc::misc::UUID(), i);
            event->setScope(*scopeIt);
            event->setType(rsc::runtime::typeName<string>());
            event->setData(
                    VoidPtr(new string(boost::str(boost::format("%d") % i))));
            strategy->handle(event);

            boost::this_thread::sleep(boost::posix_time::milliseconds(10));

        }

        boost::this_thread::sleep(boost::posix_time::milliseconds(50));

    }

    EXPECT_EQ(size_t(totalIterations - 1), handler->getEvents().size())
        << "For equal timing of all scopes the algorithm can always "
                "produce number of published sequences - 1 sync sets. "
                "For the last it waits until another element is "
                "available on each scope, because otherwise the "
                "condition that T_m is the pivot cannot be checked.";

    for (unsigned int sequenceNumber = 0;
            sequenceNumber < handler->getEvents().size(); ++sequenceNumber) {

        boost::shared_ptr<SyncMapConverter::DataMap> data =
                boost::static_pointer_cast<SyncMapConverter::DataMap>(
                        handler->getEvents()[sequenceNumber]->getData());

        EXPECT_EQ(size_t(3), data->size());
        for (set<Scope>::const_iterator scopeIt = scopes.begin();
                scopeIt != scopes.end(); ++scopeIt) {
            ASSERT_EQ(size_t(1), data->count(*scopeIt));
            string expectd = boost::str(boost::format("%d") % sequenceNumber);
            string actual = *(boost::static_pointer_cast<string>(
                    data->at(*scopeIt).front()->getData()));
            EXPECT_EQ(expectd, actual);
        }

    }

}

TEST_F(ApproximateTimeStrategyTest, testOneDoubleFrequency) {

    const Scope doubleScope = *(scopes.begin());

    unsigned int totalIterations = 10;
    for (unsigned int i = 0; i < totalIterations; ++i) {

        for (set<Scope>::const_iterator scopeIt = scopes.begin();
                scopeIt != scopes.end(); ++scopeIt) {

            if (i % 2 == 0 || *scopeIt == doubleScope) {

                EventPtr event(new Event);
                event->setEventId(rsc::misc::UUID(), i);
                event->setScope(*scopeIt);
                event->setType(rsc::runtime::typeName<string>());
                event->setData(
                        VoidPtr(
                                new string(
                                        boost::str(boost::format("%d") % i))));
                strategy->handle(event);

                boost::this_thread::sleep(boost::posix_time::milliseconds(10));

            }

        }

        boost::this_thread::sleep(boost::posix_time::milliseconds(50));

    }

    EXPECT_EQ(size_t(totalIterations / 2 - 1), handler->getEvents().size())
        << "For equal timing of all scopes the algorithm can always "
                "produce number of published sequences - 1 sync sets. "
                "For the last it waits until another element is "
                "available on each scope, because otherwise the "
                "condition that T_m is the Pivot cannot be checked.";

    for (unsigned int i = 0; i < handler->getEvents().size(); ++i) {

        boost::shared_ptr<SyncMapConverter::DataMap> data =
                boost::static_pointer_cast<SyncMapConverter::DataMap>(
                        handler->getEvents()[i]->getData());

        EXPECT_EQ(size_t(3), data->size());
        for (set<Scope>::const_iterator scopeIt = scopes.begin();
                scopeIt != scopes.end(); ++scopeIt) {
            ASSERT_EQ(size_t(1), data->count(*scopeIt));
            string expectd = boost::str(boost::format("%d") % (i * 2));
            string actual = *(boost::static_pointer_cast<string>(
                    data->at(*scopeIt).front()->getData()));
            EXPECT_EQ(expectd, actual);
        }

    }

}
