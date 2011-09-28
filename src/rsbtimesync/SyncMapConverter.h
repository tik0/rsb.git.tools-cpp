/* ============================================================
 *
 * This file is a part of the RSB TimeSync project.
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

#pragma once

#include <rsb/converter/Converter.h>
#include <rsb/converter/Repository.h>

namespace rsbtimesync {

class SyncMapConverter: public rsb::converter::Converter<std::string> {
public:

	SyncMapConverter(
			rsb::converter::Repository<std::string>::Ptr converterRepository =
					rsb::converter::stringConverterRepository());
	virtual ~SyncMapConverter();

	std::string getClassName() const;

	std::string serialize(const rsb::converter::AnnotatedData &data,
			std::string &wire);

	rsb::converter::AnnotatedData deserialize(const std::string &wireSchema,
			const std::string &wire);

private:
	rsb::converter::Repository<std::string>::Ptr converterRepository;

};

}

