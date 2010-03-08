/***************************************************************************

    Z80 DART Dual Asynchronous Receiver/Transmitter implementation

    Copyright (c) 2008, The MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************
                            _____   _____
                    D1   1 |*    \_/     | 40  D0
                    D3   2 |             | 39  D2
                    D5   3 |             | 38  D4
                    D7   4 |             | 37  D6
                  _INT   5 |             | 36  _IORQ
                   IEI   6 |             | 35  _CE
                   IEO   7 |             | 34  B/_A
                   _M1   8 |             | 33  C/_D
                   Vdd   9 |             | 32  _RD
               _W/RDYA  10 |  Z80-DART   | 31  GND
                  _RIA  11 |             | 30  _W/RDYB
                  RxDA  12 |             | 29  _RIB
                 _RxCA  13 |             | 28  RxDB
                 _TxCA  14 |             | 27  _RxTxCB
                  TxDA  15 |             | 26  TxDB
                 _DTRA  16 |             | 25  _DTRB
                 _RTSA  17 |             | 24  _RTSB
                 _CTSA  18 |             | 23  _CTSB
                 _DCDA  19 |             | 22  _DCDB
                   CLK  20 |_____________| 21  _RESET

***************************************************************************/

#ifndef __Z80DART_H_
#define __Z80DART_H_

#include "devcb.h"

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define Z80DART	DEVICE_GET_INFO_NAME(z80dart)
/*
#define Z8470   DEVICE_GET_INFO_NAME(z8470)
#define LH0081  DEVICE_GET_INFO_NAME(lh0088)
#define MK3881  DEVICE_GET_INFO_NAME(mk3888)
*/

#define MDRV_Z80DART_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, Z80DART, _clock) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_Z80DART_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag)

#define Z80DART_INTERFACE(_name) \
	const z80dart_interface (_name) =

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

enum
{
	Z80DART_CH_A = 0,
	Z80DART_CH_B
};

typedef struct _z80dart_interface z80dart_interface;
struct _z80dart_interface
{
	int rx_clock_a;			/* channel A receive clock */
	int tx_clock_a;			/* channel A transmit clock */
	int rx_tx_clock_b;		/* channel B receive/transmit clock */

	devcb_read_line		in_rxda_func;
	devcb_write_line	out_txda_func;
	devcb_write_line	out_dtra_func;
	devcb_write_line	out_rtsa_func;
	devcb_write_line	out_wrdya_func;

	devcb_read_line		in_rxdb_func;
	devcb_write_line	out_txdb_func;
	devcb_write_line	out_dtrb_func;
	devcb_write_line	out_rtsb_func;
	devcb_write_line	out_wrdyb_func;

	devcb_write_line	out_int_func;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* register access */
READ8_DEVICE_HANDLER( z80dart_cd_ba_r );
WRITE8_DEVICE_HANDLER( z80dart_cd_ba_w );

READ8_DEVICE_HANDLER( z80dart_ba_cd_r );
WRITE8_DEVICE_HANDLER( z80dart_ba_cd_w );

/* control register access */
WRITE8_DEVICE_HANDLER( z80dart_c_w );
READ8_DEVICE_HANDLER( z80dart_c_r );

/* data register access */
WRITE8_DEVICE_HANDLER( z80dart_d_w );
READ8_DEVICE_HANDLER( z80dart_d_r );

/* serial clocks */
WRITE_LINE_DEVICE_HANDLER( z80dart_rxca_w );
WRITE_LINE_DEVICE_HANDLER( z80dart_txca_w );
WRITE_LINE_DEVICE_HANDLER( z80dart_rxtxcb_w );

/* ring indicator */
WRITE_LINE_DEVICE_HANDLER( z80dart_ria_w );
WRITE_LINE_DEVICE_HANDLER( z80dart_rib_w );

/* data carrier detected */
WRITE_LINE_DEVICE_HANDLER( z80dart_dcda_w );
WRITE_LINE_DEVICE_HANDLER( z80dart_dcdb_w );

/* clear to send */
WRITE_LINE_DEVICE_HANDLER( z80dart_ctsa_w );
WRITE_LINE_DEVICE_HANDLER( z80dart_ctsb_w );

/* receive data byte HACK */
void z80dart_receive_data(running_device *device, int channel, UINT8 data);

DEVICE_GET_INFO( z80dart );

#endif
