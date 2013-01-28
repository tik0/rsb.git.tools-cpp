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

#include "BufferRequestCallback.h"

using namespace std;
using namespace rsb;

namespace rsb {
namespace tools {
namespace simplebuffer {

BufferRequestCallback::BufferRequestCallback(BufferPtr buffer) :
        buffer(buffer) {
}

BufferRequestCallback::~BufferRequestCallback() {
}

AnnotatedData BufferRequestCallback::call(const string& /*methodName*/,
        boost::shared_ptr<EventId> input) {

    EventPtr event = buffer->get(*input);
    if (event) {
        return make_pair(event->getType(), event->getData());
    } else {
        return make_pair(rsc::runtime::typeName(typeid(void)),
                boost::shared_ptr<void>());
    }

}

}
}
}
