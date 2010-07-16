/**********************************************************************

    Motorola 6821 PIA interface and emulation

**********************************************************************/

#include "emu.h"
#include "6821pia.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _pia6821_state pia6821_state;
struct _pia6821_state
{
	devcb_resolved_read8 in_a_func;
	devcb_resolved_read8 in_b_func;
	devcb_resolved_read_line in_ca1_func;
	devcb_resolved_read_line in_cb1_func;
	devcb_resolved_read_line in_ca2_func;
	devcb_resolved_read_line in_cb2_func;
	devcb_resolved_write8 out_a_func;
	devcb_resolved_write8 out_b_func;
	devcb_resolved_write_line out_ca2_func;
	devcb_resolved_write_line out_cb2_func;
	devcb_resolved_write_line irq_a_func;
	devcb_resolved_write_line irq_b_func;

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


/***************************************************************************
    MACROS
***************************************************************************/

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

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


/***************************************************************************
    PROTOTYPES
***************************************************************************/
/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE pia6821_state *get_token(running_device *device)
{
	assert(device != NULL);
	assert((device->type() == PIA6821) || (device->type() == PIA6822));
	return (pia6821_state *) downcast<legacy_device_base *>(device)->token();
}


INLINE const pia6821_interface *get_interface(running_device *device)
{
	assert(device != NULL);
	assert((device->type() == PIA6821) || (device->type() == PIA6822));
	return (const pia6821_interface *) device->baseconfig().static_config();
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    DEVICE_START( pia6821 )
-------------------------------------------------*/

static DEVICE_START( pia6821 )
{
	pia6821_state *p = get_token(device);
	const pia6821_interface *intf = get_interface(device);

	/* clear structure */
	memset(p, 0, sizeof(*p));

	/* resolve callbacks */
	devcb_resolve_read8(&p->in_a_func, &intf->in_a_func, device);
	devcb_resolve_read8(&p->in_b_func, &intf->in_b_func, device);
	devcb_resolve_read_line(&p->in_ca1_func, &intf->in_ca1_func, device);
	devcb_resolve_read_line(&p->in_cb1_func, &intf->in_cb1_func, device);
	devcb_resolve_read_line(&p->in_ca2_func, &intf->in_ca2_func, device);
	devcb_resolve_read_line(&p->in_cb2_func, &intf->in_cb2_func, device);
	devcb_resolve_write8(&p->out_a_func, &intf->out_a_func, device);
	devcb_resolve_write8(&p->out_b_func, &intf->out_b_func, device);
	devcb_resolve_write_line(&p->out_ca2_func, &intf->out_ca2_func, device);
	devcb_resolve_write_line(&p->out_cb2_func, &intf->out_cb2_func, device);
	devcb_resolve_write_line(&p->irq_a_func, &intf->irq_a_func, device);
	devcb_resolve_write_line(&p->irq_b_func, &intf->irq_b_func, device);

	state_save_register_device_item(device, 0, p->in_a);
	state_save_register_device_item(device, 0, p->in_ca1);
	state_save_register_device_item(device, 0, p->in_ca2);
	state_save_register_device_item(device, 0, p->out_a);
	state_save_register_device_item(device, 0, p->out_ca2);
	state_save_register_device_item(device, 0, p->port_a_z_mask);
	state_save_register_device_item(device, 0, p->ddr_a);
	state_save_register_device_item(device, 0, p->ctl_a);
	state_save_register_device_item(device, 0, p->irq_a1);
	state_save_register_device_item(device, 0, p->irq_a2);
	state_save_register_device_item(device, 0, p->irq_a_state);
	state_save_register_device_item(device, 0, p->in_b);
	state_save_register_device_item(device, 0, p->in_cb1);
	state_save_register_device_item(device, 0, p->in_cb2);
	state_save_register_device_item(device, 0, p->out_b);
	state_save_register_device_item(device, 0, p->out_cb2);
	state_save_register_device_item(device, 0, p->last_out_cb2_z);
	state_save_register_device_item(device, 0, p->ddr_b);
	state_save_register_device_item(device, 0, p->ctl_b);
	state_save_register_device_item(device, 0, p->irq_b1);
	state_save_register_device_item(device, 0, p->irq_b2);
	state_save_register_device_item(device, 0, p->irq_b_state);
	state_save_register_device_item(device, 0, p->in_a_pushed);
	state_save_register_device_item(device, 0, p->out_a_needs_pulled);
	state_save_register_device_item(device, 0, p->in_ca1_pushed);
	state_save_register_device_item(device, 0, p->in_ca2_pushed);
	state_save_register_device_item(device, 0, p->out_ca2_needs_pulled);
	state_save_register_device_item(device, 0, p->in_b_pushed);
	state_save_register_device_item(device, 0, p->out_b_needs_pulled);
	state_save_register_device_item(device, 0, p->in_cb1_pushed);
	state_save_register_device_item(device, 0, p->in_cb2_pushed);
	state_save_register_device_item(device, 0, p->out_cb2_needs_pulled);
}


/*-------------------------------------------------
    DEVICE_RESET( pia6821 )
-------------------------------------------------*/

static DEVICE_RESET( pia6821 )
{
	pia6821_state *p = get_token(device);

	/*
     * set default read values.
     *
     * ports A,CA1,CA2 default to 1
     * ports B,CB1,CB2 are three-state and undefined (set to 0)
     */
	p->in_a = 0xff;
	p->in_ca1 = TRUE;
	p->in_ca2 = TRUE;
	p->out_a = 0;
	p->out_ca2 = 0;
	p->port_a_z_mask = 0;
	p->ddr_a = 0;
	p->ctl_a = 0;
	p->irq_a1 = 0;
	p->irq_a2 = 0;
	p->irq_a_state = 0;
	p->in_b = 0;
	p->in_cb1 = 0;
	p->in_cb2 = 0;
	p->out_b = 0;
	p->out_cb2 = 0;
	p->last_out_cb2_z = 0;
	p->ddr_b = 0;
	p->ctl_b = 0;
	p->irq_b1 = 0;
	p->irq_b2 = 0;
	p->irq_b_state = 0;
	p->in_a_pushed = 0;
	p->out_a_needs_pulled = 0;
	p->in_ca1_pushed = 0;
	p->in_ca2_pushed = 0;
	p->out_ca2_needs_pulled = 0;
	p->in_b_pushed = 0;
	p->out_b_needs_pulled = 0;
	p->in_cb1_pushed = 0;
	p->in_cb2_pushed = 0;
	p->out_cb2_needs_pulled = 0;
	p->logged_port_a_not_connected = 0;
	p->logged_port_b_not_connected = 0;
	p->logged_ca1_not_connected = 0;
	p->logged_ca2_not_connected = 0;
	p->logged_cb1_not_connected = 0;
	p->logged_cb2_not_connected = 0;


	/* clear the IRQs */
	devcb_call_write_line(&p->irq_a_func, FALSE);
	devcb_call_write_line(&p->irq_b_func, FALSE);
}


/*-------------------------------------------------
    update_interrupts
-------------------------------------------------*/

static void update_interrupts(running_device *device)
{
	pia6821_state *p = get_token(device);
	int new_state;

	/* start with IRQ A */
	new_state = (p->irq_a1 && IRQ1_ENABLED(p->ctl_a)) || (p->irq_a2 && IRQ2_ENABLED(p->ctl_a));

	if (new_state != p->irq_a_state)
	{
		p->irq_a_state = new_state;
		devcb_call_write_line(&p->irq_a_func, p->irq_a_state);
	}

	/* then do IRQ B */
	new_state = (p->irq_b1 && IRQ1_ENABLED(p->ctl_b)) || (p->irq_b2 && IRQ2_ENABLED(p->ctl_b));

	if (new_state != p->irq_b_state)
	{
		p->irq_b_state = new_state;
		devcb_call_write_line(&p->irq_b_func, p->irq_b_state);
	}
}


/*-------------------------------------------------
    get_in_a_value
-------------------------------------------------*/

static UINT8 get_in_a_value(running_device *device)
{
	pia6821_state *p = get_token(device);
	UINT8 port_a_data = 0;
	UINT8 ret;

	/* update the input */
	if (p->in_a_func.read != NULL)
		port_a_data = devcb_call_read8(&p->in_a_func, 0);
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
				logerror("PIA #%s: Warning! No port A read handler. Assuming pins 0x%02X not connected\n", device->tag(), p->ddr_a ^ 0xff);
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


/*-------------------------------------------------
    get_in_b_value
-------------------------------------------------*/

static UINT8 get_in_b_value(running_device *device)
{
	pia6821_state *p = get_token(device);
	UINT8 ret;

	if (p->ddr_b == 0xff)
		/* all output, just return buffer */
		ret = p->out_b;
	else
	{
		UINT8 port_b_data;

		/* update the input */
		if (p->in_b_func.read != NULL)
			port_b_data = devcb_call_read8(&p->in_b_func, 0);
		else
		{
			if (p->in_b_pushed)
				port_b_data = p->in_b;
			else
			{
				if (!p->logged_port_b_not_connected && (p->ddr_b != 0xff))
				{
					logerror("PIA #%s: Error! No port B read handler. Three-state pins 0x%02X are undefined\n", device->tag(), p->ddr_b ^ 0xff);
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


/*-------------------------------------------------
    get_out_a_value
-------------------------------------------------*/

static UINT8 get_out_a_value(running_device *device)
{
	pia6821_state *p = get_token(device);
	UINT8 ret;

	if (p->ddr_a == 0xff)
		/* all output */
		ret = p->out_a;
	else
		/* input pins don't change */
		ret = (p->out_a & p->ddr_a) | (get_in_a_value(device) & ~p->ddr_a);

	return ret;
}


/*-------------------------------------------------
    get_out_b_value
-------------------------------------------------*/

static UINT8 get_out_b_value(running_device *device)
{
	pia6821_state *p = get_token(device);

	/* input pins are high-impedance - we just send them as zeros for backwards compatibility */
	return p->out_b & p->ddr_b;
}


/*-------------------------------------------------
    set_out_ca2
-------------------------------------------------*/

static void set_out_ca2(running_device *device, int data)
{
	pia6821_state *p = get_token(device);

	if (data != p->out_ca2)
	{
		p->out_ca2 = data;

		/* send to output function */
		if (p->out_ca2_func.write)
			devcb_call_write_line(&p->out_ca2_func, p->out_ca2);
		else
		{
			if (p->out_ca2_needs_pulled)
				logerror("PIA #%s: Warning! No port CA2 write handler. Previous value has been lost!\n", device->tag());

			p->out_ca2_needs_pulled = TRUE;
		}
	}
}


/*-------------------------------------------------
    set_out_cb2
-------------------------------------------------*/

static void set_out_cb2(running_device *device, int data)
{
	pia6821_state *p = get_token(device);

	int z = pia6821_get_output_cb2_z(device);

	if ((data != p->out_cb2) || (z != p->last_out_cb2_z))
	{
		p->out_cb2 = data;
		p->last_out_cb2_z = z;

		/* send to output function */
		if (p->out_cb2_func.write)
			devcb_call_write_line(&p->out_cb2_func, p->out_cb2);
		else
		{
			if (p->out_cb2_needs_pulled)
				logerror("PIA #%s: Warning! No port CB2 write handler. Previous value has been lost!\n", device->tag());

			p->out_cb2_needs_pulled = TRUE;
		}
	}
}


/*-------------------------------------------------
    port_a_r
-------------------------------------------------*/

static UINT8 port_a_r(running_device *device)
{
	pia6821_state *p = get_token(device);

	UINT8 ret = get_in_a_value(device);

	/* IRQ flags implicitly cleared by a read */
	p->irq_a1 = FALSE;
	p->irq_a2 = FALSE;
	update_interrupts(device);

	/* CA2 is configured as output and in read strobe mode */
	if (C2_OUTPUT(p->ctl_a) && C2_STROBE_MODE(p->ctl_a))
	{
		/* this will cause a transition low */
		set_out_ca2(device, FALSE);

		/* if the CA2 strobe is cleared by the E, reset it right away */
		if (STROBE_E_RESET(p->ctl_a))
			set_out_ca2(device, TRUE);
	}

	LOG(("PIA #%s: port A read = %02X\n", device->tag(), ret));

	return ret;
}


/*-------------------------------------------------
    ddr_a_r
-------------------------------------------------*/

static UINT8 ddr_a_r(running_device *device)
{
	pia6821_state *p = get_token(device);

	UINT8 ret = p->ddr_a;

	LOG(("PIA #%s: DDR A read = %02X\n", device->tag(), ret));

	return ret;
}


/*-------------------------------------------------
    port_b_r
-------------------------------------------------*/

static UINT8 port_b_r(running_device *device)
{
	pia6821_state *p = get_token(device);

	UINT8 ret = get_in_b_value(device);

	/* This read will implicitly clear the IRQ B1 flag.  If CB2 is in write-strobe
       mode with CB1 restore, and a CB1 active transition set the flag,
       clearing it will cause CB2 to go high again.  Note that this is different
       from what happens with port A. */
	if (p->irq_b1 && C2_STROBE_MODE(p->ctl_b) && STROBE_C1_RESET(p->ctl_b))
		set_out_cb2(device, TRUE);

	/* IRQ flags implicitly cleared by a read */
	p->irq_b1 = FALSE;
	p->irq_b2 = FALSE;
	update_interrupts(device);

	LOG(("PIA #%s: port B read = %02X\n", device->tag(), ret));

	return ret;
}


/*-------------------------------------------------
    ddr_b_r
-------------------------------------------------*/

static UINT8 ddr_b_r(running_device *device)
{
	pia6821_state *p = get_token(device);

	UINT8 ret = p->ddr_b;

	LOG(("PIA #%s: DDR B read = %02X\n", device->tag(), ret));

	return ret;
}


/*-------------------------------------------------
    control_a_r
-------------------------------------------------*/

static UINT8 control_a_r(running_device *device)
{
	pia6821_state *p = get_token(device);
	UINT8 ret;

	/* update CA1 & CA2 if callback exists, these in turn may update IRQ's */
	if (p->in_ca1_func.read != NULL)
		pia6821_ca1_w(device, devcb_call_read_line(&p->in_ca1_func));
	else if (!p->logged_ca1_not_connected && (!p->in_ca1_pushed))
	{
		logerror("PIA #%s: Warning! No CA1 read handler. Assuming pin not connected\n", device->tag());
		p->logged_ca1_not_connected = TRUE;
	}

	if (p->in_ca2_func.read != NULL)
		pia6821_ca2_w(device, devcb_call_read_line(&p->in_ca2_func));
	else if ( !p->logged_ca2_not_connected && C2_INPUT(p->ctl_a) && !p->in_ca2_pushed)
	{
		logerror("PIA #%s: Warning! No CA2 read handler. Assuming pin not connected\n", device->tag());
		p->logged_ca2_not_connected = TRUE;
	}

	/* read control register */
	ret = p->ctl_a;

	/* set the IRQ flags if we have pending IRQs */
	if (p->irq_a1)
		ret |= PIA_IRQ1;

	if (p->irq_a2 && C2_INPUT(p->ctl_a))
		ret |= PIA_IRQ2;

	LOG(("PIA #%s: control A read = %02X\n", device->tag(), ret));

	return ret;
}


/*-------------------------------------------------
    control_b_r
-------------------------------------------------*/

static UINT8 control_b_r(running_device *device)
{
	pia6821_state *p = get_token(device);
	UINT8 ret;

	/* update CB1 & CB2 if callback exists, these in turn may update IRQ's */
	if (p->in_cb1_func.read != NULL)
		pia6821_cb1_w(device, devcb_call_read_line(&p->in_cb1_func));
	else if (!p->logged_cb1_not_connected && !p->in_cb1_pushed)
	{
		logerror("PIA #%s: Error! no CB1 read handler. Three-state pin is undefined\n", device->tag());
		p->logged_cb1_not_connected = TRUE;
	}

	if (p->in_cb2_func.read != NULL)
		pia6821_cb2_w(device, devcb_call_read_line(&p->in_cb2_func));
	else if (!p->logged_cb2_not_connected && C2_INPUT(p->ctl_b) && !p->in_cb2_pushed)
	{
		logerror("PIA #%s: Error! No CB2 read handler. Three-state pin is undefined\n", device->tag());
		p->logged_cb2_not_connected = TRUE;
	}

	/* read control register */
	ret = p->ctl_b;

	/* set the IRQ flags if we have pending IRQs */
	if (p->irq_b1)
		ret |= PIA_IRQ1;

	if (p->irq_b2 && C2_INPUT(p->ctl_b))
		ret |= PIA_IRQ2;

	LOG(("PIA #%s: control B read = %02X\n", device->tag(), ret));

	return ret;
}


/*-------------------------------------------------
    pia6821_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( pia6821_r )
{
	pia6821_state *p = get_token(device);
	UINT8 ret;

	switch (offset & 0x03)
	{
		default: /* impossible */
		case 0x00:
			if (OUTPUT_SELECTED(p->ctl_a))
				ret = port_a_r(device);
			else
				ret = ddr_a_r(device);
			break;

		case 0x01:
			ret = control_a_r(device);
			break;

		case 0x02:
			if (OUTPUT_SELECTED(p->ctl_b))
				ret = port_b_r(device);
			else
				ret = ddr_b_r(device);
			break;

		case 0x03:
			ret = control_b_r(device);
			break;
	}

	return ret;
}


/*-------------------------------------------------
    pia6821_alt_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( pia6821_alt_r )
{
	return pia6821_r(device, ((offset << 1) & 0x02) | ((offset >> 1) & 0x01));
}


/*-------------------------------------------------
    pia6821_get_port_b_z_mask
-------------------------------------------------*/

UINT8 pia6821_get_port_b_z_mask(running_device *device)
{
	pia6821_state *p = get_token(device);
	return ~p->ddr_b;
}


/*-------------------------------------------------
    send_to_out_a_func
-------------------------------------------------*/

static void send_to_out_a_func(running_device *device, const char* message)
{
	pia6821_state *p = get_token(device);

	/* input pins are pulled high */
	UINT8 data = get_out_a_value(device);

	LOG(("PIA #%s: %s = %02X\n", device->tag(), message, data));

	if (p->out_a_func.write != NULL)
		devcb_call_write8(&p->out_a_func, 0, data);
	else
	{
		if (p->out_a_needs_pulled)
			logerror("PIA #%s: Warning! No port A write handler. Previous value has been lost!\n", device->tag());

		p->out_a_needs_pulled = TRUE;
	}
}


/*-------------------------------------------------
    send_to_out_b_func
-------------------------------------------------*/

static void send_to_out_b_func(running_device *device, const char* message)
{
	pia6821_state *p = get_token(device);

	/* input pins are high-impedance - we just send them as zeros for backwards compatibility */
	UINT8 data = get_out_b_value(device);

	LOG(("PIA #%s: %s = %02X\n", device->tag(), message, data));

	if (p->out_b_func.write != NULL)
		devcb_call_write8(&p->out_b_func, 0, data);
	else
	{
		if (p->out_b_needs_pulled)
			logerror("PIA #%s: Warning! No port B write handler. Previous value has been lost!\n", device->tag());

		p->out_b_needs_pulled = TRUE;
	}
}


/*-------------------------------------------------
    port_a_w
-------------------------------------------------*/

static void port_a_w(running_device *device, UINT8 data)
{
	pia6821_state *p = get_token(device);

	/* buffer the output value */
	p->out_a = data;

	send_to_out_a_func(device, "port A write");
}


/*-------------------------------------------------
    ddr_a_w
-------------------------------------------------*/

static void ddr_a_w(running_device *device, UINT8 data)
{
	pia6821_state *p = get_token(device);

	if (data == 0x00)
		LOG(("PIA #%s: DDR A write = %02X (input mode)\n", device->tag(), data));
	else if (data == 0xff)
		LOG(("PIA #%s: DDR A write = %02X (output mode)\n", device->tag(), data));
	else
		LOG(("PIA #%s: DDR A write = %02X (mixed mode)\n", device->tag(), data));

	if (p->ddr_a != data)
	{
		/* DDR changed, call the callback again */
		p->ddr_a = data;
		p->logged_port_a_not_connected = FALSE;
		send_to_out_a_func(device, "port A write due to DDR change");
	}
}


/*-------------------------------------------------
    port_b_w
-------------------------------------------------*/

static void port_b_w(running_device *device, UINT8 data)
{
	pia6821_state *p = get_token(device);

	/* buffer the output value */
	p->out_b = data;

	send_to_out_b_func(device, "port B write");

	/* CB2 in write strobe mode */
	if (C2_STROBE_MODE(p->ctl_b))
	{
		/* this will cause a transition low */
		set_out_cb2(device, FALSE);

		/* if the CB2 strobe is cleared by the E, reset it right away */
		if (STROBE_E_RESET(p->ctl_b))
			set_out_cb2(device, TRUE);
	}
}


/*-------------------------------------------------
    ddr_b_w
-------------------------------------------------*/

static void ddr_b_w(running_device *device, UINT8 data)
{
	pia6821_state *p = get_token(device);

	if (data == 0x00)
		LOG(("PIA #%s: DDR B write = %02X (input mode)\n", device->tag(), data));
	else if (data == 0xff)
		LOG(("PIA #%s: DDR B write = %02X (output mode)\n", device->tag(), data));
	else
		LOG(("PIA #%s: DDR B write = %02X (mixed mode)\n", device->tag(), data));

	if (p->ddr_b != data)
	{
		/* DDR changed, call the callback again */
		p->ddr_b = data;
		p->logged_port_b_not_connected = FALSE;
		send_to_out_b_func(device, "port B write due to DDR change");
	}
}


/*-------------------------------------------------
    control_a_w
-------------------------------------------------*/

static void control_a_w(running_device *device, UINT8 data)
{
	pia6821_state *p = get_token(device);

	/* bit 7 and 6 are read only */
	data &= 0x3f;

	LOG(("PIA #%s: control A write = %02X\n", device->tag(), data));

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

		set_out_ca2(device, temp);
	}

	/* update externals */
	update_interrupts(device);
}


/*-------------------------------------------------
    control_b_w
-------------------------------------------------*/

static void control_b_w(running_device *device, UINT8 data)
{
	pia6821_state *p = get_token(device);
	int temp;

	/* bit 7 and 6 are read only */
	data &= 0x3f;

	LOG(("PIA #%s: control B write = %02X\n", device->tag(), data));

	/* update the control register */
	p->ctl_b = data;

	if (C2_SET_MODE(p->ctl_b))
		/* set/reset mode - bit value determines the new output */
		temp = C2_SET(p->ctl_b);
	else
		/* strobe mode - output is always high unless strobed */
		temp = TRUE;

	set_out_cb2(device, temp);

	/* update externals */
	update_interrupts(device);
}


/*-------------------------------------------------
    pia6821_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( pia6821_w )
{
	pia6821_state *p = get_token(device);

	switch (offset & 0x03)
	{
		default: /* impossible */
		case 0x00:
			if (OUTPUT_SELECTED(p->ctl_a))
				port_a_w(device, data);
			else
				ddr_a_w(device, data);
			break;

		case 0x01:
			control_a_w(device, data);
			break;

		case 0x02:
			if (OUTPUT_SELECTED(p->ctl_b))
				port_b_w(device, data);
			else
				ddr_b_w(device, data);
			break;

		case 0x03:
			control_b_w(device, data);
			break;
	}
}


/*-------------------------------------------------
    pia6821_alt_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( pia6821_alt_w )
{
	pia6821_w(device, ((offset << 1) & 0x02) | ((offset >> 1) & 0x01), data);
}


/*-------------------------------------------------
    pia6821_set_port_a_z_mask
-------------------------------------------------*/

void pia6821_set_port_a_z_mask(running_device *device, UINT8 data)
{
	pia6821_state *p = get_token(device);

	p->port_a_z_mask = data;
}


/*-------------------------------------------------
    pia6821_porta_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( pia6821_porta_r )
{
	pia6821_state *p = get_token(device);

	return p->in_a;
}


/*-------------------------------------------------
    pia6821_set_input_a
-------------------------------------------------*/

void pia6821_set_input_a(running_device *device, UINT8 data, UINT8 z_mask)
{
	pia6821_state *p = get_token(device);

	assert_always(p->in_a_func.read == NULL, "pia6821_porta_w() called when in_a_func implemented");

	LOG(("PIA #%s: set input port A = %02X\n", device->tag(), data));

	p->in_a = data;
	p->port_a_z_mask = z_mask;
	p->in_a_pushed = TRUE;
}


/*-------------------------------------------------
    pia6821_porta_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( pia6821_porta_w )
{
	pia6821_set_input_a(device, data, 0);
}


/*-------------------------------------------------
    pia6821_get_output_a
-------------------------------------------------*/

UINT8 pia6821_get_output_a(running_device *device)
{
	pia6821_state *p = get_token(device);

	p->out_a_needs_pulled = FALSE;

	return get_out_a_value(device);
}


/*-------------------------------------------------
    pia6821_ca1_r
-------------------------------------------------*/

READ_LINE_DEVICE_HANDLER( pia6821_ca1_r )
{
	pia6821_state *p = get_token(device);

	return p->in_ca1;
}


/*-------------------------------------------------
    pia6821_ca1_w
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( pia6821_ca1_w )
{
	pia6821_state *p = get_token(device);

	LOG(("PIA #%s: set input CA1 = %d\n", device->tag(), state));

	/* the new state has caused a transition */
	if ((p->in_ca1 != state) &&
		((state && C1_LOW_TO_HIGH(p->ctl_a)) || (!state && C1_HIGH_TO_LOW(p->ctl_a))))
	{
		LOG(("PIA #%s: CA1 triggering\n", device->tag()));

		/* mark the IRQ */
		p->irq_a1 = TRUE;

		/* update externals */
		update_interrupts(device);

		/* CA2 is configured as output and in read strobe mode and cleared by a CA1 transition */
		if (C2_OUTPUT(p->ctl_a) && C2_STROBE_MODE(p->ctl_a) && STROBE_C1_RESET(p->ctl_a))
			set_out_ca2(device, TRUE);
	}

	/* set the new value for CA1 */
	p->in_ca1 = state;
	p->in_ca1_pushed = TRUE;
}


/*-------------------------------------------------
    pia6821_ca2_r
-------------------------------------------------*/

READ_LINE_DEVICE_HANDLER( pia6821_ca2_r )
{
	pia6821_state *p = get_token(device);

	return p->in_ca2;
}


/*-------------------------------------------------
    pia6821_ca2_w
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( pia6821_ca2_w )
{
	pia6821_state *p = get_token(device);

	LOG(("PIA #%s: set input CA2 = %d\n", device->tag(), state));

	/* if input mode and the new state has caused a transition */
	if (C2_INPUT(p->ctl_a) &&
		(p->in_ca2 != state) &&
		((state && C2_LOW_TO_HIGH(p->ctl_a)) || (!state && C2_HIGH_TO_LOW(p->ctl_a))))
	{
		LOG(("PIA #%s: CA2 triggering\n", device->tag()));

		/* mark the IRQ */
		p->irq_a2 = TRUE;

		/* update externals */
		update_interrupts(device);
	}

	/* set the new value for CA2 */
	p->in_ca2 = state;
	p->in_ca2_pushed = TRUE;
}


/*-------------------------------------------------
    pia6821_get_output_ca2
-------------------------------------------------*/

int pia6821_get_output_ca2(running_device *device)
{
	pia6821_state *p = get_token(device);

	p->out_ca2_needs_pulled = FALSE;

	return p->out_ca2;
}


/*-------------------------------------------------
    pia6821_get_output_ca2_z - version of
    pia6821_get_output_ca2, which takes account of internal
    pullup resistor
-------------------------------------------------*/

int pia6821_get_output_ca2_z(running_device *device)
{
	pia6821_state *p = get_token(device);

	p->out_ca2_needs_pulled = FALSE;

	// If it's an output, output the bit, if it's an input, it's
	// pulled up
	return p->out_ca2 |
	       C2_INPUT(p->ctl_a);
}


/*-------------------------------------------------
    pia6821_portb_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( pia6821_portb_r )
{
	pia6821_state *p = get_token(device);

	return p->in_b;
}


/*-------------------------------------------------
    pia6821_portb_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( pia6821_portb_w )
{
	pia6821_state *p = get_token(device);

	assert_always(p->in_b_func.read == NULL, "pia_set_input_b() called when in_b_func implemented");

	LOG(("PIA #%s: set input port B = %02X\n", device->tag(), data));

	p->in_b = data;
	p->in_b_pushed = TRUE;
}


/*-------------------------------------------------
    pia6821_get_output_b
-------------------------------------------------*/

UINT8 pia6821_get_output_b(running_device *device)
{
	pia6821_state *p = get_token(device);

	p->out_b_needs_pulled = FALSE;

	return get_out_b_value(device);
}


/*-------------------------------------------------
    pia6821_cb1_r
-------------------------------------------------*/

READ_LINE_DEVICE_HANDLER( pia6821_cb1_r )
{
	pia6821_state *p = get_token(device);

	return p->in_cb1;
}


/*-------------------------------------------------
    pia6821_cb1_w
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( pia6821_cb1_w )
{
	pia6821_state *p = get_token(device);

	LOG(("PIA #%s: set input CB1 = %d\n", device->tag(), state));

	/* the new state has caused a transition */
	if ((p->in_cb1 != state) &&
		((state && C1_LOW_TO_HIGH(p->ctl_b)) || (!state && C1_HIGH_TO_LOW(p->ctl_b))))
	{
		LOG(("PIA #%s: CB1 triggering\n", device->tag()));

		/* mark the IRQ */
		p->irq_b1 = 1;

		/* update externals */
		update_interrupts(device);

		/* If CB2 is configured as a write-strobe output which is reset by a CB1
           transition, this reset will only happen when a read from port B implicitly
           clears the IRQ B1 flag.  So we handle the CB2 reset there.  Note that this
           is different from what happens with port A. */
	}

	/* set the new value for CB1 */
	p->in_cb1 = state;
	p->in_cb1_pushed = TRUE;
}


/*-------------------------------------------------
    pia6821_cb2_r
-------------------------------------------------*/

READ_LINE_DEVICE_HANDLER( pia6821_cb2_r )
{
	pia6821_state *p = get_token(device);

	return p->in_cb2;
}


/*-------------------------------------------------
    pia6821_cb2_w
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( pia6821_cb2_w )
{
	pia6821_state *p = get_token(device);

	LOG(("PIA #%s: set input CB2 = %d\n", device->tag(), state));

	/* if input mode and the new state has caused a transition */
	if (C2_INPUT(p->ctl_b) &&
		(p->in_cb2 != state) &&
		((state && C2_LOW_TO_HIGH(p->ctl_b)) || (!state && C2_HIGH_TO_LOW(p->ctl_b))))
	{
		LOG(("PIA #%s: CB2 triggering\n", device->tag()));

		/* mark the IRQ */
		p->irq_b2 = 1;

		/* update externals */
		update_interrupts(device);
	}

	/* set the new value for CA2 */
	p->in_cb2 = state;
	p->in_cb2_pushed = TRUE;
}


/*-------------------------------------------------
    pia6821_get_output_cb2
-------------------------------------------------*/

int pia6821_get_output_cb2(running_device *device)
{
	pia6821_state *p = get_token(device);

	p->out_cb2_needs_pulled = FALSE;

	return p->out_cb2;
}


/*-------------------------------------------------
    pia6821_get_output_cb2_z
-------------------------------------------------*/

int pia6821_get_output_cb2_z(running_device *device)
{
	pia6821_state *p = get_token(device);

	return !C2_OUTPUT(p->ctl_b);
}


/*-------------------------------------------------
    pia6821_get_irq_a
-------------------------------------------------*/

int pia6821_get_irq_a(running_device *device)
{
	pia6821_state *p = get_token(device);

	return p->irq_a_state;
}


/*-------------------------------------------------
    pia6821_get_irq_b
-------------------------------------------------*/

int pia6821_get_irq_b(running_device *device)
{
	pia6821_state *p = get_token(device);

	return p->irq_b_state;
}


/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)				p##pia6821##s
#define DEVTEMPLATE_FEATURES			DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME				"6821 PIA"
#define DEVTEMPLATE_FAMILY				"6821 PIA"
#include "devtempl.h"

#define DEVTEMPLATE_DERIVED_ID(p,s)		p##pia6822##s
#define DEVTEMPLATE_DERIVED_FEATURES	0
#define DEVTEMPLATE_DERIVED_NAME		"6822 PIA"
#include "devtempl.h"

DEFINE_LEGACY_DEVICE(PIA6821, pia6821);
DEFINE_LEGACY_DEVICE(PIA6822, pia6822);
