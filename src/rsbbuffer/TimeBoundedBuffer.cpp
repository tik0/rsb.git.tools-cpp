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

#include "TimeBoundedBuffer.h"

#include <rsb/EventId.h>

using namespace std;

namespace rsbbuffer {

TimeBoundedBuffer::TimeBoundedBuffer(const boost::uint64_t &delta) :
        delta(delta) {
}

TimeBoundedBuffer::~TimeBoundedBuffer() {
}

void TimeBoundedBuffer::insert(rsb::EventPtr event) {
    boost::recursive_mutex::scoped_lock lock(eventMapMutex);
    eventMap.insert(make_pair(event->getEventId(), event));
}

rsb::EventPtr TimeBoundedBuffer::get(const rsb::EventId &id) {
    boost::recursive_mutex::scoped_lock lock(eventMapMutex);
    map<rsb::EventId, rsb::EventPtr>::const_iterator it = eventMap.find(id);
    if (it != eventMap.end()) {
        return it->second;
    } else {
        return rsb::EventPtr();
    }
}

}

