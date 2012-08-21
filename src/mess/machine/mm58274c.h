#ifndef MM58274C_H
#define MM58274C_H

/***************************************************************************
    MACROS
***************************************************************************/

DECLARE_LEGACY_DEVICE(MM58274C, mm58274c);

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
/* interface */
/*
    Initializes the clock chip.
    day1 must be set to a value from 0 (sunday), 1 (monday) ...
    to 6 (saturday) and is needed to correctly retrieve the day-of-week
    from the host system clock.
*/
typedef struct _mm58274c_interface mm58274c_interface;
struct _mm58274c_interface
{
	int	mode24;		/* 24/12 mode */
	int	day1;		/* first day of week */
};

READ8_DEVICE_HANDLER ( mm58274c_r );
WRITE8_DEVICE_HANDLER( mm58274c_w );

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MM58274C_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MM58274C, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif /* MM58274C_H */
