/*************************************************************************

    taitoio.h

    Implementation of various Taito custom I/O ICs

**************************************************************************/

#ifndef __TAITOIO_H__
#define __TAITOIO_H__

#include "devlegcy.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _tc0220ioc_interface tc0220ioc_interface;
struct _tc0220ioc_interface
{
	devcb_read8 read_0;
	devcb_read8 read_1;
	devcb_read8 read_2;
	devcb_read8 read_3;
	devcb_read8 read_7;
};


typedef struct _tc0510nio_interface tc0510nio_interface;
struct _tc0510nio_interface
{
	devcb_read8 read_0;
	devcb_read8 read_1;
	devcb_read8 read_2;
	devcb_read8 read_3;
	devcb_read8 read_7;
};


typedef struct _tc0640fio_interface tc0640fio_interface;
struct _tc0640fio_interface
{
	devcb_read8 read_0;
	devcb_read8 read_1;
	devcb_read8 read_2;
	devcb_read8 read_3;
	devcb_read8 read_7;
};

DECLARE_LEGACY_DEVICE(TC0220IOC, tc0220ioc);
DECLARE_LEGACY_DEVICE(TC0510NIO, tc0510nio);
DECLARE_LEGACY_DEVICE(TC0640FIO, tc0640fio);

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_TC0220IOC_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, TC0220IOC, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_TC0510NIO_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, TC0510NIO, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_TC0640FIO_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, TC0640FIO, 0) \
	MDRV_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

/** TC0220IOC **/
READ8_DEVICE_HANDLER( tc0220ioc_r );
WRITE8_DEVICE_HANDLER( tc0220ioc_w );
READ8_DEVICE_HANDLER( tc0220ioc_port_r );
WRITE8_DEVICE_HANDLER( tc0220ioc_port_w );
READ8_DEVICE_HANDLER( tc0220ioc_portreg_r );
WRITE8_DEVICE_HANDLER( tc0220ioc_portreg_w );


/** TC0510NIO **/
READ8_DEVICE_HANDLER( tc0510nio_r );
WRITE8_DEVICE_HANDLER( tc0510nio_w );
READ16_DEVICE_HANDLER( tc0510nio_halfword_r );
WRITE16_DEVICE_HANDLER( tc0510nio_halfword_w );
READ16_DEVICE_HANDLER( tc0510nio_halfword_wordswap_r );
WRITE16_DEVICE_HANDLER( tc0510nio_halfword_wordswap_w );


/** TC0640FIO**/
READ8_DEVICE_HANDLER( tc0640fio_r );
WRITE8_DEVICE_HANDLER( tc0640fio_w );
READ16_DEVICE_HANDLER( tc0640fio_halfword_r );
WRITE16_DEVICE_HANDLER( tc0640fio_halfword_w );
READ16_DEVICE_HANDLER( tc0640fio_halfword_byteswap_r );
WRITE16_DEVICE_HANDLER( tc0640fio_halfword_byteswap_w );


#endif	/* __TAITOIO_H__ */
