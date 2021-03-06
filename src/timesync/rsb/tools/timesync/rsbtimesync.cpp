/* ============================================================
 *
 * This file is a part of the RSB TimeSync project.
 *
 * Copyright (C) 2011 by Johannes Wienke <jwienke at techfak dot uni-bielefeld dot de>
 * Copyright (C) 2012, 2014, 2016 Jan Moringen <jmoringe@techfak.dot uni-bielefeld.de>
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

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include <rsc/runtime/ContainerIO.h>

#include <rsc/threading/SynchronizedQueue.h>

#include <rsc/misc/SignalWaiter.h>

#include <rsb/util/EventQueuePushHandler.h>

#include <rsb/Factory.h>
#include <rsb/Listener.h>
#include <rsb/Scope.h>
#include <rsb/MetaData.h>
#include <rsb/EventCollections.h>

#include <rsb/converter/Converter.h>
#include <rsb/converter/ConverterSelectionStrategy.h>
#include <rsb/converter/EventsByScopeMapConverter.h>
#include <rsb/converter/PredicateConverterList.h>
#include <rsb/converter/SchemaAndByteArrayConverter.h>

#include "ApproximateTimeStrategy.h"
#include "FirstMatchStrategy.h"
#include "InformerHandler.h"
#include "PriorityTimestampSelector.h"
#include "StaticTimestampSelectors.h"
#include "SyncStrategy.h"
#include "TimeFrameStrategy.h"

using namespace std;
using namespace rsb;
using namespace rsb::converter;
using namespace rsb::tools::timesync;
namespace po = boost::program_options;

const char *OPTION_HELP = "help";
const char *OPTION_OUT_SCOPE = "outscope";
const char *OPTION_PRIMARY_SCOPE = "primscope";
const char *OPTION_SUPPLEMENTARY_SCOPE = "supscope";
const char *OPTION_STRATEGY = "strategy";
const char *OPTION_TIMESTAMP = "timestamp";

rsc::logging::LoggerPtr logger = rsc::logging::Logger::getLogger("rsbtimesync");

Scope outScope;
Scope primaryScope;
set<Scope> supplementaryScopes;

ConverterSelectionStrategy<string>::Ptr noConversionSelectionStrategy;

TimestampSelectorPtr timestampSelector(new CreateTimestampSelector);

map<string, SyncStrategyPtr> createStrategies() {

    map<string, SyncStrategyPtr> strategiesByName;

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

    return strategiesByName;

}

TimestampSelectorPtr createSelectorFromName(const string &name) {

    // system timestamps
    if (name == TimestampSelector::CREATE) {
        return TimestampSelectorPtr(new CreateTimestampSelector);
    } else if (name == TimestampSelector::SEND) {
        return TimestampSelectorPtr(new SendTimestampSelector);
    } else if (name == TimestampSelector::RECEIVE) {
        return TimestampSelectorPtr(new ReceiveTimestampSelector);
    } else if (name == TimestampSelector::DELIVER) {
        return TimestampSelectorPtr(new DeliverTimestampSelector);
    } else {
        return TimestampSelectorPtr(new UserTimestampSelector(name));
    }

}

bool parseOptions(int argc, char **argv,
            const map<string, SyncStrategyPtr>& strategiesByName,
            SyncStrategyPtr& selectedStrategy) {

    stringstream strategiesDescription;
    strategiesDescription << "Specifies the strategy to be used for syncing {";
    for (map<string, SyncStrategyPtr>::const_iterator strategyIt =
            strategiesByName.begin(); strategyIt != strategiesByName.end();
            ++strategyIt) {
        strategiesDescription << " " << strategyIt->first;
    }
    strategiesDescription << " }";

    stringstream timestampsDescription;
    timestampsDescription
            << "The timestamps to use for synchronizing. Possible values are "
            << TimestampSelector::CREATE << ", " << TimestampSelector::SEND
            << ", " << TimestampSelector::RECEIVE << ", "
            << TimestampSelector::DELIVER << " and names of user timestamps. ";
    timestampsDescription
            << "Multiple timestamps can be specified separated by ',', e.g. 'fooTime,"
            << TimestampSelector::CREATE << "'. ";
    timestampsDescription
            << "This specifies the priority to take timestamps with but allows missing user timestamps with the next item in the list as a fallback. ";
    timestampsDescription << "Default: " << TimestampSelector::CREATE;

    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()(OPTION_HELP, "produce help message")(OPTION_OUT_SCOPE,
            po::value<string>(), "output scope for the synchronized results")(
            OPTION_PRIMARY_SCOPE, po::value<string>(),
            "primary scope for the synchronization")(OPTION_SUPPLEMENTARY_SCOPE,
            po::value<vector<string> >(),
            "supplemental scope for the synchronization")(OPTION_STRATEGY,
            po::value<string>(), strategiesDescription.str().c_str())(
            OPTION_TIMESTAMP, po::value<string>(),
            timestampsDescription.str().c_str());

    // also for the strategies
    for (map<string, SyncStrategyPtr>::const_iterator strategyIt =
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
        outScope = Scope(vm[OPTION_OUT_SCOPE].as<string>());
    } else {
        cerr << "No out scope defined." << endl;
        return false;
    }

    // primary scope
    if (vm.count(OPTION_PRIMARY_SCOPE)) {
        primaryScope = Scope(vm[OPTION_PRIMARY_SCOPE].as<string>());
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
            supplementaryScopes.insert(Scope(*it));
        }
    } else {
        cerr << "No supplementary scopes defined." << endl;
        return false;
    }

    // timestamp selection
    if (vm.count(OPTION_TIMESTAMP)) {
        string nameString = vm[OPTION_TIMESTAMP].as<string>();
        vector<string> names;
        boost::algorithm::split(names, nameString,
                boost::algorithm::is_any_of(","),
                boost::algorithm::token_compress_on);

        if (names.empty()) {
            cerr << "No valid timestamps specified." << endl;
            return false;
        }

        // simple case with only one name
        if (names.size() == 1
                && TimestampSelector::systemNames().count(names.front()) > 0) {

            timestampSelector = createSelectorFromName(names.front());
            // as this will only create system time selectors we do not need to
            // take care of eventually missing timestamps

        } else {
            // priority-based

            vector<TimestampSelectorPtr> atomicSelectors;
            for (vector<string>::const_iterator nameIt = names.begin();
                    nameIt != names.end(); ++nameIt) {
                atomicSelectors.push_back(createSelectorFromName(*nameIt));
            }
            timestampSelector.reset(
                    new PriorityTimestampSelector(atomicSelectors));

        }

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
    selectedStrategy = strategiesByName.find(strategyKey)->second;
    try {
        selectedStrategy->handleOptions(vm);
    } catch (invalid_argument &e) {
        cerr << "Error parsing arguments for strategy " << strategyKey << ": "
                << e.what() << endl;
        return false;
    }

    RSCINFO(logger, "Configured:\n"
    "  " << OPTION_OUT_SCOPE << " = " << outScope << "\n"
    "  " << OPTION_PRIMARY_SCOPE << " = " << primaryScope << "\n"
    "  " << OPTION_SUPPLEMENTARY_SCOPE << " = " << supplementaryScopes << "\n"
    "  " << OPTION_STRATEGY << " = " << selectedStrategy->getKey() << "\n"
    "  " << OPTION_TIMESTAMP << " = " << timestampSelector);

    return true;

}

void configureConversion() {

    // set up converters
    list<pair<ConverterPredicatePtr, Converter<string>::Ptr> > converters;
    converters.push_back(
            make_pair(ConverterPredicatePtr(new AlwaysApplicable()),
                    Converter<string>::Ptr(new SchemaAndByteArrayConverter())));
    noConversionSelectionStrategy.reset(
            new PredicateConverterList<string>(converters.begin(),
                    converters.end()));
    // adapt default participant configuration
    ParticipantConfig config =
            getFactory().getDefaultParticipantConfig();
    set<ParticipantConfig::Transport> transports = config.getTransports();
    for (set<ParticipantConfig::Transport>::const_iterator it =
            transports.begin(); it != transports.end(); ++it) {
        ParticipantConfig::Transport& transport = config.mutableTransport(
                it->getName());
        rsc::runtime::Properties options = transport.getOptions();
        options["converters"] = noConversionSelectionStrategy;
        transport.setOptions(options);
    }
    getFactory().setDefaultParticipantConfig(config);

}

ParticipantConfig createInformerConfig() {

    list<pair<ConverterPredicatePtr, Converter<string>::Ptr> > converters;
    converters.push_back(
            make_pair(
                    ConverterPredicatePtr(new AlwaysApplicable()),
                    Converter<string>::Ptr(
                            new EventsByScopeMapConverter(
                                    noConversionSelectionStrategy,
                                    noConversionSelectionStrategy))));
    ConverterSelectionStrategy<string>::Ptr selectionStrategy(
            new PredicateConverterList<string>(converters.begin(),
                    converters.end()));
    // adapt default participant configuration
    ParticipantConfig config =
            getFactory().getDefaultParticipantConfig();
    config.setQualityOfServiceSpec(
            QualityOfServiceSpec(QualityOfServiceSpec::ORDERED,
                    QualityOfServiceSpec::RELIABLE));
    set<ParticipantConfig::Transport> transports = config.getTransports();
    for (set<ParticipantConfig::Transport>::const_iterator it =
            transports.begin(); it != transports.end(); ++it) {
        ParticipantConfig::Transport& transport = config.mutableTransport(
                it->getName());
        rsc::runtime::Properties options = transport.getOptions();
        options["converters"] = selectionStrategy;
        transport.setOptions(options);
    }
    return config;

}

class InformingSyncDataHandler: public SyncDataHandler {
public:

    InformingSyncDataHandler(InformerBasePtr informer,
                                 TimestampSelectorPtr timestampSelector) :
            informer(informer), timestampSelector(timestampSelector) {
    }

    virtual ~InformingSyncDataHandler() {
    }

    EventPtr createEvent() {
        EventPtr event = informer->createEvent();
        event->setType(rsc::runtime::typeName<EventsByScopeMap>());
        return event;
    }

    void handle(EventPtr event) {
        // Put the earliest and latest timestamps (w.r.t. the
        // timestamp selector) of all events in the handled event into
        // the user timestamps timesync:{earliest,latest} of the
        // handled events.
        //
        // This allows downstream components to operate on these
        // timestamps without having to traverse the EventsByScopeMap.
        boost::shared_ptr<EventsByScopeMap> eventMap
            = boost::static_pointer_cast<EventsByScopeMap>(event->getData());
        boost::uint64_t earliest = std::numeric_limits<boost::uint64_t>::max();
        boost::uint64_t latest = 0;
        for (EventsByScopeMap::const_iterator scopeIt = eventMap->begin();
                 scopeIt != eventMap->end(); ++scopeIt) {
            for (vector<EventPtr>::const_iterator eventIt = scopeIt->second.begin();
                     eventIt != scopeIt->second.end(); ++eventIt) {
                boost::uint64_t timestamp = this->timestampSelector->getTimestamp(*eventIt);
                earliest = std::min(earliest, timestamp);
                latest = std::max(latest, timestamp);
            }
        }
        event->mutableMetaData().setUserTime("timesync:earliest", earliest);
        event->mutableMetaData().setUserTime("timesync:latest", latest);

        informer->publish(event);
    }

private:
    InformerBasePtr informer;
    TimestampSelectorPtr timestampSelector;

};

int main(int argc, char **argv) {

    rsc::misc::initSignalWaiter();

    map<string, SyncStrategyPtr> strategiesByName = createStrategies();

    SyncStrategyPtr strategy;
    bool parsed = parseOptions(argc, argv, strategiesByName, strategy);
    if (!parsed) {
        cerr << "Error parsing arguments. Terminating." << endl;
        return EXIT_FAILURE;
    }

    configureConversion();

    Informer<EventsByScopeMap>::Ptr informer =
            getFactory().createInformer<EventsByScopeMap>(outScope,
                    createInformerConfig());
    SyncDataHandlerPtr handler(new InformingSyncDataHandler(informer, timestampSelector));

    // configure selected sync strategy
    strategy->initializeChannels(primaryScope, supplementaryScopes);
    strategy->setSyncDataHandler(handler);
    strategy->setTimestampSelector(timestampSelector);

    ListenerPtr primaryListener = getFactory().createListener(
            primaryScope);
    primaryListener->addHandler(strategy);

    map<Scope, ListenerPtr> supplementaryListeners;
    for (set<Scope>::const_iterator scopeIt = supplementaryScopes.begin();
            scopeIt != supplementaryScopes.end(); ++scopeIt) {

        supplementaryListeners[*scopeIt] =
                getFactory().createListener(*scopeIt);
        supplementaryListeners[*scopeIt]->addHandler(strategy);

    }

    return rsc::misc::suggestedExitCode(rsc::misc::waitForSignal());

}
