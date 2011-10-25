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

#include <rsb/EventId.h>
#include <rsb/MetaData.h>

using namespace std;
using namespace rsb;

namespace rsbtimesync {

class ApproximateTimeStrategy::Candidate: public virtual rsc::runtime::Printable {
public:

	Candidate() {
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
			boost::uint64_t currentTimestamp =
					it->second->getMetaData().getCreateTime();
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
			boost::uint64_t currentTimestamp =
					it->second->getMetaData().getCreateTime();
			if (currentTimestamp > referenceTimestamp) {
				referenceTimestamp = currentTimestamp;
				event = it->second;
			}
		}

		assert(event);
		return event;

	}

	unsigned int size() {
		return getYoungestEvent()->getMetaData().getCreateTime()
				- getOldestEvent()->getMetaData().getCreateTime();
	}

	void printContents(ostream &stream) const {
		stream << "\n" << events;
	}

	map<Scope, EventPtr> &getEvents() {
		return events;
	}

private:

	map<Scope, EventPtr> events;

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

void ApproximateTimeStrategy::initializeChannels(const Scope &primaryScope,
		const set<Scope> &subsidiaryScopes) {
	newEventsByScope[primaryScope];
	for (set<Scope>::const_iterator scopeIt = subsidiaryScopes.begin();
			scopeIt != subsidiaryScopes.end(); ++scopeIt) {
		newEventsByScope[*scopeIt];
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

	CandidatePtr candidate(new Candidate);
	for (EventQueueMap::const_iterator queueIt = newEventsByScope.begin();
			queueIt != newEventsByScope.end(); ++queueIt) {
		candidate->addEvent(queueIt->second.front());
	}

	return candidate;

}

void ApproximateTimeStrategy::shift(const Scope &scope) {

	RSCTRACE(logger, "Shifting on scope " << scope);
	trackBackQueuesByScope[scope].push_back(newEventsByScope[scope].front());
	newEventsByScope[scope].pop_front();

}

void ApproximateTimeStrategy::clearProcessed() {
	RSCDEBUG(logger, "Clearing processed events");
	trackBackQueuesByScope.clear();
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

void ApproximateTimeStrategy::recover() {

	RSCDEBUG(logger, "recovering all channels");

	for (EventQueueMap::iterator queueIt = newEventsByScope.begin();
			queueIt != newEventsByScope.end(); ++queueIt) {

		while (!trackBackQueuesByScope[queueIt->first].empty()) {
			queueIt->second.push_front(
					trackBackQueuesByScope[queueIt->first].back());
			trackBackQueuesByScope[queueIt->first].pop_back();
		}

	}

	trackBackQueuesByScope.clear();

}

void ApproximateTimeStrategy::publishCandidate() {

	RSCINFO(logger, "Publishing candidate " << currentCandidate);

	rsb::EventPtr resultEvent = handler->createEvent();

	// prepare message with primary event
	boost::shared_ptr<SyncMapConverter::DataMap> message(
			new SyncMapConverter::DataMap);

	for (map<Scope, EventPtr>::const_iterator eventIt =
			currentCandidate->getEvents().begin();
			eventIt != currentCandidate->getEvents().end(); ++eventIt) {

		(*message)[eventIt->first].push_back(eventIt->second);
		resultEvent->addCause(eventIt->second->getEventId());

	}

	// finally emit the event
	resultEvent->setData(message);
	handler->handle(resultEvent);

	recover();
	deleteOlderThanCandidate();
	currentCandidate.reset();
	pivot.reset();

}

void ApproximateTimeStrategy::deleteOlderThanCandidate() {

	assert(currentCandidate);
	assert(trackBackQueuesByScope.empty());

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

// TODO check drops for pivot elements
void ApproximateTimeStrategy::handle(EventPtr event) {

	RSCDEBUG(logger, "Handling event " << event);

	boost::mutex::scoped_lock(mutex);

	if (!newEventsByScope.count(event->getScope())) {
		throw invalid_argument(
				boost::str(
						boost::format(
								"Received an event on scope %s, which is not one of the configured scopes.")
								% event->getScope()));
	}

	newEventsByScope[event->getScope()].push_back(event);
	// we may not yet ensure the size of the queue because then the event which
	// is closest to the last emitted set might be thrown away. This would cause
	// sets which are not contiguous.

	// As long as all queues contain at least one element we can process
	while (isAllQueuesFilled()) {

		RSCTRACE(logger, "Main processing loop iterating");

		// make a new candidate based on the current heads of the queues
		CandidatePtr newCandidate = makeCandidate();
		RSCDEBUG(logger, "proposed candidate: " << newCandidate);

		if (!currentCandidate) {
			RSCTRACE(logger, "There is no current candidate");
			// if we currently do not have a candidate, prepare one simply from the
			// heads of each queue

			pivot = newCandidate->getYoungestEvent();
			currentCandidate = newCandidate;
			clearProcessed();

		} else {
			// if we already have a candidate, we need to check whether this one is
			// better than the currently proposed one.

			if (newCandidate->size() < currentCandidate->size()) {
				// this candidate is smaller, so it's better
				currentCandidate = newCandidate;
				clearProcessed();
			}

		}

		RSCDEBUG(
				logger,
				"pivot = " << pivot << "\n" << "currentCandidate = " << currentCandidate);

		// shift the oldest element
		EventPtr currentOldestEvent = newCandidate->getOldestEvent();
		shift(currentOldestEvent->getScope());

		// check whether we reached the pivot
		if (currentOldestEvent == pivot) {
			publishCandidate();
		}

	}

	RSCTRACE(logger, "Main processing loop finished");

	// finally we can ensure the size of the queues ensure size
	// TODO this needs recovering state etc. Implement this
//	while (newEventsByScope[event->getScope()].size()
//			+ processedEventsByScope[event->getScope()].size() > queueSize) {
//		newEventsByScope[event->getScope()].pop_front();
//	}

}

}
