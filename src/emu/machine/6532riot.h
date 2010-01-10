/***************************************************************************

  RIOT 6532 emulation

***************************************************************************/

#ifndef __RIOT6532_H__
#define __RIOT6532_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


typedef struct _riot6532_interface riot6532_interface;
struct _riot6532_interface
{
	devcb_read8			in_a_func;
	devcb_read8			in_b_func;
	devcb_write8		out_a_func;
	devcb_write8		out_b_func;
	devcb_write_line	irq_func;
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_RIOT6532_ADD(_tag, _clock, _intrf) \
	MDRV_DEVICE_ADD(_tag, RIOT6532, _clock) \
	MDRV_DEVICE_CONFIG(_intrf)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

READ8_DEVICE_HANDLER( riot6532_r );
WRITE8_DEVICE_HANDLER( riot6532_w );

void riot6532_porta_in_set(const device_config *device, UINT8 data, UINT8 mask);
void riot6532_portb_in_set(const device_config *device, UINT8 data, UINT8 mask);

UINT8 riot6532_porta_in_get(const device_config *device);
UINT8 riot6532_portb_in_get(const device_config *device);

UINT8 riot6532_porta_out_get(const device_config *device);
UINT8 riot6532_portb_out_get(const device_config *device);


/* ----- device interface ----- */

#define RIOT6532 DEVICE_GET_INFO_NAME(riot6532)
DEVICE_GET_INFO( riot6532 );

#endif
