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

#include "TimeFrameStrategy.h"

#include <boost/format.hpp>

#include <rsc/threading/ThreadedTaskExecutor.h>
#include <rsc/threading/SimpleTask.h>

#include <rsb/MetaData.h>
#include <rsb/EventId.h>

#include <rsb/EventCollections.h>

using namespace std;
using namespace rsc;
using namespace rsb;

namespace rsbtimesync {

class TimeFrameStrategy::SyncPushTask: public rsc::threading::SimpleTask {
public:

    SyncPushTask(EventPtr primaryEvent, boost::mutex &subEventMutex,
            std::multimap<boost::uint64_t, rsb::EventPtr> &subEventsByTime
            , SyncDataHandlerPtr handler, const unsigned int &bufferTimeMus
            ,const unsigned int &timeFrameMus) :
            primaryEvent(primaryEvent), subEventMutex(subEventMutex), subEventsByTime(
                    subEventsByTime), handler(handler), bufferTimeMus(
                    bufferTimeMus), timeFrameMus(timeFrameMus) {
    }

    void run() {

        rsb::EventPtr resultEvent = handler->createEvent();

        // prepare message with primary event
        boost::shared_ptr<EventsByScopeMap> message(new EventsByScopeMap);
        (*message)[primaryEvent->getScope()].push_back(primaryEvent);
        resultEvent->addCause(primaryEvent->getEventId());

        // select the subsidiary events
        {
            boost::mutex::scoped_lock lock(subEventMutex);
            for (std::multimap<boost::uint64_t, rsb::EventPtr>::iterator it =
                    subEventsByTime.lower_bound(
                            primaryEvent->getMetaData().getCreateTime()
                                    - timeFrameMus);
                    it
                            != subEventsByTime.upper_bound(
                                    primaryEvent->getMetaData().getCreateTime()
                                            + timeFrameMus); ++it) {
                (*message)[it->second->getScope()].push_back(it->second);
                resultEvent->addCause(it->second->getEventId());
            }
        }

        // finally emit the event
        resultEvent->setData(message);
        handler->handle(resultEvent);

        markDone();
    }

private:

    EventPtr primaryEvent;
    boost::mutex &subEventMutex;
    std::multimap<boost::uint64_t, rsb::EventPtr> &subEventsByTime;
    SyncDataHandlerPtr handler;
    unsigned int bufferTimeMus;
    unsigned int timeFrameMus;

};

TimeFrameStrategy::TimeFrameStrategy() :
        cleaningInterrupted(false), OPTION_TIME_FRAME(getKey() + "-timeframe"), OPTION_BUFFER_TIME(
                getKey() + "-buffer"), logger(
                rsc::logging::Logger::getLogger(
                        "rsbtimesync.TimeFrameStrategy")), timeFrameMus(250000), bufferTimeMus(
                2 * timeFrameMus), executor(
                new rsc::threading::ThreadedTaskExecutor) {
}

TimeFrameStrategy::~TimeFrameStrategy() {
    cleaningInterrupted = true;
    if (cleanerThread) {
        cleanerThread->join();
    }
}

string TimeFrameStrategy::getName() const {
    return "TimeFrameStrategy";
}

string TimeFrameStrategy::getKey() const {
    return "timeframe";
}

void TimeFrameStrategy::setSyncDataHandler(SyncDataHandlerPtr handler) {
    this->handler = handler;
}

void TimeFrameStrategy::setTimestampSelector(TimestampSelectorPtr selector) {
    this->selector = selector;
}

void TimeFrameStrategy::initializeChannels(const Scope &primaryScope,
        const set<Scope> &subsidiaryScopes) {
    this->primaryScope = primaryScope;
    this->subsidiaryScopes = subsidiaryScopes;
}

void TimeFrameStrategy::provideOptions(
        boost::program_options::options_description &optionDescription) {
    optionDescription.add_options()(
            OPTION_TIME_FRAME.c_str(),
            boost::program_options::value<unsigned int>(),
            boost::str(
                    boost::format(
                            "allowed time frame to associate in microseconds in both directions of time, default %d")
                            % timeFrameMus).c_str())(
            OPTION_BUFFER_TIME.c_str(),
            boost::program_options::value<unsigned int>(),
            boost::str(
                    boost::format(
                            "buffer time in microseconds. This is the time between now and primary-event.create which is waited until the event is sent out with all synchronizable other events. Default: %d")
                            % bufferTimeMus).c_str());
}

void TimeFrameStrategy::handleOptions(
        const boost::program_options::variables_map & options) {
    if (options.count(OPTION_TIME_FRAME.c_str())) {
        timeFrameMus = options[OPTION_TIME_FRAME.c_str()].as<unsigned int>();
    }
    if (options.count(OPTION_BUFFER_TIME.c_str())) {
        bufferTimeMus = options[OPTION_BUFFER_TIME.c_str()].as<unsigned int>();
    }

    RSCINFO(
            logger,
            "Configured timeFrameMus = " << timeFrameMus << ", bufferTimeMus = " << bufferTimeMus);

    cleanerThread.reset(
            new boost::thread(
                    boost::bind(&TimeFrameStrategy::cleanerThreadMethod,
                            this)));

}

void TimeFrameStrategy::handle(rsb::EventPtr event) {
    if (event->getScope() == primaryScope
            || event->getScope().isSubScopeOf(primaryScope)) {
        // for each primary event, start a task which waits until the event has
        // to be delivered according to the buffer time. Afterwards, this task
        // collects all subsidiary events of the primary one and pushes the
        // synchronized events to the handler.

        rsc::threading::TaskPtr task(
                new SyncPushTask(event, subEventMutex, subEventsByTime, handler,
                        bufferTimeMus, timeFrameMus));
        // TODO maybe we have to compare local time to created time or something like that to get a better delay?
        executor->schedule(task, bufferTimeMus);

    } else {
        // subsidiary events are just pushed into the time-indexed pool.

        boost::mutex::scoped_lock lock(subEventMutex);
        boost::uint64_t ts;
        selector->getTimestamp(event, ts);
        subEventsByTime.insert(pair<boost::uint64_t, rsb::EventPtr>(ts, event));
        RSCDEBUG(logger, "Buffered subsidiary event " << event);
    }
}

void TimeFrameStrategy::cleanerThreadMethod() {

    while (!cleaningInterrupted) {

        {
            boost::mutex::scoped_lock lock(subEventMutex);
            subEventsByTime.erase(
                    subEventsByTime.begin(),
                    subEventsByTime.upper_bound(
                            rsc::misc::currentTimeMicros() - bufferTimeMus));
        }

        boost::this_thread::sleep(
                boost::posix_time::microseconds(2 * bufferTimeMus));

    }

}

}
