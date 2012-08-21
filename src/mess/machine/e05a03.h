/***************************************************************************

    E05A03 Gate Array (used in the Epson LX-800)

***************************************************************************/

#ifndef __E05A03_H__
#define __E05A03_H__

#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _e05a03_interface e05a03_interface;
struct _e05a03_interface
{
	devcb_read8 in_data_func;

	devcb_write_line out_nlq_lp_func;
	devcb_write_line out_pe_lp_func;
	devcb_write_line out_pe_func;
	devcb_write_line out_reso_func;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
WRITE8_DEVICE_HANDLER( e05a03_w );
READ8_DEVICE_HANDLER( e05a03_r );

WRITE_LINE_DEVICE_HANDLER( e05a03_home_w ); /* home position signal */
WRITE_LINE_DEVICE_HANDLER( e05a03_fire_w ); /* printhead solenoids trigger */
WRITE_LINE_DEVICE_HANDLER( e05a03_strobe_w );
READ_LINE_DEVICE_HANDLER( e05a03_busy_r );
WRITE_LINE_DEVICE_HANDLER( e05a03_resi_w ); /* reset input */
WRITE_LINE_DEVICE_HANDLER( e05a03_init_w ); /* centronics init */


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

DECLARE_LEGACY_DEVICE(E05A03, e05a03);

#define MCFG_E05A03_ADD(_tag, _intf) \
	MCFG_DEVICE_ADD(_tag, E05A03, 0) \
	MCFG_DEVICE_CONFIG(_intf)


#endif /* __E05A03_H__ */
