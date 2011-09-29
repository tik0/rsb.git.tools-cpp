/* ============================================================
 *
 * This file is part of the RSBTimeSync project
 *
 * Copyright (C) 2011 Johannes Wienke <jwienke at techfak dot uni-bielefeld dot de>
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

#include "SchemaAndByteArrayConverter.h"

#include <utility>

using namespace std;

namespace rsbtimesync {

SchemaAndByteArrayConverter::SchemaAndByteArrayConverter() {
}

SchemaAndByteArrayConverter::~SchemaAndByteArrayConverter() {
}

rsb::converter::AnnotatedData SchemaAndByteArrayConverter::deserialize(
		const string &wireType, const string &wire) {
	rsb::converter::AnnotatedData originalData =
			rsb::converter::ByteArrayConverter::deserialize(wireType, wire);
	return make_pair(
			"schemaandbytearray",
			boost::shared_ptr<pair<string, boost::shared_ptr<void> > >(
					new pair<string, boost::shared_ptr<void> >(wireType,
							originalData.second)));
}

}
