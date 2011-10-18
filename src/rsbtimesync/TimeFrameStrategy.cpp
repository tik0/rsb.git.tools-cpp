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

using namespace std;
using namespace rsc;
using namespace rsb;

namespace rsbtimesync {

TimeFrameStrategy::TimeFrameStrategy() :
			OPTION_TIME_FRAME(getKey() + "-timeframe"),
			OPTION_BUFFER_TIME(getKey() + "-buffer"),
			logger(
					rsc::logging::Logger::getLogger(
							"rsbtimesync.TimeFrameStrategy")),
			timeFrameMus(250000), bufferTimeMus(2 * timeFrameMus) {
}

TimeFrameStrategy::~TimeFrameStrategy() {
}

string TimeFrameStrategy::getName() const {
	return "TimeFrameStrategy";
}

string TimeFrameStrategy::getKey() const {
	return "timeframe";
}

void TimeFrameStrategy::setSyncDataHandler(SyncDataHandlerPtr handler) {

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
			boost::str(boost::format("buffer time in microseconds. "
				"This is the time between now and primary-event.create which is "
				"waited until the event is sent out with all synchronizable "
				"other events. Default: %d") % bufferTimeMus).c_str());
}

void TimeFrameStrategy::handleOptions(
		const boost::program_options::variables_map &options) {

	if (options.count(OPTION_TIME_FRAME.c_str())) {
		timeFrameMus = options[OPTION_TIME_FRAME.c_str()].as<unsigned int> ();
	}
	if (options.count(OPTION_BUFFER_TIME.c_str())) {
		bufferTimeMus = options[OPTION_BUFFER_TIME.c_str()].as<unsigned int> ();
	}

	RSCINFO(
			logger,
			"Configured timeFrameMus = " << timeFrameMus
					<< ", bufferTimeMus = " << bufferTimeMus);

}

void TimeFrameStrategy::handle(rsb::EventPtr event) {

	if (event->getScope() == primaryScope || event->getScope().isSubScopeOf(
			primaryScope)) {
		// TODO to something with the primary event
	} else {
		boost::mutex::scoped_lock lock(subEventMutex);
		subEventsByTime.insert(
				pair<boost::uint64_t, rsb::EventPtr> (
						event->getMetaData().getCreateTime(), event));
		RSCDEBUG(logger, "Buffered subsidiary event " << event);
	}

}

}
