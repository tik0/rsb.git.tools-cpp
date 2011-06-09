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

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "EventFormatter.h"

/**
 * Implementations of this interface track and print a single quantity
 * like the rate of received events.
 *
 * @author jmoringe
 */
class Quantity {
public:
    virtual unsigned int getWidth() const = 0;

    virtual void reset() =  0;
    virtual void update(rsb::EventPtr event) = 0;
    virtual void print(std::ostream &stream) = 0;
};

typedef boost::shared_ptr<Quantity> QuantityPtr;

/**
 * This formatter prints statistical information about received events
 * instead of the actual events and their paylaods.
 *
 * @author jmoringe
 */
class StatisticsEventFormatter: public EventFormatter {
public:
    StatisticsEventFormatter(std::ostream &stream,
			     double        printFrequency);

    ~StatisticsEventFormatter();

    static EventFormatter* create(const rsc::runtime::Properties &props);

    void format(std::ostream &stream, rsb::EventPtr event);
private:
    typedef std::list<std::pair< std::string, QuantityPtr> > QuantitiesMap;

    QuantitiesMap                     quantities;

    std::ostream                     &stream;
    unsigned int                      lines;
    double                            printFrequency;

    boost::shared_ptr<boost::thread>  thread;
    volatile bool                     terminate;

    void printStats();

    void printHeader();

    void printQuantity(QuantityPtr quantity);

    void run();
};
