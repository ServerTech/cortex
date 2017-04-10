/*
    Cortex - Self-learning Chess Engine
    @filename chronos.h
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
    @filename chronos.h
    @author Shreyas Vinod

    @brief A few functions to keep track of time. Requires the Boost C++
           libraries.
*/

#ifndef CHRONOS_H
#define CHRONOS_H

#include "defs.h"

#include "boost/date_time/posix_time/posix_time.hpp"

typedef boost::posix_time::ptime Time;
typedef boost::posix_time::time_duration TimeDuration;

// External function declarations

extern Time get_cur_time(); // Gets the current time.
extern uint64 get_time_diff(Time t); // Calculates the time difference.

#endif // CHRONOS_H