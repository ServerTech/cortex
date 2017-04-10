/*
    Cortex - Self-learning Chess Engine
    @filename misc.h
    @author Shreyas Vinod
    @version 1.0.0

    @brief An embarrassing file using C with code by Oliver Brausch,
           to check for input waiting.

    @see http://home.arcor.de/dreamlike/chess/.

    ******************** VERSION CONTROL ********************
    * 05/12/2015 File created.
    * 10/04/2017 1.0.0 Release 'Primeval'
*/

/**
    @file
    @filename misc.h
    @author Shreyas Vinod

    @brief An embarrassing file using C with code by Oliver Brausch,
           to check for input waiting.

    @see http://home.arcor.de/dreamlike/chess/.
*/

#ifndef MISC_H
#define MISC_H

#include "defs.h"

#include "search.h"

// External function declarations

extern bool input_waiting();
extern void read_input(SearchInfo& search_info);

#endif // MISC_H