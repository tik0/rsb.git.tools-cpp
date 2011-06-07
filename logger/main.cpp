/* ============================================================
 *
 * This file is part of the RSB project
 *
 * Copyright (C) 2011 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include <signal.h>

#include <iostream>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>

#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/filter/ScopeFilter.h>

using namespace std;
using namespace boost;
using namespace rsb;

class ShortFormattingHandler: public Handler {
public:
    void handle(EventPtr event) {
	std::cout << "event " << event << std::endl;
    }

    string getClassName() const {
	return "EventFormattingHandler";
    }
};

class DetailedFormattingHandler: public Handler {
public:
    void handle(EventPtr event) {
	std::cout << "Event" << std::endl
		  << "  Scope  " << event->getScope() << std::endl
		  << "  Id     " << event->getId() << std::endl
		  << "  Type   " << event->getType() << std::endl
		  << "  Origin " << event->getMetaData().getSenderId() << std::endl;

	const MetaData& metaData = event->getMetaData();

	std::cout << "Timestamps" << std::endl
		  << "  Create  " << metaData.getCreateTime() << std::endl
		  << "  Send    " << metaData.getSendTime() << std::endl
		  << "  Receive " << metaData.getReceiveTime() << std::endl
		  << "  Deliver " << metaData.getDeliverTime() << std::endl;

	if (metaData.userInfosBegin() != metaData.userInfosEnd()) {
	    std::cout << "Meta-data" << std::endl;
	    for (map<string, string>::const_iterator it = metaData.userInfosBegin();
		 it != metaData.userInfosEnd(); ++it) {
		std::cout << "  " << left << setw(8) << it->first
			  << " " << it->second << std::endl;
	    }
	}

	std::cout << "Payload" << std::endl
		  << "  " << *static_pointer_cast<string>(event->getData()) << std::endl;

	std::cout << string(80, '-') << std::endl;
    }

    string getClassName() const {
	return "EventFormattingHandler";
    }
};

bool doTerminate = false;
recursive_mutex terminateMutex;
condition terminateCondition;

void handleSIGINT(int /*signal*/) {
    recursive_mutex::scoped_lock lock(terminateMutex, try_to_lock);
    doTerminate = true;
    terminateCondition.notify_all();
}

int main(int argc, char* argv[]) {
    ListenerPtr listener = Factory::getInstance().createListener(Scope(argv[1]));
    listener->addHandler(HandlerPtr(new DetailedFormattingHandler()));

    // Wait until termination is requested through the SIGINT handler.
    signal(SIGINT, &handleSIGINT);
    while (!doTerminate) {
	recursive_mutex::scoped_lock lock(terminateMutex, try_to_lock);
	terminateCondition.wait(lock);
    }

    return EXIT_SUCCESS;
}
