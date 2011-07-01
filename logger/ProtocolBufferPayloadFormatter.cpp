/* ============================================================
 *
 * This file is part of RSB.
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

#include "ProtocolBufferPayloadFormatter.h"

#include <boost/format.hpp>

#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace std;

using namespace boost;

using namespace rsc::runtime;

using namespace rsb;

using namespace google::protobuf;

ProtocolBufferPayloadFormatter::ProtocolBufferPayloadFormatter(unsigned int indent):
    indent(indent) {
}

PayloadFormatter* ProtocolBufferPayloadFormatter::create(const Properties &props) {
    return new ProtocolBufferPayloadFormatter(props.get<unsigned int>("indent", 2));
}

string ProtocolBufferPayloadFormatter::getExtraTypeInfo(EventPtr event) const {
    shared_ptr<Message> message = static_pointer_cast<Message>(event->getData());

    return str(boost::format("type %1%") % message->GetDescriptor()->full_name());
}

void ProtocolBufferPayloadFormatter::format(ostream &stream, EventPtr event) {
    shared_ptr<Message> message = static_pointer_cast<Message>(event->getData());

    google::protobuf::io::OstreamOutputStream ostream(&stream);
    TextFormat::Print(*message, &ostream);
}
