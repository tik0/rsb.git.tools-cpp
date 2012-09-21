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

#pragma once

#include <map>
#include <deque>

#include <boost/thread/mutex.hpp>

#include <rsc/logging/Logger.h>

#include "SyncStrategy.h"

namespace rsb {
namespace tools {
namespace timesync {

/**
 * A sync strategy implementing the Approximate Time sync policy of ROS.
 *
 * @author jwienke
 * @link http://www.ros.org/wiki/message_filters/ApproximateTime
 */
class ApproximateTimeStrategy: public SyncStrategy {
public:
    ApproximateTimeStrategy();
    virtual ~ApproximateTimeStrategy();

    virtual std::string getKey() const;

    virtual void setSyncDataHandler(SyncDataHandlerPtr handler);

    virtual void initializeChannels(const rsb::Scope &primaryScope,
            const std::set<rsb::Scope> &subsidiaryScopes);

    virtual void provideOptions(
            boost::program_options::options_description &optionDescription);

    virtual void handleOptions(
            const boost::program_options::variables_map &options);

    virtual void handle(rsb::EventPtr event);

    void setQueueSize(const unsigned int &size);

    void setTimestampSelector(TimestampSelectorPtr selector);

private:

    class Candidate;
    typedef boost::shared_ptr<Candidate> CandidatePtr;

    bool isNoEmptyQueue() const;

    /**
     * Makes a new candidate from all heads of #newEventsByScope.
     *
     * @return candidate from head of queues
     */
    CandidatePtr makeCandidate() const;

    void publishCandidate();

    /**
     * Recovers the state which was known as the best candidate by replaying all
     * events from #trackBackQueuesByScope to #newEventsByScope.
     */
    void trackBack();

    void deleteOlderThanCandidate();

    void process();

    /**
     * Erases the current head of a queue in #newEventsByScope.
     *
     * @param scope scope of the queue to erase
     */
    void erase(const rsb::Scope &scope);

    /**
     * Shifts the current head of a queue in #newEventsByScope to
     * #trackBackQueuesByScope.
     *
     * @param scope scope of the queue to shift
     */
    void shift(const rsb::Scope &scope);

    /**
     * Clears #trackBackQueuesByScope. This makes a track back to a former
     * candidate impossible and hence should be called whenever we are sure that
     * the currently analyzed candidate is better than the old one which could
     * be tracked back with the processed queues so far.
     */
    void clearTrackBackQueues();

    /**
     * Clears the dropped state for all channels except the one given.
     *
     * @param excludeScope channel to exclude from resetting the drop state
     */
    void clearDroppedState(const rsb::Scope &excludeScope);

    bool isAllQueuesFilled() const;

    const std::string OPTION_QUEUE_SIZE;

    rsc::logging::LoggerPtr logger;

    SyncDataHandlerPtr handler;
    TimestampSelectorPtr selector;

    unsigned int queueSize;
    typedef std::map<rsb::Scope, std::deque<rsb::EventPtr> > EventQueueMap;
    typedef std::map<rsb::Scope, bool> QueueDropMap;
    EventQueueMap newEventsByScope;
    QueueDropMap queueDropMap;
    /**
     * Contains all events that have been analyzed so far and which are required
     * to track back to the best known candidate.
     */
    EventQueueMap trackBackQueuesByScope;
    boost::mutex mutex;

    rsb::EventPtr pivot;
    CandidatePtr currentCandidate;

    void debugState() const;

};

}
}
}
