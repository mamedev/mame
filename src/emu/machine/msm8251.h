/*********************************************************************

    msm8251.h

    MSM/Intel 8251 Universal Synchronous/Asynchronous Receiver Transmitter code

*********************************************************************/

#ifndef __MSM8251_H__
#define __MSM8251_H__

#include "machine/serial.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

DECLARE_LEGACY_DEVICE(MSM8251, msm8251);

#define MSM8251_EXPECTING_MODE 0x01
#define MSM8251_EXPECTING_SYNC_BYTE 0x02

#define MSM8251_STATUS_FRAMING_ERROR 0x020
#define MSM8251_STATUS_OVERRUN_ERROR 0x010
#define MSM8251_STATUS_PARITY_ERROR 0x08
#define MSM8251_STATUS_TX_EMPTY		0x04
#define MSM8251_STATUS_RX_READY	0x02
#define MSM8251_STATUS_TX_READY	0x01

#define MCFG_MSM8251_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MSM8251, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_MSM8251_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _msm8251_interface msm8251_interface;
struct _msm8251_interface
{
	devcb_read_line		in_rxd_func;
	devcb_write_line	out_txd_func;
	devcb_read_line		in_dsr_func;
	devcb_write_line	out_dtr_func;
	devcb_write_line	out_rts_func;
	devcb_write_line	out_rxrdy_func;
	devcb_write_line	out_txrdy_func;
	devcb_write_line	out_txempty_func;
	devcb_write_line	out_syndet_func;
};


/***************************************************************************
    PROTOTYPES
***************************************************************************/

extern const msm8251_interface default_msm8251_interface;

/* read data register */
READ8_DEVICE_HANDLER(msm8251_data_r);

/* read status register */
READ8_DEVICE_HANDLER(msm8251_status_r);

/* write data register */
WRITE8_DEVICE_HANDLER(msm8251_data_w);

/* write control word */
WRITE8_DEVICE_HANDLER(msm8251_control_w);

/* The 8251 has seperate transmit and receive clocks */
/* use these two functions to update the msm8251 for each clock */
/* on NC100 system, the clocks are the same */
void msm8251_transmit_clock(device_t *device);
void msm8251_receive_clock(device_t *device);

/* connecting to serial output */
void msm8251_connect_to_serial_device(device_t *device, device_t *image);
void msm8251_connect(device_t *device, serial_connection *other_connection);

void msm8251_receive_character(device_t *device, UINT8 ch);

#endif /* __MSM8251_H__ */
