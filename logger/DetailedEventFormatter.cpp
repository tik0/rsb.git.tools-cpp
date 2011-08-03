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

#include "DetailedEventFormatter.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include "PayloadFormatter.h"

using namespace std;

using namespace boost;
using namespace boost::posix_time;

using namespace rsc::runtime;

using namespace rsb;

ptime unixMicroSecnodsToPtime(uint64_t msecs) {
    typedef boost::date_time::c_local_adjustor<ptime> local_adj;

    time_t time = msecs / 1000000;
    ptime temp1 = from_time_t(time);
    ptime temp2(temp1.date(),
                temp1.time_of_day() + microseconds(msecs % 1000000));
    return local_adj::utc_to_local(temp2);
}

EventFormatter* DetailedEventFormatter::create(const Properties &/*props*/) {
    return new DetailedEventFormatter();
}

void DetailedEventFormatter::format(ostream &stream, EventPtr event) {
    stream << "Event" << std::endl
           << "  Scope           " << event->getScope().toString() << std::endl
           << "  Id              " << event->getId().getIdAsString() << std::endl
           << "  Sequence Number " << event->getSequenceNumber() << std::endl
           << "  Type            " << event->getType() << std::endl
           << "  Origin          " << event->getMetaData().getSenderId().getIdAsString() << std::endl;

    const MetaData& metaData = event->getMetaData();

    stream << "Timestamps" << std::endl
           << "  Create  " << unixMicroSecnodsToPtime(metaData.getCreateTime()) << "+??:??" << std::endl
           << "  Send    " << unixMicroSecnodsToPtime(metaData.getSendTime()) << "+??:??" << std::endl
           << "  Receive " << unixMicroSecnodsToPtime(metaData.getReceiveTime()) << "+??:??" << std::endl
           << "  Deliver " << unixMicroSecnodsToPtime(metaData.getDeliverTime()) << "+??:??" << std::endl;
    for (map<string, uint64_t>::const_iterator it = metaData.userTimesBegin();
         it != metaData.userTimesEnd(); ++it) {
        stream << "  *" << left << setw(6) << it->first
               << " " << unixMicroSecnodsToPtime(it->second) << "+??:??" << std::endl;
    }

    if (metaData.userInfosBegin() != metaData.userInfosEnd()) {
        stream << "User-Infos" << std::endl;
        for (map<string, string>::const_iterator it = metaData.userInfosBegin();
             it != metaData.userInfosEnd(); ++it) {
            stream << "  " << left << setw(8) << it->first
                   << " " << it->second << std::endl;
        }
    }

    PayloadFormatterPtr formatter = getPayloadFormatter(event);
    stream << "Payload (" << event->getType();
    string extra = formatter->getExtraTypeInfo(event);
    if (!extra.empty()) {
	stream << ", " << extra;
    }
    stream   << ")" << std::endl << "  ";
    formatter->format(stream, event);
    stream << std::endl;

    stream << string(79, '-') << std::endl;
}
