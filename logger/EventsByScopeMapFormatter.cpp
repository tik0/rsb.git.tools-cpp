/* ============================================================
 *
 * This file is part of the RSB project
 *
 * Copyright (C) 2012 Johannes Wienke <jwienke at techfak dot uni-bielefeld dot de>
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

#include "EventsByScopeMapFormatter.h"

#include <sstream>

#include <boost/format.hpp>

#include <rsb/EventCollections.h>

#include "DetailedEventFormatter.h"

using namespace std;

using namespace rsc::runtime;

using namespace rsb;

EventsByScopeMapFormatter::EventsByScopeMapFormatter(
        EventFormatterPtr containedFormatter) :
        containedFormatter(containedFormatter) {
}

PayloadFormatter* EventsByScopeMapFormatter::create(const Properties& props) {
    return new EventsByScopeMapFormatter(
            props.get<EventFormatterPtr>("containedFormatter",
                    EventFormatterPtr(new DetailedEventFormatter(6))));
}

string EventsByScopeMapFormatter::getExtraTypeInfo(EventPtr event) const {
    boost::shared_ptr<EventsByScopeMap> data = boost::static_pointer_cast<
            EventsByScopeMap>(event->getData());

    stringstream s;
    for (EventsByScopeMap::const_iterator scopeIt = data->begin();
            scopeIt != data->end(); ++scopeIt) {
        s << "(";
        s << scopeIt->first.toString();
        s << ", ";
        s << scopeIt->second.size();
        s << ")";
        if (scopeIt != (--data->end())) {
            s << ", ";
        }
    }

    return s.str();

}

void EventsByScopeMapFormatter::format(ostream &stream, EventPtr event) {
    boost::shared_ptr<EventsByScopeMap> data = boost::static_pointer_cast<
            EventsByScopeMap>(event->getData());

    for (EventsByScopeMap::const_iterator scopeIt = data->begin();
            scopeIt != data->end(); ++scopeIt) {
        vector<EventPtr> containedEvents = scopeIt->second;
        if (scopeIt != data->begin()) {
            stream << "  ";
        }
        stream << "*** " << scopeIt->first.toString() << "("
                << containedEvents.size() << "):" << endl;
        for (vector<EventPtr>::const_iterator eventIt = containedEvents.begin();
                eventIt != containedEvents.end(); ++eventIt) {
            containedFormatter->format(stream, *eventIt);
        }
    }

}
