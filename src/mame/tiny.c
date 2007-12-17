/******************************************************************************

    tiny.c

    mamedriv.c substitute file for "tiny" MAME builds.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

    The list of used drivers. Drivers have to be included here to be recognized
    by the executable.

    To save some typing, we use a hack here. This file is recursively #included
    twice, with different definitions of the DRIVER() macro. The first one
    declares external references to the drivers; the second one builds an array
    storing all the drivers.

******************************************************************************/

#include "driver.h"

#ifndef DRIVER_RECURSIVE

#define DRIVER_RECURSIVE

/* step 1: declare all external references */
#define DRIVER(NAME) extern game_driver driver_##NAME;
#include "tiny.c"

/* step 2: define the drivers[] array */
#undef DRIVER
#define DRIVER(NAME) &driver_##NAME,
const game_driver * const drivers[] =
{
#include "tiny.c"
	0	/* end of array */
};

#else	/* DRIVER_RECURSIVE */

	DRIVER( robby )		/* (c) 1981 Bally Midway */
	DRIVER( gridlee ) 	/* [1983 Videa] prototype - no copyright notice */
	DRIVER( alienar ) 	/* (c) 1985 Duncan Brown */
	DRIVER( carpolo )	/* (c) 1977 Exidy */
	DRIVER( sidetrac )	/* (c) 1979 Exidy */
	DRIVER( teetert )	/* (c) 1982 Exidy */
	DRIVER( circus )	/* (c) 1977 Exidy */
	DRIVER( robotbwl )	/* (c) 197? Exidy */
	DRIVER( crash )		/* (c) 1979 Exidy */
	DRIVER( ripcord )	/* (c) 1979 Exidy */
	DRIVER( starfire )	/* (c) 1979 Exidy */
	DRIVER( starfira )	/* (c) 1979 Exidy */
	DRIVER( fireone )	/* (c) 1979 Exidy */
	DRIVER( starfir2 )	/* (c) 1979 Exidy */

#endif	/* DRIVER_RECURSIVE */
