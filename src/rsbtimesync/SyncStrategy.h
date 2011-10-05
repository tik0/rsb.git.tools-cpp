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

#include <set>

#include <boost/shared_ptr.hpp>

#include <rsb/Event.h>
#include <rsb/Scope.h>
#include <rsb/Handler.h>

namespace rsbtimesync {

/**
 * An interface for strategies which synchronize events.
 *
 * @author jwienke
 */
class SyncStrategy: public rsb::Handler {
public:

	SyncStrategy();
	virtual ~SyncStrategy();

	/**
	 * Sets the handler which has to be called in order to send an event.
	 *
	 * @param handler handler to set
	 */
	virtual void setSyncEventHandler(
			rsb::eventprocessing::HandlerPtr handler) = 0;

	virtual void initializeChannels(const rsb::Scope &primaryScope,
			const std::set<rsb::Scope> &subsidiaryScopes) = 0;

};

typedef boost::shared_ptr<SyncStrategy> SyncStrategyPtr;

}
