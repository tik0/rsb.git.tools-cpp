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

string CreateTimestampSelector::getClassName() const {
    return "CreateTimestampSelector";
}

SendTimestampSelector::~SendTimestampSelector() {
}

void SendTimestampSelector::getTimestamp(const EventPtr &event,
        boost::uint64_t &timestamp, string &name) {
    timestamp = event->mutableMetaData().getSendTime();
    name = TimestampSelector::SEND;
}

string SendTimestampSelector::getClassName() const {
    return "SendTimestampSelector";
}

ReceiveTimestampSelector::~ReceiveTimestampSelector() {
}

void ReceiveTimestampSelector::getTimestamp(const EventPtr &event,
        boost::uint64_t &timestamp, string &name) {
    timestamp = event->mutableMetaData().getReceiveTime();
    name = TimestampSelector::RECEIVE;
}

string ReceiveTimestampSelector::getClassName() const {
    return "ReceiveTimestampSelector";
}

DeliverTimestampSelector::~DeliverTimestampSelector() {
}

void DeliverTimestampSelector::getTimestamp(const EventPtr &event,
        boost::uint64_t &timestamp, string &name) {
    timestamp = event->mutableMetaData().getDeliverTime();
    name = TimestampSelector::DELIVER;
}

string DeliverTimestampSelector::getClassName() const {
    return "DeliverTimestampSelector";
}

UserTimestampSelector::UserTimestampSelector(const string &name) :
        name(name) {
}

UserTimestampSelector::~UserTimestampSelector() {
}

void UserTimestampSelector::getTimestamp(const EventPtr &event,
        boost::uint64_t &timestamp, string &name) {
    if (!event->mutableMetaData().hasUserTime(this->name)) {
        throw NoSuchTimestampException(this->name);
    }
    timestamp = event->mutableMetaData().getUserTime(this->name);
    name = this->name;
}

string UserTimestampSelector::getClassName() const {
    return "UserTimestampSelector";
}

void UserTimestampSelector::printContents(ostream &stream) const {
    stream << "name = " << name;
}

}
