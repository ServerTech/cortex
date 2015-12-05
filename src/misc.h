/*
    Cortex - Self-learning Chess Engine
    @filename misc.h
    @author Oliver Brausch <http://home.arcor.de/dreamlike/chess/>
    @version 1.0.0

    @brief An embarrassing file using C with code by Oliver Brausch,
           to check for input waiting.

    Refer http://home.arcor.de/dreamlike/chess/.

    ******************** VERSION CONTROL ********************
    * 05/12/2015 File created.
    * 05/12/2015 1.0.0 Final. Will never be touched again.
*/

/**
    @filename misc.h

    @brief An embarrassing file using C with code by Oliver Brausch,
           to check for input waiting.

    Refer http://home.arcor.de/dreamlike/chess/.
*/

#ifndef MISC_H
#define MISC_H

#include "debug.h"

#include "defs.h"
#include "search.h"

// External function declarations.

extern bool input_waiting();
extern void read_input(SearchInfo& search_info);

#endif // MISC_H