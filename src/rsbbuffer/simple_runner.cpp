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
using namespace rsb::patterns;
using namespace rsbbuffer;

set<Scope> scopes;
map<Scope, ListenerPtr> listenersByScope;
Scope bufferScope;

void handleCommandline(int argc, char *argv[]) {

    vector<string> scopeNames;
    string bufferScopeName;

    options_description options("Allowed options");
    options.add_options()("help,h", "Display a help message.")("scope,s",
            value<vector<string> >(&scopeNames),
            "Adds a scope to subscribe on.")("bufferscope,b",
            value<string>(&bufferScopeName),
            "The scope this buffer is available on with its RPC interface.");

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

int main(int argc, char **argv) {

    handleCommandline(argc, argv);

    rsc::logging::Logger::getLogger("rsbbuffer")->setLevel(
            rsc::logging::Logger::LEVEL_ALL);

    // TODO configure rsb for no conversion at all

    BufferPtr buffer(new TimeBoundedBuffer(2000000));

    // set up listeners for the buffer
    for (set<Scope>::const_iterator scopeIt = scopes.begin();
            scopeIt != scopes.end(); ++scopeIt) {
        ListenerPtr listener = Factory::getInstance().createListener(*scopeIt);
        listenersByScope[*scopeIt] = listener;
        listener->addHandler(HandlerPtr(new BufferInsertHandler(buffer)), true);
    }

    // make buffer available over RPC
    ServerPtr server = Factory::getInstance().createServer(bufferScope);
    server->registerMethod("get",
            Server::CallbackPtr(new BufferRequestCallback(buffer)));

    // TODO add better sleep logic
    while (true) {
        boost::this_thread::sleep(boost::posix_time::seconds(10));
    }

    return EXIT_SUCCESS;

}

