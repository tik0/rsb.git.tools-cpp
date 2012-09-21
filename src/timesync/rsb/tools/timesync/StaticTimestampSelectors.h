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

#include "TimestampSelector.h"

namespace rsb {
namespace tools {
namespace timesync {

/**
 * A TimestampSelector specifically optimized for accessing the create
 * timestamp.
 *
 * @author jwienke
 */
class CreateTimestampSelector: public TimestampSelector {
public:

    virtual ~CreateTimestampSelector();

    virtual void getTimestamp(const rsb::EventPtr &event,
            boost::uint64_t &timestamp, std::string &name);

    virtual std::string getClassName() const;

};

/**
 * A TimestampSelector specifically optimized for accessing the send
 * timestamp.
 *
 * @author jwienke
 */
class SendTimestampSelector: public TimestampSelector {
public:

    virtual ~SendTimestampSelector();

    virtual void getTimestamp(const rsb::EventPtr &event,
            boost::uint64_t &timestamp, std::string &name);

    virtual std::string getClassName() const;

};

/**
 * A TimestampSelector specifically optimized for accessing the receive
 * timestamp.
 *
 * @author jwienke
 */
class ReceiveTimestampSelector: public TimestampSelector {
public:

    virtual ~ReceiveTimestampSelector();

    virtual void getTimestamp(const rsb::EventPtr &event,
            boost::uint64_t &timestamp, std::string &name);

    virtual std::string getClassName() const;

};

/**
 * A TimestampSelector specifically optimized for accessing the deliver
 * timestamp.
 *
 * @author jwienke
 */
class DeliverTimestampSelector: public TimestampSelector {
public:

    virtual ~DeliverTimestampSelector();

    virtual void getTimestamp(const rsb::EventPtr &event,
            boost::uint64_t &timestamp, std::string &name);

    virtual std::string getClassName() const;

};

/**
 * A TimestampSelector specifically optimized for accessing a single user
 * timestamp.
 *
 * @author jwienke
 */
class UserTimestampSelector: public TimestampSelector {
public:

    UserTimestampSelector(const std::string &name);
    virtual ~UserTimestampSelector();

    virtual void getTimestamp(const rsb::EventPtr &event,
            boost::uint64_t &timestamp, std::string &name);

    virtual std::string getClassName() const;
    virtual void printContents(std::ostream &stream) const;

private:
    std::string name;

};

}
}
}
