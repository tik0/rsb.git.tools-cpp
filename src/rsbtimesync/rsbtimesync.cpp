/* ============================================================
 *
 * This file is a part of the RSB TimeSync project.
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

#include <iostream>
#include <map>
#include <set>
#include <vector>

#include <stdlib.h>

#include <boost/program_options.hpp>

#include <rsb/EventQueuePushHandler.h>
#include <rsb/Factory.h>
#include <rsb/Listener.h>
#include <rsb/Scope.h>
#include <rsb/converter/Converter.h>
#include <rsb/converter/ConverterSelectionStrategy.h>
#include <rsb/converter/PredicateConverterList.h>

#include <rsc/logging/Logger.h>
#include <rsc/logging/LoggerFactory.h>
#include <rsc/runtime/ContainerIO.h>
#include <rsc/threading/SynchronizedQueue.h>

#include "SchemaAndByteArrayConverter.h"
#include "SyncMapConverter.h"

using namespace std;
using namespace rsbtimesync;
namespace po = boost::program_options;

const char *OPTION_HELP = "help";
const char *OPTION_OUT_SCOPE = "outscope";
const char *OPTION_PRIMARY_SCOPE = "primscope";
const char *OPTION_SUPPLEMENTARY_SCOPE = "supscope";

rsc::logging::LoggerPtr logger = rsc::logging::Logger::getLogger("rsbtimesync");

rsb::Scope outScope;
rsb::Scope primaryScope;
set<rsb::Scope> supplementaryScopes;

bool parseOptions(int argc, char **argv) {

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()(OPTION_HELP, "produce help message")(OPTION_OUT_SCOPE,
			po::value<std::string>(),
			"output scope for the synchronized results")(OPTION_PRIMARY_SCOPE,
			po::value<std::string>(), "primary scope for the synchronization")(
			OPTION_SUPPLEMENTARY_SCOPE, po::value<vector<string> >(),
			"supplemental scope for the synchronization");

	// positional arguments will got into supplementary scopes
	po::positional_options_description p;
	p.add(OPTION_SUPPLEMENTARY_SCOPE, -1);

	po::variables_map vm;
	po::store(
			po::command_line_parser(argc, argv). options(desc).positional(p).run(),
			vm);
	po::notify(vm);

	// start processing the options

	if (vm.count(OPTION_HELP)) {
		cout << desc << "\n";
		return false;
	}

	// out scope
	if (vm.count(OPTION_OUT_SCOPE)) {
		outScope = rsb::Scope(vm[OPTION_OUT_SCOPE].as<string> ());
	} else {
		cerr << "No out scope defined." << endl;
		return false;
	}

	// primary scope
	if (vm.count(OPTION_PRIMARY_SCOPE)) {
		primaryScope = rsb::Scope(vm[OPTION_PRIMARY_SCOPE].as<string> ());
	} else {
		cerr << "No primary scope defined." << endl;
		return false;
	}

	// supplementary scopes
	if (vm.count(OPTION_SUPPLEMENTARY_SCOPE)) {
		vector<string> scopeStrings = vm[OPTION_SUPPLEMENTARY_SCOPE].as<vector<
				string> > ();
		for (vector<string>::const_iterator it = scopeStrings.begin(); it
				!= scopeStrings.end(); ++it) {
			supplementaryScopes.insert(rsb::Scope(*it));
		}
	} else {
		cerr << "No supplementary scopes defined." << endl;
		return false;
	}

	RSCINFO(logger, "Configured:\n"
			"  " << OPTION_OUT_SCOPE << " = " << outScope << "\n"
			"  " << OPTION_PRIMARY_SCOPE << " = " << primaryScope << "\n"
			"  " << OPTION_SUPPLEMENTARY_SCOPE << " = " << supplementaryScopes);

	return true;

}

void configureConversion() {

	// set up converters
	list<pair<rsb::converter::ConverterPredicatePtr, rsb::converter::Converter<
			string>::Ptr> > converters;
	converters.push_back(
			make_pair(
					rsb::converter::ConverterPredicatePtr(
							new rsb::converter::AlwaysApplicable()),
					rsb::converter::Converter<string>::Ptr(
							new SchemaAndByteArrayConverter())));
	rsb::converter::ConverterSelectionStrategy<string>::Ptr selectionStrategy(
			new rsb::converter::PredicateConverterList<string>(
					converters.begin(), converters.end()));
	// adapt default participant configuration
	rsb::ParticipantConfig config =
			rsb::Factory::getInstance().getDefaultParticipantConfig();
	rsb::ParticipantConfig::Transport transport = config.getTransport("spread");
	rsc::runtime::Properties options = transport.getOptions();
	options["converters"] = selectionStrategy;
	transport.setOptions(options);
	config.addTransport(transport);
	rsb::Factory::getInstance().setDefaultParticipantConfig(config);

}

rsb::ParticipantConfig createInformerConfig() {

	list<pair<rsb::converter::ConverterPredicatePtr, rsb::converter::Converter<
			string>::Ptr> > converters;
	converters.push_back(
			make_pair(
					rsb::converter::ConverterPredicatePtr(
							new rsb::converter::AlwaysApplicable()),
					rsb::converter::Converter<string>::Ptr(new SyncMapConverter)));
	rsb::converter::ConverterSelectionStrategy<string>::Ptr selectionStrategy(
			new rsb::converter::PredicateConverterList<string>(
					converters.begin(), converters.end()));
	// adapt default participant configuration
	rsb::ParticipantConfig config =
			rsb::Factory::getInstance().getDefaultParticipantConfig();
	rsb::ParticipantConfig::Transport transport = config.getTransport("spread");
	rsc::runtime::Properties options = transport.getOptions();
	options["converters"] = selectionStrategy;
	transport.setOptions(options);
	config.addTransport(transport);
	return config;

}

int main(int argc, char **argv) {

	rsc::logging::LoggerFactory::getInstance()->reconfigure(
			rsc::logging::Logger::LEVEL_TRACE);

	bool parsed = parseOptions(argc, argv);
	if (!parsed) {
		cerr << "Error parsing arguments. Terminating." << endl;
		return EXIT_FAILURE;
	}

	// TODO hacky stuff begins

	configureConversion();

	rsb::ListenerPtr primaryListener =
			rsb::Factory::getInstance().createListener(primaryScope);
	boost::shared_ptr<rsc::threading::SynchronizedQueue<rsb::EventPtr> >
			primaryQueue(
					new rsc::threading::SynchronizedQueue<rsb::EventPtr>(1));
	primaryListener->addHandler(
			rsb::HandlerPtr(new rsb::EventQueuePushHandler(primaryQueue)));

	map<rsb::Scope, rsb::ListenerPtr> supplementaryListeners;
	map<rsb::Scope, boost::shared_ptr<rsc::threading::SynchronizedQueue<
			rsb::EventPtr> > > supplementaryQueues;
	for (set<rsb::Scope>::const_iterator scopeIt = supplementaryScopes.begin(); scopeIt
			!= supplementaryScopes.end(); ++scopeIt) {

		supplementaryListeners[*scopeIt]
				= rsb::Factory::getInstance().createListener(*scopeIt);
		supplementaryQueues[*scopeIt].reset(
				new rsc::threading::SynchronizedQueue<rsb::EventPtr>(1));
		supplementaryListeners[*scopeIt]->addHandler(
				rsb::HandlerPtr(
						new rsb::EventQueuePushHandler(
								supplementaryQueues[*scopeIt])));

	}

	rsb::Informer<void>::Ptr informer =
			rsb::Factory::getInstance().createInformer<void> (outScope,
					createInformerConfig(), "SyncMap");

	// main loop
	while (true) {

		boost::shared_ptr<map<rsb::Scope, vector<rsb::EventPtr> > > message(
				new map<rsb::Scope, vector<rsb::EventPtr> > );

		{
			rsb::EventPtr primaryEvent = primaryQueue->pop();
			RSCTRACE(logger, "Received primary event " << primaryEvent);
			(*message)[primaryEvent->getScope()].push_back(primaryEvent);
		}

		for (set<rsb::Scope>::const_iterator scopeIt =
				supplementaryScopes.begin(); scopeIt
				!= supplementaryScopes.end(); ++scopeIt) {

			rsb::EventPtr supEvent = supplementaryQueues[*scopeIt]->pop();
			RSCTRACE(logger, "Received supplementary event (" << *scopeIt << ") " << supEvent);
			(*message)[supEvent->getScope()].push_back(supEvent);

		}

		rsb::EventPtr event(new rsb::Event);
		event->setType("SyncMap");
		event->setScope(outScope);
		event->setData(message);
		informer->publish(event);

	}

	return EXIT_SUCCESS;

}

