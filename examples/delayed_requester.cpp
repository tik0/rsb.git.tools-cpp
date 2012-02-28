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

#include <string>

#include <stdlib.h>

#include <boost/program_options.hpp>

#include <rsb/EventId.h>
#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/Listener.h>
#include <rsb/patterns/RemoteServer.h>

#include <rsc/threading/SimpleTask.h>
#include <rsc/threading/TaskExecutor.h>
#include <rsc/threading/ThreadedTaskExecutor.h>

using namespace std;
using namespace boost::program_options;
using namespace rsb;
using namespace rsb::patterns;

string dataScopeName;
string bufferScopeName;
int delaySec = 1;

class DelayedRequestAndPrintTask: public rsc::threading::SimpleTask {
public:

    DelayedRequestAndPrintTask(EventPtr originalEvent,
            RemoteServerPtr bufferServer) :
            originalEvent(originalEvent), bufferServer(bufferServer) {
    }

    void run() {
        boost::this_thread::sleep(boost::posix_time::seconds(delaySec));

        EventPtr requestEvent = bufferServer->call(
                "get",
                bufferServer->prepareRequestEvent(
                        boost::shared_ptr<EventId>(
                                new EventId(originalEvent->getEventId()))));

        cout << "#########" << endl;
        cout << "Original: " << originalEvent << endl;
        if (originalEvent->getData()
                && originalEvent->getType() == rsc::runtime::typeName<string>()) {
            cout
                    << "          "
                    << *boost::static_pointer_cast<string>(
                            originalEvent->getData()) << endl;
        }
        cout << "Request:  " << requestEvent << endl;
        if (requestEvent->getData()
                && requestEvent->getType() == rsc::runtime::typeName<string>()) {
            cout
                    << "          "
                    << *boost::static_pointer_cast<string>(
                            requestEvent->getData()) << endl;
        }
    }

private:

    EventPtr originalEvent;
    RemoteServerPtr bufferServer;

};

class DelayedRequestingHandler: public Handler {
public:

    DelayedRequestingHandler(RemoteServerPtr bufferServer) :
            bufferServer(bufferServer), executor(
                    new rsc::threading::ThreadedTaskExecutor) {
    }

    void handle(EventPtr event) {
        executor->schedule(
                rsc::threading::TaskPtr(
                        new DelayedRequestAndPrintTask(event, bufferServer)));
    }

private:
    RemoteServerPtr bufferServer;
    rsc::threading::TaskExecutorPtr executor;
};

int main(int argc, char **argv) {

    options_description options("Allowed options");
    options.add_options()("help,h", "Display a help message.")("scope,s",
            value<string>(&dataScopeName), "The scope of original data")(
            "bufferscope,b", value<string>(&bufferScopeName),
            "The scope this buffer is available on with its RPC interface.")(
            "delay,d", value<int>(&delaySec),
            "delay until requesting in seconds");

    variables_map map;
    store(command_line_parser(argc, argv).options(options).run(), map);
    notify(map);
    if (map.count("help")) {
        cout << "usage: info [OPTIONS]" << endl;
        cout << options << endl;
        exit(EXIT_SUCCESS);
    }

    // create a remove server for the buffer
    RemoteServerPtr bufferServer = Factory::getInstance().createRemoteServer(
            bufferScopeName);

    // create a listener on the original data
    ListenerPtr listener = Factory::getInstance().createListener(dataScopeName);
    listener->addHandler(HandlerPtr(new DelayedRequestingHandler(bufferServer)),
            true);

    // TODO add better sleep logic
    while (true) {
        boost::this_thread::sleep(boost::posix_time::seconds(10));
    }

    return EXIT_SUCCESS;

}

