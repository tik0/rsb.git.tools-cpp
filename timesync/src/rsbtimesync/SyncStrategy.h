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

#pragma once

#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>

#include <rsb/Event.h>
#include <rsb/Scope.h>
#include <rsb/Handler.h>

#include "TimestampSelector.h"
#include "SyncDataHandler.h"

namespace rsbtimesync {

/**
 * An interface for strategies which synchronize events.
 *
 * @author jwienke
 */
class SyncStrategy: public rsb::Handler {
public:

    SyncStrategy();
    virtual ~SyncStrategy();

    /**
     * Returns a command line-friendly name for this strategy.
     * @return string without spaces
     */
    virtual std::string getKey() const = 0;

    /**
     * Sets the handler which has to be called in order to send an event.
     *
     * @param handler handler to set
     */
    virtual void setSyncDataHandler(SyncDataHandlerPtr handler) = 0;

    virtual void initializeChannels(const rsb::Scope &primaryScope,
            const std::set<rsb::Scope> &subsidiaryScopes) = 0;

    virtual void setTimestampSelector(TimestampSelectorPtr selector) = 0;

    /**
     * This method is called in order to add new command line options this
     * strategy may use. Please make sure that all options you provide have a
     * prefix that makes their name unique and identifiable for this strategy.
     * The best solution is probably to use the result of #getKey.
     *
     * @param optionDescription option object to add options add.
     */
    virtual void provideOptions(
            boost::program_options::options_description &optionDescription);

    /**
     * Method to handle parsed options from the command line.
     *
     * @param options parsed options
     * @throw std::invalid_argument error in parsed options. Exception text will
     *                              be displayed to the user
     */
    virtual void handleOptions(
            const boost::program_options::variables_map &options);

};

typedef boost::shared_ptr<SyncStrategy> SyncStrategyPtr;

}
