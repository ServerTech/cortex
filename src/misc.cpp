/*
    Cortex - Self-learning Chess Engine
    @filename misc.cpp
    @author Shreyas Vinod
    @version 1.0.0

    @brief An embarrassing file using C with code by Oliver Brausch,
           to check for input waiting.

    @see http://home.arcor.de/dreamlike/chess/.

    ******************** VERSION CONTROL ********************
    * 05/12/2015 File created.
    * 05/12/2015 1.0.0 Final. Will never be touched again.
*/

/**
    @file
    @filename misc.cpp
    @author Shreyas Vinod

    @brief An embarrassing file using C with code by Oliver Brausch,
           to check for input waiting.

    @see http://home.arcor.de/dreamlike/chess/.
*/

#include "defs.h"

#include <cstdio>
#include <string.h>

#ifdef WIN32
#include "windows.h"
#include "unistd.h"
#else
#include "sys/time.h"
#include "sys/select.h"
#endif // #ifdef WIN32

#include "misc.h"
#include "search.h"

// Prototypes

bool input_waiting();
void read_input(SearchInfo& search_info);

// Function definitions

/**
    @brief Checks if there's input waiting on stdin.

    @return bool denoting whether there is input waiting on stdin.
*/

bool input_waiting()
{
#ifndef WIN32
    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    FD_SET(fileno(stdin), &readfds);
    tv.tv_sec=0; tv.tv_usec=0;
    select(16, &readfds, nullptr, nullptr, &tv);

    return (FD_ISSET(fileno(stdin), &readfds));
#else
    static int init = 0, pipe;
    static HANDLE inh;
    DWORD dw;

    if(!init)
    {
        init = 1;
        inh = GetStdHandle(STD_INPUT_HANDLE);
        pipe = !GetConsoleMode(inh, &dw);
        if(!pipe)
        {
            SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(inh);
        }
    }
    if(pipe)
    {
        if(!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
        return dw;
    }
    else
    {
        GetNumberOfConsoleInputEvents(inh, &dw);
        return dw <= 1 ? 0 : dw;
    }
#endif // #inndef WIN32
}

/**
    @brief Reads input from stdin if any is waiting, and appropriately
           sets the stop/quit flags in the search information structure.

    @param search_info is the search information structure.

    @return void.
*/

void read_input(SearchInfo& search_info)
{
    int bytes;
    char input[256] = "", *endc;

    if(input_waiting())
    {
        search_info.stopped = 1;
        do
        {
            bytes = read(fileno(stdin), input, 256);
        } while (bytes < 0);
        endc = strchr(input, '\n');
        if(endc) *endc=0;

        if(strlen(input) > 0)
        {
            if(!strncmp(input, "quit", 4))
            {
                search_info.quit = 1;
            }
        }
        return;
    }
}