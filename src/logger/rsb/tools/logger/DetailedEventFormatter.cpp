/* ============================================================
 *
 * This file is part of the RSB project
 *
 * Copyright (C) 2011-2017 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include <rsb/Scope.h>
#include <rsb/EventId.h>
#include <rsb/MetaData.h>

#include "PayloadFormatter.h"

using namespace std;

using namespace boost;
using namespace boost::posix_time;

using namespace rsc::runtime;

using namespace rsb;

namespace rsb {
namespace tools {
namespace logger {

ptime unixMicroSecondsToPtime(uint64_t msecs) {
    typedef boost::date_time::c_local_adjustor<ptime> local_adj;

    time_t time = msecs / 1000000;
    ptime temp1 = from_time_t(time);
    ptime temp2(temp1.date(),
                temp1.time_of_day() + microseconds(msecs % 1000000));
    return local_adj::utc_to_local(temp2);
}

EventFormatter* DetailedEventFormatter::create(const Properties &props) {
    return new DetailedEventFormatter(props.getAs<unsigned int>("indentSpaces", 0),
                                      props.getAs<bool>        ("separator",    true));
}

DetailedEventFormatter::DetailedEventFormatter(unsigned int indentSpaces,
                                               bool         separator)
    : separator(separator) {
    indent.append(indentSpaces, ' ');
}

void DetailedEventFormatter::format(ostream &stream, EventPtr event) {
    stream << indent << "Event" << std::endl
           << indent << "  Scope           " << event->getScopePtr()->toString() << std::endl
           << indent << "  Id              " << event->getId() << std::endl
           << indent << "  Type            " << event->getType() << std::endl
           << indent << "  Origin          " << event->getId().getParticipantId().getIdAsString() << std::endl;

    const MetaData& metaData = event->getMetaData();

    stream << indent << "Timestamps" << std::endl
           << indent << "  Create  " << unixMicroSecondsToPtime(metaData.getCreateTime()) << "+??:??" << std::endl
           << indent << "  Send    " << unixMicroSecondsToPtime(metaData.getSendTime()) << "+??:??" << std::endl
           << indent << "  Receive " << unixMicroSecondsToPtime(metaData.getReceiveTime()) << "+??:??" << std::endl
           << indent << "  Deliver " << unixMicroSecondsToPtime(metaData.getDeliverTime()) << "+??:??" << std::endl;
    for (map<string, uint64_t>::const_iterator it = metaData.userTimesBegin();
         it != metaData.userTimesEnd(); ++it) {
        stream << indent << "  *" << left << setw(6) << it->first
               << " " << unixMicroSecondsToPtime(it->second) << "+??:??" << std::endl;
    }

    if (metaData.userInfosBegin() != metaData.userInfosEnd()) {
        stream << indent << "User-Infos" << std::endl;
        for (map<string, string>::const_iterator it = metaData.userInfosBegin();
             it != metaData.userInfosEnd(); ++it) {
            stream << indent << "  " << left << setw(8) << it->first
                   << " " << it->second << std::endl;
        }
    }

    set<EventId> causes = event->getCauses();
    if (!causes.empty()) {
        stream << indent << "Causes" << std::endl;
        for (set<EventId>::iterator it = causes.begin(); it != causes.end();
                ++it) {
            stream << indent << "  " << *it << std::endl;
        }
    }

    PayloadFormatterPtr formatter = getPayloadFormatter(event);
    stream << indent << "Payload (" << event->getType();
    string extra = formatter->getExtraTypeInfo(event);
    if (!extra.empty()) {
        stream << ", " << extra;
    }
    stream << ")" << std::endl << "  ";
    formatter->format(stream, event);
    stream << std::endl;

    if (this->separator) {
        stream << indent << string(79 - indent.length(), '-') << std::endl;
    }
}

}
}
}
