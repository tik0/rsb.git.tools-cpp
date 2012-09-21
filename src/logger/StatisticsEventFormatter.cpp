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

#include "StatisticsEventFormatter.h"

#include <iomanip>

#include <boost/bind.hpp>

#include <boost/thread.hpp>

#include <boost/io/ios_state.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <rsb/MetaData.h>

using namespace std;

using namespace boost;
using namespace boost::io;
using namespace boost::posix_time;

using namespace rsc::runtime;

using namespace rsb;

namespace ac = boost::accumulators;

//

typedef ac::accumulator_set<double,
                            ac::features< ac::tag::mean,
                                          ac::tag::variance(ac::lazy) > > StatsType;

typedef boost::date_time::microsec_clock<boost::posix_time::ptime>        msecsClock;
typedef boost::posix_time::ptime                                          timestamp;
typedef boost::date_time::c_local_adjustor<ptime>                         local_adj;

class Time: public Quantity {
public:
    unsigned int getWidth() const {
        return 27;
    }

    void reset() {}
    void update(EventPtr /*event*/) {}
    void print(ostream &stream)  {
        stream << local_adj::utc_to_local(msecsClock::universal_time());
    }
};

class StatsQuantity: public Quantity {
public:
    unsigned int getWidth() const {
        return 16;
    }

    void reset() {
        this->stats = StatsType();
    }

    void print(ostream &stream)  {
        ios_all_saver saver(stream);

        stream <<fixed << setprecision(2) << right << setw(7) << ac::mean(this->stats)
               << " ±"
               << setw(7) << sqrt(ac::variance(this->stats));
    }
protected:
    void update(double point) {
        this->stats(point);
    }
private:
    StatsType stats;
};

class Latency: public StatsQuantity {
    void update(EventPtr event) {
        StatsQuantity::update(static_cast<double>(event->getMetaData().getDeliverTime())
                              - static_cast<double>(event->getMetaData().getCreateTime()));
    }
};

class Rate: public Quantity {
public:
    Rate():
        count(0), start(msecsClock::universal_time()) {
    }

    unsigned int getWidth() const {
        return 8;
    }

    void reset() {
        this->count = 0;
        this->start = msecsClock::universal_time();
    }

    void update(EventPtr /*event*/) {
        ++this->count;
    }

    void print(ostream &stream)  {
        ios_all_saver saver(stream);

        double delta = static_cast<double>((msecsClock::universal_time() - this->start).total_nanoseconds()) / 1000000000.0;
        stream << setw(getWidth() - 3) << fixed << setprecision(0) << right << static_cast<double>(this->count) / delta << " Hz";
    }
private:
    unsigned int count;
    timestamp start;
};

//

StatisticsEventFormatter::StatisticsEventFormatter(ostream &stream,
                                                   double   printFrequency):
    stream(stream), lines(0), printFrequency(printFrequency) {
    this->quantities.push_back(make_pair("Time",    QuantityPtr(new Time())));
    this->quantities.push_back(make_pair("Latency", QuantityPtr(new Latency())));
    this->quantities.push_back(make_pair("Rate",    QuantityPtr(new Rate())));

    this->terminate = false;
    this->thread.reset(new boost::thread(bind(&StatisticsEventFormatter::run, this)));
}

StatisticsEventFormatter::~StatisticsEventFormatter() {
    this->terminate = true;
    this->thread->join();
}

EventFormatter* StatisticsEventFormatter::create(const Properties &props) {
    return new StatisticsEventFormatter(*props.get<ostream*>("stream"),
                                        props.get<double>("print-frequency", 1.0));
}

void StatisticsEventFormatter::format(ostream &/*stream*/, EventPtr event) {
    recursive_mutex::scoped_lock lock(this->quantitiesMutex);

    for (QuantitiesMap::iterator it = this->quantities.begin();
         it != this->quantities.end(); ++it) {
        it->second->update(event);
    }
}

void StatisticsEventFormatter::printStats() {
    recursive_mutex::scoped_lock lock(this->quantitiesMutex);

    if (((this->lines) % 24) == 0) {
        printHeader();
    }

    for (QuantitiesMap::iterator it = this->quantities.begin();
         it != this->quantities.end(); ++it) {
        printQuantity(it->second);
        this->stream << "|";
        it->second->reset();
    }
    this->stream << endl;
    ++this->lines;
}

void StatisticsEventFormatter::printHeader() {
    for (QuantitiesMap::const_iterator it = this->quantities.begin();
         it != this->quantities.end(); ++it) {
        this->stream << setw(it->second->getWidth()) << left << it->first
                     << "|";
    }
    this->stream << endl;
    ++this->lines;
}

void StatisticsEventFormatter::printQuantity(QuantityPtr quantity) {
    quantity->print(this->stream);
}

void StatisticsEventFormatter::run() {
    while (!this->terminate) {
        printStats();
        boost::this_thread::sleep(boost::posix_time::seconds(1));
    }
}
