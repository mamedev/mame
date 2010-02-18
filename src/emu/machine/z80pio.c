/***************************************************************************

    Zilog Z80 Parallel Input/Output Controller implementation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.
	
***************************************************************************/

#include "emu.h"
#include "z80pio.h"
#include "cpu/z80/z80daisy.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG 0

enum
{
	PORT_A = 0,
	PORT_B,
	PORT_COUNT
};

enum
{
	MODE_OUTPUT = 0,
	MODE_INPUT,
	MODE_BIDIRECTIONAL,
	MODE_BIT_CONTROL
};

enum
{
	ANY = 0,
	IOR,
	MASK
};

#define ICW_ENABLE_INT		0x80
#define ICW_AND_OR			0x40
#define ICW_AND				0x40
#define ICW_OR				0x00
#define ICW_HIGH_LOW		0x20
#define ICW_HIGH			0x20
#define ICW_LOW				0x00
#define ICW_MASK_FOLLOWS	0x10

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _pio_port pio_port;
struct _pio_port
{
	devcb_resolved_read8		in_p_func;
	devcb_resolved_write8		out_p_func;
	devcb_resolved_write_line	out_rdy_func;

	int mode;					/* mode register */
	int next_control_word;		/* next control word */
	UINT8 input;				/* input latch */
	UINT8 output;				/* output latch */
	UINT8 ior;					/* input/output register */
	int rdy;					/* ready */
	int stb;					/* strobe */

	/* interrupts */
	int ie;						/* interrupt enabled */
	int ip;						/* interrupt pending */
	int ius;					/* interrupt under service */
	UINT8 icw;					/* interrupt control word */
	UINT8 vector;				/* interrupt vector */
	UINT8 mask;					/* interrupt mask */
	int match;					/* logic equation match */
};

typedef struct _z80pio_t z80pio_t;
struct _z80pio_t
{
	pio_port port[PORT_COUNT];

	devcb_resolved_write_line	out_int_func;
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE z80pio_t *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == Z80PIO);
	return (z80pio_t *) device->token;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static void check_interrupts(running_device *device)
{
	z80pio_t *z80pio = get_safe_token(device);

	int state = CLEAR_LINE;
	
	for (int index = PORT_A; index < PORT_COUNT; index++)
	{
		pio_port *port = &z80pio->port[index];

		if (port->mode == MODE_BIT_CONTROL)
		{
			/* fetch input data (ignore output lines) */
			UINT8 data = (port->input & port->ior) | (port->output & ~port->ior);
			UINT8 mask = ~port->mask;
			int match = 0;

			data &= mask;

			if ((port->icw & 0x60) == 0 && data != mask) match = 1;
			else if ((port->icw & 0x60) == 0x20 && data != 0) match = 1;
			else if ((port->icw & 0x60) == 0x40 && data == 0) match = 1;
			else if ((port->icw & 0x60) == 0x60 && data == mask) match = 1;

			if (!port->match && match)
			{
				/* trigger interrupt */
				port->ip = 1;
				if (LOG) logerror("Z80PIO '%s' Port %c Interrupt Pending\n", device->tag.cstr(), 'A' + index);
			}

			port->match = match;
		}

		if (port->ie && port->ip && !port->ius)
		{
			state = ASSERT_LINE;
		}
	}

	devcb_call_write_line(&z80pio->out_int_func, state);
}

static void trigger_interrupt(running_device *device, int index)
{
	z80pio_t *z80pio = get_safe_token(device);
	pio_port *port = &z80pio->port[index];

	port->ip = 1;
	if (LOG) logerror("Z80PIO '%s' Port %c Interrupt Pending\n", device->tag.cstr(), 'A' + index);

	check_interrupts(device);
}

static void set_rdy(running_device *device, int index, int state)
{
	z80pio_t *z80pio = get_safe_token(device);
	pio_port *port = &z80pio->port[index];

	if (port->rdy == state) return;

	if (LOG) logerror("Z80PIO '%s' Port %c Ready: %u\n", device->tag.cstr(), 'A' + index, state);

	port->rdy = state;
	devcb_call_write_line(&port->out_rdy_func, state);
}

/*-------------------------------------------------
    z80pio_c_r - control register read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80pio_c_r )
{
	z80pio_t *z80pio = get_safe_token(device);

	return (z80pio->port[PORT_A].icw & 0xc0) | (z80pio->port[PORT_B].icw >> 4);
}

/*-------------------------------------------------
    set_mode - set port mode
-------------------------------------------------*/

static void set_mode(running_device *device, int index, int mode)
{
	z80pio_t *z80pio = get_safe_token(device);
	pio_port *port = &z80pio->port[index];

	if (LOG) logerror("Z80PIO '%s' Port %c Mode: %u\n", device->tag.cstr(), 'A' + index, mode);

	switch (mode)
	{
	case MODE_OUTPUT:
		/* enable data output */
		devcb_call_write8(&port->out_p_func, 0, port->output);

		/* assert ready line */
		set_rdy(device, index, 1);

		/* set mode register */
		port->mode = mode;
		break;

	case MODE_INPUT:
		/* set mode register */
		port->mode = mode;
		break;

	case MODE_BIDIRECTIONAL:
		if (index == PORT_B)
		{
			logerror("Z80PIO '%s' Port %c Invalid Mode: %u!\n", device->tag.cstr(), 'A' + index, mode);
		}
		else
		{
			/* set mode register */
			port->mode = mode;
		}
		break;

	case MODE_BIT_CONTROL:
		if ((index == PORT_A) || (z80pio->port[PORT_A].mode != MODE_BIDIRECTIONAL))
		{
			/* clear ready line */
			set_rdy(device, index, 0);
		}

		/* disable interrupts until IOR is written */
		port->ie = 0;
		check_interrupts(device);

		/* set logic equation to false */
		port->match = 0;

		/* next word is I/O register */
		port->next_control_word = IOR;

		/* set mode register */
		port->mode = mode;
		break;
	}
}

/*-------------------------------------------------
    z80pio_c_w - control register write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80pio_c_w )
{
	z80pio_t *z80pio = get_safe_token(device);
	int index = offset & 0x01;
	pio_port *port = &z80pio->port[index];

	switch (port->next_control_word)
	{
	case ANY:
		if (!BIT(data, 0))
		{
			/* load interrupt vector */
			port->vector = data;
			if (LOG) logerror("Z80PIO '%s' Port %c Interrupt Vector: %02x\n", device->tag.cstr(), 'A' + index, data);

			/* set interrupt enable */
			port->icw |= ICW_ENABLE_INT;
			port->ie = 1;
			check_interrupts(device);
		}
		else
		{
			switch (data & 0x0f)
			{
			case 0x0f: /* select operating mode */
				set_mode(device, index, data >> 6);
				break;

			case 0x07: /* set interrupt control word */
				port->icw = data;

				if (LOG)
				{
					logerror("Z80PIO '%s' Port %c Interrupt Enable: %u\n", device->tag.cstr(), 'A' + index, BIT(data, 7));
					logerror("Z80PIO '%s' Port %c Logic: %s\n", device->tag.cstr(), 'A' + index, BIT(data, 6) ? "AND" : "OR");
					logerror("Z80PIO '%s' Port %c Active %s\n", device->tag.cstr(), 'A' + index, BIT(data, 5) ? "High" : "Low");
					logerror("Z80PIO '%s' Port %c Mask Follows: %u\n", device->tag.cstr(), 'A' + index, BIT(data, 4));
				}

				if (port->icw & ICW_MASK_FOLLOWS)
				{
					/* disable interrupts until mask is written */
					port->ie = 0;

					/* reset pending interrupts */
					port->ip = 0;
					check_interrupts(device);

					/* set logic equation to false */
					port->match = 0;

					/* next word is mask control */
					port->next_control_word = MASK;
				}
				else
				{
					/* monitor all bits */
					port->mask = 0;
				}
				break;

			case 0x03: /* set interrupt enable flip-flop */
				port->icw = (data & 0x80) | (port->icw & 0x7f);
				if (LOG) logerror("Z80PIO '%s' Port %c Interrupt Enable: %u\n", device->tag.cstr(), 'A' + index, BIT(data, 7));

				/* set interrupt enable */
				port->ie = BIT(port->icw, 7);
				check_interrupts(device);
				break;

			default:
				logerror("Z80PIO '%s' Port %c Invalid Control Word: %02x!\n", device->tag.cstr(), 'A' + index, data);
			}
		}
		break;

	case IOR: /* data direction register */
		port->ior = data;
		if (LOG) logerror("Z80PIO '%s' Port %c IOR: %02x\n", device->tag.cstr(), 'A' + index, data);

		/* set interrupt enable */
		port->ie = BIT(port->icw, 7);
		check_interrupts(device);

		/* next word is any */
		port->next_control_word = ANY;
		break;

	case MASK: /* interrupt mask */
		port->mask = data;
		if (LOG) logerror("Z80PIO '%s' Port %c Mask: %02x\n", device->tag.cstr(), 'A' + index, data);
		
		/* set interrupt enable */
		port->ie = BIT(port->icw, 7);
		check_interrupts(device);

		/* next word is any */
		port->next_control_word = ANY;
		break;
	}
}

/*-------------------------------------------------
    z80pio_d_r - data register read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80pio_d_r )
{
	z80pio_t *z80pio = get_safe_token(device);
	int index = offset & 0x01;
	pio_port *port = &z80pio->port[index];

	UINT8 data = 0;

	switch (port->mode)
	{
	case MODE_OUTPUT:
		data = port->output;
		break;

	case MODE_INPUT:
		if (!port->stb)
		{
			/* input port data */
			port->input = devcb_call_read8(&port->in_p_func, 0);
		}

		data = port->input;

		/* clear ready line */
		set_rdy(device, index, 0);

		/* assert ready line */
		set_rdy(device, index, 1);
		break;

	case MODE_BIDIRECTIONAL:
		data = port->input;

		/* clear ready line */
		set_rdy(device, PORT_B, 0);

		/* assert ready line */
		set_rdy(device, PORT_B, 1);
		break;

	case MODE_BIT_CONTROL:
		/* input port data */
		port->input = devcb_call_read8(&port->in_p_func, 0);

		data = (port->input & port->ior) | (port->output & (port->ior ^ 0xff));
		break;
	}

	return data;
}

/*-------------------------------------------------
    z80pio_d_w - data register write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80pio_d_w )
{
	z80pio_t *z80pio = get_safe_token(device);
	int index = offset & 0x01;
	pio_port *port = &z80pio->port[index];

	switch (port->mode)
	{
	case MODE_OUTPUT:
		/* clear ready line */
		set_rdy(device, index, 0);

		/* latch output data */
		port->output = data;

		/* output data to port */
		devcb_call_write8(&port->out_p_func, 0, data);

		/* assert ready line */
		set_rdy(device, index, 1);
		break;

	case MODE_INPUT:
		/* latch output data */
		port->output = data;
		break;

	case MODE_BIDIRECTIONAL:
		/* clear ready line */
		set_rdy(device, index, 0);

		/* latch output data */
		port->output = data;

		if (!port->stb)
		{
			/* output data to port */
			devcb_call_write8(&port->out_p_func, 0, data);
		}

		/* assert ready line */
		set_rdy(device, index, 1);
		break;

	case MODE_BIT_CONTROL:
		/* latch output data */
		port->output = data;

		/* output data to port */
		devcb_call_write8(&port->out_p_func, 0, port->ior | (port->output & (port->ior ^ 0xff)));
		break;
	}
}

/*-------------------------------------------------
    z80pio_cd_ba_r - register read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80pio_cd_ba_r )
{
	int index = BIT(offset, 0);

	return BIT(offset, 1) ? z80pio_c_r(device, index) : z80pio_d_r(device, index);
}

/*-------------------------------------------------
    z80pio_cd_ba_w - register write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80pio_cd_ba_w )
{
	int index = BIT(offset, 0);

	BIT(offset, 1) ? z80pio_c_w(device, index, data) : z80pio_d_w(device, index, data);
}

/*-------------------------------------------------
    z80pio_ba_cd_r - register read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80pio_ba_cd_r )
{
	int index = BIT(offset, 1);

	return BIT(offset, 0) ? z80pio_c_r(device, index) : z80pio_d_r(device, index);
}

/*-------------------------------------------------
    z80pio_ba_cd_w - register write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80pio_ba_cd_w )
{
	int index = BIT(offset, 1);

	BIT(offset, 0) ? z80pio_c_w(device, index, data) : z80pio_d_w(device, index, data);
}

/*-------------------------------------------------
    z80pio_pa_r - port A read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80pio_pa_r )
{
	z80pio_t *z80pio = get_safe_token(device);
	pio_port *port = &z80pio->port[PORT_A];
	
	UINT8 data = 0xff;

	switch (port->mode)
	{
	case MODE_OUTPUT:
		data = port->output;
		break;

	case MODE_BIDIRECTIONAL:
		data = port->output;
		break;

	case MODE_BIT_CONTROL:
		data = port->ior | (port->output & (port->ior ^ 0xff));
		break;
	}

	return data;
}

/*-------------------------------------------------
    z80pio_pa_w - port A write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80pio_pa_w )
{
	z80pio_t *z80pio = get_safe_token(device);
	pio_port *port = &z80pio->port[PORT_A];
	
	if (port->mode == MODE_BIT_CONTROL)
	{
		/* latch data */
		port->input = data;
		check_interrupts(device);
	}
}

/*-------------------------------------------------
    z80pio_pb_r - port B read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80pio_pb_r )
{
	z80pio_t *z80pio = get_safe_token(device);
	pio_port *port = &z80pio->port[PORT_B];
	
	UINT8 data = 0xff;

	switch (port->mode)
	{
	case MODE_OUTPUT:
		data = port->output;
		break;

	case MODE_BIT_CONTROL:
		data = port->ior | (port->output & (port->ior ^ 0xff));
		break;
	}

	return data;
}

/*-------------------------------------------------
    z80pio_pb_w - port B write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80pio_pb_w )
{
	z80pio_t *z80pio = get_safe_token(device);
	pio_port *port = &z80pio->port[PORT_B];
	
	if (port->mode == MODE_BIT_CONTROL)
	{
		/* latch data */
		port->input = data;
		check_interrupts(device);
	}
}

/*-------------------------------------------------
    strobe - handle strobe signal
-------------------------------------------------*/

static void strobe(running_device *device, int index, int state)
{
	z80pio_t *z80pio = get_safe_token(device);
	pio_port *port = &z80pio->port[index];

	if (LOG) logerror("Z80PIO '%s' Port %c Strobe: %u\n", device->tag.cstr(), 'A' + index, state);

	if (z80pio->port[PORT_A].mode == MODE_BIDIRECTIONAL)
	{
		pio_port *port_a = &z80pio->port[PORT_A];

		switch (index)
		{
		case PORT_A:
			if (port_a->rdy) /* port A ready */
			{
				if (port->stb && !state) /* falling edge */
				{
					devcb_call_write8(&port_a->out_p_func, 0, port_a->output);
				}
				else if (!port->stb && state) /* rising edge */
				{
					trigger_interrupt(device, index);
					
					/* clear ready line */
					set_rdy(device, index, 0);
				}
			}
			break;

		case PORT_B:
			if (port->rdy) /* port B ready */
			{
				if (port->stb && !state) /* falling edge */
				{
					port_a->input = devcb_call_read8(&port_a->in_p_func, 0);
				}
				else if (!port->stb && state) /* rising edge */
				{
					trigger_interrupt(device, index);

					/* clear ready line */
					set_rdy(device, index, 0);
				}
			}
			break;
		}
	}
	else
	{
		switch (port->mode)
		{
		case MODE_OUTPUT:
			if (port->rdy)
			{
				if (!port->stb && state) /* rising edge */
				{
					trigger_interrupt(device, index);

					/* clear ready line */
					set_rdy(device, index, 0);
				}
			}
			break;

		case MODE_INPUT:
			if (!state)
			{
				/* input port data */
				port->input = devcb_call_read8(&port->in_p_func, 0);
			}
			else if (!port->stb && state) /* rising edge */
			{
				trigger_interrupt(device, index);

				/* clear ready line */
				set_rdy(device, index, 0);
			}
			break;
		}
	}

	port->stb = state;
}

/*-------------------------------------------------
    z80pio_astb_w - port A strobe
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80pio_astb_w )
{
	strobe(device, PORT_A, state);
}

/*-------------------------------------------------
    z80pio_bstb_w - port B strobe
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80pio_bstb_w )
{
	strobe(device, PORT_B, state);
}

/*-------------------------------------------------
    z80pio_irq_state - read interrupt state
-------------------------------------------------*/

static int z80pio_irq_state(running_device *device)
{
	z80pio_t *z80pio = get_safe_token(device);

	int state = 0;
	
	for (int index = PORT_A; index < PORT_COUNT; index++)
	{
		pio_port *port = &z80pio->port[index];

		if (port->ius)
		{
			/* interrupt under service */
			return Z80_DAISY_IEO;
		}
		else if (port->ie && port->ip)
		{
			/* interrupt pending */
			state = Z80_DAISY_INT;
		}
	}

	return state;
}

/*-------------------------------------------------
    z80pio_irq_ack - interrupt acknowledge
-------------------------------------------------*/

static int z80pio_irq_ack(running_device *device)
{
	z80pio_t *z80pio = get_safe_token(device);

	for (int index = PORT_A; index < PORT_COUNT; index++)
	{
		pio_port *port = &z80pio->port[index];

		if (port->ip)
		{
			if (LOG) logerror("Z80PIO '%s' Interrupt Acknowledge\n", device->tag.cstr());

			/* clear interrupt pending flag */
			port->ip = 0;

			/* set interrupt under service flag */
			port->ius = 1;

			check_interrupts(device);

			return port->vector;
		}
	}

	logerror("z80pio_irq_ack: failed to find an interrupt to ack!\n");

	return 0;
}

/*-------------------------------------------------
    z80pio_irq_reti - return from interrupt
-------------------------------------------------*/

static void z80pio_irq_reti(running_device *device)
{
	z80pio_t *z80pio = get_safe_token(device);

	for (int index = PORT_A; index < PORT_COUNT; index++)
	{
		pio_port *port = &z80pio->port[index];

		if (port->ius)
		{
			if (LOG) logerror("Z80PIO '%s' Return from Interrupt\n", device->tag.cstr());

			/* clear interrupt under service flag */
			port->ius = 0;
			check_interrupts(device);

			return;
		}
	}

	logerror("z80pio_irq_reti: failed to find an interrupt to clear IEO on!\n");
}

/*-------------------------------------------------
    DEVICE_START( z80pio )
-------------------------------------------------*/

static DEVICE_START( z80pio )
{
	z80pio_t *z80pio = get_safe_token(device);
	z80pio_interface *intf = (z80pio_interface *)device->baseconfig().static_config;

	/* resolve callbacks */
	devcb_resolve_write_line(&z80pio->out_int_func, &intf->out_int_func, device);
	devcb_resolve_read8(&z80pio->port[PORT_A].in_p_func, &intf->in_pa_func, device);
	devcb_resolve_write8(&z80pio->port[PORT_A].out_p_func, &intf->out_pa_func, device);
	devcb_resolve_write_line(&z80pio->port[PORT_A].out_rdy_func, &intf->out_ardy_func, device);
	devcb_resolve_read8(&z80pio->port[PORT_B].in_p_func, &intf->in_pb_func, device);
	devcb_resolve_write8(&z80pio->port[PORT_B].out_p_func, &intf->out_pb_func, device);
	devcb_resolve_write_line(&z80pio->port[PORT_B].out_rdy_func, &intf->out_brdy_func, device);

	/* register for state saving */
	for (int index = PORT_A; index < PORT_COUNT; index++)
	{
		state_save_register_device_item(device, index, z80pio->port[index].mode);
		state_save_register_device_item(device, index, z80pio->port[index].next_control_word);
		state_save_register_device_item(device, index, z80pio->port[index].input);
		state_save_register_device_item(device, index, z80pio->port[index].output);
		state_save_register_device_item(device, index, z80pio->port[index].ior);
		state_save_register_device_item(device, index, z80pio->port[index].rdy);
		state_save_register_device_item(device, index, z80pio->port[index].stb);
		state_save_register_device_item(device, index, z80pio->port[index].ie);
		state_save_register_device_item(device, index, z80pio->port[index].ip);
		state_save_register_device_item(device, index, z80pio->port[index].ius);
		state_save_register_device_item(device, index, z80pio->port[index].icw);
		state_save_register_device_item(device, index, z80pio->port[index].vector);
		state_save_register_device_item(device, index, z80pio->port[index].mask);
		state_save_register_device_item(device, index, z80pio->port[index].match);
	}
}

/*-------------------------------------------------
    DEVICE_RESET( z80pio )
-------------------------------------------------*/

static DEVICE_RESET( z80pio )
{
	z80pio_t *z80pio = get_safe_token(device);

	for (int index = PORT_A; index < PORT_COUNT; index++)
	{
		pio_port *port = &z80pio->port[index];

		/* set mode 1 */
		set_mode(device, index, MODE_INPUT);

		/* reset interrupt enable flip-flops */
		port->icw &= ~ICW_ENABLE_INT;
		port->ie = 0;
		port->ip = 0;
		port->ius = 0;
		port->match = 0;

		/* reset all bits of the data I/O register */
		port->ior = 0;

		/* set all bits of the mask control register */
		port->mask = 0xff;

		/* reset output register */
		port->output = 0;

		/* clear ready line */
		set_rdy(device, index, 0);
	}
}

/*-------------------------------------------------
    DEVICE_GET_INFO( z80pio )
-------------------------------------------------*/

DEVICE_GET_INFO( z80pio )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(z80pio_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;							break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(z80pio);break;
		case DEVINFO_FCT_STOP:							/* Nothing */							break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(z80pio);break;
		case DEVINFO_FCT_IRQ_STATE:						info->f = (genf *)z80pio_irq_state;		break;
		case DEVINFO_FCT_IRQ_ACK:						info->f = (genf *)z80pio_irq_ack;		break;
		case DEVINFO_FCT_IRQ_RETI:						info->f = (genf *)z80pio_irq_reti;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Z8420");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Z80");					break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
