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

#include "ApproximateTimeStrategy.h"

#include <limits>

#include <boost/numeric/conversion/cast.hpp>

#include <rsc/runtime/ContainerIO.h>

#include <rsb/EventId.h>
#include <rsb/MetaData.h>
#include <rsb/EventCollections.h>

using namespace std;
using namespace rsb;

namespace rsb {
namespace tools {
namespace timesync {

class ApproximateTimeStrategy::Candidate: public virtual rsc::runtime::Printable {
public:

    Candidate(TimestampSelectorPtr selector) :
            selector(selector) {
    }

    string getClassName() const {
        return "Candidate";
    }

    void addEvent(EventPtr event) {
        events[event->getScope()] = event;
    }

    EventPtr getOldestEvent() const {

        boost::uint64_t referenceTimestamp =
                numeric_limits<boost::uint64_t>::max();
        EventPtr event;
        for (map<Scope, EventPtr>::const_iterator it = events.begin();
                it != events.end(); ++it) {
            boost::uint64_t currentTimestamp = selector->getTimestamp(
                    it->second);
            if (currentTimestamp < referenceTimestamp) {
                referenceTimestamp = currentTimestamp;
                event = it->second;
            }
        }

        assert(event);
        return event;

    }

    EventPtr getYoungestEvent() const {

        boost::uint64_t referenceTimestamp = 0;
        EventPtr event;
        for (map<Scope, EventPtr>::const_iterator it = events.begin();
                it != events.end(); ++it) {
            boost::uint64_t currentTimestamp = selector->getTimestamp(
                    it->second);
            if (currentTimestamp > referenceTimestamp) {
                referenceTimestamp = currentTimestamp;
                event = it->second;
            }
        }

        assert(event);
        return event;

    }

    unsigned int size() {
        boost::uint64_t youngestTime = selector->getTimestamp(
                getYoungestEvent());
        boost::uint64_t oldestTime = selector->getTimestamp(getOldestEvent());
        return youngestTime - oldestTime;
    }

    void printContents(ostream &stream) const {
        stream << "\n" << events;
    }

    map<Scope, EventPtr> &getEvents() {
        return events;
    }

private:

    map<Scope, EventPtr> events;
    TimestampSelectorPtr selector;

};

ApproximateTimeStrategy::ApproximateTimeStrategy() :
        OPTION_QUEUE_SIZE(getKey() + "-qs"), logger(
                rsc::logging::Logger::getLogger(
                        "rsbtimesync.ApproximateTimeStrategy")), queueSize(2) {
}

ApproximateTimeStrategy::~ApproximateTimeStrategy() {
}

string ApproximateTimeStrategy::getKey() const {
    return "approxt";
}

void ApproximateTimeStrategy::setSyncDataHandler(SyncDataHandlerPtr handler) {
    this->handler = handler;
}

void ApproximateTimeStrategy::setTimestampSelector(
        TimestampSelectorPtr selector) {
    this->selector = selector;
}

void ApproximateTimeStrategy::initializeChannels(const Scope &primaryScope,
        const set<Scope> &subsidiaryScopes) {
    newEventsByScope[primaryScope];
    queueDropMap[primaryScope] = false;
    for (set<Scope>::const_iterator scopeIt = subsidiaryScopes.begin();
            scopeIt != subsidiaryScopes.end(); ++scopeIt) {
        newEventsByScope[*scopeIt];
        queueDropMap[*scopeIt] = false;
    }
}

void ApproximateTimeStrategy::provideOptions(
        boost::program_options::options_description &optionDescription) {

    optionDescription.add_options()(
            OPTION_QUEUE_SIZE.c_str(),
            boost::program_options::value<int>(),
            boost::str(
                    boost::format("The queue size to use, default is %d")
                            % queueSize).c_str());

}

void ApproximateTimeStrategy::handleOptions(
        const boost::program_options::variables_map &options) {

    if (options.count(OPTION_QUEUE_SIZE.c_str())) {
        queueSize = boost::numeric_cast<unsigned int>(
                options[OPTION_QUEUE_SIZE.c_str()].as<int>());
    }

}

bool ApproximateTimeStrategy::isNoEmptyQueue() const {

    for (EventQueueMap::const_iterator queueIt = newEventsByScope.begin();
            queueIt != newEventsByScope.end(); ++queueIt) {
        if (queueIt->second.empty()) {
            return false;
        }
    }

    return true;

}

ApproximateTimeStrategy::CandidatePtr ApproximateTimeStrategy::makeCandidate() const {

    CandidatePtr candidate(new Candidate(selector));
    for (EventQueueMap::const_iterator queueIt = newEventsByScope.begin();
            queueIt != newEventsByScope.end(); ++queueIt) {
        candidate->addEvent(queueIt->second.front());
    }

    return candidate;

}

void ApproximateTimeStrategy::erase(const Scope &scope) {

    RSCTRACE(logger, "Erasing on scope " << scope);
    assert(!newEventsByScope[scope].empty());
    newEventsByScope[scope].pop_front();

}

void ApproximateTimeStrategy::shift(const Scope &scope) {

    RSCTRACE(logger, "Shifting on scope " << scope);
    assert(!newEventsByScope[scope].empty());
    trackBackQueuesByScope[scope].push_back(newEventsByScope[scope].front());
    erase(scope);

}

void ApproximateTimeStrategy::clearTrackBackQueues() {
    RSCDEBUG(logger, "Clearing track back events");
    for (EventQueueMap::iterator queueIt = trackBackQueuesByScope.begin();
            queueIt != trackBackQueuesByScope.end(); ++queueIt) {
        queueIt->second.clear();
    }
}

bool ApproximateTimeStrategy::isAllQueuesFilled() const {

    for (EventQueueMap::const_iterator queueIt = newEventsByScope.begin();
            queueIt != newEventsByScope.end(); ++queueIt) {
        if (queueIt->second.empty()) {
            return false;
        }
    }

    return true;

}

void ApproximateTimeStrategy::trackBack() {

    RSCDEBUG(logger, "recovering all channels");

    for (EventQueueMap::iterator queueIt = newEventsByScope.begin();
            queueIt != newEventsByScope.end(); ++queueIt) {

        Scope scope = queueIt->first;

        deque<EventPtr> &trackBackQueue = trackBackQueuesByScope[scope];
        while (!trackBackQueue.empty()) {
            queueIt->second.push_front(trackBackQueue.back());
            trackBackQueue.pop_back();
        }

    }

}

void ApproximateTimeStrategy::publishCandidate() {

    RSCINFO(logger, "Publishing candidate " << currentCandidate);
    debugState();

    rsb::EventPtr resultEvent = handler->createEvent();

    // prepare message with primary event
    boost::shared_ptr<EventsByScopeMap> message(new EventsByScopeMap);

    for (map<Scope, EventPtr>::const_iterator eventIt =
            currentCandidate->getEvents().begin();
            eventIt != currentCandidate->getEvents().end(); ++eventIt) {

        (*message)[eventIt->first].push_back(eventIt->second);
        resultEvent->addCause(eventIt->second->getEventId());

    }

    // finally emit the event
    resultEvent->setData(message);
    handler->handle(resultEvent);

    trackBack();
    debugState();
    deleteOlderThanCandidate();
    currentCandidate.reset();
    pivot.reset();

}

void ApproximateTimeStrategy::deleteOlderThanCandidate() {

    assert(currentCandidate);

    // with the processing logic so far we can assume that after a recover call
    // only the head of each queue needs to be remove in order to remove the
    // candidate it self. This is due to the fact that the processed queues only
    // contain the events required to track back to that state of the so far
    // assumed to be optimal candidate and no more other elements. With the
    // recover call we get back to the latest assumed to be optimal candidate.

    for (EventQueueMap::iterator queueIt = newEventsByScope.begin();
            queueIt != newEventsByScope.end(); ++queueIt) {
        assert(!queueIt->second.empty());
        queueIt->second.pop_front();
    }

}

void ApproximateTimeStrategy::clearDroppedState(const Scope &excludeScope) {
    for (QueueDropMap::iterator scopeIt = queueDropMap.begin();
            scopeIt != queueDropMap.end(); ++scopeIt) {
        if (scopeIt->first != excludeScope) {
            scopeIt->second = false;
        }
    }
}

void ApproximateTimeStrategy::process() {

    // As long as all queues contain at least one element we can process
    while (isAllQueuesFilled()) {

        RSCTRACE(logger, "Main processing loop iterating");

        // make a new candidate based on the current heads of the queues
        CandidatePtr newCandidate = makeCandidate();
        EventPtr newCandidateYoungest = newCandidate->getYoungestEvent();
        RSCDEBUG(logger, "proposed candidate: " << newCandidate);

        // TODO why exactly is this the right thing to do?
        clearDroppedState(newCandidateYoungest->getScope());

        if (!currentCandidate) {
            RSCTRACE(logger, "There is no current candidate");
            // if we currently do not have a candidate, prepare one simply from the
            // heads of each queue

            if (queueDropMap[newCandidateYoungest->getScope()]) {
                RSCDEBUG(
                        logger,
                        "The proposed pivot is from a queue with dropped messages. Ignoring this candidate.");
                // TODO include a warning in the case
                // TODO XXX This does not seem to be most efficient regarding
                // the contiguous criterion. this will possibly result in
                // dropping on every topic even though there would be a
                // contiguous set.
                erase(newCandidate->getOldestEvent()->getScope());
                continue;
            }

            pivot = newCandidateYoungest;
            currentCandidate = newCandidate;
            clearTrackBackQueues();

        } else {
            // if we already have a candidate, we need to check whether this one is
            // better than the currently proposed one.

            if (newCandidate->size() < currentCandidate->size()) {
                // this candidate is smaller, so it's better
                currentCandidate = newCandidate;
                clearTrackBackQueues();
            }

        }

        RSCDEBUG(
                logger,
                "pivot = " << pivot << "\n" << "currentCandidate = " << currentCandidate);

        // shift the oldest element
        EventPtr newCandidateOldest = newCandidate->getOldestEvent();
        shift(newCandidateOldest->getScope());

        // get some timing information which can help to prove that the current
        // candidate is optimal
        boost::uint64_t newYoungestTime = selector->getTimestamp(
                newCandidateYoungest);
        boost::uint64_t currentYoungestTime = selector->getTimestamp(
                currentCandidate->getYoungestEvent());
        boost::uint64_t youngestInterval = newYoungestTime
                - currentYoungestTime;
        boost::uint64_t pivotTime = selector->getTimestamp(pivot);
        boost::uint64_t pivotOldInterval = pivotTime
                - selector->getTimestamp(currentCandidate->getOldestEvent());
        RSCDEBUG(
                logger,
                "youngestInterval = " << youngestInterval << ", pivotOldInterval = " << pivotOldInterval);

        // check whether we can publish an event
        if (newCandidateOldest == pivot) {
            // the search exhausted all candidates by definition
            RSCDEBUG(
                    logger,
                    "Exhausted search: newCandidateOldest = " << newCandidateOldest << ", pivot = " << pivot);
            publishCandidate();
        } else if (youngestInterval >= pivotOldInterval) {
            RSCDEBUG(
                    logger,
                    "Provably optimal condition reached even though search was not complete.");
            // all possible new candidates increase the size more to the younger
            // side than they can reduce it on the older side
            publishCandidate();
        }

    }

    RSCTRACE(logger, "Main processing loop finished");

}

void ApproximateTimeStrategy::handle(EventPtr event) {

    Scope scope = event->getScope();

    boost::mutex::scoped_lock lock(mutex);

    RSCDEBUG(logger, "Handling event " << event);
    debugState();

    if (!newEventsByScope.count(scope)) {
        throw invalid_argument(
                boost::str(
                        boost::format(
                                "Received an event on scope %s, which is not one of the configured scopes. Event: %s")
                                % scope % event));
    }

    deque<EventPtr> &newQueue = newEventsByScope[scope];
    deque<EventPtr> &trackBackQueue = trackBackQueuesByScope[scope];
    RSCTRACE(
            logger,
            "before add: newQueue size = " << newQueue.size() << ", trackBackQueue size = " << trackBackQueue.size() << ", desired queueSize = " << queueSize);

    newQueue.push_back(event);
    // we may not yet ensure the size of the queue because then the event which
    // is closest to the last emitted set might be thrown away. This would cause
    // sets which are not contiguous.

    RSCTRACE(logger, "Added event to new queue.");
    debugState();
    process();
    debugState();

    RSCTRACE(
            logger,
            "after process: newQueue size = " << newQueue.size() << ", trackBackQueue size = " << trackBackQueue.size() << ", desired queueSize = " << queueSize);

    // finally we can ensure the size of the queues
    // TODO does this still ensure that sets are contiguous?
    // Probably yes because dropped channels are not used as pivot
    if (newQueue.size() + trackBackQueue.size() > queueSize) {

        RSCDEBUG(
                logger,
                "Queues for scope " << scope << " are filled up. Dropping events.");

        // for being able to drop a message, we first track back to initial state
        trackBack();

        // afterwards we can simply drop the last element from the offending new
        // queue
        RSCDEBUG(logger, "Dropping event " << newQueue.front());
        newQueue.pop_front();
        assert(newQueue.size() + trackBackQueue.size() == queueSize);

        // mark the queue as having dropped elements
        queueDropMap[event->getScope()] = true;

        // also we have to invalidate the current candidate and pivot
        pivot.reset();
        currentCandidate.reset();

        // finally, it still might be possible to find a good candidate now, so
        // try processing again
        process();
        debugState();

    }

}

void ApproximateTimeStrategy::setQueueSize(const unsigned int &size) {
    this->queueSize = size;
}

void ApproximateTimeStrategy::debugState() const {

    if (logger->isDebugEnabled()) {

#if RSC_VERSION_NUMERIC >= 000600
        RSCDEBUG(
                logger,
                "\n#################### STATE ####################\n" << "pivot = " << pivot << "\n" << "currentCandidate = " << currentCandidate << "\n" << "queueDropMap = " << queueDropMap << "\n" << "newEventsByState = " << newEventsByScope << "\n" << "trackBackQueuesByScope = " << trackBackQueuesByScope << "\n#################### XXXXX ####################");
#endif

    }

}

}
}
}
