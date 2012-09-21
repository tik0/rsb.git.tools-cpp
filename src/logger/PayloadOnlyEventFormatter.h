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

#pragma once

#include "EventFormatter.h"

/**
 * An event formatter which only prints event payloads.
 *
 * @author jmoringe
 */
class PayloadOnlyEventFormatter: public EventFormatter {
public:
    PayloadOnlyEventFormatter(bool printNewline);

    static EventFormatter* create(const rsc::runtime::Properties &props);

    void format(std::ostream &stream, rsb::EventPtr event);
private:
    bool printNewline;
};
