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

#include "SyncStrategy.h"

#include <boost/thread/mutex.hpp>

#include <rsc/logging/Logger.h>
#include <rsc/threading/TaskExecutor.h>

#include <rsb/Event.h>

namespace rsbtimesync {

/**
 * A strategy that associates supplementary streams to the primary stream using
 * a specified time window.
 *
 * @author jwienke
 */
class TimeFrameStrategy: public SyncStrategy {
public:

	TimeFrameStrategy();
	virtual ~TimeFrameStrategy();

	virtual std::string getName() const;
	virtual std::string getKey() const;

	virtual void setSyncDataHandler(SyncDataHandlerPtr handler);

	virtual void initializeChannels(const rsb::Scope &primaryScope,
			const std::set<rsb::Scope> &subsidiaryScopes);

	virtual void provideOptions(
			boost::program_options::options_description &optionDescription);

	virtual void handleOptions(
			const boost::program_options::variables_map &options);

	virtual void handle(rsb::EventPtr event);

private:

	class SyncPushTask;

	const std::string OPTION_TIME_FRAME;
	const std::string OPTION_BUFFER_TIME;

	rsc::logging::LoggerPtr logger;

	SyncDataHandlerPtr handler;

	rsb::Scope primaryScope;
	std::set<rsb::Scope> subsidiaryScopes;

	boost::mutex subEventMutex;
	std::multimap<boost::uint64_t, rsb::EventPtr> subEventsByTime;

	unsigned int timeFrameMus;
	unsigned int bufferTimeMus;

	rsc::threading::TaskExecutorPtr executor;

};

}

