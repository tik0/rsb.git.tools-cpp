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

#include "PayloadOnlyEventFormatter.h"

#include "PayloadFormatter.h"

using namespace std;

using namespace rsc::runtime;

using namespace rsb;

namespace rsb {
namespace tools {
namespace logger {

PayloadOnlyEventFormatter::PayloadOnlyEventFormatter(bool printNewline):
    printNewline(printNewline) {
}

EventFormatter* PayloadOnlyEventFormatter::create(const Properties &props) {
    return new PayloadOnlyEventFormatter(props.get<bool>("printNewline", true));
}

void PayloadOnlyEventFormatter::format(ostream &stream, EventPtr event) {
    PayloadFormatterPtr formatter = getPayloadFormatter(event);
    formatter->format(stream, event);
    if (this->printNewline) {
        stream << endl;
    }
}

}
}
}
