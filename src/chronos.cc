/*
    Cortex - Self-learning Chess Engine
    @filename chronos.cc
    @author Shreyas Vinod
    @version 1.0.0

    @brief A few functions to keep track of time. Requires the Boost C++
           libraries.

    ******************** VERSION CONTROL ********************
    * 02/12/2015 File created.
    * 02/12/2015 0.1.0 Initial version.
    * 10/04/2017 1.0.0 Release 'Primeval'
*/

/**
    @file
    @filename chronos.cc
    @author Shreyas Vinod

    @brief A few functions to keep track of time.
*/

#include "defs.h"

#include "boost/date_time/posix_time/posix_time.hpp"

#include "chronos.h"

// Prototypes

Time get_cur_time();
uint64 get_time_diff(Time t);

// Function definitions

/**
    @brief Returns the current time.

    @return boost::posix_time::ptime object representing the current time.
*/

Time get_cur_time()
{
    Time t(boost::posix_time::microsec_clock::local_time());

    return t;
}

/**
    @brief Returns the time difference in milliseconds elapsed from the given
           boost::posix_time::ptime object.

    @return uint64 denoting time difference in milliseconds.
*/

uint64 get_time_diff(Time t)
{
    Time time_cur(boost::posix_time::microsec_clock::local_time());

    TimeDuration time_diff = time_cur - t;

    return time_diff.total_milliseconds();
}