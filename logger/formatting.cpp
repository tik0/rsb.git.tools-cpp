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

#include "formatting.h"
#include "StringPayloadFormatter.h"
#include "BytesPayloadFormatter.h"

using namespace rsc::patterns;

using namespace rsb;

PayloadFormatterFactory::PayloadFormatterFactory() {
    this->register_("std::string", &StringPayloadFormatter::create);
    this->register_("bytes",       &BytesPayloadFormatter::create);
}

PayloadFormatterPtr getFormatter(EventPtr event) {
    PayloadFormatterFactory &factory = PayloadFormatterFactory::getInstance();

    try {
	return PayloadFormatterPtr(factory.createInst(event->getType()));
    } catch (const NoSuchImpl&) {
        return PayloadFormatterPtr(new BytesPayloadFormatter());
    }
}
