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

#include "ApproximateTimeStrategy.h"
#include "FirstMatchStrategy.h"
#include "InformerHandler.h"
#include "SchemaAndByteArrayConverter.h"
#include "SyncMapConverter.h"
#include "SyncStrategy.h"
#include "TimeFrameStrategy.h"

using namespace std;
using namespace rsbtimesync;
namespace po = boost::program_options;

const char *OPTION_HELP = "help";
const char *OPTION_OUT_SCOPE = "outscope";
const char *OPTION_PRIMARY_SCOPE = "primscope";
const char *OPTION_SUPPLEMENTARY_SCOPE = "supscope";
const char *OPTION_STRATEGY = "strategy";

rsc::logging::LoggerPtr logger = rsc::logging::Logger::getLogger("rsbtimesync");

rsb::Scope outScope;
rsb::Scope primaryScope;
set<rsb::Scope> supplementaryScopes;

map<string, SyncStrategyPtr> strategiesByName;
SyncStrategyPtr strategy;

void registerStrategies() {

	{
		SyncStrategyPtr newMatch(new FirstMatchStrategy);
		strategiesByName[newMatch->getKey()] = newMatch;
	}
	{
		SyncStrategyPtr newMatch(new TimeFrameStrategy);
		strategiesByName[newMatch->getKey()] = newMatch;
	}
	{
		SyncStrategyPtr newMatch(new ApproximateTimeStrategy);
		strategiesByName[newMatch->getKey()] = newMatch;
	}

	RSCINFO(logger, "Registered strategies: " << strategiesByName);

}

bool parseOptions(int argc, char **argv) {

	stringstream strategiesDescription;
	strategiesDescription << "Specifies the strategy to be used for syncing {";
	for (map<string, SyncStrategyPtr>::iterator strategyIt =
			strategiesByName.begin(); strategyIt != strategiesByName.end();
			++strategyIt) {
		strategiesDescription << " " << strategyIt->first;
	}
	strategiesDescription << " }";

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()(OPTION_HELP, "produce help message")(OPTION_OUT_SCOPE,
			po::value<std::string>(),
			"output scope for the synchronized results")(OPTION_PRIMARY_SCOPE,
			po::value<std::string>(), "primary scope for the synchronization")(
			OPTION_SUPPLEMENTARY_SCOPE, po::value<vector<string> >(),
			"supplemental scope for the synchronization")(OPTION_STRATEGY,
			po::value<std::string>(), strategiesDescription.str().c_str());

	// also for the strategies
	for (map<string, SyncStrategyPtr>::iterator strategyIt =
			strategiesByName.begin(); strategyIt != strategiesByName.end();
			++strategyIt) {
		strategyIt->second->provideOptions(desc);
	}

	// positional arguments will got into supplementary scopes
	po::positional_options_description p;
	p.add(OPTION_SUPPLEMENTARY_SCOPE, -1);

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
	if (vm.count(OPTION_OUT_SCOPE)) {
		outScope = rsb::Scope(vm[OPTION_OUT_SCOPE].as<string>());
	} else {
		cerr << "No out scope defined." << endl;
		return false;
	}

	// primary scope
	if (vm.count(OPTION_PRIMARY_SCOPE)) {
		primaryScope = rsb::Scope(vm[OPTION_PRIMARY_SCOPE].as<string>());
	} else {
		cerr << "No primary scope defined." << endl;
		return false;
	}

	// supplementary scopes
	if (vm.count(OPTION_SUPPLEMENTARY_SCOPE)) {
		vector<string> scopeStrings = vm[OPTION_SUPPLEMENTARY_SCOPE].as<
				vector<string> >();
		for (vector<string>::const_iterator it = scopeStrings.begin();
				it != scopeStrings.end(); ++it) {
			supplementaryScopes.insert(rsb::Scope(*it));
		}
	} else {
		cerr << "No supplementary scopes defined." << endl;
		return false;
	}

	// finally, select the strategy and process its options
	if (!vm.count(OPTION_STRATEGY)) {
		cerr << "No sync strategy specified." << endl;
		return false;
	}
	string strategyKey = vm[OPTION_STRATEGY].as<string>();
	if (!strategiesByName.count(strategyKey)) {
		cerr << "Unknown sync strategy '" << strategyKey << "' requested."
				<< endl;
		return false;
	}
	strategy = strategiesByName[strategyKey];
	try {
		strategy->handleOptions(vm);
	} catch (invalid_argument &e) {
		cerr << "Error parsing arguments for strategy " << strategyKey << ": "
				<< e.what() << endl;
		return false;
	}

	RSCINFO( logger, "Configured:\n"
	"  " << OPTION_OUT_SCOPE << " = " << outScope << "\n"
	"  " << OPTION_PRIMARY_SCOPE << " = " << primaryScope << "\n"
	"  " << OPTION_SUPPLEMENTARY_SCOPE << " = " << supplementaryScopes << "\n"
	"  " << OPTION_STRATEGY << " = " << strategy->getKey());

	return true;

}

void configureConversion() {

	// set up converters
	list<
			pair<rsb::converter::ConverterPredicatePtr,
					rsb::converter::Converter<string>::Ptr> > converters;
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

	list<
			pair<rsb::converter::ConverterPredicatePtr,
					rsb::converter::Converter<string>::Ptr> > converters;
	converters.push_back(
			make_pair(
					rsb::converter::ConverterPredicatePtr(
							new rsb::converter::AlwaysApplicable()),
					rsb::converter::Converter<string>::Ptr(
							new SyncMapConverter)));
	rsb::converter::ConverterSelectionStrategy<string>::Ptr selectionStrategy(
			new rsb::converter::PredicateConverterList<string>(
					converters.begin(), converters.end()));
	// adapt default participant configuration
	rsb::ParticipantConfig config =
			rsb::Factory::getInstance().getDefaultParticipantConfig();
	config.setQualityOfServiceSpec(
			rsb::QualityOfServiceSpec(rsb::QualityOfServiceSpec::ORDERED,
					rsb::QualityOfServiceSpec::RELIABLE));
	rsb::ParticipantConfig::Transport transport = config.getTransport("spread");
	rsc::runtime::Properties options = transport.getOptions();
	options["converters"] = selectionStrategy;
	transport.setOptions(options);
	config.addTransport(transport);
	return config;

}

class InformingSyncDataHandler: public SyncDataHandler {
public:

	InformingSyncDataHandler(rsb::InformerBasePtr informer) :
			informer(informer) {
	}

	virtual ~InformingSyncDataHandler() {
	}

	rsb::EventPtr createEvent() {
		rsb::EventPtr event = informer->createEvent();
		event->setType("SyncMap");
		return event;
	}

	void handle(rsb::EventPtr event) {
		informer->publish(event);
	}

private:
	rsb::InformerBasePtr informer;

};

int main(int argc, char **argv) {

	rsc::logging::LoggerFactory::getInstance()->reconfigure(
			rsc::logging::Logger::LEVEL_INFO);
	rsc::logging::Logger::getLogger("rsbtimesync.TimeFrameStrategy")->setLevel(
			rsc::logging::Logger::LEVEL_TRACE);

	registerStrategies();

	bool parsed = parseOptions(argc, argv);
	if (!parsed) {
		cerr << "Error parsing arguments. Terminating." << endl;
		return EXIT_FAILURE;
	}

	configureConversion();

	rsb::Informer<void>::Ptr informer =
			rsb::Factory::getInstance().createInformer<void>(outScope,
					createInformerConfig(), "SyncMap");
	SyncDataHandlerPtr handler(new InformingSyncDataHandler(informer));

	// configure selected sync strategy
	strategy->initializeChannels(primaryScope, supplementaryScopes);
	strategy->setSyncDataHandler(handler);

	rsb::ListenerPtr primaryListener =
			rsb::Factory::getInstance().createListener(primaryScope);
	primaryListener->addHandler(strategy);

	map<rsb::Scope, rsb::ListenerPtr> supplementaryListeners;
	for (set<rsb::Scope>::const_iterator scopeIt = supplementaryScopes.begin();
			scopeIt != supplementaryScopes.end(); ++scopeIt) {

		supplementaryListeners[*scopeIt] =
				rsb::Factory::getInstance().createListener(*scopeIt);
		supplementaryListeners[*scopeIt]->addHandler(strategy);

	}

	// main loop
	// TODO add better sleep logic with interrupt handler
	while (true) {

		boost::this_thread::sleep(boost::posix_time::seconds(10));

	}

	return EXIT_SUCCESS;

}

