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

#include <rsb/EventId.h>

#include <rsb/EventCollections.h>

using namespace std;
using namespace rsb;

namespace rsbtimesync {

FirstMatchStrategy::FirstMatchStrategy() {
}

FirstMatchStrategy::~FirstMatchStrategy() {
}

string FirstMatchStrategy::getClassName() const {
    return "FirstMatchStrategy";
}

string FirstMatchStrategy::getKey() const {
    return "firstmatch";
}

void FirstMatchStrategy::setSyncDataHandler(SyncDataHandlerPtr handler) {
    this->handler = handler;
}

void FirstMatchStrategy::initializeChannels(const Scope &primaryScope,
        const set<Scope> &subsidiaryScopes) {
    this->primaryScope = primaryScope;
    for (set<Scope>::const_iterator scopeIt = subsidiaryScopes.begin();
            scopeIt != subsidiaryScopes.end(); ++scopeIt) {
        supplementaryEvents[*scopeIt].reset();
    }
}

void FirstMatchStrategy::handle(EventPtr event) {

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
    for (std::map<Scope, EventPtr>::const_iterator it =
            supplementaryEvents.begin(); it != supplementaryEvents.end();
            ++it) {
        if (!it->second) {
            return;
        }
    }

    EventPtr resultEvent = handler->createEvent();

    // all buffers are filled, we can emit an event
    boost::shared_ptr<EventsByScopeMap> message(new EventsByScopeMap);
    (*message)[primaryEvent->getScope()].push_back(primaryEvent);
    resultEvent->addCause(primaryEvent->getEventId());
    primaryEvent.reset();
    for (std::map<Scope, EventPtr>::iterator it = supplementaryEvents.begin();
            it != supplementaryEvents.end(); ++it) {
        (*message)[it->second->getScope()].push_back(it->second);
        resultEvent->addCause(it->second->getEventId());
        it->second.reset();
    }
    resultEvent->setData(message);

    handler->handle(resultEvent);

}

void FirstMatchStrategy::provideOptions(
        boost::program_options::options_description &optionDescription) {
    optionDescription.add_options()((getKey() + "-foo").c_str(),
            "do not use this, it's a test");
}

void FirstMatchStrategy::handleOptions(
        const boost::program_options::variables_map &options) {
    if (options.count((getKey() + "-foo").c_str())) {
        throw invalid_argument("You should not use this option!");
    }
}

}
