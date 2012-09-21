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

#pragma once

#include "EventFormatter.h"
#include "PayloadFormatter.h"

namespace rsb {
namespace tools {
namespace logger {

/**
 * A formatter for EventsByScopeMap containing events.
 *
 * @author jwienke
 */
class EventsByScopeMapFormatter: public PayloadFormatter {
public:
    /**
     * Constructs a new EventsByScopeMapFormatter.
     *
     * @param containedFormatter formatter to use for events contained in the map
     */
    EventsByScopeMapFormatter(EventFormatterPtr containedFormatter);

    static PayloadFormatter* create(const rsc::runtime::Properties &props);

    std::string getExtraTypeInfo(rsb::EventPtr event) const;

    void format(std::ostream &stream, rsb::EventPtr event);
private:
    /**
     * Formatter used for contained events in the map.
     */
    EventFormatterPtr containedFormatter;
};

}
}
}
