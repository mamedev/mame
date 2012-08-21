/***************************************************************************

    Epson TF-20

    Dual floppy drive with HX-20 factory option

***************************************************************************/

#ifndef __TF20_H__
#define __TF20_H__

#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

#if 0
typedef struct _tf20_interface tf20_interface;
struct _tf20_interface
{
};
#endif


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* serial interface in (to the host computer) */
WRITE_LINE_DEVICE_HANDLER( tf20_txs_w );
READ_LINE_DEVICE_HANDLER( tf20_rxs_r );
WRITE_LINE_DEVICE_HANDLER( tf20_pouts_w );
READ_LINE_DEVICE_HANDLER( tf20_pins_r );

#ifdef UNUSED_FUNCTION
/* serial interface out (to another terminal) */
WRITE_LINE_DEVICE_HANDLER( tf20_txc_r );
READ_LINE_DEVICE_HANDLER( tf20_rxc_w );
WRITE_LINE_DEVICE_HANDLER( tf20_poutc_r );
READ_LINE_DEVICE_HANDLER( tf20_pinc_w );
#endif

INPUT_PORTS_EXTERN( tf20 );


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

DECLARE_LEGACY_DEVICE(TF20, tf20);

#define MCFG_TF20_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TF20, 0) \


#endif /* __TF20_H__ */
