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

#include "EventFormatter.h"

#include <algorithm>

#include "CompactEventFormatter.h"
#include "DetailedEventFormatter.h"
#ifndef RSB_LOGGER_NO_STATISTICS_FORMATTER
#include "StatisticsEventFormatter.h"
#include "MonitorEventFormatter.h"
#endif
#include "PayloadOnlyEventFormatter.h"

using namespace std;

using namespace rsc::patterns;

using namespace rsb;

EventFormatter::~EventFormatter() {
}

EventFormatterFactory::EventFormatterFactory() {
    this->register_("compact", &CompactEventFormatter::create);
    this->register_("detailed", &DetailedEventFormatter::create);
#ifndef RSB_LOGGER_NO_STATISTICS_FORMATTER
    this->register_("stats", &StatisticsEventFormatter::create);
    this->register_("monitor", &MonitorEventFormatter::create);
#endif
    this->register_("payload", &PayloadOnlyEventFormatter::create);
}

set<string> getEventFormatterNames() {
    set<string> result;

    EventFormatterFactory &factory = EventFormatterFactory::getInstance();
    for (EventFormatterFactory::ImplMapProxy::const_iterator it =
            factory.impls().begin(); it != factory.impls().end(); ++it) {
        result.insert(it->first);
    }

    return result;
}

EventFormatterPtr getEventFormatter(EventPtr event) {
    EventFormatterFactory &factory = EventFormatterFactory::getInstance();

    return EventFormatterPtr(factory.createInst(event->getType()));
}
