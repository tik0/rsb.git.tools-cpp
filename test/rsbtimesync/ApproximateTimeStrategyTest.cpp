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

TEST(ApproximateTimeStrategyTest, testProcessing) {

	set<Scope> scopes;
	scopes.insert("/aaa");
	scopes.insert("/bbb");
	scopes.insert("/ccc");

	ApproximateTimeStrategy strategy;
	strategy.initializeChannels(*(scopes.begin()), scopes);

	// very simple case, completely synchronized events
	for (unsigned int i = 0; i < 5; ++i) {

		for (set<Scope>::const_iterator scopeIt = scopes.begin();
				scopeIt != scopes.end(); ++scopeIt) {

			EventPtr event(new Event);
			event->setEventId(rsc::misc::UUID(), i);
			event->setScope(*scopeIt);
			strategy.handle(event);

			boost::this_thread::sleep(boost::posix_time::milliseconds(10));

		}

		boost::this_thread::sleep(boost::posix_time::seconds(1));

	}

}
