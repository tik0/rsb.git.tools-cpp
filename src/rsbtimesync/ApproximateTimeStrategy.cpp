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

#include <boost/numeric/conversion/cast.hpp>

using namespace std;
using namespace rsb;

namespace rsbtimesync {

ApproximateTimeStrategy::ApproximateTimeStrategy() :
		OPTION_QUEUE_SIZE(getKey() + "-qs"), queueSize(2) {
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
	eventQueuesByScope[primaryScope];
	for (set<Scope>::const_iterator scopeIt = subsidiaryScopes.begin();
			scopeIt != subsidiaryScopes.end(); ++scopeIt) {
		eventQueuesByScope[*scopeIt];
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

void ApproximateTimeStrategy::handle(EventPtr) {

}

}
