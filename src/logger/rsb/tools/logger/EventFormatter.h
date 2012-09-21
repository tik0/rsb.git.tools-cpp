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

#pragma once

#include <iostream>
#include <set>

#include <boost/shared_ptr.hpp>

#include <rsc/patterns/Factory.h>
#include <rsc/patterns/Singleton.h>

#include <rsb/Event.h>

namespace rsb {
namespace tools {
namespace logger {

/**
 * Implementations of this interface format @ref rsb::Event
 * objects. Each implementation formats events in a specific way.
 *
 * @author jmoringe
 */
class EventFormatter {
public:
    virtual ~EventFormatter();

    /**
     * Format @a event onto @a stream ..
     *
     * @param stream The stream onto which the event should be
     * formatted.
     * @param event The event that should be formatted.
     */
    virtual void format(std::ostream &stream, rsb::EventPtr event) = 0;
};

typedef boost::shared_ptr<EventFormatter> EventFormatterPtr;

class EventFormatterFactory: public rsc::patterns::Factory<std::string, EventFormatter>,
                             public rsc::patterns::Singleton<EventFormatterFactory> {
    friend class rsc::patterns::Singleton<EventFormatterFactory>;
private:
    EventFormatterFactory();
};

std::set<std::string> getEventFormatterNames();

EventFormatterPtr getEventFormatter(rsb::EventPtr event);

}
}
}
