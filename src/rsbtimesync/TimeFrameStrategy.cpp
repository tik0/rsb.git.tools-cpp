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

using namespace std;
using namespace rsc;
using namespace rsb;

namespace rsbtimesync {

TimeFrameStrategy::TimeFrameStrategy() :
	OPTION_TIME_FRAME(getKey() + "-timeframe") {
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

}

void TimeFrameStrategy::provideOptions(
		boost::program_options::options_description &optionDescription) {
	optionDescription.add_options()(OPTION_TIME_FRAME.c_str(),
			boost::program_options::value<unsigned int>(),
			"allowed time frame to associate in microseconds.");
}

void TimeFrameStrategy::handleOptions(
		const boost::program_options::variables_map &options) {

	if (!options.count(OPTION_TIME_FRAME.c_str())) {
		throw invalid_argument("No time frame specified.");
	}
	timeFrameMus = options[OPTION_TIME_FRAME.c_str()].as<unsigned int> ();

}

void TimeFrameStrategy::handle(rsb::EventPtr event) {

}

}
