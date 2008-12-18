/*********************************************************************

    6850acia.h

    6850 ACIA code

*********************************************************************/

#ifndef __ACIA6850_H__
#define __ACIA6850_H__


/***************************************************************************
    MACROS
***************************************************************************/

#define ACIA6850		DEVICE_GET_INFO_NAME(acia6850)

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

#define MDRV_ACIA6850_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag, ACIA6850)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _acia6850_interface acia6850_interface;
struct _acia6850_interface
{
	int	tx_clock;
	int	rx_clock;

	UINT8 *rx_pin;
	UINT8 *tx_pin;
	UINT8 *cts_pin;
	UINT8 *rts_pin;
	UINT8 *dcd_pin;

	void (*int_callback)(const device_config *device, int state);
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( acia6850 );

void acia_tx_clock_in(const device_config *device) ATTR_NONNULL;
void acia_rx_clock_in(const device_config *device) ATTR_NONNULL;

void acia6850_set_rx_clock(const device_config *device, int clock) ATTR_NONNULL;
void acia6850_set_tx_clock(const device_config *device, int clock) ATTR_NONNULL;

WRITE8_DEVICE_HANDLER( acia6850_ctrl_w );
READ8_DEVICE_HANDLER( acia6850_stat_r );
WRITE8_DEVICE_HANDLER( acia6850_data_w );
READ8_DEVICE_HANDLER( acia6850_data_r );

READ16_DEVICE_HANDLER( acia6850_stat_lsb_r );
READ16_DEVICE_HANDLER( acia6850_stat_msb_r );
READ16_DEVICE_HANDLER( acia6850_data_lsb_r );
READ16_DEVICE_HANDLER( acia6850_data_msb_r );

WRITE16_DEVICE_HANDLER( acia6850_ctrl_msb_w );
WRITE16_DEVICE_HANDLER( acia6850_ctrl_lsb_w );
WRITE16_DEVICE_HANDLER( acia6850_data_msb_w );
WRITE16_DEVICE_HANDLER( acia6850_data_lsb_w );

#endif /* __ACIA6850_H__ */
