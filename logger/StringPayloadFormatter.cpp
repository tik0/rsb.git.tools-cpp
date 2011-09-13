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

#include "StringPayloadFormatter.h"

using namespace std;

using namespace boost;

using namespace rsc::runtime;

using namespace rsb;

StringPayloadFormatter::StringPayloadFormatter(unsigned int indent,
					       unsigned int maxLines,
					       unsigned int maxColumns):
    indent(indent), maxLines(maxLines), maxColumns(maxColumns) {
}

PayloadFormatter* StringPayloadFormatter::create(const Properties &props) {
    return new StringPayloadFormatter(props.get<unsigned int>("indent",     2),
				      props.get<unsigned int>("maxLines",   4),
				      props.get<unsigned int>("maxColumns", 79));
}

string StringPayloadFormatter::getExtraTypeInfo(EventPtr event) const {
    boost::shared_ptr<string> data = boost::static_pointer_cast<string>(event->getData());

    return str(boost::format("length %1%") % data->size());
}

void StringPayloadFormatter::format(ostream &stream, EventPtr event) {
    shared_ptr<string> data = boost::static_pointer_cast<string>(event->getData());
    if (!data) {
	stream << "Failed to decode event data as string." << endl
	       << "  Event: " << event << endl;
	return;
    }

    /** TODO(jmoringe): this breaks down if data contains newlines, tabs etc. */
    unsigned int line   = 0;
    unsigned int column = this->indent;
    string::const_iterator it;

    for (it = data->begin();
	 (it != data->end())
	     && (line < this->maxLines)
	     && ((line < (this->maxLines - 1))
		 || (column < (this->maxColumns - 3)));
	 ++it, ++column) {
	char c = *it;
	if (c > 31) {
	    stream << c;
	} else {
	    stream << "?";
	}
	if (column == (this->maxColumns - 1)) {
	    column = this->indent - 1;
	    ++line;
	    stream << endl << string(this->indent, ' ');
	}
    }
    if (it != data->end()) {
	stream << "...";
    }
}
