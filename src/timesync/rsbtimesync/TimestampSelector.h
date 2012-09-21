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

#pragma once

#include <string>

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#include <rsc/runtime/Printable.h>

#include <rsb/Event.h>

namespace rsbtimesync {

/**
 * Interface for classes which implement a strategy to select timestamps from
 * RSB events.
 *
 * @author jwienke
 */
class TimestampSelector: public virtual rsc::runtime::Printable {
public:

    /**
     * Thrown to indicate that a desired timestamp does not exist.
     *
     * @author jwienke
     */
    class NoSuchTimestampException: public std::runtime_error {
    public:
        NoSuchTimestampException(const std::string &name);
        virtual ~NoSuchTimestampException() throw();
    };

    TimestampSelector();
    virtual ~TimestampSelector();

    virtual std::string getClassName() const;

    /**
     * Selects a timestamp from the given event according to the implemented
     * strategy.
     *
     * @param event event to get the timestamp from
     * @param timestamp return parameter with the selected timestamp value
     * @param name return parameter with the name of the selected timestamp
     * @throw NoSuchTimestampException desired timestamp does not exist in the
     *                                 given event
     */
    virtual void getTimestamp(const rsb::EventPtr &event,
            boost::uint64_t &timestamp, std::string &name) = 0;

    /**
     * Selects a timestamp from the given event according to the implemented
     * strategy.
     *
     * @param event event to get the timestamp from
     */
    virtual boost::uint64_t getTimestamp(const rsb::EventPtr &event);

    static const std::string CREATE;
    static const std::string SEND;
    static const std::string RECEIVE;
    static const std::string DELIVER;

    static std::set<std::string> systemNames();

};

typedef boost::shared_ptr<TimestampSelector> TimestampSelectorPtr;

}
