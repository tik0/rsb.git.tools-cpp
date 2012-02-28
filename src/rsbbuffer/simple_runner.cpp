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
#include <string>
#include <vector>

#include <stdlib.h>

#include <boost/program_options.hpp>

#include <rsb/Scope.h>

using namespace std;
using namespace boost::program_options;
using namespace rsb;

set<Scope> scopes;
Scope bufferScope;

void handleCommandline(int argc, char *argv[]) {

    vector<string> scopeNames;
    string bufferScopeName;

    options_description options("Allowed options");
    options.add_options()("help,h", "Display a help message.")("scope,s",
            value<vector<string> >(&scopeNames),
            "Adds a scope to subscribe on.")("bufferscope,b",
            value<string>(&bufferScopeName),
            "The scope this buffer is available on");

    variables_map map;
    store(command_line_parser(argc, argv).options(options).run(), map);
    notify(map);
    if (map.count("help")) {
        cout << "usage: info [OPTIONS]" << endl;
        cout << options << endl;
        exit(0);
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

    // indexing structure:

    return EXIT_SUCCESS;

}

