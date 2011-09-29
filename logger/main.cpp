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

#include <signal.h>

#include <iostream>

#include <boost/format.hpp>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>

#include <boost/program_options.hpp>

#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/filter/ScopeFilter.h>

#include <rsb/converter/PredicateConverterList.h>
#include <rsb/converter/RegexConverterPredicate.h>
#include <rsb/converter/ByteArrayConverter.h>
#include <rsb/converter/StringConverter.h>

#include "EventFormatter.h"
#include "PayloadFormatter.h"

using namespace std;

using namespace boost;
using namespace boost::posix_time;
using namespace boost::program_options;

using namespace rsc::runtime;

using namespace rsb;
using namespace rsb::converter;

class FormattingHandler: public Handler {
public:
    FormattingHandler(EventFormatterPtr formatter):
        formatter(formatter) {
    }

    void handle(EventPtr event) {
        this->formatter->format(std::cout, event);
    }

    string getClassName() const {
    return "FormattingHandler";
    }
private:
    EventFormatterPtr formatter;
};

template <typename WireType>
typename ConverterSelectionStrategy<WireType>::Ptr createConverterSelectionStrategy() {
    list< pair<ConverterPredicatePtr, typename Converter<WireType>::Ptr> > converters;
    converters.push_back(make_pair(ConverterPredicatePtr(new RegexConverterPredicate("(utf-8|ascii)-string")),
                   typename Converter<WireType>::Ptr(new StringConverter())));
    converters.push_back(make_pair(ConverterPredicatePtr(new AlwaysApplicable()),
                   typename Converter<WireType>::Ptr(new ByteArrayConverter())));
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
    ("format",
     value<string>(&eventFormat)->default_value("compact"),
     str(format("The format that should be used to print received events. Value has to be one of %1%.")
         % getEventFormatterNames()).c_str());

    positional_options_description positional_options;
    positional_options.add("scope", 1);

    variables_map map;
    store(command_line_parser(argc, argv)
      .options(options)
      .positional(positional_options)
      .run(), map);
    notify(map);
    if (map.count("help"))
        return true;
    if (!getEventFormatterNames().count(eventFormat)) {
        throw invalid_argument(str(format("Argument of --format option has to one of %1%.")
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
recursive_mutex terminateMutex;
condition terminateCondition;

void handleSIGINT(int /*signal*/) {
    recursive_mutex::scoped_lock lock(terminateMutex, try_to_lock);
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
        = Factory::getInstance().getDefaultParticipantConfig();
    ParticipantConfig::Transport transport = config.getTransport("spread");
    Properties options = transport.getOptions();
    options["converters"] = createConverterSelectionStrategy<string>();
    transport.setOptions(options);
    config.addTransport(transport);
    ListenerPtr listener
        = Factory::getInstance().createListener(Scope(scope), config);
    listener->addHandler(HandlerPtr(new FormattingHandler(formatter)));

    // Wait until termination is requested through the SIGINT handler.
    signal(SIGINT, &handleSIGINT);
    while (!doTerminate) {
        recursive_mutex::scoped_lock lock(terminateMutex, try_to_lock);
        terminateCondition.wait(lock);
    }

    return EXIT_SUCCESS;
}
