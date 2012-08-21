/**********************************************************************

    "Dave" Sound Chip

**********************************************************************/

#ifndef __DAVE_H__
#define __DAVE_H__

#include "devcb.h"


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

DECLARE_LEGACY_SOUND_DEVICE(DAVE, dave_sound);

#define DAVE_INT_SELECTABLE		0
#define DAVE_INT_1KHZ_50HZ_TG	1
#define DAVE_INT_1HZ			2
#define DAVE_INT_INT1			3
#define DAVE_INT_INT2			4

#define DAVE_FIFTY_HZ_COUNTER_RELOAD	20
#define DAVE_ONE_HZ_COUNTER_RELOAD		1000

/* id's of external ints */
enum
{
	DAVE_INT1_ID,
	DAVE_INT2_ID
};


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _dave_interface dave_interface;
struct _dave_interface
{
	devcb_read8 reg_r;
	devcb_write8 reg_w;
	devcb_write_line int_callback;
};


/***************************************************************************
    PROTOTYPES
***************************************************************************/
void dave_set_reg(device_t *device, offs_t offset, UINT8 data);

READ8_DEVICE_HANDLER ( dave_reg_r );
WRITE8_DEVICE_HANDLER ( dave_reg_w );

#endif /* __DAVE_H__ */
