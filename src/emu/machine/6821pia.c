/**********************************************************************

    Motorola 6821 PIA interface and emulation

**********************************************************************/

#include "emu.h"
#include "6821pia.h"


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
    IMPLEMENTATION
***************************************************************************/

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  pia6821_device_config - constructor
//-------------------------------------------------

pia6821_device_config::pia6821_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
    : device_config(mconfig, static_alloc_device_config, "PIA6821", tag, owner, clock)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *pia6821_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
    return global_alloc(pia6821_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *pia6821_device_config::alloc_device(running_machine &machine) const
{
    return auto_alloc(&machine, pia6821_device(machine, *this));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void pia6821_device_config::device_config_complete()
{

}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pia6821_device - constructor
//-------------------------------------------------

pia6821_device::pia6821_device(running_machine &_machine, const pia6821_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{

}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pia6821_device::device_start()
{
	/* resolve callbacks */
    devcb_resolve_read8(&m_in_a_func, &m_config.m_in_a_func, this);
    devcb_resolve_read8(&m_in_b_func, &m_config.m_in_b_func, this);
    devcb_resolve_read_line(&m_in_ca1_func, &m_config.m_in_ca1_func, this);
    devcb_resolve_read_line(&m_in_cb1_func, &m_config.m_in_cb1_func, this);
    devcb_resolve_read_line(&m_in_ca2_func, &m_config.m_in_ca2_func, this);
    devcb_resolve_read_line(&m_in_cb2_func, &m_config.m_in_cb2_func, this);
    devcb_resolve_write8(&m_out_a_func, &m_config.m_out_a_func, this);
    devcb_resolve_write8(&m_out_b_func, &m_config.m_out_b_func, this);
    devcb_resolve_write_line(&m_out_ca2_func, &m_config.m_out_ca2_func, this);
    devcb_resolve_write_line(&m_out_cb2_func, &m_config.m_out_cb2_func, this);
    devcb_resolve_write_line(&m_irq_a_func, &m_config.m_irq_a_func, this);
    devcb_resolve_write_line(&m_irq_b_func, &m_config.m_irq_b_func, this);

    state_save_register_device_item(this, 0, m_in_a);
    state_save_register_device_item(this, 0, m_in_ca1);
    state_save_register_device_item(this, 0, m_in_ca2);
    state_save_register_device_item(this, 0, m_out_a);
    state_save_register_device_item(this, 0, m_out_ca2);
    state_save_register_device_item(this, 0, m_port_a_z_mask);
    state_save_register_device_item(this, 0, m_ddr_a);
    state_save_register_device_item(this, 0, m_ctl_a);
    state_save_register_device_item(this, 0, m_irq_a1);
    state_save_register_device_item(this, 0, m_irq_a2);
    state_save_register_device_item(this, 0, m_irq_a_state);
    state_save_register_device_item(this, 0, m_in_b);
    state_save_register_device_item(this, 0, m_in_cb1);
    state_save_register_device_item(this, 0, m_in_cb2);
    state_save_register_device_item(this, 0, m_out_b);
    state_save_register_device_item(this, 0, m_out_cb2);
    state_save_register_device_item(this, 0, m_last_out_cb2_z);
    state_save_register_device_item(this, 0, m_ddr_b);
    state_save_register_device_item(this, 0, m_ctl_b);
    state_save_register_device_item(this, 0, m_irq_b1);
    state_save_register_device_item(this, 0, m_irq_b2);
    state_save_register_device_item(this, 0, m_irq_b_state);
    state_save_register_device_item(this, 0, m_in_a_pushed);
    state_save_register_device_item(this, 0, m_out_a_needs_pulled);
    state_save_register_device_item(this, 0, m_in_ca1_pushed);
    state_save_register_device_item(this, 0, m_in_ca2_pushed);
    state_save_register_device_item(this, 0, m_out_ca2_needs_pulled);
    state_save_register_device_item(this, 0, m_in_b_pushed);
    state_save_register_device_item(this, 0, m_out_b_needs_pulled);
    state_save_register_device_item(this, 0, m_in_cb1_pushed);
    state_save_register_device_item(this, 0, m_in_cb2_pushed);
    state_save_register_device_item(this, 0, m_out_cb2_needs_pulled);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pia6821_device::device_reset()
{
	/*
     * set default read values.
     *
     * ports A,CA1,CA2 default to 1
     * ports B,CB1,CB2 are three-state and undefined (set to 0)
     */
	m_in_a = 0xff;
	m_in_ca1 = TRUE;
	m_in_ca2 = TRUE;
	m_out_a = 0;
	m_out_ca2 = 0;
	m_port_a_z_mask = 0;
	m_ddr_a = 0;
	m_ctl_a = 0;
	m_irq_a1 = 0;
	m_irq_a2 = 0;
	m_irq_a_state = 0;
	m_in_b = 0;
	m_in_cb1 = 0;
	m_in_cb2 = 0;
	m_out_b = 0;
	m_out_cb2 = 0;
	m_last_out_cb2_z = 0;
	m_ddr_b = 0;
	m_ctl_b = 0;
	m_irq_b1 = 0;
	m_irq_b2 = 0;
	m_irq_b_state = 0;
	m_in_a_pushed = 0;
	m_out_a_needs_pulled = 0;
	m_in_ca1_pushed = 0;
	m_in_ca2_pushed = 0;
	m_out_ca2_needs_pulled = 0;
	m_in_b_pushed = 0;
	m_out_b_needs_pulled = 0;
	m_in_cb1_pushed = 0;
	m_in_cb2_pushed = 0;
	m_out_cb2_needs_pulled = 0;
	m_logged_port_a_not_connected = 0;
	m_logged_port_b_not_connected = 0;
	m_logged_ca1_not_connected = 0;
	m_logged_ca2_not_connected = 0;
	m_logged_cb1_not_connected = 0;
	m_logged_cb2_not_connected = 0;


	/* clear the IRQs */
	devcb_call_write_line(&m_irq_a_func, FALSE);
	devcb_call_write_line(&m_irq_b_func, FALSE);
}


/*-------------------------------------------------
    update_interrupts
-------------------------------------------------*/

void pia6821_device::update_interrupts()
{
    /* start with IRQ A */
	int new_state = (m_irq_a1 && IRQ1_ENABLED(m_ctl_a)) || (m_irq_a2 && IRQ2_ENABLED(m_ctl_a));

	if (new_state != m_irq_a_state)
	{
		m_irq_a_state = new_state;
		devcb_call_write_line(&m_irq_a_func, m_irq_a_state);
	}

	/* then do IRQ B */
	new_state = (m_irq_b1 && IRQ1_ENABLED(m_ctl_b)) || (m_irq_b2 && IRQ2_ENABLED(m_ctl_b));

	if (new_state != m_irq_b_state)
	{
		m_irq_b_state = new_state;
		devcb_call_write_line(&m_irq_b_func, m_irq_b_state);
	}
}


/*-------------------------------------------------
    get_in_a_value
-------------------------------------------------*/

UINT8 pia6821_device::get_in_a_value()
{
	UINT8 port_a_data = 0;
	UINT8 ret;

	/* update the input */
	if (m_in_a_func.read != NULL)
    {
		port_a_data = devcb_call_read8(&m_in_a_func, 0);
    }
	else
	{
		if (m_in_a_pushed)
        {
			port_a_data = m_in_a;
        }
		else
		{
			/* mark all pins disconnected */
			m_port_a_z_mask = 0xff;

			if (!m_logged_port_a_not_connected && (m_ddr_a != 0xff))
			{
				logerror("PIA #%s: Warning! No port A read handler. Assuming pins 0x%02X not connected\n", tag(), m_ddr_a ^ 0xff);
				m_logged_port_a_not_connected = TRUE;
			}
		}
	}

	/* - connected pins are always read
       - disconnected pins read the output buffer in output mode
       - disconnected pins are HI in input mode */
	ret = (~m_port_a_z_mask             & port_a_data) |
	      ( m_port_a_z_mask &  m_ddr_a & m_out_a) |
	      ( m_port_a_z_mask & ~m_ddr_a);

	return ret;
}


/*-------------------------------------------------
    get_in_b_value
-------------------------------------------------*/

UINT8 pia6821_device::get_in_b_value()
{
	UINT8 ret;

	if (m_ddr_b == 0xff)
    {
		/* all output, just return buffer */
		ret = m_out_b;
    }
	else
	{
		UINT8 port_b_data;

		/* update the input */
		if (m_in_b_func.read != NULL)
        {
			port_b_data = devcb_call_read8(&m_in_b_func, 0);
        }
		else
		{
			if (m_in_b_pushed)
            {
				port_b_data = m_in_b;
            }
			else
			{
				if (!m_logged_port_b_not_connected && (m_ddr_b != 0xff))
				{
					logerror("PIA #%s: Error! No port B read handler. Three-state pins 0x%02X are undefined\n", tag(), m_ddr_b ^ 0xff);
					m_logged_port_b_not_connected = TRUE;
				}

				/* undefined -- need to return something */
				port_b_data = 0x00;
			}
		}

		/* the DDR determines if the pin or the output buffer is read */
		ret = (m_out_b & m_ddr_b) | (port_b_data & ~m_ddr_b);
	}

	return ret;
}


/*-------------------------------------------------
    get_out_a_value
-------------------------------------------------*/

UINT8 pia6821_device::get_out_a_value()
{
	UINT8 ret;

	if (m_ddr_a == 0xff)
    {
		/* all output */
		ret = m_out_a;
    }
	else
    {
		/* input pins don't change */
		ret = (m_out_a & m_ddr_a) | (get_in_a_value() & ~m_ddr_a);
    }

	return ret;
}


/*-------------------------------------------------
    get_out_b_value
-------------------------------------------------*/

UINT8 pia6821_device::get_out_b_value()
{
	/* input pins are high-impedance - we just send them as zeros for backwards compatibility */
	return m_out_b & m_ddr_b;
}


/*-------------------------------------------------
    set_out_ca2
-------------------------------------------------*/

void pia6821_device::set_out_ca2(int data)
{
	if (data != m_out_ca2)
	{
		m_out_ca2 = data;

		/* send to output function */
		if (m_out_ca2_func.write)
        {
			devcb_call_write_line(&m_out_ca2_func, m_out_ca2);
        }
		else
		{
			if (m_out_ca2_needs_pulled)
            {
				logerror("PIA #%s: Warning! No port CA2 write handler. Previous value has been lost!\n", tag());
            }

			m_out_ca2_needs_pulled = TRUE;
		}
	}
}


/*-------------------------------------------------
    set_out_cb2
-------------------------------------------------*/

void pia6821_device::set_out_cb2(int data)
{
	int z = pia6821_get_output_cb2_z(this);

	if ((data != m_out_cb2) || (z != m_last_out_cb2_z))
	{
		m_out_cb2 = data;
		m_last_out_cb2_z = z;

		/* send to output function */
		if (m_out_cb2_func.write)
        {
			devcb_call_write_line(&m_out_cb2_func, m_out_cb2);
        }
		else
		{
			if (m_out_cb2_needs_pulled)
            {
				logerror("PIA #%s: Warning! No port CB2 write handler. Previous value has been lost!\n", tag());
            }

			m_out_cb2_needs_pulled = TRUE;
		}
	}
}


/*-------------------------------------------------
    port_a_r
-------------------------------------------------*/

UINT8 pia6821_device::port_a_r()
{
	UINT8 ret = get_in_a_value();

	/* IRQ flags implicitly cleared by a read */
	m_irq_a1 = FALSE;
	m_irq_a2 = FALSE;
	update_interrupts();

	/* CA2 is configured as output and in read strobe mode */
	if(C2_OUTPUT(m_ctl_a) && C2_STROBE_MODE(m_ctl_a))
	{
		/* this will cause a transition low */
		set_out_ca2(FALSE);

		/* if the CA2 strobe is cleared by the E, reset it right away */
		if(STROBE_E_RESET(m_ctl_a))
        {
			set_out_ca2(TRUE);
        }
	}

	LOG(("PIA #%s: port A read = %02X\n", tag(), ret));

	return ret;
}


/*-------------------------------------------------
    ddr_a_r
-------------------------------------------------*/

UINT8 pia6821_device::ddr_a_r()
{
	UINT8 ret = m_ddr_a;

	LOG(("PIA #%s: DDR A read = %02X\n", tag(), ret));

	return ret;
}


/*-------------------------------------------------
    port_b_r
-------------------------------------------------*/

UINT8 pia6821_device::port_b_r()
{
	UINT8 ret = get_in_b_value();

	/* This read will implicitly clear the IRQ B1 flag.  If CB2 is in write-strobe
       mode with CB1 restore, and a CB1 active transition set the flag,
       clearing it will cause CB2 to go high again.  Note that this is different
       from what happens with port A. */
	if(m_irq_b1 && C2_STROBE_MODE(m_ctl_b) && STROBE_C1_RESET(m_ctl_b))
    {
		set_out_cb2(TRUE);
    }

	/* IRQ flags implicitly cleared by a read */
	m_irq_b1 = FALSE;
	m_irq_b2 = FALSE;
	update_interrupts();

	LOG(("PIA #%s: port B read = %02X\n", tag(), ret));

	return ret;
}


/*-------------------------------------------------
    ddr_b_r
-------------------------------------------------*/

UINT8 pia6821_device::ddr_b_r()
{
	UINT8 ret = m_ddr_b;

	LOG(("PIA #%s: DDR B read = %02X\n", tag(), ret));

	return ret;
}


/*-------------------------------------------------
    control_a_r
-------------------------------------------------*/

UINT8 pia6821_device::control_a_r()
{
	UINT8 ret;

	/* update CA1 & CA2 if callback exists, these in turn may update IRQ's */
	if (m_in_ca1_func.read != NULL)
    {
		ca1_w(devcb_call_read_line(&m_in_ca1_func));
    }
	else if(!m_logged_ca1_not_connected && (!m_in_ca1_pushed))
	{
		logerror("PIA #%s: Warning! No CA1 read handler. Assuming pin not connected\n", tag());
		m_logged_ca1_not_connected = TRUE;
	}

	if (m_in_ca2_func.read != NULL)
    {
		ca2_w(devcb_call_read_line(&m_in_ca2_func));
    }
	else if ( !m_logged_ca2_not_connected && C2_INPUT(m_ctl_a) && !m_in_ca2_pushed)
	{
		logerror("PIA #%s: Warning! No CA2 read handler. Assuming pin not connected\n", tag());
		m_logged_ca2_not_connected = TRUE;
	}

	/* read control register */
	ret = m_ctl_a;

	/* set the IRQ flags if we have pending IRQs */
	if(m_irq_a1)
    {
		ret |= PIA_IRQ1;
    }

	if(m_irq_a2 && C2_INPUT(m_ctl_a))
    {
		ret |= PIA_IRQ2;
    }

	LOG(("PIA #%s: control A read = %02X\n", tag(), ret));

	return ret;
}


/*-------------------------------------------------
    control_b_r
-------------------------------------------------*/

UINT8 pia6821_device::control_b_r()
{
	UINT8 ret;

	/* update CB1 & CB2 if callback exists, these in turn may update IRQ's */
	if(m_in_cb1_func.read != NULL)
    {
		cb1_w(devcb_call_read_line(&m_in_cb1_func));
    }
	else if(!m_logged_cb1_not_connected && !m_in_cb1_pushed)
	{
		logerror("PIA #%s: Error! no CB1 read handler. Three-state pin is undefined\n", tag());
		m_logged_cb1_not_connected = TRUE;
	}

	if(m_in_cb2_func.read != NULL)
    {
		cb2_w(devcb_call_read_line(&m_in_cb2_func));
    }
	else if(!m_logged_cb2_not_connected && C2_INPUT(m_ctl_b) && !m_in_cb2_pushed)
	{
		logerror("PIA #%s: Error! No CB2 read handler. Three-state pin is undefined\n", tag());
		m_logged_cb2_not_connected = TRUE;
	}

	/* read control register */
	ret = m_ctl_b;

	/* set the IRQ flags if we have pending IRQs */
	if(m_irq_b1)
    {
		ret |= PIA_IRQ1;
    }

	if(m_irq_b2 && C2_INPUT(m_ctl_b))
    {
		ret |= PIA_IRQ2;
    }

	LOG(("PIA #%s: control B read = %02X\n", tag(), ret));

	return ret;
}


/*-------------------------------------------------
    pia6821_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( pia6821_r )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->reg_r(offset);
}

UINT8 pia6821_device::reg_r(UINT8 offset)
{
	UINT8 ret;

	switch (offset & 0x03)
	{
		default: /* impossible */
		case 0x00:
			if (OUTPUT_SELECTED(m_ctl_a))
            {
				ret = port_a_r();
            }
			else
            {
				ret = ddr_a_r();
            }
			break;

		case 0x01:
			ret = control_a_r();
			break;

		case 0x02:
			if (OUTPUT_SELECTED(m_ctl_b))
            {
				ret = port_b_r();
            }
			else
            {
				ret = ddr_b_r();
            }
			break;

		case 0x03:
			ret = control_b_r();
			break;
	}

	return ret;
}


/*-------------------------------------------------
    pia6821_alt_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( pia6821_alt_r )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->reg_r(((offset << 1) & 0x02) | ((offset >> 1) & 0x01));
}



/*-------------------------------------------------
    pia6821_get_port_b_z_mask
-------------------------------------------------*/

UINT8 pia6821_get_port_b_z_mask(running_device *device)
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->get_port_b_z_mask();
}

UINT8 pia6821_device::get_port_b_z_mask()
{
	return ~m_ddr_b;
}


/*-------------------------------------------------
    send_to_out_a_func
-------------------------------------------------*/

void pia6821_device::send_to_out_a_func(const char* message)
{
	/* input pins are pulled high */
	UINT8 data = get_out_a_value();

	LOG(("PIA #%s: %s = %02X\n", tag(), message, data));

	if(m_out_a_func.write != NULL)
    {
		devcb_call_write8(&m_out_a_func, 0, data);
    }
	else
	{
		if(m_out_a_needs_pulled)
        {
			logerror("PIA #%s: Warning! No port A write handler. Previous value has been lost!\n", tag());
        }

		m_out_a_needs_pulled = TRUE;
	}
}


/*-------------------------------------------------
    send_to_out_b_func
-------------------------------------------------*/

void pia6821_device::send_to_out_b_func(const char* message)
{
	/* input pins are high-impedance - we just send them as zeros for backwards compatibility */
	UINT8 data = get_out_b_value();

	LOG(("PIA #%s: %s = %02X\n", tag(), message, data));

	if(m_out_b_func.write != NULL)
    {
		devcb_call_write8(&m_out_b_func, 0, data);
    }
	else
	{
		if(m_out_b_needs_pulled)
        {
			logerror("PIA #%s: Warning! No port B write handler. Previous value has been lost!\n", tag());
        }

		m_out_b_needs_pulled = TRUE;
	}
}


/*-------------------------------------------------
    port_a_w
-------------------------------------------------*/

void pia6821_device::port_a_w(UINT8 data)
{
	/* buffer the output value */
	m_out_a = data;

	send_to_out_a_func("port A write");
}


/*-------------------------------------------------
    ddr_a_w
-------------------------------------------------*/

void pia6821_device::ddr_a_w(UINT8 data)
{
	if(data == 0x00)
    {
		LOG(("PIA #%s: DDR A write = %02X (input mode)\n", tag(), data));
    }
	else if(data == 0xff)
    {
		LOG(("PIA #%s: DDR A write = %02X (output mode)\n", tag(), data));
    }
	else
    {
		LOG(("PIA #%s: DDR A write = %02X (mixed mode)\n", tag(), data));
    }

	if(m_ddr_a != data)
	{
		/* DDR changed, call the callback again */
		m_ddr_a = data;
		m_logged_port_a_not_connected = FALSE;
		send_to_out_a_func("port A write due to DDR change");
	}
}


/*-------------------------------------------------
    port_b_w
-------------------------------------------------*/

void pia6821_device::port_b_w(UINT8 data)
{
	/* buffer the output value */
	m_out_b = data;

	send_to_out_b_func("port B write");

	/* CB2 in write strobe mode */
	if(C2_STROBE_MODE(m_ctl_b))
	{
		/* this will cause a transition low */
		set_out_cb2(FALSE);

		/* if the CB2 strobe is cleared by the E, reset it right away */
		if(STROBE_E_RESET(m_ctl_b))
        {
			set_out_cb2(TRUE);
        }
	}
}


/*-------------------------------------------------
    ddr_b_w
-------------------------------------------------*/

void pia6821_device::ddr_b_w(UINT8 data)
{
	if (data == 0x00)
    {
		LOG(("PIA #%s: DDR B write = %02X (input mode)\n", tag(), data));
    }
	else if (data == 0xff)
    {
		LOG(("PIA #%s: DDR B write = %02X (output mode)\n", tag(), data));
    }
	else
    {
		LOG(("PIA #%s: DDR B write = %02X (mixed mode)\n", tag(), data));
    }

	if(m_ddr_b != data)
	{
		/* DDR changed, call the callback again */
		m_ddr_b = data;
		m_logged_port_b_not_connected = FALSE;
		send_to_out_b_func("port B write due to DDR change");
	}
}


/*-------------------------------------------------
    control_a_w
-------------------------------------------------*/

void pia6821_device::control_a_w(UINT8 data)
{
	/* bit 7 and 6 are read only */
	data &= 0x3f;

	LOG(("PIA #%s: control A write = %02X\n", tag(), data));

	/* update the control register */
	m_ctl_a = data;

	/* CA2 is configured as output */
	if(C2_OUTPUT(m_ctl_a))
	{
		int temp;

		if(C2_SET_MODE(m_ctl_a))
        {
			/* set/reset mode - bit value determines the new output */
			temp = C2_SET(m_ctl_a);
        }
		else
        {
			/* strobe mode - output is always high unless strobed */
			temp = TRUE;
        }

		set_out_ca2(temp);
	}

	/* update externals */
	update_interrupts();
}


/*-------------------------------------------------
    control_b_w
-------------------------------------------------*/

void pia6821_device::control_b_w(UINT8 data)
{
	int temp;

	/* bit 7 and 6 are read only */
	data &= 0x3f;

	LOG(("PIA #%s: control B write = %02X\n", tag(), data));

	/* update the control register */
	m_ctl_b = data;

	if (C2_SET_MODE(m_ctl_b))
    {
		/* set/reset mode - bit value determines the new output */
		temp = C2_SET(m_ctl_b);
    }
	else
    {
		/* strobe mode - output is always high unless strobed */
		temp = TRUE;
    }

	set_out_cb2(temp);

	/* update externals */
	update_interrupts();
}


/*-------------------------------------------------
    pia6821_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( pia6821_w )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    dev->reg_w(offset, data);
}

void pia6821_device::reg_w(UINT8 offset, UINT8 data)
{
	switch (offset & 0x03)
	{
		default: /* impossible */
		case 0x00:
			if (OUTPUT_SELECTED(m_ctl_a))
            {
				port_a_w(data);
            }
			else
            {
				ddr_a_w(data);
            }
			break;

		case 0x01:
			control_a_w( data);
			break;

		case 0x02:
			if(OUTPUT_SELECTED(m_ctl_b))
            {
				port_b_w(data);
            }
			else
            {
				ddr_b_w(data);
            }
			break;

		case 0x03:
			control_b_w(data);
			break;
	}
}


/*-------------------------------------------------
    pia6821_alt_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( pia6821_alt_w )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    dev->reg_w(((offset << 1) & 0x02) | ((offset >> 1) & 0x01), data);
}


/*-------------------------------------------------
    pia6821_set_port_a_z_mask
-------------------------------------------------*/

void pia6821_set_port_a_z_mask(running_device *device, UINT8 data)
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    dev->set_port_a_z_mask(data);
}

void pia6821_device::set_port_a_z_mask(UINT8 data)
{
	m_port_a_z_mask = data;
}


/*-------------------------------------------------
    pia6821_porta_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( pia6821_porta_r )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->porta_r();
}

UINT8 pia6821_device::porta_r()
{
	return m_in_a;
}


/*-------------------------------------------------
    pia6821_set_input_a
-------------------------------------------------*/

void pia6821_set_input_a(running_device *device, UINT8 data, UINT8 z_mask)
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    dev->set_input_a(data, z_mask);
}

void pia6821_device::set_input_a(UINT8 data, UINT8 z_mask)
{
	assert_always(m_in_a_func.read == NULL, "pia6821_porta_w() called when in_a_func implemented");

	LOG(("PIA #%s: set input port A = %02X\n", tag(), data));

	m_in_a = data;
	m_port_a_z_mask = z_mask;
	m_in_a_pushed = TRUE;
}


/*-------------------------------------------------
    pia6821_porta_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( pia6821_porta_w )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    dev->porta_w(data);
}

void pia6821_device::porta_w(UINT8 data)
{
	pia6821_set_input_a(this, data, 0);
}


/*-------------------------------------------------
    pia6821_get_output_a
-------------------------------------------------*/

UINT8 pia6821_get_output_a(running_device *device)
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->get_output_a();
}

UINT8 pia6821_device::get_output_a()
{
	m_out_a_needs_pulled = FALSE;

	return get_out_a_value();
}


/*-------------------------------------------------
    pia6821_ca1_r
-------------------------------------------------*/

READ_LINE_DEVICE_HANDLER( pia6821_ca1_r )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->ca1_r();
}

UINT8 pia6821_device::ca1_r()
{
	return m_in_ca1;
}


/*-------------------------------------------------
    pia6821_ca1_w
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( pia6821_ca1_w )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    dev->ca1_w(state);
}

void pia6821_device::ca1_w(UINT8 state)
{
	LOG(("PIA #%s: set input CA1 = %d\n", tag(), state));

	/* the new state has caused a transition */
	if((m_in_ca1 != state) && ((state && C1_LOW_TO_HIGH(m_ctl_a)) || (!state && C1_HIGH_TO_LOW(m_ctl_a))))
	{
		LOG(("PIA #%s: CA1 triggering\n", tag()));

		/* mark the IRQ */
		m_irq_a1 = TRUE;

		/* update externals */
		update_interrupts();

		/* CA2 is configured as output and in read strobe mode and cleared by a CA1 transition */
		if(C2_OUTPUT(m_ctl_a) && C2_STROBE_MODE(m_ctl_a) && STROBE_C1_RESET(m_ctl_a))
        {
			set_out_ca2(TRUE);
        }
	}

	/* set the new value for CA1 */
	m_in_ca1 = state;
	m_in_ca1_pushed = TRUE;
}


/*-------------------------------------------------
    pia6821_ca2_r
-------------------------------------------------*/

READ_LINE_DEVICE_HANDLER( pia6821_ca2_r )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->ca2_r();
}

UINT8 pia6821_device::ca2_r()
{
	return m_in_ca2;
}


/*-------------------------------------------------
    pia6821_ca2_w
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( pia6821_ca2_w )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    dev->ca2_w(state);
}

void pia6821_device::ca2_w(UINT8 state)
{
	LOG(("PIA #%s: set input CA2 = %d\n", tag(), state));

	/* if input mode and the new state has caused a transition */
	if(C2_INPUT(m_ctl_a) && (m_in_ca2 != state) && ((state && C2_LOW_TO_HIGH(m_ctl_a)) || (!state && C2_HIGH_TO_LOW(m_ctl_a))))
	{
		LOG(("PIA #%s: CA2 triggering\n", tag()));

		/* mark the IRQ */
		m_irq_a2 = TRUE;

		/* update externals */
		update_interrupts();
	}

	/* set the new value for CA2 */
	m_in_ca2 = state;
	m_in_ca2_pushed = TRUE;
}


/*-------------------------------------------------
    pia6821_get_output_ca2
-------------------------------------------------*/

int pia6821_get_output_ca2(running_device *device)
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->get_output_ca2();
}

int pia6821_device::get_output_ca2()
{
	m_out_ca2_needs_pulled = FALSE;

	return m_out_ca2;
}


/*-------------------------------------------------
    pia6821_get_output_ca2_z - version of
    pia6821_get_output_ca2, which takes account of internal
    pullup resistor
-------------------------------------------------*/

int pia6821_get_output_ca2_z(running_device *device)
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->get_output_ca2_z();
}

int pia6821_device::get_output_ca2_z()
{
	m_out_ca2_needs_pulled = FALSE;

	// If it's an output, output the bit, if it's an input, it's
	// pulled up
	return m_out_ca2 | C2_INPUT(m_ctl_a);
}


/*-------------------------------------------------
    pia6821_portb_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( pia6821_portb_r )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->portb_r();
}

UINT8 pia6821_device::portb_r()
{
	return m_in_b;
}


/*-------------------------------------------------
    pia6821_portb_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( pia6821_portb_w )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    dev->portb_w(data);
}

void pia6821_device::portb_w(UINT8 data)
{
	assert_always(m_in_b_func.read == NULL, "pia_set_input_b() called when in_b_func implemented");

	LOG(("PIA #%s: set input port B = %02X\n", tag(), data));

	m_in_b = data;
	m_in_b_pushed = TRUE;
}


/*-------------------------------------------------
    pia6821_get_output_b
-------------------------------------------------*/

UINT8 pia6821_get_output_b(running_device *device)
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->get_output_b();
}

UINT8 pia6821_device::get_output_b()
{
	m_out_b_needs_pulled = FALSE;

	return get_out_b_value();
}


/*-------------------------------------------------
    pia6821_cb1_r
-------------------------------------------------*/

READ_LINE_DEVICE_HANDLER( pia6821_cb1_r )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->cb1_r();
}

UINT8 pia6821_device::cb1_r()
{
	return m_in_cb1;
}


/*-------------------------------------------------
    pia6821_cb1_w
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( pia6821_cb1_w )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    dev->cb1_w(state);
}

void pia6821_device::cb1_w(UINT8 state)
{
	LOG(("PIA #%s: set input CB1 = %d\n", tag(), state));

	/* the new state has caused a transition */
	if((m_in_cb1 != state) && ((state && C1_LOW_TO_HIGH(m_ctl_b)) || (!state && C1_HIGH_TO_LOW(m_ctl_b))))
	{
		LOG(("PIA #%s: CB1 triggering\n", tag()));

		/* mark the IRQ */
		m_irq_b1 = 1;

		/* update externals */
		update_interrupts();

		/* If CB2 is configured as a write-strobe output which is reset by a CB1
           transition, this reset will only happen when a read from port B implicitly
           clears the IRQ B1 flag.  So we handle the CB2 reset there.  Note that this
           is different from what happens with port A. */
	}

	/* set the new value for CB1 */
	m_in_cb1 = state;
	m_in_cb1_pushed = TRUE;
}


/*-------------------------------------------------
    pia6821_cb2_r
-------------------------------------------------*/

READ_LINE_DEVICE_HANDLER( pia6821_cb2_r )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->cb2_r();
}

UINT8 pia6821_device::cb2_r()
{
	return m_in_cb2;
}


/*-------------------------------------------------
    pia6821_cb2_w
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( pia6821_cb2_w )
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    dev->cb2_w(state);
}

void pia6821_device::cb2_w(UINT8 state)
{
	LOG(("PIA #%s: set input CB2 = %d\n", tag(), state));

	/* if input mode and the new state has caused a transition */
	if (C2_INPUT(m_ctl_b) &&
		(m_in_cb2 != state) &&
		((state && C2_LOW_TO_HIGH(m_ctl_b)) || (!state && C2_HIGH_TO_LOW(m_ctl_b))))
	{
		LOG(("PIA #%s: CB2 triggering\n", tag()));

		/* mark the IRQ */
		m_irq_b2 = 1;

		/* update externals */
		update_interrupts();
	}

	/* set the new value for CA2 */
	m_in_cb2 = state;
	m_in_cb2_pushed = TRUE;
}


/*-------------------------------------------------
    pia6821_get_output_cb2
-------------------------------------------------*/

int pia6821_get_output_cb2(running_device *device)
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->get_output_cb2();
}

int pia6821_device::get_output_cb2()
{
	m_out_cb2_needs_pulled = FALSE;

	return m_out_cb2;
}


/*-------------------------------------------------
    pia6821_get_output_cb2_z
-------------------------------------------------*/

int pia6821_get_output_cb2_z(running_device *device)
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->get_output_cb2_z();
}

int pia6821_device::get_output_cb2_z()
{
	return !C2_OUTPUT(m_ctl_b);
}


/*-------------------------------------------------
    pia6821_get_irq_a
-------------------------------------------------*/

int pia6821_get_irq_a(running_device *device)
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->get_irq_a();
}

int pia6821_device::get_irq_a()
{
	return m_irq_a_state;
}


/*-------------------------------------------------
    pia6821_get_irq_b
-------------------------------------------------*/

int pia6821_get_irq_b(running_device *device)
{
    pia6821_device *dev = reinterpret_cast<pia6821_device *>(device);
    return dev->get_irq_b();
}

int pia6821_device::get_irq_b()
{
	return m_irq_b_state;
}

const device_type PIA6821 = pia6821_device_config::static_alloc_device_config;
