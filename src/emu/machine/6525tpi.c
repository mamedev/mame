/***************************************************************************
    mos tri port interface 6525
    mos triple interface adapter 6523

    peter.trauner@jk.uni-linz.ac.at

    used in commodore b series
    used in commodore c1551 floppy disk drive
***************************************************************************/

/*
 mos tpi 6525
 40 pin package
 3 8 bit ports (pa, pb, pc)
 8 registers to pc
 0 port a 0 in low
 1 port a data direction register 1 output
 2 port b
 3 port b ddr
 4 port c
  handshaking, interrupt mode
  0 interrupt 0 input, 1 interrupt enabled
   interrupt set on falling edge
  1 interrupt 1 input
  2 interrupt 2 input
  3 interrupt 3 input
  4 interrupt 4 input
  5 irq output
  6 ca handshake line (read handshake answer on I3 preferred)
  7 cb handshake line (write handshake clear on I4 preferred)
 5 port c ddr

 6 cr configuration register
  0 mc
    0 port c normal input output mode like port a and b
    1 port c used for handshaking and interrupt input
  1 1 interrupt prioritized
  2 i3 configure edge
    1 interrupt set on positive edge
  3 i4 configure edge
  5,4 ca handshake
   00 on read
      rising edge of i3 sets ca high
      read a data from computers sets ca low
   01 pulse output
      1 microsecond low after read a operation
   10 manual output low
   11 manual output high
  7,6 cb handshake
   00 handshake on write
      write b data from computer sets cb 0
      rising edge of i4 sets cb high
   01 pulse output
      1 microsecond low after write b operation
   10 manual output low
   11 manual output high
 7 air active interrupt register
   0 I0 occurred
   1 I1 occurred
   2 I2 occurred
   3 I3 occurred
   4 I4 occurred
   read clears interrupt

 non prioritized interrupts
  interrupt is set when occurred
  read clears all interrupts

 prioritized interrupts
  I4>I3>I2>I1>I0
  highest interrupt can be found in air register
  read clears highest interrupt
*/

#include "emu.h"
#include "6525tpi.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define VERBOSE_LEVEL 0
#define DBG_LOG( MACHINE, N, M, A ) \
	do { \
		if(VERBOSE_LEVEL >= N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s", MACHINE.time().as_double(), (char*) M ); \
			logerror A; \
		} \
	} while (0)


#define INTERRUPT_MODE (tpi6525->cr & 1)
#define PRIORIZED_INTERRUPTS (tpi6525->cr & 2)
#define INTERRUPT3_RISING_EDGE (tpi6525->cr & 4)
#define INTERRUPT4_RISING_EDGE (tpi6525->cr & 8)
#define CA_MANUAL_OUT (tpi6525->cr & 0x20)
#define CA_MANUAL_LEVEL ((tpi6525->cr & 0x10) ? 1 : 0)
#define CB_MANUAL_OUT (tpi6525->cr & 0x80)
#define CB_MANUAL_LEVEL ((tpi6525->cr & 0x40) ? 1 : 0)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct tpi6525_state
{
	devcb_resolved_write_line	out_irq_func;
	devcb_resolved_read8		in_pa_func;
	devcb_resolved_write8		out_pa_func;
	devcb_resolved_read8		in_pb_func;
	devcb_resolved_write8		out_pb_func;
	devcb_resolved_read8		in_pc_func;
	devcb_resolved_write8		out_pc_func;
	devcb_resolved_write_line	out_ca_func;
	devcb_resolved_write_line	out_cb_func;

	UINT8 port_a, ddr_a, in_a;
	UINT8 port_b, ddr_b, in_b;
	UINT8 port_c, ddr_c, in_c;

	UINT8 ca_level, cb_level, interrupt_level;

	UINT8 cr;
	UINT8 air;

	UINT8 irq_level[5];
};


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE tpi6525_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TPI6525);

	return (tpi6525_state *)downcast<tpi6525_device *>(device)->token();
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( tpi6525 )
{
	tpi6525_state *tpi6525 = get_safe_token(device);
	const tpi6525_interface *intf = (const tpi6525_interface*)device->static_config();

	// resolve callbacks
	tpi6525->out_irq_func.resolve(intf->out_irq_func, *device);
	tpi6525->in_pa_func.resolve(intf->in_pa_func, *device);
	tpi6525->out_pa_func.resolve(intf->out_pa_func, *device);
	tpi6525->in_pb_func.resolve(intf->in_pb_func, *device);
	tpi6525->out_pb_func.resolve(intf->out_pb_func, *device);
	tpi6525->in_pc_func.resolve(intf->in_pc_func, *device);
	tpi6525->out_pc_func.resolve(intf->out_pc_func, *device);
	tpi6525->out_ca_func.resolve(intf->out_ca_func, *device);
	tpi6525->out_cb_func.resolve(intf->out_cb_func, *device);

	/* verify that we have an interface assigned */
	assert(device->static_config() != NULL);

	/* register for state saving */
	device->save_item(NAME(tpi6525->port_a));
	device->save_item(NAME(tpi6525->ddr_a));
	device->save_item(NAME(tpi6525->in_a));
	device->save_item(NAME(tpi6525->port_b));
	device->save_item(NAME(tpi6525->ddr_b));
	device->save_item(NAME(tpi6525->in_b));
	device->save_item(NAME(tpi6525->port_c));
	device->save_item(NAME(tpi6525->ddr_c));
	device->save_item(NAME(tpi6525->in_c));
	device->save_item(NAME(tpi6525->ca_level));
	device->save_item(NAME(tpi6525->cb_level));
	device->save_item(NAME(tpi6525->interrupt_level));
	device->save_item(NAME(tpi6525->cr));
	device->save_item(NAME(tpi6525->air));
	device->save_item(NAME(tpi6525->irq_level));
}


static DEVICE_RESET( tpi6525 )
{
	tpi6525_state *tpi6525 = get_safe_token(device);

	/* setup some initial values */
	tpi6525->in_a = 0xff;
	tpi6525->in_b = 0xff;
	tpi6525->in_c = 0xff;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static void tpi6525_set_interrupt(device_t *device)
{
	tpi6525_state *tpi6525 = get_safe_token(device);

	if (!tpi6525->interrupt_level && (tpi6525->air != 0))
	{
		tpi6525->interrupt_level = 1;

		DBG_LOG(device->machine(), 3, "tpi6525", ("%s set interrupt\n", device->tag()));

		tpi6525->out_irq_func(tpi6525->interrupt_level);
	}
}


static void tpi6525_clear_interrupt(device_t *device)
{
	tpi6525_state *tpi6525 = get_safe_token(device);

	if (tpi6525->interrupt_level && (tpi6525->air == 0))
	{
		tpi6525->interrupt_level = 0;

		DBG_LOG(device->machine(), 3, "tpi6525", ("%s clear interrupt\n", device->tag()));

		tpi6525->out_irq_func(tpi6525->interrupt_level);
	}
}


WRITE_LINE_DEVICE_HANDLER( tpi6525_i0_w )
{
	tpi6525_state *tpi6525 = get_safe_token(device);

	if (INTERRUPT_MODE && (state != tpi6525->irq_level[0]))
	{
		tpi6525->irq_level[0] = state;

		if ((state == 0) && !(tpi6525->air & 1) && (tpi6525->ddr_c & 1))
		{
			tpi6525->air |= 1;
			tpi6525_set_interrupt(device);
		}
	}
}


WRITE_LINE_DEVICE_HANDLER( tpi6525_i1_w )
{
	tpi6525_state *tpi6525 = get_safe_token(device);

	if (INTERRUPT_MODE && (state != tpi6525->irq_level[1]))
	{
		tpi6525->irq_level[1] = state;

		if ((state == 0) && !(tpi6525->air & 2) && (tpi6525->ddr_c & 2))
		{
			tpi6525->air |= 2;
			tpi6525_set_interrupt(device);
		}
	}
}


WRITE_LINE_DEVICE_HANDLER( tpi6525_i2_w )
{
	tpi6525_state *tpi6525 = get_safe_token(device);

	if (INTERRUPT_MODE && (state != tpi6525->irq_level[2]))
	{
		tpi6525->irq_level[2] = state;

		if ((state == 0) && !(tpi6525->air & 4) && (tpi6525->ddr_c & 4))
		{
			tpi6525->air |= 4;
			tpi6525_set_interrupt(device);
		}
	}
}


WRITE_LINE_DEVICE_HANDLER( tpi6525_i3_w )
{
	tpi6525_state *tpi6525 = get_safe_token(device);

	if (INTERRUPT_MODE && (state != tpi6525->irq_level[3]))
	{
		tpi6525->irq_level[3] = state;

		if (((INTERRUPT3_RISING_EDGE && (state == 1))
			|| (!INTERRUPT3_RISING_EDGE && (state == 0)))
			&& !(tpi6525->air & 8) && (tpi6525->ddr_c & 8))
		{
			tpi6525->air |= 8;
			tpi6525_set_interrupt(device);
		}
	}
}


WRITE_LINE_DEVICE_HANDLER( tpi6525_i4_w )
{
	tpi6525_state *tpi6525 = get_safe_token(device);

	if (INTERRUPT_MODE && (state != tpi6525->irq_level[4]) )
	{
		tpi6525->irq_level[4] = state;

		if (((INTERRUPT4_RISING_EDGE && (state == 1))
			||(!INTERRUPT4_RISING_EDGE&&(state == 0)))
			&& !(tpi6525->air & 0x10) && (tpi6525->ddr_c & 0x10))
		{
			tpi6525->air |= 0x10;
			tpi6525_set_interrupt(device);
		}
	}
}

WRITE_LINE_MEMBER( tpi6525_device::i0_w ) { tpi6525_i0_w(this, state); }
WRITE_LINE_MEMBER( tpi6525_device::i1_w ) { tpi6525_i1_w(this, state); }
WRITE_LINE_MEMBER( tpi6525_device::i2_w ) { tpi6525_i2_w(this, state); }
WRITE_LINE_MEMBER( tpi6525_device::i3_w ) { tpi6525_i3_w(this, state); }
WRITE_LINE_MEMBER( tpi6525_device::i4_w ) { tpi6525_i4_w(this, state); }


READ8_DEVICE_HANDLER( tpi6525_porta_r )
{
	tpi6525_state *tpi6525 = get_safe_token(device);
	UINT8 data = tpi6525->in_a;

	if (!tpi6525->in_pa_func.isnull())
		data = tpi6525->in_pa_func(offset);

	data = (data & ~tpi6525->ddr_a) | (tpi6525->ddr_a & tpi6525->port_a);

	return data;
}


WRITE8_DEVICE_HANDLER( tpi6525_porta_w )
{
	tpi6525_state *tpi6525 = get_safe_token(device);

	tpi6525->in_a = data;
}


READ8_DEVICE_HANDLER( tpi6525_portb_r )
{
	tpi6525_state *tpi6525 = get_safe_token(device);
	UINT8 data = tpi6525->in_b;

	if (!tpi6525->in_pb_func.isnull())
		data = tpi6525->in_pb_func(offset);

	data = (data & ~tpi6525->ddr_b) | (tpi6525->ddr_b & tpi6525->port_b);

	return data;
}


WRITE8_DEVICE_HANDLER( tpi6525_portb_w )
{
	tpi6525_state *tpi6525 = get_safe_token(device);

	tpi6525->in_b = data;
}


READ8_DEVICE_HANDLER( tpi6525_portc_r )
{
	tpi6525_state *tpi6525 = get_safe_token(device);
	UINT8 data = tpi6525->in_c;

	if (!tpi6525->in_pc_func.isnull())
		data &= tpi6525->in_pc_func(offset);

	data = (data & ~tpi6525->ddr_c) | (tpi6525->ddr_c & tpi6525->port_c);

	return data;
}


WRITE8_DEVICE_HANDLER( tpi6525_portc_w )
{
	tpi6525_state *tpi6525 = get_safe_token(device);

	tpi6525->in_c = data;
}


READ8_DEVICE_HANDLER( tpi6525_r )
{
	tpi6525_state *tpi6525 = get_safe_token(device);
	UINT8 data = 0xff;

	switch (offset & 7)
	{
	case 0:
		data = tpi6525->in_a;

		if (!tpi6525->in_pa_func.isnull())
			data &= tpi6525->in_pa_func(0);

		data = (data & ~tpi6525->ddr_a) | (tpi6525->ddr_a & tpi6525->port_a);

		break;

	case 1:
		data = tpi6525->in_b;

		if (!tpi6525->in_pb_func.isnull())
			data &= tpi6525->in_pb_func(0);

		data = (data & ~tpi6525->ddr_b) | (tpi6525->ddr_b & tpi6525->port_b);

		break;

	case 2:
		if (INTERRUPT_MODE)
		{
			data = 0;

			if (tpi6525->irq_level[0]) data |= 0x01;
			if (tpi6525->irq_level[1]) data |= 0x02;
			if (tpi6525->irq_level[2]) data |= 0x04;
			if (tpi6525->irq_level[3]) data |= 0x08;
			if (tpi6525->irq_level[4]) data |= 0x10;
			if (!tpi6525->interrupt_level) data |= 0x20;
			if (tpi6525->ca_level) data |= 0x40;
			if (tpi6525->cb_level) data |= 0x80;
		}
		else
		{
			data = tpi6525->in_c;

			if (!tpi6525->in_pc_func.isnull())
				data &= tpi6525->in_pc_func(0);

			data = (data & ~tpi6525->ddr_c) | (tpi6525->ddr_c & tpi6525->port_c);
		}

		DBG_LOG(device->machine(), 2, "tpi6525", ("%s read %.2x %.2x\n", device->tag(), offset, data));
		break;

	case 3:
		data = tpi6525->ddr_a;
		break;

	case 4:
		data = tpi6525->ddr_b;
		break;

	case 5:
		data = tpi6525->ddr_c;
		break;

	case 6:
		data = tpi6525->cr;
		break;

	case 7: /* air */
		if (PRIORIZED_INTERRUPTS)
		{
			if (tpi6525->air & 0x10)
			{
				data = 0x10;
				tpi6525->air &= ~0x10;
			}
			else if (tpi6525->air & 8)
			{
				data = 8;
				tpi6525->air &= ~8;
			}
			else if (tpi6525->air & 4)
			{
				data = 4;
				tpi6525->air &= ~4;
			}
			else if (tpi6525->air & 2)
			{
				data = 2;
				tpi6525->air &= ~2;
			}
			else if (tpi6525->air & 1)
			{
				data = 1;
				tpi6525->air &= ~1;
			}
		}
		else
		{
			data = tpi6525->air;
			tpi6525->air = 0;
		}

		tpi6525_clear_interrupt(device);
		break;

	}

	DBG_LOG(device->machine(), 3, "tpi6525", ("%s read %.2x %.2x\n", device->tag(), offset, data));

	return data;
}

READ8_MEMBER( tpi6525_device::read )
{
	return tpi6525_r(this, space, offset);
}


WRITE8_DEVICE_HANDLER( tpi6525_w )
{
	tpi6525_state *tpi6525 = get_safe_token(device);

	DBG_LOG(device->machine(), 2, "tpi6525", ("%s write %.2x %.2x\n", device->tag(), offset, data));

	switch (offset & 7)
	{
	case 0:
		tpi6525->port_a = data;
		tpi6525->out_pa_func(0, (tpi6525->port_a & tpi6525->ddr_a) | (tpi6525->ddr_a ^ 0xff));
		break;

	case 1:
		tpi6525->port_b = data;
		tpi6525->out_pb_func(0, (tpi6525->port_b & tpi6525->ddr_b) | (tpi6525->ddr_b ^ 0xff));
		break;

	case 2:
		tpi6525->port_c = data;

		if (!INTERRUPT_MODE)
			tpi6525->out_pc_func(0, (tpi6525->port_c & tpi6525->ddr_c) | (tpi6525->ddr_c ^ 0xff));
		break;

	case 3:
		tpi6525->ddr_a = data;
		tpi6525->out_pa_func(0, (tpi6525->port_a & tpi6525->ddr_a) | (tpi6525->ddr_a ^ 0xff));
		break;

	case 4:
		tpi6525->ddr_b = data;
		tpi6525->out_pb_func(0, (tpi6525->port_b & tpi6525->ddr_b) | (tpi6525->ddr_b ^ 0xff));
		break;

	case 5:
		tpi6525->ddr_c = data;

		if (!INTERRUPT_MODE)
			tpi6525->out_pc_func(0, (tpi6525->port_c & tpi6525->ddr_c) | (tpi6525->ddr_c ^ 0xff));
		break;

	case 6:
		tpi6525->cr = data;

		if (INTERRUPT_MODE)
		{
			if (CA_MANUAL_OUT)
			{
				if (tpi6525->ca_level != CA_MANUAL_LEVEL)
				{
					tpi6525->ca_level = CA_MANUAL_LEVEL;
					tpi6525->out_ca_func(tpi6525->ca_level);
				}
			}
			if (CB_MANUAL_OUT)
			{
				if (tpi6525->cb_level != CB_MANUAL_LEVEL)
				{
					tpi6525->cb_level = CB_MANUAL_LEVEL;
					tpi6525->out_cb_func(tpi6525->cb_level);
				}
			}
		}

		break;

	case 7:
		/* tpi6525->air = data; */
		break;
	}
}

WRITE8_MEMBER( tpi6525_device::write )
{
	tpi6525_w(this, space, offset, data);
}


/* this should probably be done better, needed for amigacd.c */

UINT8 tpi6525_get_ddr_a(device_t *device)
{
	tpi6525_state *tpi6525 = get_safe_token(device);
	return tpi6525->ddr_a;
}

UINT8 tpi6525_get_ddr_b(device_t *device)
{
	tpi6525_state *tpi6525 = get_safe_token(device);
	return tpi6525->ddr_b;
}

UINT8 tpi6525_get_ddr_c(device_t *device)
{
	tpi6525_state *tpi6525 = get_safe_token(device);
	return tpi6525->ddr_c;
}

const device_type TPI6525 = &device_creator<tpi6525_device>;

tpi6525_device::tpi6525_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TPI6525, "6525 TPI", tag, owner, clock)
{
	m_token = global_alloc_clear(tpi6525_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tpi6525_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tpi6525_device::device_start()
{
	DEVICE_START_NAME( tpi6525 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tpi6525_device::device_reset()
{
	DEVICE_RESET_NAME( tpi6525 )(this);
}


