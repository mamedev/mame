/**********************************************************************

    DALLAS DS1302

    RTC + BACKUP RAM

**********************************************************************/

#ifndef __DS1302_H__
#define __DS1302_H__

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define DS1302		DEVICE_GET_INFO_NAME(ds1302)

#define MDRV_DS1302_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, DS1302, 0)


/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* device interface */
DEVICE_GET_INFO( ds1302 );

extern WRITE8_DEVICE_HANDLER( ds1302_dat_w );
extern WRITE8_DEVICE_HANDLER( ds1302_clk_w );
extern READ8_DEVICE_HANDLER( ds1302_read );


#endif /* __DS1302_H__ */
