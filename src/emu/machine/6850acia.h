/*********************************************************************

    6850acia.h

    6850 ACIA code

*********************************************************************/

#ifndef __ACIA6850_H__
#define __ACIA6850_H__

#include "devlegcy.h"


/***************************************************************************
    MACROS
***************************************************************************/

DECLARE_LEGACY_DEVICE(ACIA6850, acia6850);

#define ACIA6850_STATUS_RDRF	0x01
#define ACIA6850_STATUS_TDRE	0x02
#define ACIA6850_STATUS_DCD		0x04
#define ACIA6850_STATUS_CTS		0x08
#define ACIA6850_STATUS_FE		0x10
#define ACIA6850_STATUS_OVRN	0x20
#define ACIA6850_STATUS_PE		0x40
#define ACIA6850_STATUS_IRQ		0x80

#define MDRV_ACIA6850_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, ACIA6850, 0) \
	MDRV_DEVICE_CONFIG(_config)

#define ACIA6850_INTERFACE(_name) \
	const acia6850_interface(_name) =


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _acia6850_interface acia6850_interface;
struct _acia6850_interface
{
	int	tx_clock;
	int	rx_clock;

	devcb_read_line		in_rx_func;
	devcb_write_line	out_tx_func;

	devcb_read_line		in_cts_func;
	devcb_write_line	out_rts_func;
	devcb_read_line		in_dcd_func;

	devcb_write_line	out_irq_func;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

void acia6850_tx_clock_in(running_device *device) ATTR_NONNULL(1);
void acia6850_rx_clock_in(running_device *device) ATTR_NONNULL(1);

void acia6850_set_rx_clock(running_device *device, int clock) ATTR_NONNULL(1);
void acia6850_set_tx_clock(running_device *device, int clock) ATTR_NONNULL(1);

WRITE8_DEVICE_HANDLER( acia6850_ctrl_w );
READ8_DEVICE_HANDLER( acia6850_stat_r );
WRITE8_DEVICE_HANDLER( acia6850_data_w );
READ8_DEVICE_HANDLER( acia6850_data_r );

#endif /* __ACIA6850_H__ */
