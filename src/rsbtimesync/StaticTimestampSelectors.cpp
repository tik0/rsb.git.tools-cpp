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

#include "StaticTimestampSelectors.h"

#include <rsb/MetaData.h>

using namespace std;
using namespace rsb;

namespace rsbtimesync {

CreateTimestampSelector::~CreateTimestampSelector() {
}

void CreateTimestampSelector::getTimestamp(const EventPtr &event,
        boost::uint64_t &timestamp, string &name) {
    timestamp = event->mutableMetaData().getCreateTime();
    name = TimestampSelector::CREATE;
}

SendTimestampSelector::~SendTimestampSelector() {
}

void SendTimestampSelector::getTimestamp(const EventPtr &event,
        boost::uint64_t &timestamp, string &name) {
    timestamp = event->mutableMetaData().getSendTime();
    name = TimestampSelector::SEND;
}

ReceiveTimestampSelector::~ReceiveTimestampSelector() {
}

void ReceiveTimestampSelector::getTimestamp(const EventPtr &event,
        boost::uint64_t &timestamp, string &name) {
    timestamp = event->mutableMetaData().getReceiveTime();
    name = TimestampSelector::RECEIVE;
}

DeliverTimestampSelector::~DeliverTimestampSelector() {
}

void DeliverTimestampSelector::getTimestamp(const EventPtr &event,
        boost::uint64_t &timestamp, string &name) {
    timestamp = event->mutableMetaData().getDeliverTime();
    name = TimestampSelector::DELIVER;
}

}
