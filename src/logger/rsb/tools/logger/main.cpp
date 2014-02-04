/* ============================================================
 *
 * This file is part of the RSB project
 *
 * Copyright (C) 2011, 2012 Jan Moringen <jmoringe@techfak.uni-bielefeld.de>
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

#include <signal.h>

#include <iostream>

#include <boost/format.hpp>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>

#include <boost/program_options.hpp>

#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/EventCollections.h>
#include <rsb/filter/ScopeFilter.h>

#include <rsb/converter/ByteArrayConverter.h>
#include <rsb/converter/EventsByScopeMapConverter.h>
#include <rsb/converter/PredicateConverterList.h>
#include <rsb/converter/RegexConverterPredicate.h>
#include <rsb/converter/TypeNameConverterPredicate.h>
#include <rsb/converter/StringConverter.h>

#include "EventFormatter.h"
#include "PayloadFormatter.h"

using namespace std;

using namespace boost::posix_time;
using namespace boost::program_options;

using namespace rsc::runtime;

using namespace rsb;
using namespace rsb::converter;

using namespace rsb::tools::logger;

class FormattingHandler: public Handler {
public:
    FormattingHandler(EventFormatterPtr formatter):
        formatter(formatter) {
    }

    void handle(EventPtr event) {
        this->formatter->format(std::cout, event);
    }
private:
    EventFormatterPtr formatter;
};

template <typename WireType>
typename ConverterSelectionStrategy<WireType>::Ptr createConverterSelectionStrategy() {
    // Construct a list of pairs of converters and predicates. When
    // wrapped in a PredicateConverterList (see end of function), each
    // predicate is used to determine whether the associated converter
    // should be applied to a particular payload.
    list< pair<ConverterPredicatePtr, typename Converter<WireType>::Ptr> > converters;

    // Add the two string converters first since they operate on
    // specific wire-schemas.
    converters.push_back(make_pair(ConverterPredicatePtr(new RegexConverterPredicate("(utf-8|ascii)-string")),
                   typename Converter<WireType>::Ptr(new StringConverter())));

    // Pass the above converters to the event collection converter as
    // a baseline for what can be deserialized.
    {
        list< pair<ConverterPredicatePtr, typename Converter<WireType>::Ptr> > converters_(converters);
        converters_.push_back(make_pair(ConverterPredicatePtr(new AlwaysApplicable()),
                                       typename Converter<WireType>::Ptr(new ByteArrayConverter())));

        typename Converter<WireType>::Ptr collectionConverter(
            new EventsByScopeMapConverter(
                typename ConverterSelectionStrategy<WireType>::Ptr(
                    new PredicateConverterList<WireType>(
                        converters_.begin(), converters_.end())),
                typename ConverterSelectionStrategy<WireType>::Ptr(
                    new PredicateConverterList<WireType>(
                        converters_.begin(), converters_.end()))));
        converters.push_back(make_pair(ConverterPredicatePtr(new TypeNameConverterPredicate(collectionConverter->getWireSchema())),
                                       collectionConverter));
    }

    // Last but not least, use a converter which can handle everything.
    converters.push_back(make_pair(ConverterPredicatePtr(new AlwaysApplicable()),
                                   typename Converter<WireType>::Ptr(new ByteArrayConverter())));

    // Return a pointer to the constructed converter selection
    // strategy.
    return typename ConverterSelectionStrategy<WireType>::Ptr(new PredicateConverterList<WireType>(converters.begin(), converters.end()));
}

string scope;
string eventFormat;

options_description options("Allowed options");

bool handleCommandline(int argc, char *argv[]) {
    options.add_options()
    ("help",
     "Display a help message.")
    ("scope",
     value<string>(&scope),
     "The scope of the channel for which events should be logged.")
    ("style",
     value<string>(&eventFormat)->default_value("compact"),
     boost::str(boost::format("The style that should be used to print received events. Value has to be one of %1%.")
         % getEventFormatterNames()).c_str());

    positional_options_description positional_options;
    positional_options.add("scope", 1);

    variables_map map;
    store(command_line_parser(argc, argv)
      .options(options)
      .positional(positional_options)
      .run(), map);
    notify(map);
    if (map.count("help")) {
        return true;
    }
    if (!getEventFormatterNames().count(eventFormat)) {
        throw invalid_argument(boost::str(boost::format("Argument of --format option has to one of %1%.")
                   % getEventFormatterNames()));
    }
    if (scope.empty()) {
        throw invalid_argument("A Scope has to be specified.");
    }

    return false;
}

void usage() {
    cout << "usage: logger SCOPE [OPTIONS]" << endl;
    cout << options << endl;
}

bool doTerminate = false;
boost::recursive_mutex terminateMutex;
boost::condition terminateCondition;

void handleSIGINT(int /*signal*/) {
    boost::recursive_mutex::scoped_lock lock(terminateMutex, boost::try_to_lock);
    doTerminate = true;
    terminateCondition.notify_all();
}

int main(int argc, char* argv[]) {
    // Handle commandline arguments.
    try {
        if (handleCommandline(argc, argv)) {
            usage(); // --help
            return EXIT_SUCCESS;
        }
    } catch (const std::exception& e) {
        cerr << "Error parsing command line: " << e.what() << endl;
        usage();
        return EXIT_FAILURE;
    }

    // Create an event formatter
    Properties props;
    props["stream"] = &std::cout;
    EventFormatterPtr formatter(EventFormatterFactory::getInstance().createInst(eventFormat, props));

    // Configure a Listener object.
    ParticipantConfig config
        = getFactory().getDefaultParticipantConfig();

    set<ParticipantConfig::Transport> transports = config.getTransports();
    for (set<ParticipantConfig::Transport>::const_iterator it =
            transports.begin(); it != transports.end(); ++it) {
        ParticipantConfig::Transport& transport = config.mutableTransport(
                it->getName());
        Properties options = transport.getOptions();
        options["converters"] = createConverterSelectionStrategy<string>();
        transport.setOptions(options);
    }

    ListenerPtr listener
        = getFactory().createListener(Scope(scope), config);
    listener->addHandler(HandlerPtr(new FormattingHandler(formatter)));

    // Wait until termination is requested through the SIGINT handler.
    signal(SIGINT, &handleSIGINT);
    while (!doTerminate) {
        boost::recursive_mutex::scoped_lock lock(terminateMutex, boost::try_to_lock);
        terminateCondition.wait(lock);
    }

    return EXIT_SUCCESS;
}
