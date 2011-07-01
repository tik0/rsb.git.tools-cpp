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

#include "PayloadFormatter.h"

/**
 * A formatter for protocol buffer message payloads.
 *
 * @author jmoringe
 */
class ProtocolBufferPayloadFormatter: public PayloadFormatter {
public:
    ProtocolBufferPayloadFormatter(unsigned int indent = 2);

    static PayloadFormatter* create(const rsc::runtime::Properties &props);

    std::string getExtraTypeInfo(rsb::EventPtr event) const;

    void format(std::ostream &stream, rsb::EventPtr event);
private:
    unsigned int indent;
};
