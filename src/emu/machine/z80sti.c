/***************************************************************************

    Mostek MK3801 Serial Timer Interrupt Controller (Z80-STI) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

/*

    TODO:

    - timers (other than delay mode)
    - serial I/O
    - reset behavior

*/

#include "emu.h"
#include "z80sti.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/* registers */
enum
{
	Z80STI_REGISTER_IR = 0,
	Z80STI_REGISTER_GPIP,
	Z80STI_REGISTER_IPRB,
	Z80STI_REGISTER_IPRA,
	Z80STI_REGISTER_ISRB,
	Z80STI_REGISTER_ISRA,
	Z80STI_REGISTER_IMRB,
	Z80STI_REGISTER_IMRA,
	Z80STI_REGISTER_PVR,
	Z80STI_REGISTER_TABC,
	Z80STI_REGISTER_TBDR,
	Z80STI_REGISTER_TADR,
	Z80STI_REGISTER_UCR,
	Z80STI_REGISTER_RSR,
	Z80STI_REGISTER_TSR,
	Z80STI_REGISTER_UDR
};

/* variable registers */
enum
{
	Z80STI_REGISTER_IR_SCR = 0,
	Z80STI_REGISTER_IR_TDDR,
	Z80STI_REGISTER_IR_TCDR,
	Z80STI_REGISTER_IR_AER,
	Z80STI_REGISTER_IR_IERB,
	Z80STI_REGISTER_IR_IERA,
	Z80STI_REGISTER_IR_DDR,
	Z80STI_REGISTER_IR_TCDC
};

/* timers */
enum
{
	TIMER_A = 0,
	TIMER_B,
	TIMER_C,
	TIMER_D,
	TIMER_COUNT
};

/* interrupt levels */
enum
{
	Z80STI_IR_P0 = 0,
	Z80STI_IR_P1,
	Z80STI_IR_P2,
	Z80STI_IR_P3,
	Z80STI_IR_TD,
	Z80STI_IR_TC,
	Z80STI_IR_P4,
	Z80STI_IR_P5,
	Z80STI_IR_TB,
	Z80STI_IR_XE,
	Z80STI_IR_XB,
	Z80STI_IR_RE,
	Z80STI_IR_RB,
	Z80STI_IR_TA,
	Z80STI_IR_P6,
	Z80STI_IR_P7
};

/* timer C/D control register */
#define Z80STI_TCDC_TARS	0x80
#define Z80STI_TCDC_TBRS	0x08

/* interrupt vector register */
#define Z80STI_PVR_ISE		0x08
#define Z80STI_PVR_VR4		0x10

/* general purpose I/O interrupt levels */
static const int INT_LEVEL_GPIP[] =
{
	Z80STI_IR_P0, Z80STI_IR_P1, Z80STI_IR_P2, Z80STI_IR_P3,	Z80STI_IR_P4, Z80STI_IR_P5, Z80STI_IR_P6, Z80STI_IR_P7
};

/* timer interrupt levels */
static const int INT_LEVEL_TIMER[] =
{
	Z80STI_IR_TA, Z80STI_IR_TB, Z80STI_IR_TC, Z80STI_IR_TD
};

/* interrupt vectors */
static const UINT8 INT_VECTOR[] =
{
	0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e,
	0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e
};

/* timer prescaler divisors */
static const int PRESCALER[] = { 0, 4, 10, 16, 50, 64, 100, 200 };

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _z80sti_t z80sti_t;
struct _z80sti_t
{
	/* device callbacks */
	devcb_resolved_read8				in_gpio_func;
	devcb_resolved_write8				out_gpio_func;
	devcb_resolved_read_line			in_si_func;
	devcb_resolved_write_line			out_so_func;
	devcb_resolved_write_line			out_tao_func;
	devcb_resolved_write_line			out_tbo_func;
	devcb_resolved_write_line			out_tco_func;
	devcb_resolved_write_line			out_tdo_func;
	devcb_resolved_write_line			out_int_func;

	/* I/O state */
	UINT8 gpip;							/* general purpose I/O register */
	UINT8 aer;							/* active edge register */
	UINT8 ddr;							/* data direction register */

	/* interrupt state */
	UINT16 ier;							/* interrupt enable register */
	UINT16 ipr;							/* interrupt pending register */
	UINT16 isr;							/* interrupt in-service register */
	UINT16 imr;							/* interrupt mask register */
	UINT8 pvr;							/* interrupt vector register */
	int int_state[16];					/* interrupt state */

	/* timer state */
	UINT8 tabc;							/* timer A/B control register */
	UINT8 tcdc;							/* timer C/D control register */
	UINT8 tdr[TIMER_COUNT];				/* timer data registers */
	UINT8 tmc[TIMER_COUNT];				/* timer main counters */
	int to[TIMER_COUNT];				/* timer out latch */

	/* serial state */
	UINT8 scr;							/* synchronous character register */
	UINT8 ucr;							/* USART control register */
	UINT8 tsr;							/* transmitter status register */
	UINT8 rsr;							/* receiver status register */
	UINT8 udr;							/* USART data register */

	/* timers */
	emu_timer *timer[TIMER_COUNT];		/* counter timers */
	emu_timer *rx_timer;				/* serial receive timer */
	emu_timer *tx_timer;				/* serial transmit timer */
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static int z80sti_irq_state(running_device *device);
static void z80sti_irq_reti(running_device *device);

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE z80sti_t *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == Z80STI);
	return (z80sti_t *)device->token;
}

INLINE const z80sti_interface *get_interface(running_device *device)
{
	assert(device != NULL);
	assert((device->type == Z80STI));
	return (const z80sti_interface *) device->baseconfig().static_config;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    check_interrupts - set the interrupt request
    line state
-------------------------------------------------*/

static void check_interrupts(z80sti_t *z80sti)
{
	if (z80sti->ipr & z80sti->imr)
	{
		devcb_call_write_line(&z80sti->out_int_func, ASSERT_LINE);
	}
	else
	{
		devcb_call_write_line(&z80sti->out_int_func, CLEAR_LINE);
	}
}

/*-------------------------------------------------
    take_interrupt - mark an interrupt pending
-------------------------------------------------*/

static void take_interrupt(z80sti_t *z80sti, int level)
{
	/* set interrupt pending register bit */
	z80sti->ipr |= 1 << level;

	/* trigger interrupt */
	z80sti->int_state[level] |= Z80_DAISY_INT;

	check_interrupts(z80sti);
}

/*-------------------------------------------------
    serial_receive - receive serial bit
-------------------------------------------------*/

static void serial_receive(z80sti_t *z80sti)
{
}

/*-------------------------------------------------
    TIMER_CALLBACK( rx_tick )
-------------------------------------------------*/

static TIMER_CALLBACK( rx_tick )
{
	running_device *device = (running_device *)ptr;
	z80sti_t *z80sti = get_safe_token(device);

	serial_receive(z80sti);
}

/*-------------------------------------------------
    serial_transmit - transmit serial bit
-------------------------------------------------*/

static void serial_transmit(z80sti_t *z80sti)
{
}

/*-------------------------------------------------
    TIMER_CALLBACK( tx_tick )
-------------------------------------------------*/

static TIMER_CALLBACK( tx_tick )
{
	running_device *device = (running_device *)ptr;
	z80sti_t *z80sti = get_safe_token(device);

	serial_transmit(z80sti);
}

/*-------------------------------------------------
    z80sti_r - register read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80sti_r )
{
	z80sti_t *z80sti = get_safe_token(device);

	switch (offset & 0x0f)
	{
	case Z80STI_REGISTER_IR:
		switch (z80sti->pvr & 0x07)
		{
		case Z80STI_REGISTER_IR_SCR:	return z80sti->scr;
		case Z80STI_REGISTER_IR_TDDR:	return z80sti->tmc[TIMER_D];
		case Z80STI_REGISTER_IR_TCDR:	return z80sti->tmc[TIMER_C];
		case Z80STI_REGISTER_IR_AER:	return z80sti->aer;
		case Z80STI_REGISTER_IR_IERB:	return z80sti->ier & 0xff;
		case Z80STI_REGISTER_IR_IERA:	return z80sti->ier >> 8;
		case Z80STI_REGISTER_IR_DDR:	return z80sti->ddr;
		case Z80STI_REGISTER_IR_TCDC:	return z80sti->tcdc;
		}
		break;

	case Z80STI_REGISTER_GPIP:	z80sti->gpip = (devcb_call_read8(&z80sti->in_gpio_func, 0) & ~z80sti->ddr) | (z80sti->gpip & z80sti->ddr); return z80sti->gpip;
	case Z80STI_REGISTER_IPRB:	return z80sti->ipr & 0xff;
	case Z80STI_REGISTER_IPRA:	return z80sti->ipr >> 8;
	case Z80STI_REGISTER_ISRB:	return z80sti->isr & 0xff;
	case Z80STI_REGISTER_ISRA:	return z80sti->isr >> 8;
	case Z80STI_REGISTER_IMRB:	return z80sti->imr & 0xff;
	case Z80STI_REGISTER_IMRA:	return z80sti->imr >> 8;
	case Z80STI_REGISTER_PVR:	return z80sti->pvr;
	case Z80STI_REGISTER_TABC:	return z80sti->tabc;
	case Z80STI_REGISTER_TBDR:	return z80sti->tmc[TIMER_B];
	case Z80STI_REGISTER_TADR:	return z80sti->tmc[TIMER_A];
	case Z80STI_REGISTER_UCR:	return z80sti->ucr;
	case Z80STI_REGISTER_RSR:	return z80sti->rsr;
	case Z80STI_REGISTER_TSR:	return z80sti->tsr;
	case Z80STI_REGISTER_UDR:	return z80sti->udr;
	}

	return 0;
}

/*-------------------------------------------------
    z80sti_w - register write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80sti_w )
{
	z80sti_t *z80sti = get_safe_token(device);

	switch (offset & 0x0f)
	{
	case Z80STI_REGISTER_IR:
		switch (z80sti->pvr & 0x07)
		{
		case Z80STI_REGISTER_IR_SCR:
			LOG(("Z80STI '%s' Sync Character Register: %x\n", device->tag(), data));
			z80sti->scr = data;
			break;

		case Z80STI_REGISTER_IR_TDDR:
			LOG(("Z80STI '%s' Timer D Data Register: %x\n", device->tag(), data));
			z80sti->tdr[TIMER_D] = data;
			break;

		case Z80STI_REGISTER_IR_TCDR:
			LOG(("Z80STI '%s' Timer C Data Register: %x\n", device->tag(), data));
			z80sti->tdr[TIMER_C] = data;
			break;

		case Z80STI_REGISTER_IR_AER:
			LOG(("Z80STI '%s' Active Edge Register: %x\n", device->tag(), data));
			z80sti->aer = data;
			break;

		case Z80STI_REGISTER_IR_IERB:
			LOG(("Z80STI '%s' Interrupt Enable Register B: %x\n", device->tag(), data));
			z80sti->ier = (z80sti->ier & 0xff00) | data;
			check_interrupts(z80sti);
			break;

		case Z80STI_REGISTER_IR_IERA:
			LOG(("Z80STI '%s' Interrupt Enable Register A: %x\n", device->tag(), data));
			z80sti->ier = (data << 8) | (z80sti->ier & 0xff);
			check_interrupts(z80sti);
			break;

		case Z80STI_REGISTER_IR_DDR:
			LOG(("Z80STI '%s' Data Direction Register: %x\n", device->tag(), data));
			z80sti->ddr = data;
			break;

		case Z80STI_REGISTER_IR_TCDC:
			{
			int tcc = PRESCALER[(data >> 4) & 0x07];
			int tdc = PRESCALER[data & 0x07];

			z80sti->tcdc = data;

			LOG(("Z80STI '%s' Timer C Prescaler: %u\n", device->tag(), tcc));
			LOG(("Z80STI '%s' Timer D Prescaler: %u\n", device->tag(), tdc));

			if (tcc)
				timer_adjust_periodic(z80sti->timer[TIMER_C], ATTOTIME_IN_HZ(device->clock / tcc), 0, ATTOTIME_IN_HZ(device->clock / tcc));
			else
				timer_enable(z80sti->timer[TIMER_C], 0);

			if (tdc)
				timer_adjust_periodic(z80sti->timer[TIMER_D], ATTOTIME_IN_HZ(device->clock / tdc), 0, ATTOTIME_IN_HZ(device->clock / tdc));
			else
				timer_enable(z80sti->timer[TIMER_D], 0);

			if (BIT(data, 7))
			{
				LOG(("Z80STI '%s' Timer A Reset\n", device->tag()));
				z80sti->to[TIMER_A] = 0;

				devcb_call_write_line(&z80sti->out_tao_func, z80sti->to[TIMER_A]);
			}

			if (BIT(data, 3))
			{
				LOG(("Z80STI '%s' Timer B Reset\n", device->tag()));
				z80sti->to[TIMER_B] = 0;

				devcb_call_write_line(&z80sti->out_tbo_func, z80sti->to[TIMER_B]);
			}
			}
			break;
		}
		break;

	case Z80STI_REGISTER_GPIP:
		LOG(("Z80STI '%s' General Purpose I/O Register: %x\n", device->tag(), data));
		z80sti->gpip = data & z80sti->ddr;
		devcb_call_write8(&z80sti->out_gpio_func, 0, z80sti->gpip);
		break;

	case Z80STI_REGISTER_IPRB:
		{
		int i;
		LOG(("Z80STI '%s' Interrupt Pending Register B: %x\n", device->tag(), data));
		z80sti->ipr &= (z80sti->ipr & 0xff00) | data;

		for (i = 0; i < 16; i++)
		{
			if (!BIT(z80sti->ipr, i) && (z80sti->int_state[i] == Z80_DAISY_INT)) z80sti->int_state[i] = 0;
		}

		check_interrupts(z80sti);
		}
		break;

	case Z80STI_REGISTER_IPRA:
		{
		int i;
		LOG(("Z80STI '%s' Interrupt Pending Register A: %x\n", device->tag(), data));
		z80sti->ipr &= (data << 8) | (z80sti->ipr & 0xff);

		for (i = 0; i < 16; i++)
		{
			if (!BIT(z80sti->ipr, i) && (z80sti->int_state[i] == Z80_DAISY_INT)) z80sti->int_state[i] = 0;
		}

		check_interrupts(z80sti);
		}
		break;

	case Z80STI_REGISTER_ISRB:
		LOG(("Z80STI '%s' Interrupt In-Service Register B: %x\n", device->tag(), data));
		z80sti->isr &= (z80sti->isr & 0xff00) | data;
		break;

	case Z80STI_REGISTER_ISRA:
		LOG(("Z80STI '%s' Interrupt In-Service Register A: %x\n", device->tag(), data));
		z80sti->isr &= (data << 8) | (z80sti->isr & 0xff);
		break;

	case Z80STI_REGISTER_IMRB:
		LOG(("Z80STI '%s' Interrupt Mask Register B: %x\n", device->tag(), data));
		z80sti->imr = (z80sti->imr & 0xff00) | data;
		z80sti->isr &= z80sti->imr;
		check_interrupts(z80sti);
		break;

	case Z80STI_REGISTER_IMRA:
		LOG(("Z80STI '%s' Interrupt Mask Register A: %x\n", device->tag(), data));
		z80sti->imr = (data << 8) | (z80sti->imr & 0xff);
		z80sti->isr &= z80sti->imr;
		check_interrupts(z80sti);
		break;

	case Z80STI_REGISTER_PVR:
		LOG(("Z80STI '%s' Interrupt Vector: %02x\n", device->tag(), data & 0xe0));
		LOG(("Z80STI '%s' IR Address: %01x\n", device->tag(), data & 0x07));
		z80sti->pvr = data;
		break;

	case Z80STI_REGISTER_TABC:
		{
		int tac = PRESCALER[(data >> 4) & 0x07];
		int tbc = PRESCALER[data & 0x07];

		z80sti->tabc = data;

		LOG(("Z80STI '%s' Timer A Prescaler: %u\n", device->tag(), tac));
		LOG(("Z80STI '%s' Timer B Prescaler: %u\n", device->tag(), tbc));

		if (tac)
			timer_adjust_periodic(z80sti->timer[TIMER_A], ATTOTIME_IN_HZ(device->clock / tac), 0, ATTOTIME_IN_HZ(device->clock / tac));
		else
			timer_enable(z80sti->timer[TIMER_A], 0);

		if (tbc)
			timer_adjust_periodic(z80sti->timer[TIMER_B], ATTOTIME_IN_HZ(device->clock / tbc), 0, ATTOTIME_IN_HZ(device->clock / tbc));
		else
			timer_enable(z80sti->timer[TIMER_B], 0);
		}
		break;

	case Z80STI_REGISTER_TBDR:
		LOG(("Z80STI '%s' Timer B Data Register: %x\n", device->tag(), data));
		z80sti->tdr[TIMER_B] = data;
		break;

	case Z80STI_REGISTER_TADR:
		LOG(("Z80STI '%s' Timer A Data Register: %x\n", device->tag(), data));
		z80sti->tdr[TIMER_A] = data;
		break;
#if 0
    case Z80STI_REGISTER_UCR:
        z80sti->ucr = data;
        break;

    case Z80STI_REGISTER_RSR:
        z80sti->rsr = data;
        break;

    case Z80STI_REGISTER_TSR:
        z80sti->tsr = data;
        break;

    case Z80STI_REGISTER_UDR:
        z80sti->udr = data;
        break;
#endif
	default:
		LOG(("Z80STI '%s' Unsupported Register %x\n", device->tag(), offset & 0x0f));
	}
}

/*-------------------------------------------------
    z80sti_rc_w - receiver clock write
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80sti_rc_w )
{
	z80sti_t *z80sti = get_safe_token(device);

	if (state)
	{
		serial_receive(z80sti);
	}
}

/*-------------------------------------------------
    z80sti_tc_w - transmitter clock write
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80sti_tc_w )
{
	z80sti_t *z80sti = get_safe_token(device);

	if (state)
	{
		serial_transmit(z80sti);
	}
}

/*-------------------------------------------------
    timer_count - timer count down
-------------------------------------------------*/

static void timer_count(running_device *device, int index)
{
	z80sti_t *z80sti = get_safe_token(device);

	if (z80sti->tmc[index] == 0x01)
	{
		//LOG(("Z80STI '%s' Timer %c Expired\n", device->tag, 'A' + index));

		/* toggle timer output signal */
		z80sti->to[index] = !z80sti->to[index];

		switch (index)
		{
		case TIMER_A: devcb_call_write_line(&z80sti->out_tao_func, z80sti->to[index]); break;
		case TIMER_B: devcb_call_write_line(&z80sti->out_tbo_func, z80sti->to[index]); break;
		case TIMER_C: devcb_call_write_line(&z80sti->out_tco_func, z80sti->to[index]); break;
		case TIMER_D: devcb_call_write_line(&z80sti->out_tdo_func, z80sti->to[index]); break;
		}

		if (z80sti->ier & (1 << INT_LEVEL_TIMER[index]))
		{
			LOG(("Z80STI '%s' Interrupt Pending for Timer %c\n", device->tag(), 'A' + index));

			/* signal timer elapsed interrupt */
			take_interrupt(z80sti, INT_LEVEL_TIMER[index]);
		}

		/* load timer main counter */
		z80sti->tmc[index] = z80sti->tdr[index];
	}
	else
	{
		/* count down */
		z80sti->tmc[index]--;
	}
}

/*-------------------------------------------------
    TIMER_CALLBACK( timer_# )
-------------------------------------------------*/

static TIMER_CALLBACK( timer_a ) { timer_count((running_device *)ptr, TIMER_A); }
static TIMER_CALLBACK( timer_b ) { timer_count((running_device *)ptr, TIMER_B); }
static TIMER_CALLBACK( timer_c ) { timer_count((running_device *)ptr, TIMER_C); }
static TIMER_CALLBACK( timer_d ) { timer_count((running_device *)ptr, TIMER_D); }

/*-------------------------------------------------
    gpip_input - GPIP input line write
-------------------------------------------------*/

static void gpip_input(running_device *device, int bit, int state)
{
	z80sti_t *z80sti = get_safe_token(device);

	int aer = BIT(z80sti->aer, bit);
	int old_state = BIT(z80sti->gpip, bit);

	if ((old_state ^ aer) && !(state ^ aer))
	{
		LOG(("Z80STI '%s' Edge Transition Detected on Bit: %u\n", device->tag(), bit));

		if (z80sti->ier & (1 << INT_LEVEL_GPIP[bit]))
		{
			LOG(("Z80STI '%s' Interrupt Pending for P%u\n", device->tag(), bit));

			take_interrupt(z80sti, INT_LEVEL_GPIP[bit]);
		}
	}

	z80sti->gpip = (z80sti->gpip & ~(1 << bit)) | (state << bit);
}

/*-------------------------------------------------
    z80sti_i#_w - GPIP input line # write
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80sti_i0_w ) { gpip_input(device, 0, state); }
WRITE_LINE_DEVICE_HANDLER( z80sti_i1_w ) { gpip_input(device, 1, state); }
WRITE_LINE_DEVICE_HANDLER( z80sti_i2_w ) { gpip_input(device, 2, state); }
WRITE_LINE_DEVICE_HANDLER( z80sti_i3_w ) { gpip_input(device, 3, state); }
WRITE_LINE_DEVICE_HANDLER( z80sti_i4_w ) { gpip_input(device, 4, state); }
WRITE_LINE_DEVICE_HANDLER( z80sti_i5_w ) { gpip_input(device, 5, state); }
WRITE_LINE_DEVICE_HANDLER( z80sti_i6_w ) { gpip_input(device, 6, state); }
WRITE_LINE_DEVICE_HANDLER( z80sti_i7_w ) { gpip_input(device, 7, state); }

/*-------------------------------------------------
    z80sti_irq_state - get interrupt status
-------------------------------------------------*/

static int z80sti_irq_state(running_device *device)
{
	z80sti_t *z80sti = get_safe_token(device);
	int state = 0, i;

	/* loop over all interrupt sources */
	for (i = 15; i >= 0; i--)
	{
		/* if we're servicing a request, don't indicate more interrupts */
		if (z80sti->int_state[i] & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}

		if (BIT(z80sti->imr, i))
		{
			state |= z80sti->int_state[i];
		}
	}

	LOG(("Z80STI '%s' Interrupt State: %u\n", device->tag(), state));

	return state;
}

/*-------------------------------------------------
    z80sti_irq_ack - interrupt acknowledge
-------------------------------------------------*/

static int z80sti_irq_ack(running_device *device)
{
	z80sti_t *z80sti = get_safe_token(device);
	int i;

	/* loop over all interrupt sources */
	for (i = 15; i >= 0; i--)
	{
		/* find the first channel with an interrupt requested */
		if (z80sti->int_state[i] & Z80_DAISY_INT)
		{
			UINT8 vector = (z80sti->pvr & 0xe0) | INT_VECTOR[i];

			/* clear interrupt, switch to the IEO state, and update the IRQs */
			z80sti->int_state[i] = Z80_DAISY_IEO;

			/* clear interrupt pending register bit */
			z80sti->ipr &= ~(1 << i);

			/* set interrupt in-service register bit */
			z80sti->isr |= (1 << i);

			check_interrupts(z80sti);

			LOG(("Z80STI '%s' Interrupt Acknowledge Vector: %02x\n", device->tag(), vector));

			return vector;
		}
	}

	logerror("z80sti_irq_ack: failed to find an interrupt to ack!\n");

	return 0;
}

/*-------------------------------------------------
    z80sti_irq_reti - return from interrupt
-------------------------------------------------*/

static void z80sti_irq_reti(running_device *device)
{
	z80sti_t *z80sti = get_safe_token(device);
	int i;

	LOG(("Z80STI '%s' Return from Interrupt\n", device->tag()));

	/* loop over all interrupt sources */
	for (i = 15; i >= 0; i--)
	{
		/* find the first channel with an IEO pending */
		if (z80sti->int_state[i] & Z80_DAISY_IEO)
		{
			/* clear the IEO state and update the IRQs */
			z80sti->int_state[i] &= ~Z80_DAISY_IEO;

			/* clear interrupt in-service register bit */
			z80sti->isr &= ~(1 << i);

			check_interrupts(z80sti);
			return;
		}
	}

	logerror("z80sti_irq_reti: failed to find an interrupt to clear IEO on!\n");
}

/*-------------------------------------------------
    DEVICE_START( z80sti )
-------------------------------------------------*/

static DEVICE_START( z80sti )
{
	z80sti_t *z80sti = get_safe_token(device);
	const z80sti_interface *intf = (const z80sti_interface *)device->baseconfig().static_config;

	/* resolve callbacks */
	devcb_resolve_read8(&z80sti->in_gpio_func, &intf->in_gpio_func, device);
	devcb_resolve_write8(&z80sti->out_gpio_func, &intf->out_gpio_func, device);
	devcb_resolve_read_line(&z80sti->in_si_func, &intf->in_si_func, device);
	devcb_resolve_write_line(&z80sti->out_so_func, &intf->out_so_func, device);
	devcb_resolve_write_line(&z80sti->out_tao_func, &intf->out_tao_func, device);
	devcb_resolve_write_line(&z80sti->out_tbo_func, &intf->out_tbo_func, device);
	devcb_resolve_write_line(&z80sti->out_tco_func, &intf->out_tco_func, device);
	devcb_resolve_write_line(&z80sti->out_tdo_func, &intf->out_tdo_func, device);
	devcb_resolve_write_line(&z80sti->out_int_func, &intf->out_int_func, device);

	/* create the counter timers */
	z80sti->timer[TIMER_A] = timer_alloc(device->machine, timer_a, (void *)device);
	z80sti->timer[TIMER_B] = timer_alloc(device->machine, timer_b, (void *)device);
	z80sti->timer[TIMER_C] = timer_alloc(device->machine, timer_c, (void *)device);
	z80sti->timer[TIMER_D] = timer_alloc(device->machine, timer_d, (void *)device);

	/* create serial receive clock timer */
	if (intf->rx_clock > 0)
	{
		z80sti->rx_timer = timer_alloc(device->machine, rx_tick, (void *)device);
		timer_adjust_periodic(z80sti->rx_timer, attotime_zero, 0, ATTOTIME_IN_HZ(intf->rx_clock));
	}

	/* create serial transmit clock timer */
	if (intf->tx_clock > 0)
	{
		z80sti->tx_timer = timer_alloc(device->machine, tx_tick, (void *)device);
		timer_adjust_periodic(z80sti->tx_timer, attotime_zero, 0, ATTOTIME_IN_HZ(intf->tx_clock));
	}

	/* register for state saving */
	state_save_register_device_item(device, 0, z80sti->gpip);
	state_save_register_device_item(device, 0, z80sti->aer);
	state_save_register_device_item(device, 0, z80sti->ddr);
	state_save_register_device_item(device, 0, z80sti->ier);
	state_save_register_device_item(device, 0, z80sti->ipr);
	state_save_register_device_item(device, 0, z80sti->isr);
	state_save_register_device_item(device, 0, z80sti->imr);
	state_save_register_device_item(device, 0, z80sti->pvr);
	state_save_register_device_item_array(device, 0, z80sti->int_state);
	state_save_register_device_item(device, 0, z80sti->tabc);
	state_save_register_device_item(device, 0, z80sti->tcdc);
	state_save_register_device_item_array(device, 0, z80sti->tdr);
	state_save_register_device_item_array(device, 0, z80sti->tmc);
	state_save_register_device_item_array(device, 0, z80sti->to);
	state_save_register_device_item(device, 0, z80sti->scr);
	state_save_register_device_item(device, 0, z80sti->ucr);
	state_save_register_device_item(device, 0, z80sti->rsr);
	state_save_register_device_item(device, 0, z80sti->tsr);
	state_save_register_device_item(device, 0, z80sti->udr);
}

/*-------------------------------------------------
    DEVICE_RESET( z80sti )
-------------------------------------------------*/

static DEVICE_RESET( z80sti )
{
}

/*-------------------------------------------------
    DEVICE_GET_INFO( z80sti )
-------------------------------------------------*/

DEVICE_GET_INFO( z80sti )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(z80sti_t);						break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;									break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(z80sti);		break;
		case DEVINFO_FCT_STOP:							/* Nothing */									break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(z80sti);		break;
		case DEVINFO_FCT_IRQ_STATE:						info->f = (genf *)z80sti_irq_state;				break;
		case DEVINFO_FCT_IRQ_ACK:						info->f = (genf *)z80sti_irq_ack;				break;
		case DEVINFO_FCT_IRQ_RETI:						info->f = (genf *)z80sti_irq_reti;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Mostek MK3801");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Z80-STI");						break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright the MESS Team");		break;
	}
}
