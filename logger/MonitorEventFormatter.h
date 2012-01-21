/* ============================================================
 *
 * This file is part of the RSB project
 *
 * Copyright (C) 2012 Arne Nordmann <anordman@techfak.uni-bielefeld.de>
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

#include <rsb/Scope.h>

#include "StatisticsEventFormatter.h"

/**
 * This formatter prints statistical information about received events
 * on a scope and all its sub-scopes, instead of the actual events and
 * their pay-loads.
 *
 * @author anordman
 */
class MonitorEventFormatter: public EventFormatter {
public:
    MonitorEventFormatter(std::ostream &stream, double printFrequency);

    ~MonitorEventFormatter();

    static EventFormatter* create(const rsc::runtime::Properties &props);

    void format(std::ostream &stream, rsb::EventPtr event);
private:
    typedef std::list<std::pair<std::string, QuantityPtr> > QuantitiesMap;
    typedef std::map<rsb::Scope, QuantitiesMap> ScopeMap;

    ScopeMap scopes;
    boost::recursive_mutex quantitiesMutex;

    std::ostream &stream;
    double printFrequency;

    boost::shared_ptr<boost::thread> thread;
    volatile bool terminate;

    void printStats();

    void printHeader();

    void printQuantity(QuantityPtr quantity);

    void run();
};
