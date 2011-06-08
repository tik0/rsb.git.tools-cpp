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

#include "BytesPayloadFormatter.h"

#include <iomanip>

#include <boost/io/ios_state.hpp>

using namespace std;

using namespace boost;
using namespace boost::io;

using namespace rsc::runtime;

using namespace rsb;

BytesPayloadFormatter::BytesPayloadFormatter(unsigned int indent,
					       unsigned int maxLines,
					       unsigned int maxColumns):
    indent(indent), maxLines(maxLines), maxColumns(maxColumns) {
}

PayloadFormatter* BytesPayloadFormatter::create(const Properties &props) {
    return new BytesPayloadFormatter(props.get<unsigned int>("indent",     2),
				      props.get<unsigned int>("maxLines",   4),
				      props.get<unsigned int>("maxColumns", 79));
}

void BytesPayloadFormatter::format(ostream &stream, EventPtr event) {
    shared_ptr<string> data = static_pointer_cast<string>(event->getData());

    unsigned int	   line   = 0;
    unsigned int	   column = this->indent;
    unsigned int	   offset = 0;
    string::const_iterator it     = data->begin();

    ios_all_saver saver(stream);

    for (; (it != data->end())
	     && (line < this->maxLines)
	     && ((line < (this->maxLines - 1))
		 || (column < (this->maxColumns - 4)));
	 ++it, ++offset) {
	if (column == this->indent) {
	    stream << "0x" << setw(4) << setfill('0') << right << hex << offset << " ";
	    column += 7;
	}
	
	stream << setw(2) << hex << static_cast<unsigned int>(*it) << " ";	
	column += 3;
	if (column >= (this->maxColumns - 3)) {
	    column = this->indent;
	    ++line;
	    stream << endl << string(this->indent, ' ');
	}
    }
    if (it != data->end()) {
	stream << "...";
    }
}
