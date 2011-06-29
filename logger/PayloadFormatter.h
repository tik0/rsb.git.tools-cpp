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

#include <iostream>

#include <boost/shared_ptr.hpp>

#include <rsc/patterns/Factory.h>
#include <rsc/patterns/Singleton.h>

#include <rsb/Event.h>

/**
 * Implementations of this interface format @ref rsb::Event
 * payloads. Each implementation is capable of formatting one payload
 * type.
 *
 * @author jmoringe
 */
class PayloadFormatter {
public:
    virtual ~PayloadFormatter();

    /**
     * Return a string which provides additional information regarding
     * the type of the event payload. For example, the lenght of a
     * string or array payload could be returned here.
     *
     * @param event The event, from which the information should be
     * obtained.
     * @return A string containing additional information regarding
     * the payload type.
     */
    virtual std::string getExtraTypeInfo(rsb::EventPtr event) const;

    /**
     * Format the payload of @a event onto @a stream in type-dependent
     * (of the payload) way.
     *
     * @param stream The stream onto which the payload should be
     * formatted.
     * @param event The event from which the paylaod should be
     * extracted. The payload returned by @ref rsb::Event::getData has
     * to be of the type indicated by @ref rsb::Event::getType .
     */
    virtual void format(std::ostream &stream, rsb::EventPtr event) = 0;
};

typedef boost::shared_ptr<PayloadFormatter> PayloadFormatterPtr;

class PayloadFormatterFactory: public rsc::patterns::Factory<std::string, PayloadFormatter>,
			       public rsc::patterns::Singleton<PayloadFormatterFactory> {
friend class rsc::patterns::Singleton<PayloadFormatterFactory>;
private:
    PayloadFormatterFactory();
};

PayloadFormatterPtr getPayloadFormatter(rsb::EventPtr event);
