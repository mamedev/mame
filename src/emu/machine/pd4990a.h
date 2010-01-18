/***************************************************************************

    NEC uPD4990A

    Serial I/O Calendar & Clock IC

***************************************************************************/

#ifndef __PD4990A_H__
#define __PD4990A_H__


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define UPD4990A		DEVICE_GET_INFO_NAME(upd4990a)

#define MDRV_UPD4990A_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, UPD4990A, 0)


/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* device interface */
DEVICE_GET_INFO( upd4990a );

/* this should be refactored, once RTCs get unified */
extern void upd4990a_addretrace( running_device *device );

extern READ8_DEVICE_HANDLER( upd4990a_testbit_r );
extern READ8_DEVICE_HANDLER( upd4990a_databit_r );
extern WRITE16_DEVICE_HANDLER( upd4990a_control_16_w );


#endif	/*__PD4990A_H__*/
