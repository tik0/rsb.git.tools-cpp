/* ============================================================
 *
 * This file is a part of the RSBTimeSync project.
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

#include "SyncStrategy.h"

using namespace std;

namespace rsb {
namespace tools {
namespace timesync {

SyncStrategy::SyncStrategy() {
}

SyncStrategy::~SyncStrategy() {
}

void SyncStrategy::provideOptions(
        boost::program_options::options_description &/*optionDescription*/) {

}

void SyncStrategy::handleOptions(
        const boost::program_options::variables_map &/*options*/) {

}

}
}
}
