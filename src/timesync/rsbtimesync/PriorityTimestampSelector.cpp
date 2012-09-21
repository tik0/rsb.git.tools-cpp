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

#include "PriorityTimestampSelector.h"

#include <rsc/runtime/ContainerIO.h>

#include <rsb/MetaData.h>

using namespace std;
using namespace rsb;

namespace rsbtimesync {

PriorityTimestampSelector::PriorityTimestampSelector(
        const std::vector<TimestampSelectorPtr> &selectorsByPriority) :
        selectorsByPriority(selectorsByPriority) {
}

PriorityTimestampSelector::~PriorityTimestampSelector() {
}

void PriorityTimestampSelector::getTimestamp(const EventPtr &event,
        boost::uint64_t &timestamp, string &name) {

    for (vector<TimestampSelectorPtr>::iterator selectorIt =
            selectorsByPriority.begin();
            selectorIt != selectorsByPriority.end(); ++selectorIt) {
        try {
            (*selectorIt)->getTimestamp(event, timestamp, name);
            return;
        } catch (NoSuchTimestampException &/*e*/) {
            continue;
        }
    }

    // TODO add concrete list to exception string
    throw NoSuchTimestampException("priority list");

}

string PriorityTimestampSelector::getClassName() const {
    return "PriorityTimestampSelector";
}

void PriorityTimestampSelector::printContents(std::ostream &stream) const {
    stream << "selectorsByPriority = " << selectorsByPriority;
}

}
