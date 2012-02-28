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

#pragma once

#include <map>

#include <boost/cstdint.hpp>
#include <boost/thread.hpp>

#include "Buffer.h"

namespace rsbbuffer {

/**
 * @author jwienke
 */
class TimeBoundedBuffer: public Buffer {
public:

    TimeBoundedBuffer(const boost::uint64_t &delta);
    virtual ~TimeBoundedBuffer();

    void insert(rsb::EventPtr event);
    rsb::EventPtr get(const rsb::EventId &id);

private:

    boost::uint64_t delta;

    boost::recursive_mutex eventMapMutex;
    std::map<rsb::EventId, rsb::EventPtr> eventMap;

};

}
