// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 8255(A) Programmable Peripheral Interface emulation

**********************************************************************/

#include "i8255.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

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
	MODE_INPUT
};


#define CONTROL_PORT_C_LOWER_INPUT  0x01
#define CONTROL_PORT_B_INPUT        0x02
#define CONTROL_GROUP_B_MODE_1      0x04
#define CONTROL_PORT_C_UPPER_INPUT  0x08
#define CONTROL_PORT_A_INPUT        0x10
#define CONTROL_GROUP_A_MODE_MASK   0x60
#define CONTROL_MODE_SET            0x80



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type I8255 = &device_creator<i8255_device>;
const device_type I8255A = &device_creator<i8255_device>;


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  check_interrupt -
//-------------------------------------------------

inline void i8255_device::check_interrupt(int port)
{
	switch (group_mode(port))
	{
	case MODE_1:
		switch (port_mode(port))
		{
		case MODE_INPUT:
			set_intr(port, (m_inte[port] && m_ibf[port]));
			break;

		case MODE_OUTPUT:
			set_intr(port, (m_inte[port] && m_obf[port]));
			break;
		}
		break;

	case MODE_2:
		set_intr(port, ((m_inte1 && m_obf[port]) || (m_inte2 && m_ibf[port])));
		break;
	}
}


//-------------------------------------------------
//  set_ibf -
//-------------------------------------------------

inline void i8255_device::set_ibf(int port, int state)
{
	if (LOG) logerror("I8255 '%s' Port %c IBF: %u\n", tag().c_str(), 'A' + port, state);

	m_ibf[port] = state;

	check_interrupt(port);
}


//-------------------------------------------------
//  set_obf -
//-------------------------------------------------

inline void i8255_device::set_obf(int port, int state)
{
	if (LOG) logerror("I8255 '%s' Port %c OBF: %u\n", tag().c_str(), 'A' + port, state);

	m_obf[port] = state;

	check_interrupt(port);
}


//-------------------------------------------------
//  set_inte -
//-------------------------------------------------

inline void i8255_device::set_inte(int port, int state)
{
	if (LOG) logerror("I8255 '%s' Port %c INTE: %u\n", tag().c_str(), 'A' + port, state);

	m_inte[port] = state;

	check_interrupt(port);
}


//-------------------------------------------------
//  set_inte1 -
//-------------------------------------------------

inline void i8255_device::set_inte1(int state)
{
	if (LOG) logerror("I8255 '%s' Port A INTE1: %u\n", tag().c_str(), state);

	m_inte1 = state;

	check_interrupt(PORT_A);
}


//-------------------------------------------------
//  set_inte2 -
//-------------------------------------------------

inline void i8255_device::set_inte2(int state)
{
	if (LOG) logerror("I8255 '%s' Port A INTE2: %u\n", tag().c_str(), state);

	m_inte2 = state;

	check_interrupt(PORT_A);
}


//-------------------------------------------------
//  set_intr -
//-------------------------------------------------

inline void i8255_device::set_intr(int port, int state)
{
	if (LOG) logerror("I8255 '%s' Port %c INTR: %u\n", tag().c_str(), 'A' + port, state);

	m_intr[port] = state;

	output_pc();
}


//-------------------------------------------------
//  group_mode -
//-------------------------------------------------

inline int i8255_device::group_mode(int group)
{
	int mode = 0;

	switch (group)
	{
	case GROUP_A:
		switch ((m_control & CONTROL_GROUP_A_MODE_MASK) >> 5)
		{
		case 0: mode = MODE_0; break;
		case 1: mode = MODE_1; break;
		case 2: case 3: mode = MODE_2; break;
		}
		break;

	case GROUP_B:
		mode = m_control & CONTROL_GROUP_B_MODE_1 ? MODE_1 : MODE_0;
		break;
	}

	return mode;
}


//-------------------------------------------------
//  port_mode -
//-------------------------------------------------

inline int i8255_device::port_mode(int port)
{
	int mode = 0;

	switch (port)
	{
	case PORT_A: mode = m_control & CONTROL_PORT_A_INPUT ? MODE_INPUT : MODE_OUTPUT; break;
	case PORT_B: mode = m_control & CONTROL_PORT_B_INPUT ? MODE_INPUT : MODE_OUTPUT; break;
	}

	return mode;
}


//-------------------------------------------------
//  port_c_lower_mode -
//-------------------------------------------------

inline int i8255_device::port_c_lower_mode()
{
	return m_control & CONTROL_PORT_C_LOWER_INPUT ? MODE_INPUT : MODE_OUTPUT;
}


//-------------------------------------------------
//  port_c_upper_mode -
//-------------------------------------------------

inline int i8255_device::port_c_upper_mode()
{
	return m_control & CONTROL_PORT_C_UPPER_INPUT ? MODE_INPUT : MODE_OUTPUT;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i8255_device - constructor
//-------------------------------------------------

i8255_device::i8255_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, I8255, "8255 PPI", tag, owner, clock, "i8255", __FILE__),
		m_in_pa_cb(*this),
		m_in_pb_cb(*this),
		m_in_pc_cb(*this),
		m_out_pa_cb(*this),
		m_out_pb_cb(*this),
		m_out_pc_cb(*this)
{
	m_intr[PORT_A] = m_intr[PORT_B] = 0;
	m_control = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8255_device::device_start()
{
	// resolve callbacks
	m_in_pa_cb.resolve_safe(0);
	m_in_pb_cb.resolve_safe(0);
	m_in_pc_cb.resolve_safe(0);
	m_out_pa_cb.resolve_safe();
	m_out_pb_cb.resolve_safe();
	m_out_pc_cb.resolve_safe();

	// register for state saving
	save_item(NAME(m_control));
	save_item(NAME(m_output));
	save_item(NAME(m_input));
	save_item(NAME(m_ibf));
	save_item(NAME(m_obf));
	save_item(NAME(m_inte));
	save_item(NAME(m_inte1));
	save_item(NAME(m_inte2));
	save_item(NAME(m_intr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8255_device::device_reset()
{
	set_mode(0x9b);
}


//-------------------------------------------------
//  read_mode0 -
//-------------------------------------------------

UINT8 i8255_device::read_mode0(int port)
{
	UINT8 data;

	if (port_mode(port) == MODE_OUTPUT)
	{
		// read data from output latch
		data = m_output[port];
	}
	else
	{
		// read data from port
		data = (port == PORT_A) ? m_in_pa_cb(0) : ((port == PORT_B) ? m_in_pb_cb(0) : m_in_pc_cb(0));
	}

	return data;
}


//-------------------------------------------------
//  read_mode1 -
//-------------------------------------------------

UINT8 i8255_device::read_mode1(int port)
{
	UINT8 data;

	if (port_mode(port) == MODE_OUTPUT)
	{
		// read data from output latch
		data = m_output[port];
	}
	else
	{
		// read data from input latch
		data = m_input[port];

		// clear input buffer full flag
		set_ibf(port, 0);

		// clear interrupt
		set_intr(port, 0);

		// clear input latch
		m_input[port] = 0;
	}

	return data;
}


//-------------------------------------------------
//  read_mode2 -
//-------------------------------------------------

UINT8 i8255_device::read_mode2()
{
	UINT8 data;

	// read data from input latch
	data = m_input[PORT_A];

	// clear input buffer full flag
	set_ibf(PORT_A, 0);

	// clear interrupt
	set_intr(PORT_A, 0);

	// clear input latch
	m_input[PORT_A] = 0;

	return data;
}


//-------------------------------------------------
//  read_pc -
//-------------------------------------------------

UINT8 i8255_device::read_pc()
{
	UINT8 data = 0;
	UINT8 mask = 0;
	UINT8 b_mask = 0x0f;

	// PC upper
	switch (group_mode(GROUP_A))
	{
	case MODE_0:
		if (port_c_upper_mode() == MODE_OUTPUT)
		{
			// read data from output latch
			data |= m_output[PORT_C] & 0xf0;
		}
		else
		{
			// read data from port
			mask |= 0xf0;
		}
		break;

	case MODE_1:
		data |= m_intr[PORT_A] ? 0x08 : 0x00;

		if (port_mode(PORT_A) == MODE_OUTPUT)
		{
			data |= m_obf[PORT_A] ? 0x80 : 0x00;
			data |= m_inte[PORT_A] ? 0x40 : 0x00;
			mask |= 0x30;
		}
		else
		{
			data |= m_ibf[PORT_A] ? 0x20 : 0x00;
			data |= m_inte[PORT_A] ? 0x10 : 0x00;
			mask |= 0xc0;
		}
		break;

	case MODE_2:
		b_mask = 0x07;
		data |= m_intr[PORT_A] ? 0x08 : 0x00;
		data |= m_inte2 ? 0x10 : 0x00;
		data |= m_ibf[PORT_A] ? 0x20 : 0x00;
		data |= m_inte1 ? 0x40 : 0x00;
		data |= m_obf[PORT_A] ? 0x80 : 0x00;
		break;
	}

	// PC lower
	switch (group_mode(GROUP_B))
	{
	case MODE_0:
		if (port_c_lower_mode() == MODE_OUTPUT)
		{
			// read data from output latch
			data |= m_output[PORT_C] & b_mask;
		}
		else
		{
			// read data from port
			mask |= b_mask;
		}
		break;

	case MODE_1:
		data |= m_inte[PORT_B] ? 0x04 : 0x00;
		data |= m_intr[PORT_B] ? 0x01 : 0x00;

		if (port_mode(PORT_B) == MODE_OUTPUT)
		{
			data |= m_obf[PORT_B] ? 0x02 : 0x00;
		}
		else
		{
			data |= m_ibf[PORT_B] ? 0x02 : 0x00;
		}
	}

	if (mask)
	{
		// read data from port
		data |= m_in_pc_cb(0) & mask;
	}

	return data;
}


//-------------------------------------------------
//  write_mode0 -
//-------------------------------------------------

void i8255_device::write_mode0(int port, UINT8 data)
{
	if (port_mode(port) == MODE_OUTPUT)
	{
		// latch output data
		m_output[port] = data;

		// write data to port
		if (port == PORT_A)
			m_out_pa_cb((offs_t)0, m_output[port]);
		else if (port == PORT_B)
			m_out_pb_cb((offs_t)0, m_output[port]);
		else
			m_out_pc_cb((offs_t)0, m_output[port]);
	}
}


//-------------------------------------------------
//  write_mode1 -
//-------------------------------------------------

void i8255_device::write_mode1(int port, UINT8 data)
{
	if (port_mode(port) == MODE_OUTPUT)
	{
		// latch output data
		m_output[port] = data;

		// write data to port
		if (port == PORT_A)
			m_out_pa_cb((offs_t)0, m_output[port]);
		else if (port == PORT_B)
			m_out_pb_cb((offs_t)0, m_output[port]);
		else
			m_out_pc_cb((offs_t)0, m_output[port]);

		// set output buffer full flag
		set_obf(port, 0);

		// clear interrupt
		set_intr(port, 0);
	}
}


//-------------------------------------------------
//  write_mode2 -
//-------------------------------------------------

void i8255_device::write_mode2(UINT8 data)
{
	// latch output data
	m_output[PORT_A] = data;

	// write data to port
	m_out_pa_cb((offs_t)0, data);

	// set output buffer full flag
	set_obf(PORT_A, 0);

	// clear interrupt
	set_intr(PORT_A, 0);
}


//-------------------------------------------------
//  output_pc -
//-------------------------------------------------

void i8255_device::output_pc()
{
	UINT8 data = 0;
	UINT8 mask = 0;
	UINT8 b_mask = 0x0f;

	// PC upper
	switch (group_mode(GROUP_A))
	{
	case MODE_0:
		if (port_c_upper_mode() == MODE_OUTPUT)
		{
			mask |= 0xf0;
		}
		else
		{
			// TTL inputs float high
			data |= 0xf0;
		}
		break;

	case MODE_1:
		data |= m_intr[PORT_A] ? 0x08 : 0x00;

		if (port_mode(PORT_A) == MODE_OUTPUT)
		{
			data |= m_obf[PORT_A] ? 0x80 : 0x00;
			mask |= 0x30;
		}
		else
		{
			data |= m_ibf[PORT_A] ? 0x20 : 0x00;
			mask |= 0xc0;
		}
		break;

	case MODE_2:
		b_mask = 0x07;
		data |= m_intr[PORT_A] ? 0x08 : 0x00;
		data |= m_ibf[PORT_A] ? 0x20 : 0x00;
		data |= m_obf[PORT_A] ? 0x80 : 0x00;
		break;
	}

	// PC lower
	switch (group_mode(GROUP_B))
	{
	case MODE_0:
		if (port_c_lower_mode() == MODE_OUTPUT)
		{
			mask |= b_mask;
		}
		else
		{
			// TTL inputs float high
			data |= b_mask;
		}
		break;

	case MODE_1:
		data |= m_intr[PORT_B] ? 0x01 : 0x00;

		if (port_mode(PORT_B) == MODE_OUTPUT)
		{
			data |= m_obf[PORT_B] ? 0x02 : 0x00;
		}
		else
		{
			data |= m_ibf[PORT_B] ? 0x02 : 0x00;
		}
	}

	data |= m_output[PORT_C] & mask;

	m_out_pc_cb((offs_t)0, data);
}


//-------------------------------------------------
//  set_mode -
//-------------------------------------------------

void i8255_device::set_mode(UINT8 data)
{
	m_control = data;

	// group A
	m_output[PORT_A] = 0;
	m_input[PORT_A] = 0;
	m_ibf[PORT_A] = 0;
	m_obf[PORT_A] = 1;
	m_inte[PORT_A] = 0;
	m_inte1 = 0;
	m_inte2 = 0;

	if (port_mode(PORT_A) == MODE_OUTPUT)
	{
		m_out_pa_cb((offs_t)0, m_output[PORT_A]);
	}
	else
	{
		// TTL inputs float high
		m_out_pa_cb((offs_t)0, 0xff);
	}

	if (LOG)
	{
		logerror("I8255 '%s' Group A Mode: %u\n", tag().c_str(), group_mode(GROUP_A));
		logerror("I8255 '%s' Port A Mode: %s\n", tag().c_str(), (port_mode(PORT_A) == MODE_OUTPUT) ? "output" : "input");
		logerror("I8255 '%s' Port C Upper Mode: %s\n", tag().c_str(), (port_c_upper_mode() == MODE_OUTPUT) ? "output" : "input");
		logerror("I8255 '%s' Group B Mode: %u\n", tag().c_str(), group_mode(GROUP_B));
		logerror("I8255 '%s' Port B Mode: %s\n", tag().c_str(), (port_mode(PORT_B) == MODE_OUTPUT) ? "output" : "input");
		logerror("I8255 '%s' Port C Lower Mode: %s\n", tag().c_str(), (port_c_lower_mode() == MODE_OUTPUT) ? "output" : "input");
	}

	// group B
	m_output[PORT_B] = 0;
	m_input[PORT_B] = 0;
	m_ibf[PORT_B] = 0;
	m_obf[PORT_B] = 1;
	m_inte[PORT_B] = 0;

	if (port_mode(PORT_B) == MODE_OUTPUT)
	{
		m_out_pb_cb((offs_t)0, m_output[PORT_B]);
	}
	else
	{
		// TTL inputs float high
		m_out_pb_cb((offs_t)0, 0xff);
	}

	m_output[PORT_C] = 0;
	m_input[PORT_C] = 0;

	output_pc();
}


//-------------------------------------------------
//  set_pc_bit -
//-------------------------------------------------

void i8255_device::set_pc_bit(int bit, int state)
{
	// set output latch bit
	m_output[PORT_C] &= ~(1 << bit);
	m_output[PORT_C] |= state << bit;

	switch (group_mode(GROUP_A))
	{
	case MODE_1:
		if (port_mode(PORT_A) == MODE_OUTPUT)
		{
			switch (bit)
			{
			case 3: set_intr(PORT_A, state); break;
			case 6: set_inte(PORT_A, state); break;
			case 7: set_obf(PORT_A, state); break;
			default: break;
			}
		}
		else
		{
			switch (bit)
			{
			case 3: set_intr(PORT_A, state); break;
			case 4: set_inte(PORT_A, state); break;
			case 5: set_ibf(PORT_A, state); break;
			default: break;
			}
		}
		break;

	case MODE_2:
		switch (bit)
		{
		case 3: set_intr(PORT_A, state); break;
		case 4: set_inte2(state); break;
		case 5: set_ibf(PORT_A, state); break;
		case 6: set_inte1(state); break;
		case 7: set_obf(PORT_A, state); break;
		default: break;
		}
		break;
	}

	if (group_mode(GROUP_B) == MODE_1)
	{
		switch (bit)
		{
		case 0: set_intr(PORT_B, state); break;
		case 1:
			if (port_mode(PORT_B) == MODE_OUTPUT)
				set_obf(PORT_B, state);
			else
				set_ibf(PORT_B, state);
			break;
		case 2: set_inte(PORT_B, state); break;
		default: break;
		}
	}

	output_pc();
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( i8255_device::read )
{
	UINT8 data = 0;

	switch (offset & 0x03)
	{
	case PORT_A:
		switch (group_mode(GROUP_A))
		{
		case MODE_0: data = read_mode0(PORT_A); break;
		case MODE_1: data = read_mode1(PORT_A); break;
		case MODE_2: data = read_mode2(); break;
		}
		if (LOG) logerror("I8255 '%s' Port A Read: %02x\n", tag().c_str(), data);
		break;

	case PORT_B:
		switch (group_mode(GROUP_B))
		{
		case MODE_0: data = read_mode0(PORT_B); break;
		case MODE_1: data = read_mode1(PORT_B); break;
		}
		if (LOG) logerror("I8255 '%s' Port B Read: %02x\n", tag().c_str(), data);
		break;

	case PORT_C:
		data = read_pc();
		if (LOG) logerror("I8255 '%s' Port C Read: %02x\n", tag().c_str(), data);
		break;

	case CONTROL:
		data = m_control;
		if (LOG) logerror("I8255 '%s' Mode Control Word Read: %02x\n", tag().c_str(), data);
		break;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( i8255_device::write )
{
	switch (offset & 0x03)
	{
	case PORT_A:
		if (LOG) logerror("I8255 '%s' Port A Write: %02x\n", tag().c_str(), data);

		switch (group_mode(GROUP_A))
		{
		case MODE_0: write_mode0(PORT_A, data); break;
		case MODE_1: write_mode1(PORT_A, data); break;
		case MODE_2: write_mode2(data); break;
		}
		break;

	case PORT_B:
		if (LOG) logerror("I8255 '%s' Port B Write: %02x\n", tag().c_str(), data);

		switch (group_mode(GROUP_B))
		{
		case MODE_0: write_mode0(PORT_B, data); break;
		case MODE_1: write_mode1(PORT_B, data); break;
		}
		break;

	case PORT_C:
		if (LOG) logerror("I8255 '%s' Port C Write: %02x\n", tag().c_str(), data);

		m_output[PORT_C] = data;
		output_pc();
		break;

	case CONTROL:
		if (data & CONTROL_MODE_SET)
		{
			if (LOG) logerror("I8255 '%s' Mode Control Word: %02x\n", tag().c_str(), data);

			set_mode(data);
		}
		else
		{
			int bit = (data >> 1) & 0x07;
			int state = BIT(data, 0);

			if (LOG) logerror("I8255 '%s' %s Port C Bit %u\n", tag().c_str(), state ? "Set" : "Reset", bit);

			set_pc_bit(bit, state);
		}
		break;
	}
}


//-------------------------------------------------
//  pa_r -
//-------------------------------------------------

READ8_MEMBER( i8255_device::pa_r )
{
	return pa_r();
}


//-------------------------------------------------
//  pb_r - port A read
//-------------------------------------------------

UINT8 i8255_device::pa_r()
{
	UINT8 data = 0xff;

	if (port_mode(PORT_A) == MODE_OUTPUT)
	{
		data = m_output[PORT_A];
	}

	return data;
}


//-------------------------------------------------
//  pb_r -
//-------------------------------------------------

READ8_MEMBER( i8255_device::pb_r )
{
	return pb_r();
}


//-------------------------------------------------
//  pb_r - port B read
//-------------------------------------------------

UINT8 i8255_device::pb_r()
{
	UINT8 data = 0xff;

	if (port_mode(PORT_B) == MODE_OUTPUT)
	{
		data = m_output[PORT_B];
	}

	return data;
}


//-------------------------------------------------
//  pc2_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( i8255_device::pc2_w )
{
	if (group_mode(GROUP_B) == 1)
	{
		if (port_mode(PORT_B) == MODE_OUTPUT)
		{
			// port B acknowledge
			if (!m_obf[PORT_B] && !state)
			{
				if (LOG) logerror("I8255 '%s' Port B Acknowledge\n", tag().c_str());

				// clear output buffer flag
				set_obf(PORT_B, 1);
			}
		}
		else
		{
			// port B strobe
			if (!m_ibf[PORT_B] && !state)
			{
				if (LOG) logerror("I8255 '%s' Port B Strobe\n", tag().c_str());

				// read port into latch
				m_input[PORT_B] = m_in_pb_cb(0);

				// set input buffer flag
				set_ibf(PORT_B, 1);
			}
		}
	}
}


//-------------------------------------------------
//  pc4_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( i8255_device::pc4_w )
{
	if ((group_mode(GROUP_A) == 2) || ((group_mode(GROUP_A) == 1) && (port_mode(PORT_A) == MODE_INPUT)))
	{
		// port A strobe
		if (!m_ibf[PORT_A] && !state)
		{
			if (LOG) logerror("I8255 '%s' Port A Strobe\n", tag().c_str());

			// read port into latch
			m_input[PORT_A] = m_in_pa_cb(0);

			// set input buffer flag
			set_ibf(PORT_A, 1);
		}
	}
}


//-------------------------------------------------
//  pc6_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( i8255_device::pc6_w )
{
	if ((group_mode(GROUP_A) == 2) || ((group_mode(GROUP_A) == 1) && (port_mode(PORT_A) == MODE_OUTPUT)))
	{
		// port A acknowledge
		if (!m_obf[PORT_A] && !state)
		{
			if (LOG) logerror("I8255 '%s' Port A Acknowledge\n", tag().c_str());

			// clear output buffer flag
			set_obf(PORT_A, 1);
		}
	}
}
