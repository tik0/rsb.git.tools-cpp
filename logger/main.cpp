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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/filter/ScopeFilter.h>

#include <rsb/converter/PredicateConverterList.h>
#include <rsb/converter/ByteArrayConverter.h>

#include "formatting.h"

using namespace std;

using namespace boost;
using namespace boost::posix_time;

using namespace rsc::runtime;

using namespace rsb;
using namespace rsb::converter;

class ShortFormattingHandler: public Handler {
public:
    void handle(EventPtr event) {
	std::cout << "event " << event << std::endl;
    }

    string getClassName() const {
	return "EventFormattingHandler";
    }
};

ptime unixMicroSecnodsToPtime(uint64_t msecs) {
    typedef boost::date_time::c_local_adjustor<ptime> local_adj;

    time_t time = msecs / 1000000;
    ptime temp1 = from_time_t(time);
    ptime temp2(temp1.date(),
		temp1.time_of_day() + microseconds(msecs % 1000000));
    return local_adj::utc_to_local(temp2);
}

class DetailedFormattingHandler: public Handler {
public:
    void handle(EventPtr event) {
	std::cout << "Event" << std::endl
		  << "  Scope  " << event->getScope().toString() << std::endl
		  << "  Id     " << event->getId().getIdAsString() << std::endl
		  << "  Type   " << event->getType() << std::endl
		  << "  Origin " << event->getMetaData().getSenderId().getIdAsString() << std::endl;

	const MetaData& metaData = event->getMetaData();

	std::cout << "Timestamps" << std::endl
	          << "  Create  " << unixMicroSecnodsToPtime(metaData.getCreateTime()) << "+??:??" << std::endl
	          << "  Send    " << unixMicroSecnodsToPtime(metaData.getSendTime()) << "+??:??" << std::endl
                  << "  Receive " << unixMicroSecnodsToPtime(metaData.getReceiveTime()) << "+??:??" << std::endl
	          << "  Deliver " << unixMicroSecnodsToPtime(metaData.getDeliverTime()) << "+??:??" << std::endl;
	for (map<string, uint64_t>::const_iterator it = metaData.userTimesBegin();
	     it != metaData.userTimesEnd(); ++it) {
	    std::cout << "  *" << left << setw(6) << it->first
	              << " " << unixMicroSecnodsToPtime(it->second) << "+??:??" << std::endl;
	}

	if (metaData.userInfosBegin() != metaData.userInfosEnd()) {
	    std::cout << "User-Infos" << std::endl;
	    for (map<string, string>::const_iterator it = metaData.userInfosBegin();
		 it != metaData.userInfosEnd(); ++it) {
		std::cout << "  " << left << setw(8) << it->first
			  << " " << it->second << std::endl;
	    }
	}

	PayloadFormatterPtr formatter = getFormatter(event);
	std::cout << "Payload" << std::endl
		  << "  ";
	formatter->format(std::cout, event);
	std::cout << std::endl;

	std::cout << string(79, '-') << std::endl;
    }

    string getClassName() const {
	return "EventFormattingHandler";
    }
};

template <typename WireType>
typename ConverterSelectionStrategy<WireType>::Ptr createConverterSelectionStrategy() {
    list< pair<ConverterPredicatePtr, typename Converter<WireType>::Ptr> > converters;
    converters.push_back(make_pair(ConverterPredicatePtr(new AlwaysApplicable()),
				   typename Converter<WireType>::Ptr(new ByteArrayConverter())));
    return typename ConverterSelectionStrategy<WireType>::Ptr(new PredicateConverterList<WireType>(converters.begin(), converters.end()));
}

bool doTerminate = false;
recursive_mutex terminateMutex;
condition terminateCondition;

void handleSIGINT(int /*signal*/) {
    recursive_mutex::scoped_lock lock(terminateMutex, try_to_lock);
    doTerminate = true;
    terminateCondition.notify_all();
}

int main(int argc, char* argv[]) {
    ParticipantConfig config
	= Factory::getInstance().getDefaultParticipantConfig();
    ParticipantConfig::Transport transport = config.getTransport("spread");
    Properties options = transport.getOptions();
    options["converters"] = createConverterSelectionStrategy<string>();
    transport.setOptions(options);
    config.addTransport(transport);
    ListenerPtr listener
	= Factory::getInstance().createListener(Scope(argv[1]), config);
    listener->addHandler(HandlerPtr(new DetailedFormattingHandler()));

    // Wait until termination is requested through the SIGINT handler.
    signal(SIGINT, &handleSIGINT);
    while (!doTerminate) {
	recursive_mutex::scoped_lock lock(terminateMutex, try_to_lock);
	terminateCondition.wait(lock);
    }

    return EXIT_SUCCESS;
}
