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

#include "TimestampSelector.h"

namespace rsbtimesync {

const std::string TimestampSelector::CREATE = "rsb::create";
const std::string TimestampSelector::SEND = "rsb::send";
const std::string TimestampSelector::RECEIVE = "rsb::receive";
const std::string TimestampSelector::DELIVER = "rsb::deliver";

TimestampSelector::NoSuchTimestampException::NoSuchTimestampException(
        const std::string &name) :
        std::runtime_error(
                "Event does not contain a timestamp with name '" + name + "'") {

}

TimestampSelector::NoSuchTimestampException::~NoSuchTimestampException() throw () {

}

TimestampSelector::TimestampSelector() {
}

TimestampSelector::~TimestampSelector() {
}

std::string TimestampSelector::getClassName() const {
    return "TimestampSelector";
}

std::set<std::string> TimestampSelector::systemNames() {
    std::set<std::string> names;
    names.insert(CREATE);
    names.insert(SEND);
    names.insert(RECEIVE);
    names.insert(DELIVER);
    return names;
}

boost::uint64_t TimestampSelector::getTimestamp(const rsb::EventPtr &event) {
    boost::uint64_t timestamp;
    std::string dummy;
    getTimestamp(event, timestamp, dummy);
    return timestamp;
}

}
