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

#include "MonitorEventFormatter.h"

#include <iomanip>
#include <string.h>

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
#include <rsb/Scope.h>

using namespace std;

using namespace boost;
using namespace boost::io;
using namespace boost::posix_time;

using namespace rsc::runtime;

using namespace rsb;

namespace ac = boost::accumulators;

namespace rsb {
namespace tools {
namespace logger {

//

typedef ac::accumulator_set<double, ac::features<ac::tag::mean,
        ac::tag::variance(ac::lazy)> > StatsType;

typedef boost::date_time::microsec_clock<boost::posix_time::ptime> msecsClock;
typedef boost::posix_time::ptime timestamp;
typedef boost::date_time::c_local_adjustor<ptime> local_adj;

class Time: public Quantity {
public:
    unsigned int getWidth() const {
        return 27;
    }

    void reset() {
    }

    void update(EventPtr /*event*/) {
    }

    void print(ostream &stream) {
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

    void print(ostream &stream) {
        ios_all_saver saver(stream);

        stream << fixed << setprecision(2) << right
               << setw(7) << ac::mean(this->stats)
               << " ±" << setw(7) << sqrt(ac::variance(this->stats));
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
        StatsQuantity::update(
                static_cast<double> (event->getMetaData().getDeliverTime())
                        - static_cast<double> (event->getMetaData().getCreateTime()));
    }
};

class Rate: public Quantity {
public:
    Rate() :
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

    void print(ostream &stream) {
        ios_all_saver saver(stream);

        double delta = static_cast<double> ((msecsClock::universal_time()
                - this->start).total_nanoseconds()) / 1000000000.0;
        stream << setw(getWidth() - 3) << fixed << setprecision(0) << right
                << static_cast<double> (this->count) / delta << " Hz";
    }
private:
    unsigned int count;
    timestamp start;
};

//

MonitorEventFormatter::MonitorEventFormatter(ostream &stream,
        double printFrequency) :
    stream(stream), printFrequency(printFrequency) {

    QuantitiesMap quantities;
    quantities.push_back(make_pair(" Latency (us)", QuantityPtr(new Latency())));
    quantities.push_back(make_pair(" Rate", QuantityPtr(new Rate())));

    this->scopes.insert(make_pair(Scope("/"), quantities));

    this->terminate = false;
    this->thread.reset(
            new boost::thread(bind(&MonitorEventFormatter::run, this)));
}

MonitorEventFormatter::~MonitorEventFormatter() {
    this->terminate = true;
    this->thread->join();
}

EventFormatter* MonitorEventFormatter::create(const Properties &props) {
    return new MonitorEventFormatter(*props.get<ostream*> ("stream"),
            props.get<double> ("print-frequency", 1.0));
}

void MonitorEventFormatter::format(ostream &/*stream*/, EventPtr event) {
    boost::recursive_mutex::scoped_lock lock(this->quantitiesMutex);

    ScopeMap::iterator scps;

    scps = this->scopes.find(event->getScope());

    if (scps == this->scopes.end()) {

        // Add scope and all superscopes
        std::vector<Scope> superscopes = event->getScope().superScopes(true);

        for (std::vector<Scope>::iterator sscps = superscopes.begin(); sscps
                != superscopes.end(); ++sscps) {

            QuantitiesMap quantities;
            quantities.push_back(
                    make_pair("| Latency (us)", QuantityPtr(new Latency())));
            quantities.push_back(make_pair(" Rate", QuantityPtr(new Rate())));

            this->scopes.insert(make_pair(*sscps, quantities));
        }

    }

    for (ScopeMap::iterator scps = this->scopes.begin(); scps
            != this->scopes.end(); ++scps) {
        if ((scps->first == event->getScope()) || (scps->first.isSuperScopeOf(
                event->getScope()))) {
            for (QuantitiesMap::iterator it = scps->second.begin(); it
                    != scps->second.end(); ++it) {
                it->second->update(event);
            }
        }
    }
}

void MonitorEventFormatter::printStats() {
    boost::recursive_mutex::scoped_lock lock(this->quantitiesMutex);

    printHeader();

    for (ScopeMap::const_iterator scps = this->scopes.begin(); scps
            != this->scopes.end(); ++scps) {

        for (QuantitiesMap::const_iterator it = scps->second.begin(); it
                != scps->second.end(); ++it) {

            printQuantity(it->second);
            this->stream << "|";
            it->second->reset();

        }

        this->stream << left << " " << scps->first.toString() << endl;

    }
}

void MonitorEventFormatter::printHeader() {
    this->stream << "\x1b[1;1f\x1b[J";

    stream << "RSB Scope Monitor";
    this->stream << endl;

    stream << local_adj::utc_to_local(msecsClock::universal_time());
    this->stream << endl << endl;

    for (QuantitiesMap::const_iterator it = this->scopes["/"].begin(); it
            != this->scopes["/"].end(); ++it) {
        this->stream << setw(it->second->getWidth()) << left << it->first
                << "|";
    }

    this->stream << endl;
}

void MonitorEventFormatter::printQuantity(QuantityPtr quantity) {
    quantity->print(this->stream);
}

void MonitorEventFormatter::run() {
    while (!this->terminate) {
        printStats();
        boost::this_thread::sleep(boost::posix_time::seconds(1));
    }
}

}
}
}
