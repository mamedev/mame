// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 8255(A) Programmable Peripheral Interface emulation

**********************************************************************/

#include "emu.h"
#include "i8255.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

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

DEFINE_DEVICE_TYPE(I8255, i8255_device, "i8255", "Intel 8255 PPI")
decltype(I8255) I8255A = I8255;

DEFINE_DEVICE_TYPE(AMS40489_PPI, ams40489_ppi_device, "ams40489_ppi", "Amstrad AMS40489 PPI")


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

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


inline void i8255_device::set_ibf(int port, int state)
{
	LOG("I8255 Port %c IBF: %u\n", 'A' + port, state);

	m_ibf[port] = state;

	check_interrupt(port);
}


inline void i8255_device::set_obf(int port, int state)
{
	LOG("I8255 Port %c OBF: %u\n", 'A' + port, state);

	m_obf[port] = state;

	check_interrupt(port);
}


inline void i8255_device::set_inte(int port, int state)
{
	LOG("I8255 Port %c INTE: %u\n", 'A' + port, state);

	m_inte[port] = state;

	check_interrupt(port);
}


inline void i8255_device::set_inte1(int state)
{
	LOG("I8255 Port A INTE1: %u\n", state);

	m_inte1 = state;

	check_interrupt(PORT_A);
}


inline void i8255_device::set_inte2(int state)
{
	LOG("I8255 Port A INTE2: %u\n", state);

	m_inte2 = state;

	check_interrupt(PORT_A);
}


inline void i8255_device::set_intr(int port, int state)
{
	LOG("I8255 Port %c INTR: %u\n", 'A' + port, state);

	m_intr[port] = state;

	output_pc();
}


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


inline int i8255_device::port_c_lower_mode()
{
	return m_control & CONTROL_PORT_C_LOWER_INPUT ? MODE_INPUT : MODE_OUTPUT;
}


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

i8255_device::i8255_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool is_ams40489)
	: device_t(mconfig, type, tag, owner, clock)
	, m_force_portb_in(is_ams40489)
	, m_force_portc_out(is_ams40489)
	, m_dont_clear_output_latches(is_ams40489)
	, m_in_pa_cb(*this)
	, m_in_pb_cb(*this)
	, m_in_pc_cb(*this)
	, m_out_pa_cb(*this)
	, m_out_pb_cb(*this)
	, m_out_pc_cb(*this)
	, m_tri_pa_cb(*this)
	, m_tri_pb_cb(*this)
	, m_tri_pc_cb(*this)
	, m_control(0)
	, m_intr{ 0, 0 }
{
}

i8255_device::i8255_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i8255_device(mconfig, I8255, tag, owner, clock, false)
{
}

void i8255_device::device_resolve_objects()
{
	// resolve callbacks
	m_in_pa_cb.resolve_safe(0);
	m_in_pb_cb.resolve_safe(0);
	m_in_pc_cb.resolve_safe(0);
	m_out_pa_cb.resolve_safe();
	m_out_pb_cb.resolve_safe();
	m_out_pc_cb.resolve_safe();
	m_tri_pa_cb.resolve_safe(0xff);
	m_tri_pb_cb.resolve_safe(0xff);
	m_tri_pc_cb.resolve_safe(0xff);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8255_device::device_start()
{
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


uint8_t i8255_device::read_mode0(int port)
{
	uint8_t data;

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


uint8_t i8255_device::read_mode1(int port)
{
	uint8_t data;

	if (port_mode(port) == MODE_OUTPUT)
	{
		// read data from output latch
		data = m_output[port];
	}
	else
	{
		// read data from input latch
		data = m_input[port];

		if (!machine().side_effects_disabled())
		{
			// clear input buffer full flag
			set_ibf(port, 0);

			// clear interrupt
			set_intr(port, 0);

			// clear input latch
			m_input[port] = 0;
		}
	}

	return data;
}


uint8_t i8255_device::read_mode2()
{
	// read data from input latch
	uint8_t const data = m_input[PORT_A];

	if (!machine().side_effects_disabled())
	{
		// clear input buffer full flag
		set_ibf(PORT_A, 0);

		// clear interrupt
		set_intr(PORT_A, 0);

		// clear input latch
		m_input[PORT_A] = 0;
	}

	return data;
}


uint8_t i8255_device::read_pc()
{
	uint8_t data = 0;
	uint8_t mask = 0;
	uint8_t b_mask = 0x0f;

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

		if (port_c_upper_mode() == MODE_OUTPUT)
		{
			// read data from output latch
			data |= m_output[PORT_C] & mask;
			mask = 0;
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


void i8255_device::write_mode0(int port, uint8_t data)
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


void i8255_device::write_mode1(int port, uint8_t data)
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


void i8255_device::write_mode2(uint8_t data)
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


void i8255_device::output_pc()
{
	uint8_t data = 0;
	uint8_t mask = 0;
	uint8_t b_mask = 0x0f;

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
			// TTL inputs floating
			data |= m_tri_pc_cb(0) & 0xf0;
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
			// TTL inputs floating
			data |= m_tri_pc_cb(0) & b_mask;
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


void i8255_device::set_mode(uint8_t data)
{
	m_control = data;

	if (m_force_portb_in)
		m_control = m_control | CONTROL_PORT_B_INPUT;

	if (m_force_portc_out)
	{
		m_control = m_control & ~CONTROL_PORT_C_UPPER_INPUT;
		m_control = m_control & ~CONTROL_PORT_C_LOWER_INPUT;
	}

	// group A
	if (!m_dont_clear_output_latches)
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
		// TTL inputs floating
		m_out_pa_cb((offs_t)0, m_tri_pa_cb(0));
	}

	LOG("I8255 Group A Mode: %u\n", group_mode(GROUP_A));
	LOG("I8255 Port A Mode: %s\n", (port_mode(PORT_A) == MODE_OUTPUT) ? "output" : "input");
	LOG("I8255 Port C Upper Mode: %s\n", (port_c_upper_mode() == MODE_OUTPUT) ? "output" : "input");
	LOG("I8255 Group B Mode: %u\n", group_mode(GROUP_B));
	LOG("I8255 Port B Mode: %s\n", (port_mode(PORT_B) == MODE_OUTPUT) ? "output" : "input");
	LOG("I8255 Port C Lower Mode: %s\n", (port_c_lower_mode() == MODE_OUTPUT) ? "output" : "input");

	// group B
	if (!m_dont_clear_output_latches)
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
		// TTL inputs floating
		m_out_pb_cb((offs_t)0, m_tri_pb_cb(0));
	}

	if (!m_dont_clear_output_latches)
		m_output[PORT_C] = 0;
	m_input[PORT_C] = 0;

	output_pc();
}


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


uint8_t i8255_device::read(offs_t offset)
{
	uint8_t data = 0;

	switch (offset & 0x03)
	{
	case PORT_A:
		switch (group_mode(GROUP_A))
		{
		case MODE_0: data = read_mode0(PORT_A); break;
		case MODE_1: data = read_mode1(PORT_A); break;
		case MODE_2: data = read_mode2(); break;
		}
		LOG("I8255 Port A Read: %02x\n", data);
		break;

	case PORT_B:
		switch (group_mode(GROUP_B))
		{
		case MODE_0: data = read_mode0(PORT_B); break;
		case MODE_1: data = read_mode1(PORT_B); break;
		}
		LOG("I8255 Port B Read: %02x\n", data);
		break;

	case PORT_C:
		data = read_pc();
		LOG("I8255 Port C Read: %02x\n", data);
		break;

	case CONTROL:
		data = m_control;
		LOG("I8255 Mode Control Word Read: %02x\n", data);
		break;
	}

	return data;
}


void i8255_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x03)
	{
	case PORT_A:
		LOG("I8255 Port A Write: %02x\n", data);

		switch (group_mode(GROUP_A))
		{
		case MODE_0: write_mode0(PORT_A, data); break;
		case MODE_1: write_mode1(PORT_A, data); break;
		case MODE_2: write_mode2(data); break;
		}
		break;

	case PORT_B:
		LOG("I8255 Port B Write: %02x\n", data);

		switch (group_mode(GROUP_B))
		{
		case MODE_0: write_mode0(PORT_B, data); break;
		case MODE_1: write_mode1(PORT_B, data); break;
		}
		break;

	case PORT_C:
		LOG("I8255 Port C Write: %02x\n", data);

		m_output[PORT_C] = data;
		output_pc();
		break;

	case CONTROL:
		if (data & CONTROL_MODE_SET)
		{
			LOG("I8255 Mode Control Word: %02x\n", data);

			set_mode(data);
		}
		else
		{
			int bit = (data >> 1) & 0x07;
			int state = BIT(data, 0);

			LOG("I8255 %s Port C Bit %u\n", state ? "Set" : "Reset", bit);

			set_pc_bit(bit, state);
		}
		break;
	}
}


//-------------------------------------------------
//  pa_r - port A read
//-------------------------------------------------

uint8_t i8255_device::pa_r()
{
	uint8_t data = 0xff;

	if (port_mode(PORT_A) == MODE_OUTPUT)
		data = m_output[PORT_A];

	return data;
}


//-------------------------------------------------
//  acka_r - port A read with PC6 strobe
//-------------------------------------------------

uint8_t i8255_device::acka_r()
{
	if (!machine().side_effects_disabled())
		pc6_w(0);

	uint8_t data = pa_r();

	if (!machine().side_effects_disabled())
		pc6_w(1);

	return data;
}


//-------------------------------------------------
//  pb_r - port B read
//-------------------------------------------------

uint8_t i8255_device::pb_r()
{
	uint8_t data = 0xff;

	if (port_mode(PORT_B) == MODE_OUTPUT)
	{
		data = m_output[PORT_B];
	}

	return data;
}


//-------------------------------------------------
//  ackb_r - port B read with PC2 strobe
//-------------------------------------------------

uint8_t i8255_device::ackb_r()
{
	if (!machine().side_effects_disabled())
		pc2_w(0);

	uint8_t data = pb_r();

	if (!machine().side_effects_disabled())
		pc2_w(1);

	return data;
}


WRITE_LINE_MEMBER( i8255_device::pc2_w )
{
	if (group_mode(GROUP_B) == 1)
	{
		if (port_mode(PORT_B) == MODE_OUTPUT)
		{
			// port B acknowledge
			if (!m_obf[PORT_B] && !state)
			{
				LOG("I8255 Port B Acknowledge\n");

				// clear output buffer flag
				set_obf(PORT_B, 1);
			}
		}
		else
		{
			// port B strobe
			if (!m_ibf[PORT_B] && !state)
			{
				LOG("I8255 Port B Strobe\n");

				// read port into latch
				m_input[PORT_B] = m_in_pb_cb(0);

				// set input buffer flag
				set_ibf(PORT_B, 1);
			}
		}
	}
}


WRITE_LINE_MEMBER( i8255_device::pc4_w )
{
	if ((group_mode(GROUP_A) == 2) || ((group_mode(GROUP_A) == 1) && (port_mode(PORT_A) == MODE_INPUT)))
	{
		// port A strobe
		if (!m_ibf[PORT_A] && !state)
		{
			LOG("I8255 Port A Strobe\n");

			// read port into latch
			m_input[PORT_A] = m_in_pa_cb(0);

			// set input buffer flag
			set_ibf(PORT_A, 1);
		}
	}
}


WRITE_LINE_MEMBER( i8255_device::pc6_w )
{
	if ((group_mode(GROUP_A) == 2) || ((group_mode(GROUP_A) == 1) && (port_mode(PORT_A) == MODE_OUTPUT)))
	{
		// port A acknowledge
		if (!m_obf[PORT_A] && !state)
		{
			LOG("I8255 Port A Acknowledge\n");

			// clear output buffer flag
			set_obf(PORT_A, 1);
		}
	}
}


// AMS40489 (Amstrad Plus/GX4000 ASIC PPI implementation)
ams40489_ppi_device::ams40489_ppi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i8255_device(mconfig, AMS40489_PPI, tag, owner, clock, true)
{
}
