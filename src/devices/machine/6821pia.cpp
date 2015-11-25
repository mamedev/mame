// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/**********************************************************************

    Motorola 6821 PIA interface and emulation

**********************************************************************/

#include "emu.h"
#include "6821pia.h"


//**************************************************************************
//  MACROS
//**************************************************************************

#define VERBOSE 1

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

#define PIA_IRQ1                (0x80)
#define PIA_IRQ2                (0x40)

#define IRQ1_ENABLED(c)         ( (((c) >> 0) & 0x01))
#define C1_LOW_TO_HIGH(c)       ( (((c) >> 1) & 0x01))
#define C1_HIGH_TO_LOW(c)       (!(((c) >> 1) & 0x01))
#define OUTPUT_SELECTED(c)      ( (((c) >> 2) & 0x01))
#define IRQ2_ENABLED(c)         ( (((c) >> 3) & 0x01))
#define STROBE_E_RESET(c)       ( (((c) >> 3) & 0x01))
#define STROBE_C1_RESET(c)      (!(((c) >> 3) & 0x01))
#define C2_SET(c)               ( (((c) >> 3) & 0x01))
#define C2_LOW_TO_HIGH(c)       ( (((c) >> 4) & 0x01))
#define C2_HIGH_TO_LOW(c)       (!(((c) >> 4) & 0x01))
#define C2_SET_MODE(c)          ( (((c) >> 4) & 0x01))
#define C2_STROBE_MODE(c)       (!(((c) >> 4) & 0x01))
#define C2_OUTPUT(c)            ( (((c) >> 5) & 0x01))
#define C2_INPUT(c)             (!(((c) >> 5) & 0x01))


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type PIA6821 = &device_creator<pia6821_device>;


//-------------------------------------------------
//  pia6821_device - constructor
//-------------------------------------------------

pia6821_device::pia6821_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PIA6821, "6821 PIA", tag, owner, clock, "pia6821", __FILE__),
		m_in_a_handler(*this),
		m_in_b_handler(*this),
		m_in_ca1_handler(*this),
		m_in_cb1_handler(*this),
		m_in_ca2_handler(*this),
		m_out_a_handler(*this),
		m_out_b_handler(*this),
		m_ca2_handler(*this),
		m_cb2_handler(*this),
		m_irqa_handler(*this),
		m_irqb_handler(*this), m_in_a(0),
		m_in_ca1(0), m_in_ca2(0), m_out_a(0), m_out_ca2(0), m_port_a_z_mask(0), m_ddr_a(0),
		m_ctl_a(0), m_irq_a1(0), m_irq_a2(0),
		m_irq_a_state(0), m_in_b(0),
		m_in_cb1(0), m_in_cb2(0), m_out_b(0), m_out_cb2(0), m_last_out_cb2_z(0), m_ddr_b(0),
		m_ctl_b(0), m_irq_b1(0), m_irq_b2(0),
		m_irq_b_state(0), m_in_a_pushed(false), m_out_a_needs_pulled(false), m_in_ca1_pushed(false),
		m_in_ca2_pushed(false), m_out_ca2_needs_pulled(false), m_in_b_pushed(false), m_out_b_needs_pulled(false),
		m_in_cb1_pushed(false), m_in_cb2_pushed(false), m_out_cb2_needs_pulled(false), m_logged_port_a_not_connected(false),
		m_logged_port_b_not_connected(false), m_logged_ca1_not_connected(false), m_logged_ca2_not_connected(false),
		m_logged_cb1_not_connected(false), m_logged_cb2_not_connected(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pia6821_device::device_start()
{
	// resolve callbacks
	m_in_a_handler.resolve();
	m_in_b_handler.resolve();
	m_in_ca1_handler.resolve();
	m_in_cb1_handler.resolve();
	m_in_ca2_handler.resolve();
	m_out_a_handler.resolve();
	m_out_b_handler.resolve();
	m_ca2_handler.resolve();
	m_cb2_handler.resolve();
	m_irqa_handler.resolve_safe();
	m_irqb_handler.resolve_safe();

	save_item(NAME(m_in_a));
	save_item(NAME(m_in_ca1));
	save_item(NAME(m_in_ca2));
	save_item(NAME(m_out_a));
	save_item(NAME(m_out_ca2));
	save_item(NAME(m_port_a_z_mask));
	save_item(NAME(m_ddr_a));
	save_item(NAME(m_ctl_a));
	save_item(NAME(m_irq_a1));
	save_item(NAME(m_irq_a2));
	save_item(NAME(m_irq_a_state));
	save_item(NAME(m_in_b));
	save_item(NAME(m_in_cb1));
	save_item(NAME(m_in_cb2));
	save_item(NAME(m_out_b));
	save_item(NAME(m_out_cb2));
	save_item(NAME(m_last_out_cb2_z));
	save_item(NAME(m_ddr_b));
	save_item(NAME(m_ctl_b));
	save_item(NAME(m_irq_b1));
	save_item(NAME(m_irq_b2));
	save_item(NAME(m_irq_b_state));
	save_item(NAME(m_in_a_pushed));
	save_item(NAME(m_out_a_needs_pulled));
	save_item(NAME(m_in_ca1_pushed));
	save_item(NAME(m_in_ca2_pushed));
	save_item(NAME(m_out_ca2_needs_pulled));
	save_item(NAME(m_in_b_pushed));
	save_item(NAME(m_out_b_needs_pulled));
	save_item(NAME(m_in_cb1_pushed));
	save_item(NAME(m_in_cb2_pushed));
	save_item(NAME(m_out_cb2_needs_pulled));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pia6821_device::device_reset()
{
	//
	// set default read values.
	//
	// ports A,CA1,CA2 default to 1
	// ports B,CB1,CB2 are three-state and undefined (set to 0)
	//
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
	m_in_a_pushed = false;
	m_out_a_needs_pulled = false;
	m_in_ca1_pushed = false;
	m_in_ca2_pushed = false;
	m_out_ca2_needs_pulled = false;
	m_in_b_pushed = false;
	m_out_b_needs_pulled = false;
	m_in_cb1_pushed = false;
	m_in_cb2_pushed = false;
	m_out_cb2_needs_pulled = false;
	m_logged_port_a_not_connected = false;
	m_logged_port_b_not_connected = false;
	m_logged_ca1_not_connected = false;
	m_logged_ca2_not_connected = false;
	m_logged_cb1_not_connected = false;
	m_logged_cb2_not_connected = false;


	// clear the IRQs
	m_irqa_handler(FALSE);
	m_irqb_handler(FALSE);
}


//-------------------------------------------------
//  update_interrupts
//-------------------------------------------------

void pia6821_device::update_interrupts()
{
	// start with IRQ A
	int new_state = (m_irq_a1 && IRQ1_ENABLED(m_ctl_a)) || (m_irq_a2 && IRQ2_ENABLED(m_ctl_a));

	if (new_state != m_irq_a_state)
	{
		m_irq_a_state = new_state;
		m_irqa_handler(m_irq_a_state);
	}

	// then do IRQ B
	new_state = (m_irq_b1 && IRQ1_ENABLED(m_ctl_b)) || (m_irq_b2 && IRQ2_ENABLED(m_ctl_b));

	if (new_state != m_irq_b_state)
	{
		m_irq_b_state = new_state;
		m_irqb_handler(m_irq_b_state);
	}
}


//-------------------------------------------------
//  get_in_a_value
//-------------------------------------------------

UINT8 pia6821_device::get_in_a_value()
{
	UINT8 port_a_data = 0;
	UINT8 ret;

	// update the input
	if (!m_in_a_handler.isnull())
	{
		port_a_data = m_in_a_handler(0);
	}
	else
	{
		if (m_in_a_pushed)
		{
			port_a_data = m_in_a;
		}
		else
		{
			// mark all pins disconnected
			m_port_a_z_mask = 0xff;

			if (!m_logged_port_a_not_connected && (m_ddr_a != 0xff))
			{
				logerror("PIA #%s: Warning! No port A read handler. Assuming pins 0x%02X not connected\n", tag(), m_ddr_a ^ 0xff);
				m_logged_port_a_not_connected = true;
			}
		}
	}

	// - connected pins are always read
	// - disconnected pins read the output buffer in output mode
	// - disconnected pins are HI in input mode
	ret = (~m_port_a_z_mask             & port_a_data) |
			( m_port_a_z_mask &  m_ddr_a & m_out_a) |
			( m_port_a_z_mask & ~m_ddr_a);

	return ret;
}


//-------------------------------------------------
//  get_in_b_value
//-------------------------------------------------

UINT8 pia6821_device::get_in_b_value()
{
	UINT8 ret;

	if (m_ddr_b == 0xff)
	{
		// all output, just return buffer
		ret = m_out_b;
	}
	else
	{
		UINT8 port_b_data;

		// update the input
		if (!m_in_b_handler.isnull())
		{
			port_b_data = m_in_b_handler(0);
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
					m_logged_port_b_not_connected = true;
				}

				// undefined -- need to return something
				port_b_data = 0x00;
			}
		}

		// the DDR determines if the pin or the output buffer is read
		ret = (m_out_b & m_ddr_b) | (port_b_data & ~m_ddr_b);
	}

	return ret;
}


//-------------------------------------------------
//  get_out_a_value
//-------------------------------------------------

UINT8 pia6821_device::get_out_a_value()
{
	UINT8 ret;

	if (m_ddr_a == 0xff)
	{
		// all output
		ret = m_out_a;
	}
	else
	{
		// input pins don't change
		ret = (m_out_a & m_ddr_a) | (get_in_a_value() & ~m_ddr_a);
	}

	return ret;
}


//-------------------------------------------------
//  get_out_b_value
//-------------------------------------------------

UINT8 pia6821_device::get_out_b_value()
{
	// input pins are high-impedance - we just send them as zeros for backwards compatibility
	return m_out_b & m_ddr_b;
}


//-------------------------------------------------
//  set_out_ca2
//-------------------------------------------------

void pia6821_device::set_out_ca2(int data)
{
	if (data != m_out_ca2)
	{
		m_out_ca2 = data;

		// send to output function
		if (!m_ca2_handler.isnull())
		{
			m_ca2_handler(m_out_ca2);
		}
		else
		{
			if (m_out_ca2_needs_pulled)
			{
				logerror("PIA #%s: Warning! No port CA2 write handler. Previous value has been lost!\n", tag());
			}

			m_out_ca2_needs_pulled = true;
		}
	}
}


//-------------------------------------------------
//  set_out_cb2
//-------------------------------------------------

void pia6821_device::set_out_cb2(int data)
{
	int z = cb2_output_z();

	if ((data != m_out_cb2) || (z != m_last_out_cb2_z))
	{
		m_out_cb2 = data;
		m_last_out_cb2_z = z;

		// send to output function
		if (!m_cb2_handler.isnull())
		{
			m_cb2_handler(m_out_cb2);
		}
		else
		{
			if (m_out_cb2_needs_pulled)
			{
				logerror("PIA #%s: Warning! No port CB2 write handler. Previous value has been lost!\n", tag());
			}

			m_out_cb2_needs_pulled = true;
		}
	}
}


//-------------------------------------------------
//  port_a_r
//-------------------------------------------------

UINT8 pia6821_device::port_a_r()
{
	UINT8 ret = get_in_a_value();

	// IRQ flags implicitly cleared by a read
	m_irq_a1 = FALSE;
	m_irq_a2 = FALSE;
	update_interrupts();

	// CA2 is configured as output and in read strobe mode
	if(C2_OUTPUT(m_ctl_a) && C2_STROBE_MODE(m_ctl_a))
	{
		// this will cause a transition low
		set_out_ca2(FALSE);

		// if the CA2 strobe is cleared by the E, reset it right away
		if(STROBE_E_RESET(m_ctl_a))
		{
			set_out_ca2(TRUE);
		}
	}

	LOG(("PIA #%s: port A read = %02X\n", tag(), ret));

	return ret;
}


//-------------------------------------------------
//  ddr_a_r
//-------------------------------------------------

UINT8 pia6821_device::ddr_a_r()
{
	UINT8 ret = m_ddr_a;

	LOG(("PIA #%s: DDR A read = %02X\n", tag(), ret));

	return ret;
}


//-------------------------------------------------
//  port_b_r
//-------------------------------------------------

UINT8 pia6821_device::port_b_r()
{
	UINT8 ret = get_in_b_value();

	// This read will implicitly clear the IRQ B1 flag.  If CB2 is in write-strobe
	// mode with CB1 restore, and a CB1 active transition set the flag,
	// clearing it will cause CB2 to go high again.  Note that this is different
	// from what happens with port A.
	if(m_irq_b1 && C2_STROBE_MODE(m_ctl_b) && STROBE_C1_RESET(m_ctl_b))
	{
		set_out_cb2(TRUE);
	}

	// IRQ flags implicitly cleared by a read
	m_irq_b1 = FALSE;
	m_irq_b2 = FALSE;
	update_interrupts();

	LOG(("PIA #%s: port B read = %02X\n", tag(), ret));

	return ret;
}


//-------------------------------------------------
//  ddr_b_r
//-------------------------------------------------

UINT8 pia6821_device::ddr_b_r()
{
	UINT8 ret = m_ddr_b;

	LOG(("PIA #%s: DDR B read = %02X\n", tag(), ret));

	return ret;
}


//-------------------------------------------------
//  control_a_r
//-------------------------------------------------

UINT8 pia6821_device::control_a_r()
{
	UINT8 ret;

	// update CA1 & CA2 if callback exists, these in turn may update IRQ's
	if (!m_in_ca1_handler.isnull())
	{
		ca1_w(m_in_ca1_handler());
	}
	else if(!m_logged_ca1_not_connected && (!m_in_ca1_pushed))
	{
		logerror("PIA #%s: Warning! No CA1 read handler. Assuming pin not connected\n", tag());
		m_logged_ca1_not_connected = true;
	}

	if (!m_in_ca2_handler.isnull())
	{
		ca2_w(m_in_ca2_handler());
	}
	else if ( !m_logged_ca2_not_connected && C2_INPUT(m_ctl_a) && !m_in_ca2_pushed)
	{
		logerror("PIA #%s: Warning! No CA2 read handler. Assuming pin not connected\n", tag());
		m_logged_ca2_not_connected = true;
	}

	// read control register
	ret = m_ctl_a;

	// set the IRQ flags if we have pending IRQs
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


//-------------------------------------------------
//  control_b_r
//-------------------------------------------------

UINT8 pia6821_device::control_b_r()
{
	UINT8 ret;

	// update CB1 & CB2 if callback exists, these in turn may update IRQ's
	if(!m_in_cb1_handler.isnull())
	{
		cb1_w(m_in_cb1_handler());
	}
	else if(!m_logged_cb1_not_connected && !m_in_cb1_pushed)
	{
		logerror("PIA #%s: Error! no CB1 read handler. Three-state pin is undefined\n", tag());
		m_logged_cb1_not_connected = true;
	}

	if(!m_logged_cb2_not_connected && C2_INPUT(m_ctl_b) && !m_in_cb2_pushed)
	{
		logerror("PIA #%s: Error! Three-state pin is undefined\n", tag());
		m_logged_cb2_not_connected = true;
	}

	// read control register
	ret = m_ctl_b;

	// set the IRQ flags if we have pending IRQs
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


//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 pia6821_device::reg_r(UINT8 offset)
{
	UINT8 ret;

	switch (offset & 0x03)
	{
		default: // impossible
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



//-------------------------------------------------
//  send_to_out_a_func
//-------------------------------------------------

void pia6821_device::send_to_out_a_func(const char* message)
{
	// input pins are pulled high
	UINT8 data = get_out_a_value();

	LOG(("PIA #%s: %s = %02X\n", tag(), message, data));

	if(!m_out_a_handler.isnull())
	{
		m_out_a_handler((offs_t) 0, data);
	}
	else
	{
		if(m_out_a_needs_pulled)
		{
			logerror("PIA #%s: Warning! No port A write handler. Previous value has been lost!\n", tag());
		}

		m_out_a_needs_pulled = true;
	}
}


//-------------------------------------------------
//  send_to_out_b_func
//-------------------------------------------------

void pia6821_device::send_to_out_b_func(const char* message)
{
	// input pins are high-impedance - we just send them as zeros for backwards compatibility
	UINT8 data = get_out_b_value();

	LOG(("PIA #%s: %s = %02X\n", tag(), message, data));

	if(!m_out_b_handler.isnull())
	{
		m_out_b_handler((offs_t)0, data);
	}
	else
	{
		if(m_out_b_needs_pulled)
		{
			logerror("PIA #%s: Warning! No port B write handler. Previous value has been lost!\n", tag());
		}

		m_out_b_needs_pulled = true;
	}
}


//-------------------------------------------------
//  port_a_w
//-------------------------------------------------

void pia6821_device::port_a_w(UINT8 data)
{
	// buffer the output value
	m_out_a = data;

	send_to_out_a_func("port A write");
}


//-------------------------------------------------
//  ddr_a_w
//-------------------------------------------------

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
		// DDR changed, call the callback again
		m_ddr_a = data;
		m_logged_port_a_not_connected = false;
		send_to_out_a_func("port A write due to DDR change");
	}
}


//-------------------------------------------------
//  port_b_w
//-------------------------------------------------

void pia6821_device::port_b_w(UINT8 data)
{
	// buffer the output value
	m_out_b = data;

	send_to_out_b_func("port B write");

	// CB2 in write strobe mode
	if(C2_STROBE_MODE(m_ctl_b))
	{
		// this will cause a transition low
		set_out_cb2(FALSE);

		// if the CB2 strobe is cleared by the E, reset it right away
		if(STROBE_E_RESET(m_ctl_b))
		{
			set_out_cb2(TRUE);
		}
	}
}


//-------------------------------------------------
//  ddr_b_w
//-------------------------------------------------

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
		// DDR changed, call the callback again
		m_ddr_b = data;
		m_logged_port_b_not_connected = false;
		send_to_out_b_func("port B write due to DDR change");
	}
}


//-------------------------------------------------
//  control_a_w
//-------------------------------------------------

void pia6821_device::control_a_w(UINT8 data)
{
	// bit 7 and 6 are read only
	data &= 0x3f;

	LOG(("PIA #%s: control A write = %02X\n", tag(), data));

	// update the control register
	m_ctl_a = data;

	// CA2 is configured as output
	if(C2_OUTPUT(m_ctl_a))
	{
		int temp;

		if(C2_SET_MODE(m_ctl_a))
		{
			// set/reset mode - bit value determines the new output
			temp = C2_SET(m_ctl_a);
		}
		else
		{
			// strobe mode - output is always high unless strobed
			temp = TRUE;
		}

		set_out_ca2(temp);
	}

	// update externals
	update_interrupts();
}


//-------------------------------------------------
//  control_b_w
//-------------------------------------------------

void pia6821_device::control_b_w(UINT8 data)
{
	int temp;

	// bit 7 and 6 are read only
	data &= 0x3f;

	LOG(("PIA #%s: control B write = %02X\n", tag(), data));

	// update the control register
	m_ctl_b = data;

	if (C2_SET_MODE(m_ctl_b))
	{
		// set/reset mode - bit value determines the new output
		temp = C2_SET(m_ctl_b);
	}
	else
	{
		// strobe mode - output is always high unless strobed
		temp = TRUE;
	}

	set_out_cb2(temp);

	// update externals
	update_interrupts();
}


//-------------------------------------------------
//  write
//-------------------------------------------------

void pia6821_device::reg_w(UINT8 offset, UINT8 data)
{
	switch (offset & 0x03)
	{
		default: // impossible
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


//-------------------------------------------------
//  set_a_input
//-------------------------------------------------

void pia6821_device::set_a_input(UINT8 data, UINT8 z_mask)
{
	assert_always(m_in_a_handler.isnull(), "pia6821_porta_w() called when in_a_func implemented");

	LOG(("PIA #%s: set input port A = %02X\n", tag(), data));

	m_in_a = data;
	m_port_a_z_mask = z_mask;
	m_in_a_pushed = true;
}


//-------------------------------------------------
//  pia6821_porta_w
//-------------------------------------------------

void pia6821_device::porta_w(UINT8 data)
{
	set_a_input(data, 0);
}


//-------------------------------------------------
//  a_output
//-------------------------------------------------

UINT8 pia6821_device::a_output()
{
	m_out_a_needs_pulled = false;

	return get_out_a_value();
}


//-------------------------------------------------
//  ca1_w
//-------------------------------------------------

WRITE_LINE_MEMBER( pia6821_device::ca1_w )
{
	LOG(("PIA #%s: set input CA1 = %d\n", tag(), state));

	// the new state has caused a transition
	if((m_in_ca1 != state) && ((state && C1_LOW_TO_HIGH(m_ctl_a)) || (!state && C1_HIGH_TO_LOW(m_ctl_a))))
	{
		LOG(("PIA #%s: CA1 triggering\n", tag()));

		// mark the IRQ
		m_irq_a1 = TRUE;

		// update externals
		update_interrupts();

		// CA2 is configured as output and in read strobe mode and cleared by a CA1 transition
		if(C2_OUTPUT(m_ctl_a) && C2_STROBE_MODE(m_ctl_a) && STROBE_C1_RESET(m_ctl_a))
		{
			set_out_ca2(TRUE);
		}
	}

	// set the new value for CA1
	m_in_ca1 = state;
	m_in_ca1_pushed = true;
}


//-------------------------------------------------
//  ca2_w
//-------------------------------------------------

WRITE_LINE_MEMBER( pia6821_device::ca2_w )
{
	LOG(("PIA #%s: set input CA2 = %d\n", tag(), state));

	// if input mode and the new state has caused a transition
	if(C2_INPUT(m_ctl_a) && (m_in_ca2 != state) && ((state && C2_LOW_TO_HIGH(m_ctl_a)) || (!state && C2_HIGH_TO_LOW(m_ctl_a))))
	{
		LOG(("PIA #%s: CA2 triggering\n", tag()));

		// mark the IRQ
		m_irq_a2 = TRUE;

		// update externals
		update_interrupts();
	}

	// set the new value for CA2
	m_in_ca2 = state;
	m_in_ca2_pushed = true;
}


//-------------------------------------------------
//  ca2_output
//-------------------------------------------------

int pia6821_device::ca2_output()
{
	m_out_ca2_needs_pulled = false;

	return m_out_ca2;
}


//-------------------------------------------------
//  ca2_output_z - version of ca2_output which
//  takes account of internal pullup resistor
//-------------------------------------------------

int pia6821_device::ca2_output_z()
{
	m_out_ca2_needs_pulled = false;

	// If it's an output, output the bit, if it's an input, it's
	// pulled up
	return m_out_ca2 | C2_INPUT(m_ctl_a);
}


//-------------------------------------------------
//  portb_w
//-------------------------------------------------

void pia6821_device::portb_w(UINT8 data)
{
	assert_always(m_in_b_handler.isnull(), "pia_set_input_b() called when in_b_func implemented");

	LOG(("PIA #%s: set input port B = %02X\n", tag(), data));

	m_in_b = data;
	m_in_b_pushed = true;
}


//-------------------------------------------------
//  b_output
//-------------------------------------------------

UINT8 pia6821_device::b_output()
{
	m_out_b_needs_pulled = false;

	return get_out_b_value();
}


//-------------------------------------------------
//  cb1_w
//-------------------------------------------------

WRITE_LINE_MEMBER( pia6821_device::cb1_w )
{
	LOG(("PIA #%s: set input CB1 = %d\n", tag(), state));

	// the new state has caused a transition
	if((m_in_cb1 != state) && ((state && C1_LOW_TO_HIGH(m_ctl_b)) || (!state && C1_HIGH_TO_LOW(m_ctl_b))))
	{
		LOG(("PIA #%s: CB1 triggering\n", tag()));

		// mark the IRQ
		m_irq_b1 = 1;

		// update externals
		update_interrupts();

		// If CB2 is configured as a write-strobe output which is reset by a CB1
		// transition, this reset will only happen when a read from port B implicitly
		// clears the IRQ B1 flag.  So we handle the CB2 reset there.  Note that this
		// is different from what happens with port A.
	}

	// set the new value for CB1
	m_in_cb1 = state;
	m_in_cb1_pushed = true;
}


//-------------------------------------------------
//  cb2_w
//-------------------------------------------------

WRITE_LINE_MEMBER( pia6821_device::cb2_w )
{
	LOG(("PIA #%s: set input CB2 = %d\n", tag(), state));

	// if input mode and the new state has caused a transition
	if (C2_INPUT(m_ctl_b) &&
		(m_in_cb2 != state) &&
		((state && C2_LOW_TO_HIGH(m_ctl_b)) || (!state && C2_HIGH_TO_LOW(m_ctl_b))))
	{
		LOG(("PIA #%s: CB2 triggering\n", tag()));

		// mark the IRQ
		m_irq_b2 = 1;

		// update externals
		update_interrupts();
	}

	// set the new value for CA2
	m_in_cb2 = state;
	m_in_cb2_pushed = true;
}


//-------------------------------------------------
//  output_cb2
//-------------------------------------------------

int pia6821_device::cb2_output()
{
	m_out_cb2_needs_pulled = false;

	return m_out_cb2;
}


//-------------------------------------------------
//  cb2_output_z
//-------------------------------------------------

int pia6821_device::cb2_output_z()
{
	return !C2_OUTPUT(m_ctl_b);
}
