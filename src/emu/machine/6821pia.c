/**********************************************************************

    Motorola 6821 PIA interface and emulation

**********************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "6821pia.h"


#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)



/*************************************
 *
 *  Internal PIA data structure
 *
 *************************************/

typedef struct _pia6821 pia6821;
struct _pia6821
{
	const pia6821_interface *intf;

	UINT8 in_a;
	UINT8 in_ca1;
	UINT8 in_ca2;
	UINT8 out_a;
	UINT8 out_ca2;
	UINT8 port_a_z_mask;
	UINT8 ddr_a;
	UINT8 ctl_a;
	UINT8 irq_a1;
	UINT8 irq_a2;
	UINT8 irq_a_state;

	UINT8 in_b;
	UINT8 in_cb1;
	UINT8 in_cb2;
	UINT8 out_b;
	UINT8 out_cb2;
	UINT8 last_out_cb2_z;
	UINT8 ddr_b;
	UINT8 ctl_b;
	UINT8 irq_b1;
	UINT8 irq_b2;
	UINT8 irq_b_state;

	/* variables that indicate if access a line externally -
       used to for logging purposes ONLY */
	UINT8 in_a_pushed;
	UINT8 out_a_needs_pulled;
	UINT8 in_ca1_pushed;
	UINT8 in_ca2_pushed;
	UINT8 out_ca2_needs_pulled;
	UINT8 in_b_pushed;
	UINT8 out_b_needs_pulled;
	UINT8 in_cb1_pushed;
	UINT8 in_cb2_pushed;
	UINT8 out_cb2_needs_pulled;
	UINT8 logged_port_a_not_connected;
	UINT8 logged_port_b_not_connected;
	UINT8 logged_ca1_not_connected;
	UINT8 logged_ca2_not_connected;
	UINT8 logged_cb1_not_connected;
	UINT8 logged_cb2_not_connected;
};



/*************************************
 *
 *  Convenince macros and defines
 *
 *************************************/

#define PIA_IRQ1				(0x80)
#define PIA_IRQ2				(0x40)

#define IRQ1_ENABLED(c)			( (((c) >> 0) & 0x01))
#define C1_LOW_TO_HIGH(c)		( (((c) >> 1) & 0x01))
#define C1_HIGH_TO_LOW(c)		(!(((c) >> 1) & 0x01))
#define OUTPUT_SELECTED(c)		( (((c) >> 2) & 0x01))
#define IRQ2_ENABLED(c)			( (((c) >> 3) & 0x01))
#define STROBE_E_RESET(c)		( (((c) >> 3) & 0x01))
#define STROBE_C1_RESET(c)		(!(((c) >> 3) & 0x01))
#define C2_SET(c)				( (((c) >> 3) & 0x01))
#define C2_LOW_TO_HIGH(c)		( (((c) >> 4) & 0x01))
#define C2_HIGH_TO_LOW(c)		(!(((c) >> 4) & 0x01))
#define C2_SET_MODE(c)			( (((c) >> 4) & 0x01))
#define C2_STROBE_MODE(c)		(!(((c) >> 4) & 0x01))
#define C2_OUTPUT(c)			( (((c) >> 5) & 0x01))
#define C2_INPUT(c)				(!(((c) >> 5) & 0x01))



/*************************************
 *
 *  Static variables
 *
 *************************************/

static pia6821 pias[MAX_PIA];



/*************************************
 *
 *  Configuration
 *
 *************************************/

void pia_config(int which, const pia6821_interface *intf)
{
	pia6821 *p;

	assert_always(mame_get_phase(Machine) == MAME_PHASE_INIT, "Can only call pia_config at init time!");
	assert_always((which >= 0) && (which < MAX_PIA), "pia_config called on an invalid PIA!");
	assert_always(intf, "pia_config called with an invalid interface!");

	p = &pias[which];
	memset(p, 0, sizeof(pias[0]));

	p->intf = intf;

	state_save_register_item(Machine, "6821pia", NULL, which, p->in_a);
	state_save_register_item(Machine, "6821pia", NULL, which, p->in_ca1);
	state_save_register_item(Machine, "6821pia", NULL, which, p->in_ca2);
	state_save_register_item(Machine, "6821pia", NULL, which, p->out_a);
	state_save_register_item(Machine, "6821pia", NULL, which, p->out_ca2);
	state_save_register_item(Machine, "6821pia", NULL, which, p->port_a_z_mask);
	state_save_register_item(Machine, "6821pia", NULL, which, p->ddr_a);
	state_save_register_item(Machine, "6821pia", NULL, which, p->ctl_a);
	state_save_register_item(Machine, "6821pia", NULL, which, p->irq_a1);
	state_save_register_item(Machine, "6821pia", NULL, which, p->irq_a2);
	state_save_register_item(Machine, "6821pia", NULL, which, p->irq_a_state);
	state_save_register_item(Machine, "6821pia", NULL, which, p->in_b);
	state_save_register_item(Machine, "6821pia", NULL, which, p->in_cb1);
	state_save_register_item(Machine, "6821pia", NULL, which, p->in_cb2);
	state_save_register_item(Machine, "6821pia", NULL, which, p->out_b);
	state_save_register_item(Machine, "6821pia", NULL, which, p->out_cb2);
	state_save_register_item(Machine, "6821pia", NULL, which, p->last_out_cb2_z);
	state_save_register_item(Machine, "6821pia", NULL, which, p->ddr_b);
	state_save_register_item(Machine, "6821pia", NULL, which, p->ctl_b);
	state_save_register_item(Machine, "6821pia", NULL, which, p->irq_b1);
	state_save_register_item(Machine, "6821pia", NULL, which, p->irq_b2);
	state_save_register_item(Machine, "6821pia", NULL, which, p->irq_b_state);
	state_save_register_item(Machine, "6821pia", NULL, which, p->in_a_pushed);
	state_save_register_item(Machine, "6821pia", NULL, which, p->out_a_needs_pulled);
	state_save_register_item(Machine, "6821pia", NULL, which, p->in_ca1_pushed);
	state_save_register_item(Machine, "6821pia", NULL, which, p->in_ca2_pushed);
	state_save_register_item(Machine, "6821pia", NULL, which, p->out_ca2_needs_pulled);
	state_save_register_item(Machine, "6821pia", NULL, which, p->in_b_pushed);
	state_save_register_item(Machine, "6821pia", NULL, which, p->out_b_needs_pulled);
	state_save_register_item(Machine, "6821pia", NULL, which, p->in_cb1_pushed);
	state_save_register_item(Machine, "6821pia", NULL, which, p->in_cb2_pushed);
	state_save_register_item(Machine, "6821pia", NULL, which, p->out_cb2_needs_pulled);
}



/*************************************
 *
 *  Reset
 *
 *************************************/

void pia_reset(void)
{
	int i;

	/* zap each structure, preserving the interface and swizzle */
	for (i = 0; i < MAX_PIA; i++)
	{
		const pia6821_interface *intf = pias[i].intf;

		if (intf == NULL)  continue;

		memset(&pias[i], 0, sizeof(pias[i]));

		pias[i].intf = intf;

		/*
         * set default read values.
         *
         * ports A,CA1,CA2 default to 1
         * ports B,CB1,CB2 are three-state and undefined (set to 0)
         */
		pias[i].in_a = 0xff;
		pias[i].in_ca1 = TRUE;
		pias[i].in_ca2 = TRUE;

		/* clear the IRQs */
		if (intf->irq_a_func) (*intf->irq_a_func)(Machine, FALSE);
		if (intf->irq_b_func) (*intf->irq_b_func)(Machine, FALSE);
	}
}



/*************************************
 *
 *  External interrupt check
 *
 *************************************/

static void update_interrupts(running_machine *machine, pia6821 *p)
{
	int new_state;

	/* start with IRQ A */
	new_state = (p->irq_a1 && IRQ1_ENABLED(p->ctl_a)) || (p->irq_a2 && IRQ2_ENABLED(p->ctl_a));

	if (new_state != p->irq_a_state)
	{
		p->irq_a_state = new_state;

		if (p->intf->irq_a_func) (p->intf->irq_a_func)(machine, p->irq_a_state);
	}

	/* then do IRQ B */
	new_state = (p->irq_b1 && IRQ1_ENABLED(p->ctl_b)) || (p->irq_b2 && IRQ2_ENABLED(p->ctl_b));

	if (new_state != p->irq_b_state)
	{
		p->irq_b_state = new_state;

		if (p->intf->irq_b_func) (p->intf->irq_b_func)(machine, p->irq_b_state);
	}
}



/*************************************
 *
 *  Port A/B input pins
 *
 *************************************/

static UINT8 get_in_a_value(running_machine *machine, int which)
{
	/* temporary hack until this is converted to a device */
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	pia6821 *p = &pias[which];
	UINT8 port_a_data = 0;
	UINT8 ret;

	/* update the input */
	if (p->intf->in_a_func)
		port_a_data = p->intf->in_a_func(space, 0);
	else
	{
		if (p->in_a_pushed)
			port_a_data = p->in_a;
		else
		{
			/* mark all pins disconnected */
			p->port_a_z_mask = 0xff;

			if (!p->logged_port_a_not_connected && (p->ddr_a != 0xff))
			{
				logerror("PIA #%d: Warning! No port A read handler. Assuming pins 0x%02X not connected\n", which, p->ddr_a ^ 0xff);
				p->logged_port_a_not_connected = TRUE;
			}
		}
	}

	/* - connected pins are always read
       - disconnected pins read the output buffer in output mode
       - disconnected pins are HI in input mode */
	ret = (~p->port_a_z_mask             & port_a_data) |
	      ( p->port_a_z_mask &  p->ddr_a & p->out_a) |
	      ( p->port_a_z_mask & ~p->ddr_a);

	return ret;
}


static UINT8 get_in_b_value(running_machine *machine, int which)
{
	/* temporary hack until this is converted to a device */
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	pia6821 *p = &pias[which];
	UINT8 ret;

	if (p->ddr_b == 0xff)
		/* all output, just return buffer */
		ret = p->out_b;
	else
	{
		UINT8 port_b_data;

		/* update the input */
		if (p->intf->in_b_func)
			port_b_data = p->intf->in_b_func(space, 0);
		else
		{
			if (p->in_b_pushed)
				port_b_data = p->in_b;
			else
			{
				if (!p->logged_port_b_not_connected && (p->ddr_b != 0xff))
				{
					logerror("PIA #%d: Error! No port B read handler. Three-state pins 0x%02X are undefined\n", which, p->ddr_b ^ 0xff);
					p->logged_port_b_not_connected = TRUE;
				}

				/* undefined -- need to return something */
				port_b_data = 0x00;
			}
		}

		/* the DDR determines if the pin or the output buffer is read */
		ret = (p->out_b & p->ddr_b) | (port_b_data & ~p->ddr_b);
	}

	return ret;
}



/*************************************
 *
 *  Port A/B output pins
 *
 *************************************/

static UINT8 get_out_a_value(running_machine *machine, int which)
{
	UINT8 ret;
	pia6821 *p = &pias[which];

	if (p->ddr_a == 0xff)
		/* all output */
		ret = p->out_a;
	else
		/* input pins don't change */
		ret = (p->out_a & p->ddr_a) | (get_in_a_value(machine, which) & ~p->ddr_a);

	return ret;
}


static UINT8 get_out_b_value(int which)
{
	pia6821 *p = &pias[which];

	/* input pins are high-impedance - we just send them as zeros for backwards compatibility */
	return p->out_b & p->ddr_b;
}



/*************************************
 *
 *  Sets C2 state value and call
 *  callbacks if changed
 *
 *************************************/

static void set_out_ca2(running_machine *machine, int which, int data)
{
	/* temporary hack until this is converted to a device */
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	pia6821 *p = &pias[which];

	if (data != p->out_ca2)
	{
		p->out_ca2 = data;

		/* send to output function */
		if (p->intf->out_ca2_func)
			p->intf->out_ca2_func(space, 0, p->out_ca2);
		else
		{
			if (p->out_ca2_needs_pulled)
				logerror("PIA #%d: Warning! No port CA2 write handler. Previous value has been lost!\n", which);

			p->out_ca2_needs_pulled = TRUE;
		}
	}
}


static void set_out_cb2(running_machine *machine, int which, int data)
{
	/* temporary hack until this is converted to a device */
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	pia6821 *p = &pias[which];

	int z = pia_get_output_cb2_z(which);

	if ((data != p->out_cb2) || (z != p->last_out_cb2_z))
	{
		p->out_cb2 = data;
		p->last_out_cb2_z = z;

		/* send to output function */
		if (p->intf->out_cb2_func)
			p->intf->out_cb2_func(space, 0, p->out_cb2);
		else
		{
			if (p->out_cb2_needs_pulled)
				logerror("PIA #%d: Warning! No port CB2 write handler. Previous value has been lost!\n", which);

			p->out_cb2_needs_pulled = TRUE;
		}
	}
}



/*************************************
 *
 *  CPU interface for reading from the PIA
 *
 *************************************/

static UINT8 port_a_r(running_machine *machine, int which)
{
	pia6821 *p = &pias[which];

	UINT8 ret = get_in_a_value(machine, which);

	/* IRQ flags implicitly cleared by a read */
	p->irq_a1 = FALSE;
	p->irq_a2 = FALSE;
	update_interrupts(machine, p);

	/* CA2 is configured as output and in read strobe mode */
	if (C2_OUTPUT(p->ctl_a) && C2_STROBE_MODE(p->ctl_a))
	{
		/* this will cause a transition low */
		set_out_ca2(machine, which, FALSE);

		/* if the CA2 strobe is cleared by the E, reset it right away */
		if (STROBE_E_RESET(p->ctl_a))
			set_out_ca2(machine, which, TRUE);
	}

	LOG(("PIA #%d: port A read = %02X\n", which, ret));

	return ret;
}


static UINT8 ddr_a_r(running_machine *machine, int which)
{
	pia6821 *p = &pias[which];

	UINT8 ret = p->ddr_a;

	LOG(("PIA #%d: DDR A read = %02X\n", which, ret));

	return ret;
}


static UINT8 port_b_r(running_machine *machine, int which)
{
	pia6821 *p = &pias[which];

	UINT8 ret = get_in_b_value(machine, which);

	/* This read will implicitly clear the IRQ B1 flag.  If CB2 is in write-strobe
       mode with CB1 restore, and a CB1 active transition set the flag,
       clearing it will cause CB2 to go high again.  Note that this is different
       from what happens with port A. */
	if (p->irq_b1 && C2_STROBE_MODE(p->ctl_b) && STROBE_C1_RESET(p->ctl_b))
		set_out_cb2(machine, which, TRUE);

	/* IRQ flags implicitly cleared by a read */
	p->irq_b1 = FALSE;
	p->irq_b2 = FALSE;
	update_interrupts(machine, p);

	LOG(("PIA #%d: port B read = %02X\n", which, ret));

	return ret;
}


static UINT8 ddr_b_r(running_machine *machine, int which)
{
	pia6821 *p = &pias[which];

	UINT8 ret = p->ddr_b;

	LOG(("PIA #%d: DDR B read = %02X\n", which, ret));

	return ret;
}


static UINT8 control_a_r(running_machine *machine, int which)
{
	/* temporary hack until this is converted to a device */
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	pia6821 *p = &pias[which];
	UINT8 ret;

	/* update CA1 & CA2 if callback exists, these in turn may update IRQ's */
	if (p->intf->in_ca1_func)
		pia_set_input_ca1(which, p->intf->in_ca1_func(space, 0));
	else if (!p->logged_ca1_not_connected && (!p->in_ca1_pushed))
	{
		logerror("PIA #%d: Warning! No CA1 read handler. Assuming pin not connected\n", which);
		p->logged_ca1_not_connected = TRUE;
	}

	if (p->intf->in_ca2_func)
		pia_set_input_ca2(which, p->intf->in_ca2_func(space, 0));
	else if ( !p->logged_ca2_not_connected && C2_INPUT(p->ctl_a) && !p->in_ca2_pushed)
	{
		logerror("PIA #%d: Warning! No CA2 read handler. Assuming pin not connected\n", which);
		p->logged_ca2_not_connected = TRUE;
	}

	/* read control register */
	ret = p->ctl_a;

	/* set the IRQ flags if we have pending IRQs */
	if (p->irq_a1)
		ret |= PIA_IRQ1;

	if (p->irq_a2 && C2_INPUT(p->ctl_a))
		ret |= PIA_IRQ2;

	LOG(("PIA #%d: control A read = %02X\n", which, ret));

	return ret;
}


static UINT8 control_b_r(running_machine *machine, int which)
{
	/* temporary hack until this is converted to a device */
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	pia6821 *p = &pias[which];
	UINT8 ret;

	/* update CB1 & CB2 if callback exists, these in turn may update IRQ's */
	if (p->intf->in_cb1_func)
		pia_set_input_cb1(which, p->intf->in_cb1_func(space, 0));
	else if (!p->logged_cb1_not_connected && !p->in_cb1_pushed)
	{
		logerror("PIA #%d: Error! no CB1 read handler. Three-state pin is undefined\n", which);
		p->logged_cb1_not_connected = TRUE;
	}

	if (p->intf->in_cb2_func)
		pia_set_input_cb2(which, p->intf->in_cb2_func(space, 0));
	else if (!p->logged_cb2_not_connected && C2_INPUT(p->ctl_b) && !p->in_cb2_pushed)
	{
		logerror("PIA #%d: Error! No CB2 read handler. Three-state pin is undefined\n", which);
		p->logged_cb2_not_connected = TRUE;
	}

	/* read control register */
	ret = p->ctl_b;

	/* set the IRQ flags if we have pending IRQs */
	if (p->irq_b1)
		ret |= PIA_IRQ1;

	if (p->irq_b2 && C2_INPUT(p->ctl_b))
		ret |= PIA_IRQ2;

	LOG(("PIA #%d: control B read = %02X\n", which, ret));

	return ret;
}


UINT8 pia_read(int which, offs_t offset)
{
	pia6821 *p = &pias[which];
	UINT8 ret;

	switch (offset & 0x03)
	{
		default: /* impossible */
		case 0x00:
			if (OUTPUT_SELECTED(p->ctl_a))
				ret = port_a_r(Machine, which);
			else
				ret = ddr_a_r(Machine, which);
			break;

		case 0x01:
			ret = control_a_r(Machine, which);
			break;

		case 0x02:
			if (OUTPUT_SELECTED(p->ctl_b))
				ret = port_b_r(Machine, which);
			else
				ret = ddr_b_r(Machine, which);
			break;

		case 0x03:
			ret = control_b_r(Machine, which);
			break;
	}

	return ret;
}


UINT8 pia_alt_read(int which, offs_t offset)
{
	return pia_read(which, ((offset << 1) & 0x02) | ((offset >> 1) & 0x01));
}


UINT8 pia_get_port_b_z_mask(int which)
{
	pia6821 *p = &pias[which];

	return ~p->ddr_b;
}



/*************************************
 *
 *  CPU interface for writing to the PIA
 *
 *************************************/

static void send_to_out_a_func(running_machine *machine, int which, const char* message)
{
	/* temporary hack until this is converted to a device */
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	pia6821 *p = &pias[which];

	/* input pins are pulled high */
	UINT8 data = get_out_a_value(machine, which);

	LOG(("PIA #%d: %s = %02X\n", which, message, data));

	if (p->intf->out_a_func)
		p->intf->out_a_func(space, 0, data);
	else
	{
		if (p->out_a_needs_pulled)
			logerror("PIA #%d: Warning! No port A write handler. Previous value has been lost!\n", which);

		p->out_a_needs_pulled = TRUE;
	}
}


static void send_to_out_b_func(running_machine *machine, int which, const char* message)
{
	/* temporary hack until this is converted to a device */
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	pia6821 *p = &pias[which];

	/* input pins are high-impedance - we just send them as zeros for backwards compatibility */
	UINT8 data = get_out_b_value(which);

	LOG(("PIA #%d: %s = %02X\n", which, message, data));

	if (p->intf->out_b_func)
		p->intf->out_b_func(space, 0, data);
	else
	{
		if (p->out_b_needs_pulled)
			logerror("PIA #%d: Warning! No port B write handler. Previous value has been lost!\n", which);

		p->out_b_needs_pulled = TRUE;
	}
}


static void port_a_w(running_machine *machine, int which, UINT8 data)
{
	pia6821 *p = &pias[which];

	/* buffer the output value */
	p->out_a = data;

	send_to_out_a_func(machine, which, "port A write");
}


static void ddr_a_w(running_machine *machine, int which, UINT8 data)
{
	pia6821 *p = &pias[which];

	if (data == 0x00)
		LOG(("PIA #%d: DDR A write = %02X (input mode)\n", which, data));
	else if (data == 0xff)
		LOG(("PIA #%d: DDR A write = %02X (output mode)\n", which, data));
	else
		LOG(("PIA #%d: DDR A write = %02X (mixed mode)\n", which, data));

	if (p->ddr_a != data)
	{
		/* DDR changed, call the callback again */
		p->ddr_a = data;
		p->logged_port_a_not_connected = FALSE;
		send_to_out_a_func(machine, which, "port A write due to DDR change");
	}
}


static void port_b_w(running_machine *machine, int which, UINT8 data)
{
	pia6821 *p = &pias[which];

	/* buffer the output value */
	p->out_b = data;

	send_to_out_b_func(machine, which, "port B write");

	/* CB2 in write strobe mode */
	if (C2_STROBE_MODE(p->ctl_b))
	{
		/* this will cause a transition low */
		set_out_cb2(machine, which, FALSE);

		/* if the CB2 strobe is cleared by the E, reset it right away */
		if (STROBE_E_RESET(p->ctl_b))
			set_out_cb2(machine, which, TRUE);
	}
}


static void ddr_b_w(running_machine *machine, int which, UINT8 data)
{
	pia6821 *p = &pias[which];

	if (data == 0x00)
		LOG(("PIA #%d: DDR B write = %02X (input mode)\n", which, data));
	else if (data == 0xff)
		LOG(("PIA #%d: DDR B write = %02X (output mode)\n", which, data));
	else
		LOG(("PIA #%d: DDR B write = %02X (mixed mode)\n", which, data));

	if (p->ddr_b != data)
	{
		/* DDR changed, call the callback again */
		p->ddr_b = data;
		p->logged_port_b_not_connected = FALSE;
		send_to_out_b_func(machine, which, "port B write due to DDR change");
	}
}


static void control_a_w(running_machine *machine, int which, UINT8 data)
{
	pia6821 *p = &pias[which];

	/* bit 7 and 6 are read only */
	data &= 0x3f;

	LOG(("PIA #%d: control A write = %02X\n", which, data));

	/* update the control register */
	p->ctl_a = data;

	/* CA2 is configured as output */
	if (C2_OUTPUT(p->ctl_a))
	{
		int temp;

		if (C2_SET_MODE(p->ctl_a))
			/* set/reset mode - bit value determines the new output */
			temp = C2_SET(p->ctl_a);
		else
			/* strobe mode - output is always high unless strobed */
			temp = TRUE;

		set_out_ca2(machine, which, temp);
	}

	/* update externals */
	update_interrupts(machine, p);
}


static void control_b_w(running_machine *machine, int which, UINT8 data)
{
	pia6821 *p = &pias[which];
	int temp;

	/* bit 7 and 6 are read only */
	data &= 0x3f;

	LOG(("PIA #%d: control B write = %02X\n", which, data));

	/* update the control register */
	p->ctl_b = data;

	if (C2_SET_MODE(p->ctl_b))
		/* set/reset mode - bit value determines the new output */
		temp = C2_SET(p->ctl_b);
	else
		/* strobe mode - output is always high unless strobed */
		temp = TRUE;

	set_out_cb2(machine, which, temp);

	/* update externals */
	update_interrupts(machine, p);
}


void pia_write(int which, offs_t offset, UINT8 data)
{
	pia6821 *p = &pias[which];

	switch (offset & 0x03)
	{
		default: /* impossible */
		case 0x00:
			if (OUTPUT_SELECTED(p->ctl_a))
				port_a_w(Machine, which, data);
			else
				ddr_a_w(Machine, which, data);
			break;

		case 0x01:
			control_a_w(Machine, which, data);
			break;

		case 0x02:
			if (OUTPUT_SELECTED(p->ctl_b))
				port_b_w(Machine, which, data);
			else
				ddr_b_w(Machine, which, data);
			break;

		case 0x03:
			control_b_w(Machine, which, data);
			break;
	}
}


void pia_alt_write(int which, offs_t offset, UINT8 data)
{
	pia_write(which, ((offset << 1) & 0x02) | ((offset >> 1) & 0x01), data);
}


void pia_set_port_a_z_mask(int which, UINT8 data)
{
	pia6821 *p = &pias[which];

	p->port_a_z_mask = data;
}



/*************************************
 *
 *  Device interface to port A
 *
 *************************************/

UINT8 pia_get_input_a(int which)
{
	pia6821 *p = &pias[which];

	return p->in_a;
}


void pia_set_input_a(int which, UINT8 data, UINT8 z_mask)
{
	pia6821 *p = &pias[which];

	assert_always(p->intf->in_a_func == NULL, "pia_set_input_a() called when in_a_func implemented");

	LOG(("PIA #%d: set input port A = %02X\n", which, data));

	p->in_a = data;
	p->port_a_z_mask = z_mask;
	p->in_a_pushed = TRUE;
}


UINT8 pia_get_output_a(int which)
{
	pia6821 *p = &pias[which];

	p->out_a_needs_pulled = FALSE;

	return get_out_a_value(Machine, which);
}



/*************************************
 *
 *  Device interface to the CA1 pin
 *
 *************************************/

int pia_get_input_ca1(int which)
{
	pia6821 *p = &pias[which];

	return p->in_ca1;
}


void pia_set_input_ca1(int which, int data)
{
	pia6821 *p = &pias[which];

	/* limit the data to 0 or 1 */
	data = data ? TRUE : FALSE;

	LOG(("PIA #%d: set input CA1 = %d\n", which, data));

	/* the new state has caused a transition */
	if ((p->in_ca1 != data) &&
		((data && C1_LOW_TO_HIGH(p->ctl_a)) || (!data && C1_HIGH_TO_LOW(p->ctl_a))))
	{
		LOG(("PIA #%d: CA1 triggering\n", which));

		/* mark the IRQ */
		p->irq_a1 = TRUE;

		/* update externals */
		update_interrupts(Machine, p);

		/* CA2 is configured as output and in read strobe mode and cleared by a CA1 transition */
		if (C2_OUTPUT(p->ctl_a) && C2_STROBE_MODE(p->ctl_a) && STROBE_C1_RESET(p->ctl_a))
			set_out_ca2(Machine, which, TRUE);
	}

	/* set the new value for CA1 */
	p->in_ca1 = data;
	p->in_ca1_pushed = TRUE;
}



/*************************************
 *
 *  Device interface to the CA2 pin
 *
 *************************************/

int pia_get_input_ca2(int which)
{
	pia6821 *p = &pias[which];

	return p->in_ca2;
}


void pia_set_input_ca2(int which, int data)
{
	pia6821 *p = &pias[which];

	/* limit the data to 0 or 1 */
	data = data ? 1 : 0;

	LOG(("PIA #%d: set input CA2 = %d\n", which, data));

	/* if input mode and the new state has caused a transition */
	if (C2_INPUT(p->ctl_a) &&
		(p->in_ca2 != data) &&
		((data && C2_LOW_TO_HIGH(p->ctl_a)) || (!data && C2_HIGH_TO_LOW(p->ctl_a))))
	{
		LOG(("PIA #%d: CA2 triggering\n", which));

		/* mark the IRQ */
		p->irq_a2 = TRUE;

		/* update externals */
		update_interrupts(Machine, p);
	}

	/* set the new value for CA2 */
	p->in_ca2 = data;
	p->in_ca2_pushed = TRUE;
}


int pia_get_output_ca2(int which)
{
	pia6821 *p = &pias[which];

	p->out_ca2_needs_pulled = FALSE;

	return p->out_ca2;
}

// Version of pia_get_output_ca2, which takes account of internal
// pullup resistor
int pia_get_output_ca2_z(int which)
{
	pia6821 *p = &pias[which];

	p->out_ca2_needs_pulled = FALSE;

	// If it's an output, output the bit, if it's an input, it's
	// pulled up
	return p->out_ca2 |
	       C2_INPUT(p->ctl_a);
}


/*************************************
 *
 *  Device interface to port B
 *
 *************************************/

UINT8 pia_get_input_b(int which)
{
	pia6821 *p = &pias[which];

	return p->in_b;
}


void pia_set_input_b(int which, UINT8 data)
{
	pia6821 *p = &pias[which];

	assert_always(p->intf->in_b_func == NULL, "pia_set_input_b() called when in_b_func implemented");

	LOG(("PIA #%d: set input port B = %02X\n", which, data));

	p->in_b = data;
	p->in_b_pushed = TRUE;
}



UINT8 pia_get_output_b(int which)
{
	pia6821 *p = &pias[which];

	p->out_b_needs_pulled = FALSE;

	return get_out_b_value(which);
}



/*************************************
 *
 *  Device interface to the CB1 pin
 *
 *************************************/

int pia_get_input_cb1(int which)
{
	pia6821 *p = &pias[which];

	return p->in_cb1;
}


void pia_set_input_cb1(int which, int data)
{
	pia6821 *p = &pias[which];

	/* limit the data to 0 or 1 */
	data = data ? 1 : 0;

	LOG(("PIA #%d: set input CB1 = %d\n", which, data));

	/* the new state has caused a transition */
	if ((p->in_cb1 != data) &&
		((data && C1_LOW_TO_HIGH(p->ctl_b)) || (!data && C1_HIGH_TO_LOW(p->ctl_b))))
	{
		LOG(("PIA #%d: CB1 triggering\n", which));

		/* mark the IRQ */
		p->irq_b1 = 1;

		/* update externals */
		update_interrupts(Machine, p);

		/* If CB2 is configured as a write-strobe output which is reset by a CB1
           transition, this reset will only happen when a read from port B implicitly
           clears the IRQ B1 flag.  So we handle the CB2 reset there.  Note that this
           is different from what happens with port A. */
	}

	/* set the new value for CB1 */
	p->in_cb1 = data;
	p->in_cb1_pushed = TRUE;
}



/*************************************
 *
 *  Device interface to the CB2 pin
 *
 *************************************/

int pia_get_input_cb2(int which)
{
	pia6821 *p = &pias[which];

	return p->in_cb2;
}


void pia_set_input_cb2(int which, int data)
{
	pia6821 *p = &pias[which];

	/* limit the data to 0 or 1 */
	data = data ? 1 : 0;

	LOG(("PIA #%d: set input CB2 = %d\n", which, data));

	/* if input mode and the new state has caused a transition */
	if (C2_INPUT(p->ctl_b) &&
		(p->in_cb2 != data) &&
		((data && C2_LOW_TO_HIGH(p->ctl_b)) || (!data && C2_HIGH_TO_LOW(p->ctl_b))))
	{
		LOG(("PIA #%d: CB2 triggering\n", which));

		/* mark the IRQ */
		p->irq_b2 = 1;

		/* update externals */
		update_interrupts(Machine, p);
	}

	/* set the new value for CA2 */
	p->in_cb2 = data;
	p->in_cb2_pushed = TRUE;
}


int pia_get_output_cb2(int which)
{
	pia6821 *p = &pias[which];

	p->out_cb2_needs_pulled = FALSE;

	return p->out_cb2;
}


int pia_get_output_cb2_z(int which)
{
	pia6821 *p = &pias[which];

	return !C2_OUTPUT(p->ctl_b);
}



/*************************************
 *
 *  Convinience interface for
 *  retrieving the IRQ state
 *
 *************************************/

int pia_get_irq_a(int which)
{
	pia6821 *p = &pias[which];

	return p->irq_a_state;
}


int pia_get_irq_b(int which)
{
	pia6821 *p = &pias[which];

	return p->irq_b_state;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

READ8_HANDLER( pia_0_r ) { return pia_read(0, offset); }
READ8_HANDLER( pia_1_r ) { return pia_read(1, offset); }
READ8_HANDLER( pia_2_r ) { return pia_read(2, offset); }
READ8_HANDLER( pia_3_r ) { return pia_read(3, offset); }
READ8_HANDLER( pia_4_r ) { return pia_read(4, offset); }
READ8_HANDLER( pia_5_r ) { return pia_read(5, offset); }
READ8_HANDLER( pia_6_r ) { return pia_read(6, offset); }
READ8_HANDLER( pia_7_r ) { return pia_read(7, offset); }

WRITE8_HANDLER( pia_0_w ) { pia_write(0, offset, data); }
WRITE8_HANDLER( pia_1_w ) { pia_write(1, offset, data); }
WRITE8_HANDLER( pia_2_w ) { pia_write(2, offset, data); }
WRITE8_HANDLER( pia_3_w ) { pia_write(3, offset, data); }
WRITE8_HANDLER( pia_4_w ) { pia_write(4, offset, data); }
WRITE8_HANDLER( pia_5_w ) { pia_write(5, offset, data); }
WRITE8_HANDLER( pia_6_w ) { pia_write(6, offset, data); }
WRITE8_HANDLER( pia_7_w ) { pia_write(7, offset, data); }

READ8_HANDLER( pia_0_alt_r ) { return pia_alt_read(0, offset); }
READ8_HANDLER( pia_1_alt_r ) { return pia_alt_read(1, offset); }
READ8_HANDLER( pia_2_alt_r ) { return pia_alt_read(2, offset); }
READ8_HANDLER( pia_3_alt_r ) { return pia_alt_read(3, offset); }
READ8_HANDLER( pia_4_alt_r ) { return pia_alt_read(4, offset); }
READ8_HANDLER( pia_5_alt_r ) { return pia_alt_read(5, offset); }
READ8_HANDLER( pia_6_alt_r ) { return pia_alt_read(6, offset); }
READ8_HANDLER( pia_7_alt_r ) { return pia_alt_read(7, offset); }

WRITE8_HANDLER( pia_0_alt_w ) { pia_alt_write(0, offset, data); }
WRITE8_HANDLER( pia_1_alt_w ) { pia_alt_write(1, offset, data); }
WRITE8_HANDLER( pia_2_alt_w ) { pia_alt_write(2, offset, data); }
WRITE8_HANDLER( pia_3_alt_w ) { pia_alt_write(3, offset, data); }
WRITE8_HANDLER( pia_4_alt_w ) { pia_alt_write(4, offset, data); }
WRITE8_HANDLER( pia_5_alt_w ) { pia_alt_write(5, offset, data); }
WRITE8_HANDLER( pia_6_alt_w ) { pia_alt_write(6, offset, data); }
WRITE8_HANDLER( pia_7_alt_w ) { pia_alt_write(7, offset, data); }

/******************* 8-bit A/B port interfaces *******************/

WRITE8_HANDLER( pia_0_porta_w ) { pia_set_input_a(0, data, 0); }
WRITE8_HANDLER( pia_1_porta_w ) { pia_set_input_a(1, data, 0); }
WRITE8_HANDLER( pia_2_porta_w ) { pia_set_input_a(2, data, 0); }
WRITE8_HANDLER( pia_3_porta_w ) { pia_set_input_a(3, data, 0); }
WRITE8_HANDLER( pia_4_porta_w ) { pia_set_input_a(4, data, 0); }
WRITE8_HANDLER( pia_5_porta_w ) { pia_set_input_a(5, data, 0); }
WRITE8_HANDLER( pia_6_porta_w ) { pia_set_input_a(6, data, 0); }
WRITE8_HANDLER( pia_7_porta_w ) { pia_set_input_a(7, data, 0); }

WRITE8_HANDLER( pia_0_portb_w ) { pia_set_input_b(0, data); }
WRITE8_HANDLER( pia_1_portb_w ) { pia_set_input_b(1, data); }
WRITE8_HANDLER( pia_2_portb_w ) { pia_set_input_b(2, data); }
WRITE8_HANDLER( pia_3_portb_w ) { pia_set_input_b(3, data); }
WRITE8_HANDLER( pia_4_portb_w ) { pia_set_input_b(4, data); }
WRITE8_HANDLER( pia_5_portb_w ) { pia_set_input_b(5, data); }
WRITE8_HANDLER( pia_6_portb_w ) { pia_set_input_b(6, data); }
WRITE8_HANDLER( pia_7_portb_w ) { pia_set_input_b(7, data); }

READ8_HANDLER( pia_0_porta_r ) { return pia_get_input_a(0); }
READ8_HANDLER( pia_1_porta_r ) { return pia_get_input_a(1); }
READ8_HANDLER( pia_2_porta_r ) { return pia_get_input_a(2); }
READ8_HANDLER( pia_3_porta_r ) { return pia_get_input_a(3); }
READ8_HANDLER( pia_4_porta_r ) { return pia_get_input_a(4); }
READ8_HANDLER( pia_5_porta_r ) { return pia_get_input_a(5); }
READ8_HANDLER( pia_6_porta_r ) { return pia_get_input_a(6); }
READ8_HANDLER( pia_7_porta_r ) { return pia_get_input_a(7); }

READ8_HANDLER( pia_0_portb_r ) { return pia_get_input_b(0); }
READ8_HANDLER( pia_1_portb_r ) { return pia_get_input_b(1); }
READ8_HANDLER( pia_2_portb_r ) { return pia_get_input_b(2); }
READ8_HANDLER( pia_3_portb_r ) { return pia_get_input_b(3); }
READ8_HANDLER( pia_4_portb_r ) { return pia_get_input_b(4); }
READ8_HANDLER( pia_5_portb_r ) { return pia_get_input_b(5); }
READ8_HANDLER( pia_6_portb_r ) { return pia_get_input_b(6); }
READ8_HANDLER( pia_7_portb_r ) { return pia_get_input_b(7); }

/******************* 1-bit CA1/CA2/CB1/CB2 port interfaces *******************/

WRITE8_HANDLER( pia_0_ca1_w ) { pia_set_input_ca1(0, data); }
WRITE8_HANDLER( pia_1_ca1_w ) { pia_set_input_ca1(1, data); }
WRITE8_HANDLER( pia_2_ca1_w ) { pia_set_input_ca1(2, data); }
WRITE8_HANDLER( pia_3_ca1_w ) { pia_set_input_ca1(3, data); }
WRITE8_HANDLER( pia_4_ca1_w ) { pia_set_input_ca1(4, data); }
WRITE8_HANDLER( pia_5_ca1_w ) { pia_set_input_ca1(5, data); }
WRITE8_HANDLER( pia_6_ca1_w ) { pia_set_input_ca1(6, data); }
WRITE8_HANDLER( pia_7_ca1_w ) { pia_set_input_ca1(7, data); }

WRITE8_HANDLER( pia_0_ca2_w ) { pia_set_input_ca2(0, data); }
WRITE8_HANDLER( pia_1_ca2_w ) { pia_set_input_ca2(1, data); }
WRITE8_HANDLER( pia_2_ca2_w ) { pia_set_input_ca2(2, data); }
WRITE8_HANDLER( pia_3_ca2_w ) { pia_set_input_ca2(3, data); }
WRITE8_HANDLER( pia_4_ca2_w ) { pia_set_input_ca2(4, data); }
WRITE8_HANDLER( pia_5_ca2_w ) { pia_set_input_ca2(5, data); }
WRITE8_HANDLER( pia_6_ca2_w ) { pia_set_input_ca2(6, data); }
WRITE8_HANDLER( pia_7_ca2_w ) { pia_set_input_ca2(7, data); }

WRITE8_HANDLER( pia_0_cb1_w ) { pia_set_input_cb1(0, data); }
WRITE8_HANDLER( pia_1_cb1_w ) { pia_set_input_cb1(1, data); }
WRITE8_HANDLER( pia_2_cb1_w ) { pia_set_input_cb1(2, data); }
WRITE8_HANDLER( pia_3_cb1_w ) { pia_set_input_cb1(3, data); }
WRITE8_HANDLER( pia_4_cb1_w ) { pia_set_input_cb1(4, data); }
WRITE8_HANDLER( pia_5_cb1_w ) { pia_set_input_cb1(5, data); }
WRITE8_HANDLER( pia_6_cb1_w ) { pia_set_input_cb1(6, data); }
WRITE8_HANDLER( pia_7_cb1_w ) { pia_set_input_cb1(7, data); }

WRITE8_HANDLER( pia_0_cb2_w ) { pia_set_input_cb2(0, data); }
WRITE8_HANDLER( pia_1_cb2_w ) { pia_set_input_cb2(1, data); }
WRITE8_HANDLER( pia_2_cb2_w ) { pia_set_input_cb2(2, data); }
WRITE8_HANDLER( pia_3_cb2_w ) { pia_set_input_cb2(3, data); }
WRITE8_HANDLER( pia_4_cb2_w ) { pia_set_input_cb2(4, data); }
WRITE8_HANDLER( pia_5_cb2_w ) { pia_set_input_cb2(5, data); }
WRITE8_HANDLER( pia_6_cb2_w ) { pia_set_input_cb2(6, data); }
WRITE8_HANDLER( pia_7_cb2_w ) { pia_set_input_cb2(7, data); }

READ8_HANDLER( pia_0_ca1_r ) { return pia_get_input_ca1(0); }
READ8_HANDLER( pia_1_ca1_r ) { return pia_get_input_ca1(1); }
READ8_HANDLER( pia_2_ca1_r ) { return pia_get_input_ca1(2); }
READ8_HANDLER( pia_3_ca1_r ) { return pia_get_input_ca1(3); }
READ8_HANDLER( pia_4_ca1_r ) { return pia_get_input_ca1(4); }
READ8_HANDLER( pia_5_ca1_r ) { return pia_get_input_ca1(5); }
READ8_HANDLER( pia_6_ca1_r ) { return pia_get_input_ca1(6); }
READ8_HANDLER( pia_7_ca1_r ) { return pia_get_input_ca1(7); }

READ8_HANDLER( pia_0_ca2_r ) { return pia_get_input_ca2(0); }
READ8_HANDLER( pia_1_ca2_r ) { return pia_get_input_ca2(1); }
READ8_HANDLER( pia_2_ca2_r ) { return pia_get_input_ca2(2); }
READ8_HANDLER( pia_3_ca2_r ) { return pia_get_input_ca2(3); }
READ8_HANDLER( pia_4_ca2_r ) { return pia_get_input_ca2(4); }
READ8_HANDLER( pia_5_ca2_r ) { return pia_get_input_ca2(5); }
READ8_HANDLER( pia_6_ca2_r ) { return pia_get_input_ca2(6); }
READ8_HANDLER( pia_7_ca2_r ) { return pia_get_input_ca2(7); }

READ8_HANDLER( pia_0_cb1_r ) { return pia_get_input_cb1(0); }
READ8_HANDLER( pia_1_cb1_r ) { return pia_get_input_cb1(1); }
READ8_HANDLER( pia_2_cb1_r ) { return pia_get_input_cb1(2); }
READ8_HANDLER( pia_3_cb1_r ) { return pia_get_input_cb1(3); }
READ8_HANDLER( pia_4_cb1_r ) { return pia_get_input_cb1(4); }
READ8_HANDLER( pia_5_cb1_r ) { return pia_get_input_cb1(5); }
READ8_HANDLER( pia_6_cb1_r ) { return pia_get_input_cb1(6); }
READ8_HANDLER( pia_7_cb1_r ) { return pia_get_input_cb1(7); }

READ8_HANDLER( pia_0_cb2_r ) { return pia_get_input_cb2(0); }
READ8_HANDLER( pia_1_cb2_r ) { return pia_get_input_cb2(1); }
READ8_HANDLER( pia_2_cb2_r ) { return pia_get_input_cb2(2); }
READ8_HANDLER( pia_3_cb2_r ) { return pia_get_input_cb2(3); }
READ8_HANDLER( pia_4_cb2_r ) { return pia_get_input_cb2(4); }
READ8_HANDLER( pia_5_cb2_r ) { return pia_get_input_cb2(5); }
READ8_HANDLER( pia_6_cb2_r ) { return pia_get_input_cb2(6); }
READ8_HANDLER( pia_7_cb2_r ) { return pia_get_input_cb2(7); }
