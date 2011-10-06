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

#include "FirstMatchStrategy.h"

using namespace std;

namespace rsbtimesync {

FirstMatchStrategy::FirstMatchStrategy() {
}

FirstMatchStrategy::~FirstMatchStrategy() {
}

string FirstMatchStrategy::getClassName() const {
	return "FirstMatchStrategy";
}

void FirstMatchStrategy::setSyncDataHandler(SyncDataHandlerPtr handler) {
	this->handler = handler;
}

void FirstMatchStrategy::initializeChannels(const rsb::Scope &primaryScope,
		const set<rsb::Scope> &subsidiaryScopes) {
	this->primaryScope = primaryScope;
	for (set<rsb::Scope>::const_iterator scopeIt = subsidiaryScopes.begin(); scopeIt
			!= subsidiaryScopes.end(); ++scopeIt) {
		supplementaryEvents[*scopeIt].reset();
	}
}

void FirstMatchStrategy::handle(rsb::EventPtr event) {

	boost::recursive_mutex::scoped_lock lock(mutex);

	// insert event into data structure
	if (event->getScopePtr()->operator ==(primaryScope)) {
		primaryEvent = event;
	} else {
		assert(supplementaryEvents.count(event->getScope()));
		supplementaryEvents[event->getScope()] = event;
	}

	// check if we need to flush buffers
	if (!event) {
		return;
	}
	for (std::map<rsb::Scope, rsb::EventPtr>::const_iterator it =
			supplementaryEvents.begin(); it != supplementaryEvents.end(); ++it) {
		if (!it->second) {
			return;
		}
	}

	// all buffers are filled, we can emit an event
	boost::shared_ptr<SyncMapConverter::DataMap> message(
			new SyncMapConverter::DataMap);
	(*message)[primaryEvent->getScope()].push_back(primaryEvent);
	primaryEvent.reset();
	for (std::map<rsb::Scope, rsb::EventPtr>::iterator it =
			supplementaryEvents.begin(); it != supplementaryEvents.end(); ++it) {
		(*message)[it->second->getScope()].push_back(it->second);
		it->second.reset();
	}

	handler->handle(message);


}

}
