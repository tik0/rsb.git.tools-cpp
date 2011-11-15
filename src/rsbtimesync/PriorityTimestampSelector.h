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

#include <vector>

#include "TimestampSelector.h"

namespace rsbtimesync {

/**
 * Selects timestamps from events based on a priority list. This allows to
 * specify user times which might not be available in all events. This selector
 * then tries to select the next timestamp from its priority set.
 *
 * @author jwienke
 */
class PriorityTimestampSelector: public rsbtimesync::TimestampSelector {
public:

    PriorityTimestampSelector(
            const std::vector<TimestampSelectorPtr> &selectorsByPriority);
    virtual ~PriorityTimestampSelector();

    virtual void getTimestamp(const rsb::EventPtr &event,
            boost::uint64_t &timestamp, std::string &name);

    virtual std::string getClassName() const;
    virtual void printContents(std::ostream &stream) const;

private:
    std::vector<TimestampSelectorPtr> selectorsByPriority;

};

}
