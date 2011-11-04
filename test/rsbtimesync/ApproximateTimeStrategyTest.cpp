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

#include <rsb/EventCollections.h>

#include "rsbtimesync/ApproximateTimeStrategy.h"

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

        boost::shared_ptr<EventsByScopeMap> data = boost::static_pointer_cast<
                EventsByScopeMap>(
                handler->getEvents()[sequenceNumber]->getData());

        EXPECT_EQ(size_t(3), data->size());
        for (set<Scope>::const_iterator scopeIt = scopes.begin();
                scopeIt != scopes.end(); ++scopeIt) {
            ASSERT_EQ(size_t(1), data->count(*scopeIt));
            string expected = boost::str(boost::format("%d") % sequenceNumber);
            string actual = *(boost::static_pointer_cast<string>(
                    data->at(*scopeIt).front()->getData()));
            EXPECT_EQ(expected, actual);
        }

    }

}

TEST_F(ApproximateTimeStrategyTest, testOneDoubleFrequency) {

    // we need to increase the queue size avoiding dropping events in this case
    strategy->setQueueSize(3);

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

        // ensure that we sleep less after sending the synchronized set of
        // events. Otherwise the optimization condition about size increasing
        // in the algorithm is triggered and we do not want to test this here.
        // In other words, the intermediate event on /aaa must be shortly after
        // the three synchronized events.
        if (i % 2 != 0) {
            boost::this_thread::sleep(boost::posix_time::milliseconds(150));
        }

    }

    EXPECT_EQ(size_t(totalIterations / 2 - 1), handler->getEvents().size())
        << "For equal timing of all scopes the algorithm can always "
                "produce number of published sequences - 1 sync sets. "
                "For the last it waits until another element is "
                "available on each scope, because otherwise the "
                "condition that T_m is the Pivot cannot be checked.";

    for (unsigned int i = 0; i < handler->getEvents().size(); ++i) {

        boost::shared_ptr<EventsByScopeMap> data = boost::static_pointer_cast<
                EventsByScopeMap>(handler->getEvents()[i]->getData());

        EXPECT_EQ(size_t(3), data->size());
        for (set<Scope>::const_iterator scopeIt = scopes.begin();
                scopeIt != scopes.end(); ++scopeIt) {
            ASSERT_EQ(size_t(1), data->count(*scopeIt));
            string expected = boost::str(boost::format("%d") % (i * 2));
            string actual = *(boost::static_pointer_cast<string>(
                    data->at(*scopeIt).front()->getData()));
            EXPECT_EQ(expected, actual);
        }

    }

}

EventPtr createEvent(const Scope &scope, const string &content) {
    EventPtr event(new Event);
    event->setEventId(rsc::misc::UUID(), rand());
    event->setScope(scope);
    event->setType(rsc::runtime::typeName<string>());
    event->setData(VoidPtr(new string(content)));
    return event;
}

TEST_F(ApproximateTimeStrategyTest, testDropping) {

    set<Scope>::const_iterator scopeIt = scopes.begin();
    const Scope scopeA = *scopeIt;
    ++scopeIt;
    const Scope scopeB = *scopeIt;
    ++scopeIt;
    const Scope scopeC = *scopeIt;

    const string ignored = "ignored";
    const string wanted = "wanted";

    // first, fill up queues for a and b
    strategy->handle(createEvent(scopeA, ignored));
    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    strategy->handle(createEvent(scopeB, ignored));
    boost::this_thread::sleep(boost::posix_time::milliseconds(50));
    strategy->handle(createEvent(scopeA, ignored));
    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    strategy->handle(createEvent(scopeB, ignored));

    // now let both start dropping
    boost::this_thread::sleep(boost::posix_time::milliseconds(50));
    strategy->handle(createEvent(scopeB, ignored));
    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    strategy->handle(createEvent(scopeA, ignored));

    // now, we emit events to all scope nearly at the same time, so this will be
    // the resulting sync set. For the algorithm to really select this set it is
    // important that the event on the so far lacking scope is sent as the last
    // one.
    boost::this_thread::sleep(boost::posix_time::milliseconds(70));
    strategy->handle(createEvent(scopeA, wanted));
    boost::this_thread::sleep(boost::posix_time::milliseconds(5));
    strategy->handle(createEvent(scopeB, wanted));
    boost::this_thread::sleep(boost::posix_time::milliseconds(5));
    strategy->handle(createEvent(scopeC, wanted));

    EXPECT_TRUE(handler->getEvents().empty());

    // and finally we have to add another round of events. Otherwise the search
    // will not terminate with publishing the sync set
    boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    strategy->handle(createEvent(scopeA, wanted));
    strategy->handle(createEvent(scopeB, wanted));

    EXPECT_FALSE(handler->getEvents().empty());
    EXPECT_EQ(size_t(1), handler->getEvents().size());

    boost::shared_ptr<EventsByScopeMap> data = boost::static_pointer_cast<
            EventsByScopeMap>(handler->getEvents().front()->getData());
    EXPECT_EQ(size_t(3), data->size());
    for (set<Scope>::const_iterator scopeIt = scopes.begin();
            scopeIt != scopes.end(); ++scopeIt) {
        ASSERT_EQ(size_t(1), data->count(*scopeIt));
        string actual = *(boost::static_pointer_cast<string>(
                data->at(*scopeIt).front()->getData()));
        EXPECT_EQ(wanted, actual);
    }

}
