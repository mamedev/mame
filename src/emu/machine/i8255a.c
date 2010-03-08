/**********************************************************************

    Intel 8255A Programmable Peripheral Interface emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "i8255a.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG 0

enum
{
	PORT_A = 0,
	PORT_B,
	PORT_C,
	CONTROL
};

enum
{
	GROUP_A = 0,
	GROUP_B
};

enum
{
	MODE_0 = 0,
	MODE_1,
	MODE_2
};

enum
{
	MODE_OUTPUT = 0,
	MODE_INPUT,
};

#define I8255A_CONTROL_PORT_C_LOWER_INPUT	0x01
#define I8255A_CONTROL_PORT_B_INPUT			0x02
#define I8255A_CONTROL_GROUP_B_MODE_1		0x04
#define I8255A_CONTROL_PORT_C_UPPER_INPUT	0x08
#define I8255A_CONTROL_PORT_A_INPUT			0x10
#define I8255A_CONTROL_GROUP_A_MODE_MASK	0x60
#define I8255A_CONTROL_MODE_SET				0x80

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _i8255a_t i8255a_t;
struct _i8255a_t
{
	devcb_resolved_read8		in_port_func[3];
	devcb_resolved_write8		out_port_func[3];

	UINT8 control;				/* mode control word */
	UINT8 output[3];			/* output latch */
	UINT8 input[3];				/* input latch */

	int ibf[2];					/* input buffer full flag */
	int obf[2];					/* output buffer full flag, negative logic */
	int inte[2];				/* interrupt enable */
	int inte1;					/* interrupt enable */
	int inte2;					/* interrupt enable */
	int intr[2];				/* interrupt */
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE i8255a_t *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);

	return (i8255a_t *)device->token;
}

INLINE const i8255a_interface *get_interface(running_device *device)
{
	assert(device != NULL);
	assert((device->type == I8255A));
	return (const i8255a_interface *) device->baseconfig().static_config;
}

INLINE int group_mode(i8255a_t *i8255a, int group)
{
	int mode = 0;

	switch (group)
	{
	case GROUP_A:
		switch ((i8255a->control & I8255A_CONTROL_GROUP_A_MODE_MASK) >> 5)
		{
		case 0: mode = MODE_0; break;
		case 1: mode = MODE_1; break;
		case 2: case 3: mode = MODE_2; break;
		}
		break;

	case GROUP_B:
		mode = i8255a->control & I8255A_CONTROL_GROUP_B_MODE_1 ? MODE_1 : MODE_0;
		break;
	}

	return mode;
}

INLINE int port_mode(i8255a_t *i8255a, int port)
{
	int mode = 0;

	switch (port)
	{
	case PORT_A: mode = i8255a->control & I8255A_CONTROL_PORT_A_INPUT ? MODE_INPUT : MODE_OUTPUT; break;
	case PORT_B: mode = i8255a->control & I8255A_CONTROL_PORT_B_INPUT ? MODE_INPUT : MODE_OUTPUT; break;
	}

	return mode;
}

INLINE int port_c_lower_mode(i8255a_t *i8255a)
{
	return i8255a->control & I8255A_CONTROL_PORT_C_LOWER_INPUT ? MODE_INPUT : MODE_OUTPUT;
}

INLINE int port_c_upper_mode(i8255a_t *i8255a)
{
	return i8255a->control & I8255A_CONTROL_PORT_C_UPPER_INPUT ? MODE_INPUT : MODE_OUTPUT;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static void output_pc(i8255a_t *i8255a)
{
	UINT8 data = 0;
	UINT8 mask = 0;

	/* PC upper */
	switch (group_mode(i8255a, GROUP_A))
	{
	case MODE_0:
		if (port_c_upper_mode(i8255a) == MODE_OUTPUT)
		{
			mask |= 0xf0;
		}
		else
		{
			/* TTL inputs float high */
			data |= 0xf0;
		}
		break;

	case MODE_1:
		data |= i8255a->intr[PORT_A] ? 0x08 : 0x00;

		if (port_mode(i8255a, PORT_A) == MODE_OUTPUT)
		{
			data |= i8255a->obf[PORT_A] ? 0x80 : 0x00;
			mask |= 0x30;
		}
		else
		{
			data |= i8255a->ibf[PORT_A] ? 0x20 : 0x00;
			mask |= 0xc0;
		}
		break;

	case MODE_2:
		data |= i8255a->intr[PORT_A] ? 0x08 : 0x00;
		data |= i8255a->ibf[PORT_A] ? 0x20 : 0x00;
		data |= i8255a->obf[PORT_A] ? 0x80 : 0x00;
		break;
	}

	/* PC lower */
	switch (group_mode(i8255a, GROUP_B))
	{
	case MODE_0:
		if (port_c_lower_mode(i8255a) == MODE_OUTPUT)
		{
			mask |= 0x0f;
		}
		else
		{
			/* TTL inputs float high */
			data |= 0x0f;
		}
		break;

	case MODE_1:
		data |= i8255a->intr[PORT_B] ? 0x01 : 0x00;

		if (port_mode(i8255a, PORT_B) == MODE_OUTPUT)
		{
			data |= i8255a->obf[PORT_B] ? 0x02 : 0x00;
		}
		else
		{
			data |= i8255a->ibf[PORT_B] ? 0x02 : 0x00;
		}
	}

	data |= i8255a->output[PORT_C] & mask;

	devcb_call_write8(&i8255a->out_port_func[PORT_C], 0, data);
}

static void check_interrupt(i8255a_t *i8255a, int port)
{
	switch (group_mode(i8255a, port))
	{
	case MODE_1:
		switch (port_mode(i8255a, port))
		{
		case MODE_INPUT:
			if (i8255a->inte[port] && i8255a->ibf[port])
			{
				if (LOG) logerror("8255A Port %c INTR: 1\n", 'A' + port);

				i8255a->intr[port] = 1;
			}
			break;

		case MODE_OUTPUT:
			if (i8255a->inte[port] && i8255a->obf[port])
			{
				if (LOG) logerror("8255A Port %c INTR: 1\n", 'A' + port);

				i8255a->intr[port] = 1;
			}
			break;
		}
		break;

	case MODE_2:
		if ((i8255a->inte1 && i8255a->obf[port]) || (i8255a->inte2 && i8255a->ibf[port]))
		{
			if (LOG) logerror("8255A Port %c INTR: 1\n", 'A' + port);

			i8255a->intr[port] = 1;
		}
		break;
	}

	output_pc(i8255a);
}

static void set_ibf(i8255a_t *i8255a, int port, int state)
{
	if (LOG) logerror("8255A Port %c IBF: %u\n", 'A' + port, state);

	i8255a->ibf[port] = state;

	check_interrupt(i8255a, port);
}

static void set_obf(i8255a_t *i8255a, int port, int state)
{
	if (LOG) logerror("8255A Port %c OBF: %u\n", 'A' + port, state);

	i8255a->obf[port] = state;

	check_interrupt(i8255a, port);
}

static void set_inte(i8255a_t *i8255a, int port, int state)
{
	if (LOG) logerror("8255A Port %c INTE: %u\n", 'A' + port, state);

	i8255a->inte[port] = state;

	check_interrupt(i8255a, port);
}

static void set_inte1(i8255a_t *i8255a, int state)
{
	if (LOG) logerror("8255A Port A INTE1: %u\n", state);

	i8255a->inte1 = state;

	check_interrupt(i8255a, PORT_A);
}

static void set_inte2(i8255a_t *i8255a, int state)
{
	if (LOG) logerror("8255A Port A INTE2: %u\n", state);

	i8255a->inte2 = state;

	check_interrupt(i8255a, PORT_A);
}

static void set_intr(i8255a_t *i8255a, int port, int state)
{
	if (LOG) logerror("8255A Port %c INTR: %u\n", 'A' + port, state);

	i8255a->intr[port] = state;

	output_pc(i8255a);
}

static UINT8 read_mode0(i8255a_t *i8255a, int port)
{
	UINT8 data = 0;

	if (port_mode(i8255a, port) == MODE_OUTPUT)
	{
		/* read data from output latch */
		data = i8255a->output[port];
	}
	else
	{
		/* read data from port */
		data = devcb_call_read8(&i8255a->in_port_func[port], 0);
	}

	return data;
}

static UINT8 read_mode1(i8255a_t *i8255a, int port)
{
	UINT8 data = 0;

	if (port_mode(i8255a, port) == MODE_OUTPUT)
	{
		/* read data from output latch */
		data = i8255a->output[port];
	}
	else
	{
		/* read data from input latch */
		data = i8255a->input[port];

		/* clear input buffer full flag */
		set_ibf(i8255a, port, 0);

		/* clear interrupt */
		set_intr(i8255a, port, 0);

		/* clear input latch */
		i8255a->input[port] = 0;
	}

	return data;
}

static UINT8 read_mode2(i8255a_t *i8255a)
{
	UINT8 data = 0;

	/* read data from input latch */
	data = i8255a->input[PORT_A];

	/* clear input buffer full flag */
	set_ibf(i8255a, PORT_A, 0);

	/* clear interrupt */
	set_intr(i8255a, PORT_A, 0);

	/* clear input latch */
	i8255a->input[PORT_A] = 0;

	return data;
}

static UINT8 read_pc(i8255a_t *i8255a)
{
	UINT8 data = 0;
	UINT8 mask = 0;

	/* PC upper */
	switch (group_mode(i8255a, GROUP_A))
	{
	case MODE_0:
		if (port_c_upper_mode(i8255a) == MODE_OUTPUT)
		{
			/* read data from output latch */
			data |= i8255a->output[PORT_C] & 0xf0;
		}
		else
		{
			/* read data from port */
			mask |= 0xf0;
		}
		break;

	case MODE_1:
		data |= i8255a->intr[PORT_A] ? 0x08 : 0x00;

		if (port_mode(i8255a, PORT_A) == MODE_OUTPUT)
		{
			data |= i8255a->obf[PORT_A] ? 0x80 : 0x00;
			data |= i8255a->inte[PORT_A] ? 0x40 : 0x00;
			mask |= 0x30;
		}
		else
		{
			data |= i8255a->ibf[PORT_A] ? 0x20 : 0x00;
			data |= i8255a->inte[PORT_A] ? 0x10 : 0x00;
			mask |= 0xc0;
		}
		break;

	case MODE_2:
		data |= i8255a->intr[PORT_A] ? 0x08 : 0x00;
		data |= i8255a->inte2 ? 0x10 : 0x00;
		data |= i8255a->ibf[PORT_A] ? 0x20 : 0x00;
		data |= i8255a->inte1 ? 0x40 : 0x00;
		data |= i8255a->obf[PORT_A] ? 0x80 : 0x00;
		break;
	}

	/* PC lower */
	switch (group_mode(i8255a, GROUP_B))
	{
	case MODE_0:
		if (port_c_lower_mode(i8255a) == MODE_OUTPUT)
		{
			/* read data from output latch */
			data |= i8255a->output[PORT_C] & 0x0f;
		}
		else
		{
			/* read data from port */
			mask |= 0x0f;
		}
		break;

	case MODE_1:
		data |= i8255a->inte[PORT_B] ? 0x04 : 0x00;
		data |= i8255a->intr[PORT_B] ? 0x01 : 0x00;

		if (port_mode(i8255a, PORT_B) == MODE_OUTPUT)
		{
			data |= i8255a->obf[PORT_B] ? 0x02 : 0x00;
		}
		else
		{
			data |= i8255a->ibf[PORT_B] ? 0x02 : 0x00;
		}
	}

	if (mask)
	{
		/* read data from port */
		data |= devcb_call_read8(&i8255a->in_port_func[PORT_C], 0) & mask;
	}

	return data;
}

READ8_DEVICE_HANDLER( i8255a_r )
{
	i8255a_t *i8255a = get_safe_token(device);

	UINT8 data = 0;

	switch (offset & 0x03)
	{
	case PORT_A:
		switch (group_mode(i8255a, GROUP_A))
		{
		case MODE_0: data = read_mode0(i8255a, PORT_A); break;
		case MODE_1: data = read_mode1(i8255a, PORT_A); break;
		case MODE_2: data = read_mode2(i8255a); break;
		}
		if (LOG) logerror("8255A '%s' Port A Read: %02x\n", device->tag(), data);
		break;

	case PORT_B:
		switch (group_mode(i8255a, GROUP_B))
		{
		case MODE_0: data = read_mode0(i8255a, PORT_B); break;
		case MODE_1: data = read_mode1(i8255a, PORT_B); break;
		}
		if (LOG) logerror("8255A '%s' Port B Read: %02x\n", device->tag(), data);
		break;

	case PORT_C:
		data = read_pc(i8255a);
		//if (LOG) logerror("8255A '%s' Port C Read: %02x\n", device->tag(), data);
		break;

	case CONTROL:
		data = i8255a->control;
		if (LOG) logerror("8255A '%s' Mode Control Word Read: %02x\n", device->tag(), data);
		break;
	}

	return data;
}

static void write_mode0(i8255a_t *i8255a, int port, UINT8 data)
{
	if (port_mode(i8255a, port) == MODE_OUTPUT)
	{
		/* latch output data */
		i8255a->output[port] = data;

		/* write data to port */
		devcb_call_write8(&i8255a->out_port_func[port], 0, data);
	}
}

static void write_mode1(i8255a_t *i8255a, int port, UINT8 data)
{
	if (port_mode(i8255a, port) == MODE_OUTPUT)
	{
		/* latch output data */
		i8255a->output[port] = data;

		/* write data to port */
		devcb_call_write8(&i8255a->out_port_func[port], 0, data);

		/* set output buffer full flag */
		set_obf(i8255a, port, 0);

		/* clear interrupt */
		set_intr(i8255a, port, 0);
	}
}

static void write_mode2(i8255a_t *i8255a, UINT8 data)
{
	/* latch output data */
	i8255a->output[PORT_A] = data;

	/* write data to port */
	devcb_call_write8(&i8255a->out_port_func[PORT_A], 0, data);

	/* set output buffer full flag */
	set_obf(i8255a, PORT_A, 0);

	/* clear interrupt */
	set_intr(i8255a, PORT_A, 0);
}

static void write_pc(i8255a_t *i8255a, UINT8 data)
{
	int changed = 0;

	if (group_mode(i8255a, GROUP_A) == MODE_0)
	{
		/* PC upper */
		if (port_c_upper_mode(i8255a) == MODE_OUTPUT)
		{
			i8255a->output[PORT_C] = (data & 0xf0) | (i8255a->output[PORT_C] & 0x0f);
			changed = 1;
		}

		/* PC lower */
		if (port_c_lower_mode(i8255a) == MODE_OUTPUT)
		{
			i8255a->output[PORT_C] = (i8255a->output[PORT_C] & 0xf0) | (data & 0x0f);
			changed = 1;
		}
	}

	if (changed)
	{
		output_pc(i8255a);
	}
}

static void set_mode(running_device *device, UINT8 data)
{
	i8255a_t *i8255a = get_safe_token(device);

	i8255a->control = data;

	/* group A */
	i8255a->output[PORT_A] = 0;
	i8255a->input[PORT_A] = 0;
	i8255a->ibf[PORT_A] = 0;
	i8255a->obf[PORT_A] = 1;
	i8255a->inte[PORT_A] = 0;
	i8255a->inte1 = 0;
	i8255a->inte2 = 0;

	if (port_mode(i8255a, PORT_A) == MODE_OUTPUT)
	{
		devcb_call_write8(&i8255a->out_port_func[PORT_A], 0, i8255a->output[PORT_A]);
	}
	else
	{
		/* TTL inputs float high */
		devcb_call_write8(&i8255a->out_port_func[PORT_A], 0, 0xff);
	}

	if (LOG) logerror("8255A '%s' Group A Mode: %u\n", device->tag(), group_mode(i8255a, GROUP_A));
	if (LOG) logerror("8255A '%s' Port A Mode: %s\n", device->tag(), (port_mode(i8255a, PORT_A) == MODE_OUTPUT) ? "output" : "input");
	if (LOG) logerror("8255A '%s' Port C Upper Mode: %s\n", device->tag(), (port_c_upper_mode(i8255a) == MODE_OUTPUT) ? "output" : "input");
	if (LOG) logerror("8255A '%s' Group B Mode: %u\n", device->tag(), group_mode(i8255a, GROUP_B));
	if (LOG) logerror("8255A '%s' Port B Mode: %s\n", device->tag(), (port_mode(i8255a, PORT_B) == MODE_OUTPUT) ? "output" : "input");
	if (LOG) logerror("8255A '%s' Port C Lower Mode: %s\n", device->tag(), (port_c_lower_mode(i8255a) == MODE_OUTPUT) ? "output" : "input");

	/* group B */
	i8255a->output[PORT_B] = 0;
	i8255a->input[PORT_B] = 0;
	i8255a->ibf[PORT_B] = 0;
	i8255a->obf[PORT_B] = 1;
	i8255a->inte[PORT_B] = 0;

	if (port_mode(i8255a, PORT_B) == MODE_OUTPUT)
	{
		devcb_call_write8(&i8255a->out_port_func[PORT_B], 0, i8255a->output[PORT_B]);
	}
	else
	{
		/* TTL inputs float high */
		devcb_call_write8(&i8255a->out_port_func[PORT_B], 0, 0xff);
	}

	i8255a->output[PORT_C] = 0;
	i8255a->input[PORT_C] = 0;

	output_pc(i8255a);
}

static void set_pc_bit(running_device *device, int bit, int state)
{
	i8255a_t *i8255a = get_safe_token(device);

	/* set output latch bit */
	i8255a->output[PORT_C] &= ~(1 << bit);
	i8255a->output[PORT_C] |= state << bit;

	switch (group_mode(i8255a, GROUP_A))
	{
	case MODE_1:
		if (port_mode(i8255a, PORT_A) == MODE_OUTPUT)
		{
			switch (bit)
			{
			case 3: set_intr(i8255a, PORT_A, state); break;
			case 6: set_inte(i8255a, PORT_A, state); break;
			case 7: set_obf(i8255a, PORT_A, state); break;
			}
		}
		else
		{
			switch (bit)
			{
			case 3: set_intr(i8255a, PORT_A, state); break;
			case 4: set_inte(i8255a, PORT_A, state); break;
			case 5: set_ibf(i8255a, PORT_A, state); break;
			}
		}
		break;

	case MODE_2:
		switch (bit)
		{
		case 3: set_intr(i8255a, PORT_A, state); break;
		case 4: set_inte2(i8255a, state); break;
		case 5: set_ibf(i8255a, PORT_A, state); break;
		case 6: set_inte1(i8255a, state); break;
		case 7: set_obf(i8255a, PORT_A, state); break;
		}
		break;
	}

	if (group_mode(i8255a, GROUP_B) == MODE_1)
	{
		switch (bit)
		{
		case 0: set_intr(i8255a, PORT_B, state); break;
		case 1:
			if (port_mode(i8255a, PORT_B) == MODE_OUTPUT)
				set_obf(i8255a, PORT_B, state);
			else
				set_ibf(i8255a, PORT_B, state);
			break;
		case 2: set_inte(i8255a, PORT_B, state); break;
		}
	}

	output_pc(i8255a);
}

WRITE8_DEVICE_HANDLER( i8255a_w )
{
	i8255a_t *i8255a = get_safe_token(device);

	switch (offset & 0x03)
	{
	case PORT_A:
		if (LOG) logerror("8255A '%s' Port A Write: %02x\n", device->tag(), data);

		switch (group_mode(i8255a, GROUP_A))
		{
		case MODE_0: write_mode0(i8255a, PORT_A, data); break;
		case MODE_1: write_mode1(i8255a, PORT_A, data); break;
		case MODE_2: write_mode2(i8255a, data); break;
		}
		break;

	case PORT_B:
		if (LOG) logerror("8255A '%s' Port B Write: %02x\n", device->tag(), data);

		switch (group_mode(i8255a, GROUP_B))
		{
		case MODE_0: write_mode0(i8255a, PORT_B, data); break;
		case MODE_1: write_mode1(i8255a, PORT_B, data); break;
		}
		break;

	case PORT_C:
		if (LOG) logerror("8255A '%s' Port C Write: %02x\n", device->tag(), data);

		write_pc(i8255a, data);
		break;

	case CONTROL:
		if (data & I8255A_CONTROL_MODE_SET)
		{
			if (LOG) logerror("8255A '%s' Mode Control Word: %02x\n", device->tag(), data);

			set_mode(device, data);
		}
		else
		{
			int bit = (data >> 1) & 0x07;
			int state = BIT(data, 0);

			if (LOG) logerror("8255A '%s' %s Bit %u\n", device->tag(), state ? "Set" : "Reset", bit);

			set_pc_bit(device, bit, state);
		}
		break;
	}
}

READ8_DEVICE_HANDLER( i8255a_pa_r )
{
	i8255a_t *i8255a = get_safe_token(device);

	UINT8 data = 0xff;

	if (port_mode(i8255a, PORT_A) == MODE_OUTPUT)
	{
		data = i8255a->output[PORT_A];
	}

	return data;
}

READ8_DEVICE_HANDLER( i8255a_pb_r )
{
	i8255a_t *i8255a = get_safe_token(device);

	UINT8 data = 0xff;

	if (port_mode(i8255a, PORT_B) == MODE_OUTPUT)
	{
		data = i8255a->output[PORT_B];
	}

	return data;
}

WRITE_LINE_DEVICE_HANDLER( i8255a_pc2_w )
{
	i8255a_t *i8255a = get_safe_token(device);

	if (group_mode(i8255a, GROUP_B) == 1)
	{
		if (port_mode(i8255a, PORT_B) == MODE_OUTPUT)
		{
			/* port B acknowledge */
			if (!i8255a->obf[PORT_B] && !state)
			{
				if (LOG) logerror("8255A '%s' Port B Acknowledge\n", device->tag());

				/* clear output buffer flag */
				set_obf(i8255a, PORT_B, 1);
			}
		}
		else
		{
			/* port B strobe */
			if (!i8255a->ibf[PORT_B] && !state)
			{
				if (LOG) logerror("8255A '%s' Port B Strobe\n", device->tag());

				/* read port into latch */
				i8255a->input[PORT_B] = devcb_call_read8(&i8255a->in_port_func[PORT_B], 0);

				/* set input buffer flag */
				set_ibf(i8255a, PORT_B, 1);
			}
		}
	}
}

WRITE_LINE_DEVICE_HANDLER( i8255a_pc4_w )
{
	i8255a_t *i8255a = get_safe_token(device);

	if ((group_mode(i8255a, GROUP_A) == 2) || ((group_mode(i8255a, GROUP_A) == 1) && (port_mode(i8255a, PORT_A) == MODE_INPUT)))
	{
		/* port A strobe */
		if (!i8255a->ibf[PORT_A] && !state)
		{
			if (LOG) logerror("8255A '%s' Port A Strobe\n", device->tag());

			/* read port into latch */
			i8255a->input[PORT_A] = devcb_call_read8(&i8255a->in_port_func[PORT_A], 0);

			/* set input buffer flag */
			set_ibf(i8255a, PORT_A, 1);
		}
	}
}

WRITE_LINE_DEVICE_HANDLER( i8255a_pc6_w )
{
	i8255a_t *i8255a = get_safe_token(device);

	if ((group_mode(i8255a, GROUP_A) == 2) || ((group_mode(i8255a, GROUP_A) == 1) && (port_mode(i8255a, PORT_A) == MODE_OUTPUT)))
	{
		/* port A acknowledge */
		if (!i8255a->obf[PORT_A] && !state)
		{
			if (LOG) logerror("8255A '%s' Port A Acknowledge\n", device->tag());

			/* clear output buffer flag */
			set_obf(i8255a, PORT_A, 1);
		}
	}
}

/*-------------------------------------------------
    DEVICE_START( i8255a )
-------------------------------------------------*/

static DEVICE_START( i8255a )
{
	i8255a_t *i8255a = get_safe_token(device);
	const i8255a_interface *intf = get_interface(device);

	/* resolve callbacks */
	devcb_resolve_read8(&i8255a->in_port_func[PORT_A], &intf->in_pa_func, device);
	devcb_resolve_read8(&i8255a->in_port_func[PORT_B], &intf->in_pb_func, device);
	devcb_resolve_read8(&i8255a->in_port_func[PORT_C], &intf->in_pc_func, device);
	devcb_resolve_write8(&i8255a->out_port_func[PORT_A], &intf->out_pa_func, device);
	devcb_resolve_write8(&i8255a->out_port_func[PORT_B], &intf->out_pb_func, device);
	devcb_resolve_write8(&i8255a->out_port_func[PORT_C], &intf->out_pc_func, device);

	/* register for state saving */
	state_save_register_device_item(device, 0, i8255a->control);
	state_save_register_device_item_array(device, 0, i8255a->output);
	state_save_register_device_item_array(device, 0, i8255a->input);
	state_save_register_device_item_array(device, 0, i8255a->ibf);
	state_save_register_device_item_array(device, 0, i8255a->obf);
	state_save_register_device_item_array(device, 0, i8255a->inte);
	state_save_register_device_item(device, 0, i8255a->inte1);
	state_save_register_device_item(device, 0, i8255a->inte2);
	state_save_register_device_item_array(device, 0, i8255a->intr);
}

/*-------------------------------------------------
    DEVICE_RESET( i8255a )
-------------------------------------------------*/

static DEVICE_RESET( i8255a )
{
	i8255a_w(device, CONTROL, 0x9b);
}

/*-------------------------------------------------
    DEVICE_GET_INFO( i8255a )
-------------------------------------------------*/

DEVICE_GET_INFO( i8255a )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(i8255a_t);					break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;								break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(i8255a);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(i8255a);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Intel 8255A");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Intel 8080");				break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright MESS Team");		break;
	}
}
