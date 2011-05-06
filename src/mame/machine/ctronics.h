/***************************************************************************

    Centronics printer interface

***************************************************************************/

#ifndef __CTRONICS_H__
#define __CTRONICS_H__

#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _centronics_interface centronics_interface;
struct _centronics_interface
{
	int is_ibmpc;

	devcb_write_line out_ack_func;
	devcb_write_line out_busy_func;
	devcb_write_line out_not_busy_func;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
WRITE8_DEVICE_HANDLER( centronics_data_w );
READ8_DEVICE_HANDLER( centronics_data_r );

/* access to the individual bits */
WRITE_LINE_DEVICE_HANDLER( centronics_d0_w );
WRITE_LINE_DEVICE_HANDLER( centronics_d1_w );
WRITE_LINE_DEVICE_HANDLER( centronics_d2_w );
WRITE_LINE_DEVICE_HANDLER( centronics_d3_w );
WRITE_LINE_DEVICE_HANDLER( centronics_d4_w );
WRITE_LINE_DEVICE_HANDLER( centronics_d5_w );
WRITE_LINE_DEVICE_HANDLER( centronics_d6_w );
WRITE_LINE_DEVICE_HANDLER( centronics_d7_w );

WRITE_LINE_DEVICE_HANDLER( centronics_strobe_w );
WRITE_LINE_DEVICE_HANDLER( centronics_prime_w );
WRITE_LINE_DEVICE_HANDLER( centronics_init_w );
WRITE_LINE_DEVICE_HANDLER( centronics_autofeed_w );

READ_LINE_DEVICE_HANDLER( centronics_ack_r );
READ_LINE_DEVICE_HANDLER( centronics_busy_r );
READ_LINE_DEVICE_HANDLER( centronics_pe_r );
READ_LINE_DEVICE_HANDLER( centronics_not_busy_r );
READ_LINE_DEVICE_HANDLER( centronics_vcc_r );
READ_LINE_DEVICE_HANDLER( centronics_fault_r );


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

DECLARE_LEGACY_DEVICE(CENTRONICS, centronics);

#define MCFG_CENTRONICS_ADD(_tag, _intf) \
	MCFG_DEVICE_ADD(_tag, CENTRONICS, 0) \
	MCFG_DEVICE_CONFIG(_intf)


/***************************************************************************
    DEFAULT INTERFACES
***************************************************************************/

extern const centronics_interface standard_centronics;


#endif /* __CTRONICS_H__ */
