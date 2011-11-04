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

#include <map>
#include <vector>

#include <rsb/Event.h>
#include <rsb/Scope.h>
#include <rsb/converter/Converter.h>
#include <rsb/converter/Repository.h>

namespace rsbtimesync {

/**
 * A converter for aggregated events ordered by their scope and time for each
 * scope.
 *
 * @author jwienke
 */
class EventsByScopeMapConverter: public rsb::converter::Converter<std::string> {
public:

    /**
     * Constructs a new converter and optionally allows to specify a
     * converter::Repository which will be used for the contained events of
     * arbitrary types.
     *
     * @param serializationConverters converters to use for serialization.
     *                                Defaults to the unambiguous map from
     *                                #stringConverterRepository
     * @param deserializationConverters converters to use for deserialization.
     *                                  Defaults to the unambiguous map from
     *                                  #stringConverterRepository
     */
    EventsByScopeMapConverter(
            rsb::converter::ConverterSelectionStrategy<std::string>::Ptr serializationConverters =
                    rsb::converter::stringConverterRepository()->getConvertersForSerialization(),
            rsb::converter::ConverterSelectionStrategy<std::string>::Ptr deserializationConverters =
                    rsb::converter::stringConverterRepository()->getConvertersForDeserialization());
    virtual ~EventsByScopeMapConverter();

    std::string getClassName() const;

    std::string serialize(const rsb::converter::AnnotatedData &data,
            std::string &wire);

    rsb::converter::AnnotatedData deserialize(const std::string &wireSchema,
            const std::string &wire);

    std::string getWireSchema() const;

private:
    rsb::converter::ConverterSelectionStrategy<std::string>::Ptr serializationConverters;
    rsb::converter::ConverterSelectionStrategy<std::string>::Ptr deserializationConverters;
    rsb::converter::Converter<std::string>::Ptr converter;

};

}

