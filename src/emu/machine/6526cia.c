/**********************************************************************

    MOS 6526/8520 CIA interface and emulation

    This function emulates all the functionality MOS6526 or
    MOS8520 complex interface adapters.

**********************************************************************/

#include "emu.h"
#include "6526cia.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* CIA registers */
#define CIA_PRA			0
#define CIA_PRB			1
#define CIA_DDRA		2
#define CIA_DDRB		3
#define CIA_TALO		4
#define CIA_TAHI		5
#define CIA_TBLO		6
#define CIA_TBHI		7
#define CIA_TOD0		8		/* 6526: 1/10 seconds   8520: bits  0- 7 */
#define CIA_TOD1		9		/* 6526: seconds        8520: bits  8-15 */
#define CIA_TOD2		10		/* 6526: minutes        8520: bits 16-23 */
#define CIA_TOD3		11		/* 6526: hours          8520: N/A */
#define CIA_SDR			12
#define CIA_ICR			13
#define CIA_CRA			14
#define CIA_CRB			15

#define CIA_CRA_START	0x01
#define	CIA_CRA_PBON	0x02
#define	CIA_CRA_OUTMODE	0x04
#define	CIA_CRA_RUNMODE	0x08
#define	CIA_CRA_LOAD	0x10
#define	CIA_CRA_INMODE	0x20
#define	CIA_CRA_SPMODE	0x40
#define	CIA_CRA_TODIN	0x80

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _cia_timer cia_timer;
typedef struct _cia_port cia_port;
typedef struct _cia_state cia_state;

struct _cia_timer
{
	UINT16		latch;
	UINT16		count;
	UINT8		mode;
	UINT8		irq;
	emu_timer *timer;
	cia_state *	cia;
};

struct _cia_port
{
	UINT8		ddr;
	UINT8		latch;
	UINT8		in;
	UINT8		out;
	devcb_resolved_read8	read;
	devcb_resolved_write8	write;
	UINT8		mask_value; /* in READ operation the value can be forced by a extern electric circuit */
};

struct _cia_state
{
	running_device *device;
	devcb_resolved_write_line out_irq_func;
	devcb_resolved_write_line out_pc_func;
	devcb_resolved_write_line out_cnt_func;
	devcb_resolved_write_line out_sp_func;

	cia_port		port[2];
	cia_timer		timer[2];

	/* Time Of the Day clock (TOD) */
	UINT32			tod;
	UINT32			tod_latch;
	UINT8			tod_latched;
	UINT8			tod_running;
	UINT32			alarm;

	/* Interrupts */
	UINT8			icr;
	UINT8			ics;
	UINT8			irq;
	int				flag;

	/* Serial */
	UINT8			loaded;
	UINT8			sdr;
	UINT8			sp;
	UINT8			cnt;
	UINT8			shift;
	UINT8			serial;
};

/***************************************************************************
    PROTOTYPES
***************************************************************************/

static TIMER_CALLBACK( cia_timer_proc );
static void cia_timer_underflow(running_device *device, int timer);
static TIMER_CALLBACK( cia_clock_tod_callback );

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE cia_state *get_token(running_device *device)
{
	assert(device != NULL);
	assert((device->type() == MOS6526R1) || (device->type() == MOS6526R2) || (device->type() == MOS8520));
	return (cia_state *) downcast<legacy_device_base *>(device)->token();
}

INLINE const mos6526_interface *get_interface(running_device *device)
{
	assert(device != NULL);
	assert((device->type() == MOS6526R1) || (device->type() == MOS6526R2) || (device->type() == MOS8520));
	return (mos6526_interface *) device->baseconfig().static_config();
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    cia_set_port_mask_value
-------------------------------------------------*/

void cia_set_port_mask_value(running_device *device, int port, int data)
{
	cia_state *cia = get_token(device);
	cia->port[port].mask_value = data;
}

/*-------------------------------------------------
    DEVICE_RESET( cia )
-------------------------------------------------*/

static DEVICE_RESET( cia )
{
	int t;
	cia_state *cia = get_token(device);

	/* clear things out */
	cia->port[0].latch = 0x00;
	cia->port[0].in = 0x00;
	cia->port[0].out = 0x00;
	cia->port[0].mask_value = 0xff;
	cia->port[1].latch = 0x00;
	cia->port[1].in = 0x00;
	cia->port[1].out = 0x00;
	cia->port[1].mask_value = 0xff;
	cia->tod = 0;
	cia->tod_latch = 0;
	cia->alarm = 0;
	cia->icr = 0x00;
	cia->ics = 0x00;
	cia->irq = 0;
	cia->shift = 0;
	cia->loaded = 0;
	cia->cnt = 1;
	cia->sp = 0;

	/* initialize data direction registers */
	cia->port[0].ddr = !strcmp(device->tag(), "cia_0") ? 0x03 : 0xff;
	cia->port[1].ddr = !strcmp(device->tag(), "cia_0") ? 0x00 : 0xff;

	/* TOD running by default */
	cia->tod_running = TRUE;

	/* initialize timers */
	for (t = 0; t < 2; t++)
	{
		cia_timer *timer = &cia->timer[t];

		timer->latch = 0xffff;
		timer->count = 0x0000;
		timer->mode = 0x00;
	}
}

/*-------------------------------------------------
    DEVICE_VALIDITY_CHECK( cia )
-------------------------------------------------*/

#if 0
static DEVICE_VALIDITY_CHECK( cia )
{
	int error = FALSE;

	if (device->clock() <= 0)
	{
		mame_printf_error("%s: %s has a cia with an invalid clock\n", driver->source_file, driver->name);
		error = TRUE;
	}

	return error;
}
#endif

/*-------------------------------------------------
    cia_update_pc - pulse /pc output
-------------------------------------------------*/

static void cia_update_pc(running_device *device)
{
	cia_state *cia = get_token(device);

	/* this should really be one cycle long */
	devcb_call_write_line(&cia->out_pc_func, 0);
	devcb_call_write_line(&cia->out_pc_func, 1);
}

/*-------------------------------------------------
    cia_update_interrupts
-------------------------------------------------*/

static void cia_update_interrupts(running_device *device)
{
	UINT8 new_irq;
	cia_state *cia = get_token(device);

	/* always update the high bit of ICS */
	if (cia->ics & 0x7f)
		cia->ics |= 0x80;
	else
		cia->ics &= ~0x80;

	/* based on what is enabled, set/clear the IRQ via the custom chip */
	new_irq = (cia->ics & cia->icr) ? 1 : 0;
	if (cia->irq != new_irq)
	{
		cia->irq = new_irq;
		devcb_call_write_line(&cia->out_irq_func, cia->irq);
	}
}

/*-------------------------------------------------
    is_timer_active
-------------------------------------------------*/

static int is_timer_active(emu_timer *timer)
{
	attotime t = timer_firetime(timer);
	return attotime_compare(t, attotime_never) != 0;
}

/*-------------------------------------------------
    cia_timer_update - updates the count and
    emu_timer for a given CIA timer
-------------------------------------------------*/

static void cia_timer_update(cia_timer *timer, INT32 new_count)
{
	int which = timer - timer->cia->timer;

	/* sanity check arguments */
	assert((new_count >= -1) && (new_count <= 0xffff));

	/* update the timer count, if necessary */
	if ((new_count == -1) && is_timer_active(timer->timer))
	{
		UINT16 current_count = attotime_to_double(attotime_mul(timer_timeelapsed(timer->timer), timer->cia->device->clock()));
		timer->count = timer->count - MIN(timer->count, current_count);
	}

	/* set the timer if we are instructed to */
	if (new_count != -1)
		timer->count = new_count;

	/* now update the MAME timer */
	if ((timer->mode & 0x01) && ((timer->mode & (which ? 0x60 : 0x20)) == 0x00))
	{
		/* timer is on and is connected to clock */
		attotime period = attotime_mul(ATTOTIME_IN_HZ(timer->cia->device->clock()), (timer->count ? timer->count : 0x10000));
		timer_adjust_oneshot(timer->timer, period, 0);
	}
	else
	{
		/* timer is off or not connected to clock */
		timer_adjust_oneshot(timer->timer, attotime_never, 0);
	}
}

/*-------------------------------------------------
    cia_get_timer - get the count
    for a given CIA timer
-------------------------------------------------*/

static UINT16 cia_get_timer(cia_timer *timer)
{
	UINT16 count;

	if (is_timer_active(timer->timer))
	{
		UINT16 current_count = attotime_to_double(attotime_mul(timer_timeelapsed(timer->timer), timer->cia->device->clock()));
		count = timer->count - MIN(timer->count, current_count);
	}
	else
		count = timer->count;

	return count;
}

/*-------------------------------------------------
    cia_timer_bump
-------------------------------------------------*/

static void cia_timer_bump(running_device *device, int timer)
{
	cia_state *cia = get_token(device);

	cia_timer_update(&cia->timer[timer], -1);

	if (cia->timer[timer].count == 0x00)
		cia_timer_underflow(device, timer);
	else
		cia_timer_update(&cia->timer[timer], cia->timer[timer].count - 1);
}

/*-------------------------------------------------
    cia_timer_underflow
-------------------------------------------------*/

static void cia_timer_underflow(running_device *device, int timer)
{
	cia_state *cia = get_token(device);

	assert((timer == 0) || (timer == 1));

	/* set the status and update interrupts */
	cia->ics |= cia->timer[timer].irq;
	cia_update_interrupts(device);

	/* if one-shot mode, turn it off */
	if (cia->timer[timer].mode & 0x08)
		cia->timer[timer].mode &= 0xfe;

	/* reload the timer */
	cia_timer_update(&cia->timer[timer], cia->timer[timer].latch);

	/* timer A has some interesting properties */
	if (timer == 0)
	{
		/* such as cascading to timer B */
		if ((cia->timer[1].mode & 0x41) == 0x41)
		{
			if (cia->cnt || !(cia->timer[1].mode & 0x20))
				cia_timer_bump(device, 1);
		}

		/* also the serial line */
		if ((cia->timer[timer].irq == 0x01) && (cia->timer[timer].mode & CIA_CRA_SPMODE))
		{
			if (cia->loaded || cia->shift)
			{
				/* falling edge */
				if (cia->cnt)
				{
					if (cia->shift == 0)
					{
						/* load shift register */
						cia->loaded = 0;
						cia->serial = cia->sdr;
					}

					/* transmit MSB */
					cia->sp = BIT(cia->serial, 7);
					devcb_call_write_line(&cia->out_sp_func, cia->sp);

					/* toggle CNT */
					cia->cnt = !cia->cnt;
					devcb_call_write_line(&cia->out_cnt_func, cia->cnt);

					/* shift data */
					cia->serial <<= 1;
					cia->shift++;

					if (cia->shift == 8)
					{
						/* signal interrupt */
						cia->ics |= 0x08;
						cia_update_interrupts(device);
					}
				}
				else
				{
					/* toggle CNT */
					cia->cnt = !cia->cnt;
					devcb_call_write_line(&cia->out_cnt_func, cia->cnt);

					if (cia->shift == 8)
					{
						cia->shift = 0;
					}
				}
			}
		}
	}
}

/*-------------------------------------------------
    TIMER_CALLBACK( cia_timer_proc )
-------------------------------------------------*/

static TIMER_CALLBACK( cia_timer_proc )
{
	cia_timer *timer = (cia_timer *)ptr;
	cia_state *cia = timer->cia;

	cia_timer_underflow(cia->device, timer - cia->timer);
}

/*-------------------------------------------------
    bcd_increment
-------------------------------------------------*/

static UINT8 bcd_increment(UINT8 value)
{
	value++;
	if ((value & 0x0f) >= 0x0a)
		value += 0x10 - 0x0a;
	return value;
}

/*-------------------------------------------------
    cia6526_increment
-------------------------------------------------*/

static void cia6526_increment(cia_state *cia)
{
	/* break down TOD value into components */
	UINT8 subsecond	= (UINT8) (cia->tod >>  0);
	UINT8 second	= (UINT8) (cia->tod >>  8);
	UINT8 minute	= (UINT8) (cia->tod >> 16);
	UINT8 hour		= (UINT8) (cia->tod >> 24);

	subsecond = bcd_increment(subsecond);
	if (subsecond >= 0x10)
	{
		subsecond = 0x00;
		second = bcd_increment(second);
		if (second >= ((cia->timer[0].mode & 0x80) ? 0x50 : 0x60))
		{
			second = 0x00;
			minute = bcd_increment(minute);
			if (minute >= 0x60)
			{
				minute = 0x00;
				if (hour == 0x91)
					hour = 0x00;
				else if (hour == 0x89)
					hour = 0x90;
				else if (hour == 0x11)
					hour = 0x80;
				else if (hour == 0x09)
					hour = 0x10;
				else
					hour++;
			}
		}
	}

	/* update the TOD with new value */
	cia->tod = (((UINT32) subsecond)	<<  0)
			 | (((UINT32) second)		<<  8)
			 | (((UINT32) minute)		<< 16)
			 | (((UINT32) hour)			<< 24);
}

/*-------------------------------------------------
    cia_clock_tod - Update TOD on CIA A
-------------------------------------------------*/

static void cia_clock_tod(running_device *device)
{
	cia_state *cia;

	cia = get_token(device);

	if (cia->tod_running)
	{
		if ((device->type() == MOS6526R1) || (device->type() == MOS6526R2))
		{
			/* The 6526 split the value into hours, minutes, seconds and
             * subseconds */
			cia6526_increment(cia);
		}
		else if (device->type() == MOS8520)
		{
			/* the 8520 has a straight 24-bit counter */
			cia->tod++;
			cia->tod &= 0xffffff;
		}

		if (cia->tod == cia->alarm)
		{
			cia->ics |= 0x04;
			cia_update_interrupts(device);
		}
	}
}

/*-------------------------------------------------
    cia_clock_tod_callback
-------------------------------------------------*/

static TIMER_CALLBACK( cia_clock_tod_callback )
{
	running_device *device = (running_device *)ptr;
	cia_clock_tod(device);
}

READ8_DEVICE_HANDLER( mos6526_pa_r )
{
	return (get_token(device)->port[0].latch | ~get_token(device)->port[0].ddr);
}

READ8_DEVICE_HANDLER( mos6526_pb_r )
{
	return (get_token(device)->port[1].latch | ~get_token(device)->port[1].ddr);
}

READ_LINE_DEVICE_HANDLER( mos6526_irq_r )
{
	cia_state *cia = get_token(device);

	return cia->irq;
}

WRITE_LINE_DEVICE_HANDLER( mos6526_tod_w )
{
	if (state) cia_clock_tod(device);
}

READ_LINE_DEVICE_HANDLER( mos6526_cnt_r )
{
	cia_state *cia = get_token(device);

	return cia->cnt;
}

WRITE_LINE_DEVICE_HANDLER( mos6526_cnt_w )
{
	cia_state *cia = get_token(device);

	/* is this a rising edge? */
	if (!cia->cnt && state)
	{
		if (cia->timer[0].mode & CIA_CRA_START)
		{
			/* does timer #0 bump on CNT? */
			if (cia->timer[0].mode & CIA_CRA_INMODE)
				cia_timer_bump(device, 0);
		}

		/* if the serial port is set to input, the CNT will shift the port */
		if (!(cia->timer[0].mode & CIA_CRA_SPMODE))
		{
			cia->serial <<= 1;
			cia->shift++;

			if (cia->sp) cia->serial |= 0x01;

			if (cia->shift == 8)
			{
				cia->sdr = cia->serial;
				cia->serial = 0;
				cia->shift = 0;
				cia->ics |= 0x08;
				cia_update_interrupts(device);
			}
		}

		/* does timer #1 bump on CNT? */
		if ((cia->timer[1].mode & 0x61) == 0x21)
			cia_timer_bump(device, 1);
	}

	cia->cnt = state;
}

READ_LINE_DEVICE_HANDLER( mos6526_sp_r )
{
	cia_state *cia = get_token(device);

	return cia->sp;
}

WRITE_LINE_DEVICE_HANDLER( mos6526_sp_w )
{
	cia_state *cia = get_token(device);

	cia->sp = state;
}

WRITE_LINE_DEVICE_HANDLER( mos6526_flag_w )
{
	cia_state *cia = get_token(device);

	/* falling edge */
	if (cia->flag && !state)
	{
		cia->ics |= 0x10;
		cia_update_interrupts(device);
	}

	cia->flag = state;
}

/*-------------------------------------------------
    mos6526_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( mos6526_r )
{
	cia_timer *timer;
	cia_state *cia;
	cia_port *port;
	UINT8 data = 0x00;

	cia = get_token(device);
	offset &= 0x0F;

	switch(offset)
	{
		/* port A/B data */
		case CIA_PRA:
		case CIA_PRB:
			port = &cia->port[offset & 1];
			data = devcb_call_read8(&port->read, 0);
			data = ((data & ~port->ddr) | (port->latch & port->ddr)) & port->mask_value;

			port->in = data;

			if (offset == CIA_PRB)
			{
				/* timer #0 can change PB6 */
				if (cia->timer[0].mode & 0x02)
				{
					cia_timer_update(&cia->timer[0], -1);
					if (cia->timer[0].count != 0)
						data |= 0x40;
					else
						data &= ~0x40;
				}

				/* timer #1 can change PB7 */
				if (cia->timer[1].mode & 0x02)
				{
					cia_timer_update(&cia->timer[1], -1);
					if (cia->timer[1].count != 0)
						data |= 0x80;
					else
						data &= ~0x80;
				}

				/* pulse /PC following the read */
				cia_update_pc(device);
			}
			break;

		/* port A/B direction */
		case CIA_DDRA:
		case CIA_DDRB:
			port = &cia->port[offset & 1];
			data = port->ddr;
			break;

		/* timer A/B low byte */
		case CIA_TALO:
		case CIA_TBLO:
			timer = &cia->timer[(offset >> 1) & 1];
			data = cia_get_timer(timer) >> 0;
			break;

		/* timer A/B high byte */
		case CIA_TAHI:
		case CIA_TBHI:
			timer = &cia->timer[(offset >> 1) & 1];
			data = cia_get_timer(timer) >> 8;
			break;

		/* TOD counter */
		case CIA_TOD0:
		case CIA_TOD1:
		case CIA_TOD2:
		case CIA_TOD3:
			if (device->type() == MOS8520)
			{
				if (offset == CIA_TOD2)
				{
					cia->tod_latch = cia->tod;
					cia->tod_latched = TRUE;
				}
			}
			else
			{
				if (offset == CIA_TOD3)
				{
					cia->tod_latch = cia->tod;
					cia->tod_latched = TRUE;
				}
			}
			if (offset == CIA_TOD0)
				cia->tod_latched = FALSE;

			if (cia->tod_latched)
				data = cia->tod_latch >> ((offset - CIA_TOD0) * 8);
			else
				data = cia->tod >> ((offset - CIA_TOD0) * 8);
			break;

		/* serial data ready */
		case CIA_SDR:
			data = cia->sdr;
			break;

		/* interrupt status/clear */
		case CIA_ICR:
			data = cia->ics;
			cia->ics = 0; /* clear on read */
			cia_update_interrupts(device);
			break;

		/* timer A/B mode */
		case CIA_CRA:
		case CIA_CRB:
			timer = &cia->timer[offset & 1];
			data = timer->mode;
			break;
	}

	return data;
}

/*-------------------------------------------------
    mos6526_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( mos6526_w )
{
	cia_timer *timer;
	cia_state *cia;
	cia_port *port;
	int shift;

	cia = get_token(device);
	offset &= 0x0F;

	switch(offset)
	{
		/* port A/B data */
		case CIA_PRA:
		case CIA_PRB:
			port = &cia->port[offset & 1];
			port->latch = data;
			port->out = (data & port->ddr) | (port->in & ~port->ddr);
			devcb_call_write8(&port->write, 0, port->out);

			/* pulse /PC following the write */
			if (offset == CIA_PRB)
				cia_update_pc(device);

			break;

		/* port A/B direction */
		case CIA_DDRA:
		case CIA_DDRB:
			port = &cia->port[offset & 1];
			port->ddr = data;
			break;

		/* timer A/B latch low */
		case CIA_TALO:
		case CIA_TBLO:
			timer = &cia->timer[(offset >> 1) & 1];
			timer->latch = (timer->latch & 0xff00) | (data << 0);
			break;

		/* timer A latch high */
		case CIA_TAHI:
		case CIA_TBHI:
			timer = &cia->timer[(offset >> 1) & 1];
			timer->latch = (timer->latch & 0x00ff) | (data << 8);

			/* if the timer is one-shot, then force a start on it */
			if (timer->mode & 0x08)
			{
				timer->mode |= 1;
				cia_timer_update(timer, timer->latch);
			}
			else
			{
				/* if the timer is off, update the count */
				if (!(timer->mode & 0x01))
					cia_timer_update(timer, timer->latch);
			}
			break;

		/* time of day latches */
		case CIA_TOD0:
		case CIA_TOD1:
		case CIA_TOD2:
		case CIA_TOD3:
			shift = 8 * ((offset - CIA_TOD0));

			/* alarm setting mode? */
			if (cia->timer[1].mode & 0x80)
				cia->alarm = (cia->alarm & ~(0xff << shift)) | (data << shift);
			/* counter setting mode */
			else
				cia->tod = (cia->tod & ~(0xff << shift)) | (data << shift);

			if (device->type() == MOS8520)
			{
				if (offset == CIA_TOD2)
					cia->tod_running = FALSE;
			}
			else
			{
				if (offset == CIA_TOD3)
					cia->tod_running = FALSE;
			}
			if (offset == CIA_TOD0)
				cia->tod_running = TRUE;
			break;

		/* serial data ready */
		case CIA_SDR:
			cia->sdr = data;
			if (cia->timer[0].mode & 0x40)
				cia->loaded = 1;
			break;

		/* interrupt control register */
		case CIA_ICR:
			if (data & 0x80)
				cia->icr |= data & 0x7f;
			else
				cia->icr &= ~(data & 0x7f);
			cia_update_interrupts(device);
			break;

		/* timer A/B modes */
		case CIA_CRA:
		case CIA_CRB:
			timer = &cia->timer[offset & 1];
			timer->mode = data & 0xef;

			/* force load? */
			if (data & 0x10)
				cia_timer_update(timer, timer->latch);
			else
				cia_timer_update(timer, -1);
			break;
	}
}

/*-------------------------------------------------
    DEVICE_START( cia )
-------------------------------------------------*/

static DEVICE_START( cia )
{
	int t, p;
	cia_state *cia = get_token(device);
	const mos6526_interface *intf = get_interface(device);

	/* clear out CIA structure, and copy the interface */
	memset(cia, 0, sizeof(*cia));
	cia->device = device;
	devcb_resolve_write_line(&cia->out_irq_func, &intf->out_irq_func, device);
	devcb_resolve_write_line(&cia->out_pc_func, &intf->out_pc_func, device);
	devcb_resolve_write_line(&cia->out_cnt_func, &intf->out_cnt_func, device);
	devcb_resolve_write_line(&cia->out_sp_func, &intf->out_sp_func, device);
	cia->flag = 1;

	/* setup ports */
	devcb_resolve_read8(&cia->port[0].read, &intf->in_pa_func, device);
	devcb_resolve_write8(&cia->port[0].write, &intf->out_pa_func, device);
	devcb_resolve_read8(&cia->port[1].read, &intf->in_pb_func, device);
	devcb_resolve_write8(&cia->port[1].write, &intf->out_pb_func, device);

	for (p = 0; p < (sizeof(cia->port) / sizeof(cia->port[0])); p++)
	{
		cia->port[p].mask_value = 0xff;
	}

	/* setup timers */
	for (t = 0; t < (sizeof(cia->timer) / sizeof(cia->timer[0])); t++)
	{
		cia_timer *timer = &cia->timer[t];
		timer->timer = timer_alloc(device->machine, cia_timer_proc, timer);
		timer->cia = cia;
		timer->irq = 0x01 << t;
	}

	/* setup TOD timer, if appropriate */
	if (intf->tod_clock != 0)
		timer_pulse(device->machine, ATTOTIME_IN_HZ(intf->tod_clock), (void *) device, 0, cia_clock_tod_callback);

	/* state save support */
	state_save_register_device_item(device, 0, cia->port[0].ddr);
	state_save_register_device_item(device, 0, cia->port[0].latch);
	state_save_register_device_item(device, 0, cia->port[0].in);
	state_save_register_device_item(device, 0, cia->port[0].out);
	state_save_register_device_item(device, 0, cia->port[0].mask_value);
	state_save_register_device_item(device, 0, cia->port[1].ddr);
	state_save_register_device_item(device, 0, cia->port[1].latch);
	state_save_register_device_item(device, 0, cia->port[1].in);
	state_save_register_device_item(device, 0, cia->port[1].out);
	state_save_register_device_item(device, 0, cia->port[1].mask_value);
	state_save_register_device_item(device, 0, cia->timer[0].latch);
	state_save_register_device_item(device, 0, cia->timer[0].count);
	state_save_register_device_item(device, 0, cia->timer[0].mode);
	state_save_register_device_item(device, 0, cia->timer[0].irq);
	state_save_register_device_item(device, 0, cia->timer[1].latch);
	state_save_register_device_item(device, 0, cia->timer[1].count);
	state_save_register_device_item(device, 0, cia->timer[1].mode);
	state_save_register_device_item(device, 0, cia->timer[1].irq);
	state_save_register_device_item(device, 0, cia->tod);
	state_save_register_device_item(device, 0, cia->tod_latch);
	state_save_register_device_item(device, 0, cia->tod_latched);
	state_save_register_device_item(device, 0, cia->tod_running);
	state_save_register_device_item(device, 0, cia->alarm);
	state_save_register_device_item(device, 0, cia->icr);
	state_save_register_device_item(device, 0, cia->ics);
	state_save_register_device_item(device, 0, cia->irq);
	state_save_register_device_item(device, 0, cia->flag);
	state_save_register_device_item(device, 0, cia->loaded);
	state_save_register_device_item(device, 0, cia->sdr);
	state_save_register_device_item(device, 0, cia->sp);
	state_save_register_device_item(device, 0, cia->cnt);
	state_save_register_device_item(device, 0, cia->shift);
	state_save_register_device_item(device, 0, cia->serial);
}

/*-------------------------------------------------
    DEVICE_GET_INFO( cia6526r1 )
-------------------------------------------------*/

DEVICE_GET_INFO(cia6526r1)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(cia_state);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;								break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(cia);		break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(cia);		break;
//      case DEVINFO_FCT_VALIDITY_CHECK:                info->validity_check = DEVICE_VALIDITY_CHECK_NAME(cia); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "MOS6526 rev1");			break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "MOS6526");					break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:						/* Nothing */								break;
	}
}

/*-------------------------------------------------
    DEVICE_GET_INFO( cia8520 )
-------------------------------------------------*/

DEVICE_GET_INFO(cia6526r2)
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "MOS6526 rev2");			break;
		default:	DEVICE_GET_INFO_CALL(cia6526r1);	break;
	}
}

/*-------------------------------------------------
    DEVICE_GET_INFO( cia8520 )
-------------------------------------------------*/

DEVICE_GET_INFO(cia8520)
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "MOS8520");				break;
		default:	DEVICE_GET_INFO_CALL(cia6526r1);	break;
	}
}

DEFINE_LEGACY_DEVICE(MOS6526R1, cia6526r1);
DEFINE_LEGACY_DEVICE(MOS6526R2, cia6526r2);
DEFINE_LEGACY_DEVICE(MOS8520, cia8520);
