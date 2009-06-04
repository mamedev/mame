/***************************************************************************

    Fujistu MB3773

    Power Supply Monitor with Watch Dog Timer (i.e. Reset IC)

***************************************************************************/

#ifndef __MB3773_H__
#define __MB3773_H__


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define MB3773		DEVICE_GET_INFO_NAME(mb3773)

#define MDRV_MB3773_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, MB3773, 0)

#define MDRV_MB3773_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag)


/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* device interface */
DEVICE_GET_INFO( mb3773 );

extern WRITE8_DEVICE_HANDLER( mb3773_set_ck );

#endif	/* __MB3773_H__ */
