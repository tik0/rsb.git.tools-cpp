/* ============================================================
 *
 * This file is a part of the rsb-buffer project.
 *
 * Copyright (C) 2012 by Johannes Wienke <jwienke at techfak dot uni-bielefeld dot de>
 *
 * This file may be licensed under the terms of the
 * GNU Lesser General Public License Version 3 (the ``LGPL''),
 * or (at your option) any later version.
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the LGPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the LGPL along with this
 * program. If not, go to http://www.gnu.org/licenses/lgpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The development of this software was supported by:
 *   CoR-Lab, Research Institute for Cognition and Robotics
 *     Bielefeld University
 *
 * ============================================================ */

#include <iostream>
#include <map>
#include <set>
#include <string>

#include <stdlib.h>

#include <boost/program_options.hpp>

#include <rsb/Factory.h>
#include <rsb/Listener.h>
#include <rsb/Scope.h>
#include <rsb/converter/ConverterSelectionStrategy.h>
#include <rsb/converter/EventIdConverter.h>
#include <rsb/converter/PredicateConverterList.h>
#include <rsb/converter/SchemaAndByteArrayConverter.h>
#include <rsb/converter/TypeNameConverterPredicate.h>
#include <rsb/converter/VoidConverter.h>
#include <rsb/patterns/Server.h>

#include <rsc/logging/Logger.h>
#include <rsc/logging/LoggerFactory.h>

#include "Buffer.h"
#include "BufferInsertHandler.h"
#include "BufferRequestCallback.h"
#include "TimeBoundedBuffer.h"

using namespace std;
using namespace boost::program_options;
using namespace rsb;
using namespace rsb::converter;
using namespace rsb::patterns;
using namespace rsb::tools::simplebuffer;

set<Scope> scopes;
map<Scope, ListenerPtr> listenersByScope;
Scope bufferScope;
boost::uint64_t bufferTimeMuSec = 2000000;

void handleCommandline(int argc, char *argv[]) {

    vector<string> scopeNames;
    string bufferScopeName;

    options_description options("Allowed options");
    options.add_options()("help,h", "Display a help message.")("scope,s",
            value<vector<string> >(&scopeNames),
            "Adds a scope to subscribe on.")("bufferscope,b",
            value<string>(&bufferScopeName),
            "The scope this buffer is available on with its RPC interface.")(
            "time,t", value<boost::uint64_t>(&bufferTimeMuSec),
            "The time to retain elements in the buffer in musec");

    variables_map map;
    store(command_line_parser(argc, argv).options(options).run(), map);
    notify(map);
    if (map.count("help")) {
        cout << "usage: info [OPTIONS]" << endl;
        cout << options << endl;
        exit(EXIT_SUCCESS);
    }

    // validity checks

    if (scopeNames.empty()) {
        cerr << "No scopes specified to subscribe on." << endl;
        exit(1);
    }
    for (vector<string>::const_iterator scopeIt = scopeNames.begin();
            scopeIt != scopeNames.end(); ++scopeIt) {
        scopes.insert(*scopeIt);
    }

    if (bufferScopeName.empty()) {
        cerr << "No scope specified where this buffer will be available."
                << endl;
        exit(1);
    }
    bufferScope = bufferScopeName;

}

ParticipantConfig getNoConversionConfig() {

    // set up converters
    list<pair<ConverterPredicatePtr, Converter<string>::Ptr> > converters;
    converters.push_back(
            make_pair(ConverterPredicatePtr(new TypeNameConverterPredicate(rsc::runtime::typeName<void>())),
                    Converter<string>::Ptr(new VoidConverter())));
    converters.push_back(
            make_pair(ConverterPredicatePtr(new AlwaysApplicable()),
                    Converter<string>::Ptr(new SchemaAndByteArrayConverter())));
    ConverterSelectionStrategy<string>::Ptr noConversionSelectionStrategy(
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

    return config;

}

int main(int argc, char **argv) {

    handleCommandline(argc, argv);

//    rsc::logging::Logger::getLogger("rsbbuffer")->setLevel(
//            rsc::logging::Logger::LEVEL_ALL);

    // configure rsb for no conversion at all
    ParticipantConfig noConversionConfig = getNoConversionConfig();

    BufferPtr buffer(new TimeBoundedBuffer(bufferTimeMuSec));

    // set up listeners for the buffer
    for (set<Scope>::const_iterator scopeIt = scopes.begin();
            scopeIt != scopes.end(); ++scopeIt) {
        ListenerPtr listener = getFactory().createListener(*scopeIt,
                noConversionConfig);
        listenersByScope[*scopeIt] = listener;
        listener->addHandler(HandlerPtr(new BufferInsertHandler(buffer)), true);
    }

    // make buffer available over RPC
    ServerPtr server = getFactory().createServer(bufferScope,
            getFactory().getDefaultParticipantConfig(),
            noConversionConfig);
    server->registerMethod("get",
            Server::CallbackPtr(new BufferRequestCallback(buffer)));

    // TODO add better sleep logic
    while (true) {
        boost::this_thread::sleep(boost::posix_time::seconds(10));
    }

    return EXIT_SUCCESS;

}

