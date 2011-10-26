/* ============================================================
 *
 * This file is a part of RSBTimeSync project
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <rsc/logging/LoggerFactory.h>

#include "../testhelpers.h"

using namespace testing;

class SpreadEnvironment: public ::testing::Environment {
public:

    virtual void SetUp() {
        spreadProcess = startSpread();
    }

    virtual void TearDown() {
        spreadProcess.reset();
    }

private:
    rsc::subprocess::SubprocessPtr spreadProcess;

};

int main(int argc, char* argv[]) {

    srand(time(NULL));
    setupLogging();

    ::testing::AddGlobalTestEnvironment(new SpreadEnvironment);
    InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();

}
