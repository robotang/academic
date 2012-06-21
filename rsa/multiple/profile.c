/*
 *  Copyright (C) 2010, Robert Tang <opensource@robotang.co.nz>
 *
 *  This is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public Licence
 *  along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include "profile.h"

void profile_begin(profile_t *p)
{
    gettimeofday(&(p->start), NULL);
}

void profile_end(profile_t *p, profile_print_t print)
{
    gettimeofday(&(p->end), NULL);
    
    if(print == PRINT_SECONDS)
    {
        printf("%ld ", p->end.tv_sec  - p->start.tv_sec);
    }
    else if(print == PRINT_MILLISECONDS)
    {
        long seconds, useconds, mseconds;
        seconds  = p->end.tv_sec - p->start.tv_sec;
            useconds = p->end.tv_usec - p->start.tv_usec;
            mseconds = ((seconds) * 1000 + useconds/1000.0) + 0.5;
        printf("%ld ", mseconds);
    }
    else //PRINT_MICROSECONDS
    {
        printf("%ld ", p->end.tv_usec - p->start.tv_usec);
    }
}
