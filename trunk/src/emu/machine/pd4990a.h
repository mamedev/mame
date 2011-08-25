/***************************************************************************

    NEC uPD4990A

    Serial I/O Calendar & Clock IC

***************************************************************************/

#ifndef __PD4990A_H__
#define __PD4990A_H__

#include "devlegcy.h"


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

DECLARE_LEGACY_DEVICE(UPD4990A, upd4990a);

#define MCFG_UPD4990A_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, UPD4990A, 0)


/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* this should be refactored, once RTCs get unified */
extern void upd4990a_addretrace( device_t *device );

extern READ8_DEVICE_HANDLER( upd4990a_testbit_r );
extern READ8_DEVICE_HANDLER( upd4990a_databit_r );
extern WRITE16_DEVICE_HANDLER( upd4990a_control_16_w );


#endif	/*__PD4990A_H__*/
