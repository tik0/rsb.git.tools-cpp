/* ============================================================
 *
 * This file is a part of the RSBTimeSync project.
 *
 * Copyright (C) 2011 by Johannes Wienke <jwienke at techfak dot uni-bielefeld dot de>
 * Copyright (C) 2012, 2014 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include <iostream>

#include <stdlib.h>

#include <boost/program_options.hpp>

#include <rsc/misc/SignalWaiter.h>

#include <rsb/util/EventQueuePushHandler.h>

#include <rsb/Event.h>
#include <rsb/EventCollections.h>
#include <rsb/Factory.h>
#include <rsb/Listener.h>
#include <rsb/Scope.h>

#include <rsb/converter/Converter.h>
#include <rsb/converter/EventsByScopeMapConverter.h>

using namespace std;
namespace po = boost::program_options;
using namespace rsb;
using namespace converter;

const char *OPTION_HELP = "help";
const char *OPTION_SCOPE = "scope";

Scope scope;

boost::shared_ptr<rsc::threading::SynchronizedQueue<EventPtr> > eventQueue(
        new rsc::threading::SynchronizedQueue<EventPtr>);

bool parseOptions(int argc, char **argv) {

    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()(OPTION_HELP, "produce help message")(OPTION_SCOPE,
            po::value<std::string>(), "scope to listen on");

    // positional arguments will got into supplementary scopes
    po::positional_options_description p;
    p.add(OPTION_SCOPE, -1);

    po::variables_map vm;
    po::store(
            po::command_line_parser(argc, argv).options(desc).positional(p).run(),
            vm);
    po::notify(vm);

    // start processing the options

    if (vm.count(OPTION_HELP)) {
        cout << desc << "\n";
        return false;
    }

    // out scope
    if (vm.count(OPTION_SCOPE)) {
        scope = Scope(vm[OPTION_SCOPE].as<string>());
    } else {
        cerr << "No scope defined." << endl;
        return false;
    }

    return true;

}

int main(int argc, char **argv) {

    rsc::misc::initSignalWaiter();

    if (!parseOptions(argc, argv)) {
        cerr << "Error parsing arguments. Terminating." << endl;
        return EXIT_FAILURE;
    }

    // let factory register default converters
    getFactory();

    // register converter
    converterRepository<string>()->registerConverter(
            Converter<string>::Ptr(new EventsByScopeMapConverter));

    ListenerPtr listener = getFactory().createListener(scope);
    listener->addHandler(HandlerPtr(new rsb::util::EventQueuePushHandler(eventQueue)));

    while (rsc::misc::lastArrivedSignal() == rsc::misc::NO_SIGNAL) {

        EventPtr event = eventQueue->pop();

        cout << "received event: " << event << endl;

        boost::shared_ptr<EventsByScopeMap> map = boost::static_pointer_cast<
                EventsByScopeMap>(event->getData());

        for (EventsByScopeMap::const_iterator mapIt = map->begin();
                mapIt != map->end(); ++mapIt) {

            cout << "Scope: " << mapIt->first.toString() << ":" << endl;

            for (vector<EventPtr>::const_iterator eventIt =
                    mapIt->second.begin(); eventIt != mapIt->second.end();
                    ++eventIt) {
                cout << "\t" << *eventIt << endl;
            }

        }

    }

    return EXIT_SUCCESS;

}
