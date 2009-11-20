/***************************************************************************

    Zilog Z80 DMA Direct Memory Access Controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************
                            _____   _____
                    A5   1 |*    \_/     | 40  A6
                    A4   2 |             | 39  A7
                    A3   3 |             | 38  IEI
                    A2   4 |             | 37  _INT/_PULSE
                    A1   5 |             | 36  IEO
                    A0   6 |             | 35  D0
                   CLK   7 |             | 34  D1
                   _WR   8 |             | 33  D2
                   _RD   9 |             | 32  D3
                 _IORQ  10 |    Z8410    | 31  D4
                   +5V  11 |             | 30  GND
                 _MREQ  12 |             | 29  D5
                  _BAO  13 |             | 28  D6
                  _BAI  14 |             | 27  D7
               _BUSREQ  15 |             | 26  _M1
             _CE/_WAIT  16 |             | 25  RDY
                   A15  17 |             | 24  A8
                   A14  18 |             | 23  A9
                   A13  19 |             | 22  A10
                   A12  20 |_____________| 21  A11

***************************************************************************/

#ifndef __Z80DMA__
#define __Z80DMA__

#include "devcb.h"

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define Z80DMA DEVICE_GET_INFO_NAME(z80dma)

#define MDRV_Z80DMA_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, Z80DMA, _clock) \
	MDRV_DEVICE_CONFIG(_config)

#define Z80DMA_INTERFACE(_name) \
	const z80dma_interface (_name) =

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _z80dma_interface z80dma_interface;
struct _z80dma_interface
{
	devcb_write_line	out_busreq_func;
	devcb_write_line	out_int_func;
	devcb_write_line	out_bao_func;

	/* memory accessors */
	devcb_read8			in_mreq_func;
	devcb_write8		out_mreq_func;

	/* I/O accessors */
	devcb_read8			in_iorq_func;
	devcb_write8		out_iorq_func;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

/* register access */
READ8_DEVICE_HANDLER( z80dma_r );
WRITE8_DEVICE_HANDLER( z80dma_w );

/* ready */
WRITE_LINE_DEVICE_HANDLER( z80dma_rdy_w );

/* wait */
WRITE_LINE_DEVICE_HANDLER( z80dma_wait_w );

/* bus acknowledge in */
WRITE_LINE_DEVICE_HANDLER( z80dma_bai_w );

DEVICE_GET_INFO( z80dma );

#endif
