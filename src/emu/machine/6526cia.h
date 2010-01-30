/**********************************************************************

    MOS 6526/8520 Complex Interface Adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 40  CNT
                   PA0   2 |             | 39  SP
                   PA1   3 |             | 38  RS0
                   PA2   4 |             | 37  RS1
                   PA3   5 |             | 36  RS2
                   PA4   6 |             | 35  RS3
                   PA5   7 |             | 34  _RES
                   PA6   8 |             | 33  DB0
                   PA7   9 |             | 32  DB1
                   PB0  10 |   MOS6526   | 31  DB2
                   PB1  11 |   MOS8520   | 30  DB3
                   PB2  12 |             | 29  DB4
                   PB3  13 |             | 28  DB5
                   PB4  14 |             | 27  DB6
                   PB5  15 |             | 26  DB7
                   PB6  16 |             | 25  phi2
                   PB7  17 |             | 24  _FLAG
                   _PC  18 |             | 23  _CS
                   TOD  19 |             | 22  R/W
                   Vcc  20 |_____________| 21  _IRQ

**********************************************************************/

#ifndef __6526CIA_H__
#define __6526CIA_H__

#include "devcb.h"

/***************************************************************************
    MACROS
***************************************************************************/

#define MOS6526R1		DEVICE_GET_INFO_NAME(cia6526r1)
#define MOS6526R2		DEVICE_GET_INFO_NAME(cia6526r1)
#define MOS8520			DEVICE_GET_INFO_NAME(cia8520)

#define MDRV_MOS6526R1_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, MOS6526R1, _clock) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_MOS6526R2_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, MOS6526R2, _clock) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_MOS8520_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, MOS8520, _clock) \
	MDRV_DEVICE_CONFIG(_config)

#define MOS6526_INTERFACE(name) \
	const mos6526_interface (name)=

#define MOS8520_INTERFACE(name) \
	const mos6526_interface (name)=

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _mos6526_interface mos6526_interface;
struct _mos6526_interface
{
	int tod_clock;

	devcb_write_line	out_irq_func;
	devcb_write_line	out_pc_func;
	devcb_write_line	out_cnt_func;
	devcb_write_line	out_sp_func;

	devcb_read8			in_pa_func;
	devcb_write8		out_pa_func;

	devcb_read8			in_pb_func;
	devcb_write8		out_pb_func;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO(cia6526r1);
DEVICE_GET_INFO(cia6526r2);
DEVICE_GET_INFO(cia8520);

/* register access */
READ8_DEVICE_HANDLER( mos6526_r );
WRITE8_DEVICE_HANDLER( mos6526_w );

/* port access */
READ8_DEVICE_HANDLER( mos6526_pa_r );
READ8_DEVICE_HANDLER( mos6526_pb_r );

/* interrupt request */
READ_LINE_DEVICE_HANDLER( mos6526_irq_r );

/* time of day clock */
WRITE_LINE_DEVICE_HANDLER( mos6526_tod_w );

/* serial counter */
READ_LINE_DEVICE_HANDLER( mos6526_cnt_r );
WRITE_LINE_DEVICE_HANDLER( mos6526_cnt_w );

/* serial port */
READ_LINE_DEVICE_HANDLER( mos6526_sp_r );
WRITE_LINE_DEVICE_HANDLER( mos6526_sp_w );

/* flag */
WRITE_LINE_DEVICE_HANDLER( mos6526_flag_w );

/* port mask */
void cia_set_port_mask_value(running_device *device, int port, int data);

#endif /* __6526CIA_H__ */
