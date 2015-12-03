/*
    Cortex - Self-learning Chess Engine
    @filename chronos.cpp
    @author Shreyas Vinod
    @version 0.1.0

    @brief A few functions to keep track of time.

    ******************** VERSION CONTROL ********************
    * 02/12/2015 File created.
    * 02/12/2015 0.1.0 Initial version.
*/

#include "debug.h"

#include "chronos.h"

// Function prototypes

Time get_cur_time();
uint64 get_time_diff(Time t);

// Functions

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