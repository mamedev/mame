// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An emulation of the Zilog Z8038 FIO FIFO Input/Output Interface Unit.
 *
 * Sources:
 *
 *   http://datasheet.datasheetarchive.com/originals/scans/Scans-98/DSAIHSC00090399.pdf
 *
 * The external interface uses port number 1 and 2 per the documentation, while
 * the implementation uses port number 0 and 1 for convenience.
 *
 * TODO
 *   - more i/o lines and handshake
 *   - Z-BUS interrupt/acknowledge
 *   - dma cycles
 *   - fifo save state
 */

#include "emu.h"
#include "z8038.h"

#define LOG_GENERAL (1U << 0)
#define LOG_REG     (1U << 1)
#define LOG_FIFO    (1U << 2)
#define LOG_INT     (1U << 3)

//#define VERBOSE (LOG_GENERAL|LOG_REG|LOG_FIFO|LOG_INT)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(Z8038, z8038_device, "z8038", "FIFO Input/Output Interface Unit")

z8038_device::z8038_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, Z8038, tag, owner, clock)
	, m_out_int_cb(*this)
	, m_out_E_cb(*this)
	, m_out_F_cb(*this)
	, m_out_H_cb(*this)
	, m_out_J_cb(*this)
{
}

template <u8 Port> void z8038_device::zbus_map(address_map &map)
{
	map(0x0, 0xf).rw(FUNC(z8038_device::zbus_reg_r<Port>), FUNC(z8038_device::zbus_reg_w<Port>));

	// port 2 can not write to control register 2
	if (Port == 2)
		map(0x9, 0x9).unmapw();

	// Message In register is read-only
	map(0xc, 0xc).unmapw();
}

// instantiate maps for port 1 and 2
template void z8038_device::zbus_map<1>(address_map &map);
template void z8038_device::zbus_map<2>(address_map &map);

void z8038_device::device_start()
{
	m_out_int_cb.resolve_all_safe();

	m_out_E_cb.resolve_safe();
	m_out_F_cb.resolve_safe();
	m_out_H_cb.resolve_safe();
	m_out_J_cb.resolve_safe();

	save_item(NAME(m_control_2));
	save_item(NAME(m_control_3));

	for (u8 port = 0; port < 2; port++)
	{
		save_item(m_port[port].reg_state, "state", port + 1);
		save_item(m_port[port].reg_pointer, "pointer", port + 1);
		save_item(m_port[port].int_code, "int_code", port + 1);
		save_item(m_port[port].int_asserted, "int_asserted", port + 1);

		save_item(m_port[port].control_0, "control_0", port + 1);
		save_item(m_port[port].control_1, "control_1", port + 1);
		save_item(m_port[port].interrupt_status, "interrupt_status", port + 1);
		save_item(m_port[port].interrupt_vector, "interrupt_vector", port + 1);
		save_item(m_port[port].byte_count, "byte_count", port + 1);
		save_item(m_port[port].byte_count_comparison, "byte_count_comparison", port + 1);
		save_item(m_port[port].message_in, "message_in", port + 1);
		save_item(m_port[port].pattern_match, "pattern_match", port + 1);
		save_item(m_port[port].pattern_mask, "pattern_mask", port + 1);
		save_item(m_port[port].data_buffer, "data_buffer", port + 1);
	}

	//save_item(NAME(m_fifo));

	m_int_check = timer_alloc(FUNC(z8038_device::int_check), this);

	// suppress startup interrupt line changes
	m_port[0].int_asserted = false;
	m_port[1].int_asserted = false;
}

void z8038_device::device_reset()
{
	m_control_2 = 0;

	for (u8 port = 0; port < 2; port++)
	{
		m_port[port].reg_state = 0;
		m_port[port].reg_pointer = 0;
		m_port[port].int_code = 0;
		m_port[port].control_0 = CR0_RESET;
		set_int_state(port, false);
	}
}

u8 z8038_device::reg_r(u8 const port)
{
	/*
	 * The Port 2 CPU can determine when it is enabled by reading its Control
	 * Register 0, which is read as a "floating" data bus if not enabled or as
	 * 01H if enabled.
	 *
	 * FIXME: unsure what value to return for floating data bus
	 */
	if (port && !(m_control_2 & CR2_P2EN))
		return 0xff;

	// FIXME: reads from port 2 in i/o mode
	if (port && (m_port[port].control_0 & CR0_P2M_IO))
		fatalerror("unexpected register read from Port 2 in i/o mode\n");

	/*
	 * After reset is asserted, the only register that can be read from or
	 * written to is Control Register 0 (Control Register 0 will read a 01H).
	 */
	if (m_port[port].control_0 & CR0_RESET)
		return m_port[port].control_0;

	u8 data = 0;
	switch (m_port[port].reg_pointer)
	{
	case 0x0: data = control_0_r(port); break;
	case 0x1: data = control_1_r(port); break;
	case 0x2: data = interrupt_status_r<0>(port); break;
	case 0x3: data = interrupt_status_r<1>(port); break;
	case 0x4: data = interrupt_status_r<2>(port); break;
	case 0x5: data = interrupt_status_r<3>(port); break;
	case 0x6: data = interrupt_vector_r(port); break;
	case 0x7: data = byte_count_r(port); break;
	case 0x8: data = byte_count_comparison_r(port); break;
	case 0x9: data = control_2_r(port); break;
	case 0xa: data = control_3_r(port); break;
	case 0xb: data = message_out_r(port); break;
	case 0xc: data = message_in_r(port); break;
	case 0xd: data = pattern_match_r(port); break;
	case 0xe: data = pattern_mask_r(port); break;
	case 0xf: data = fifo_r(port); break;
	}

	m_port[port].reg_state = 0;

	LOGMASKED(LOG_REG, "reg_r port %d reg %d data 0x%02x\n", port + 1, m_port[port].reg_pointer, data);

	return data;
}

void z8038_device::reg_w(u8 const port, u8 data)
{
	// check port 2 enabled
	if (port && !(m_control_2 & CR2_P2EN))
		return;

	// FIXME: writes from port 2 in i/o mode
	if (port && (m_port[port].control_0 & CR0_P2M_IO))
		fatalerror("unexpected register write from Port 2 in i/o mode\n");

	/*
	 * If C/D̅ is 1, the next byte writes into Control Register 0. When in the
	 * reset state, a write should not be done when C/D̅ is 0. The reset state
	 * is exited by writing 00H when C/D̅ is 1.
	 */
	if (m_port[port].control_0 & CR0_RESET)
	{
		if (data == 0)
		{
			LOG("reg_w port %d reset state cleared\n", port + 1);
			m_port[port].reg_pointer = 0;
			m_port[port].reg_state = 0;

			m_port[port].control_0 = data;
		}

		return;
	}

	if (m_port[port].reg_state == 1)
	{
		switch (m_port[port].reg_pointer)
		{
		case 0x0: control_0_w(port, data); break;
		case 0x1: control_1_w(port, data); break;
		case 0x2: interrupt_status_w<0>(port, data); break;
		case 0x3: interrupt_status_w<1>(port, data); break;
		case 0x4: interrupt_status_w<2>(port, data); break;
		case 0x5: interrupt_status_w<3>(port, data); break;
		case 0x6: interrupt_vector_w(port, data); break;
		case 0x7: break; // byte count register (read only)
		case 0x8: byte_count_comparison_w(port, data); break;
		case 0x9: control_2_w(port, data); break;
		case 0xa: control_3_w(port, data); break;
		case 0xb: message_out_w(port, data); break;
		case 0xc: break; // message in register (read only)
		case 0xd: pattern_match_w(port, data); break;
		case 0xe: pattern_mask_w(port, data); break;
		case 0xf: fifo_w(port, data); break;
		}

		LOGMASKED(LOG_REG, "reg_w port %d reg %d data 0x%02x\n", port + 1, m_port[port].reg_pointer, data);

		// schedule interrupt check (don't duplicate for fifo)
		if (m_port[port].reg_pointer != 0xf)
			m_int_check->adjust(attotime::zero);
	}
	else
		m_port[port].reg_pointer = data & 0xf;

	m_port[port].reg_state = !m_port[port].reg_state;
}

u8 z8038_device::fifo_r(u8 const port)
{
	// check for underflow
	if (!m_fifo.empty())
	{
		m_port[port].data_buffer = m_fifo.dequeue();

		fifo_update();

		LOGMASKED(LOG_FIFO, "fifo_r port %d data 0x%02x\n", port + 1, m_port[port].data_buffer);
	}
	else
		m_port[port].interrupt_status[2] |= (ISR2_UF | ISR2_EIP);

	// schedule interrupt check
	m_int_check->adjust(attotime::zero);

	return m_port[port].data_buffer;
}

void z8038_device::fifo_w(u8 const port, u8 data)
{
	// check for overflow
	if (!m_fifo.full())
	{
		m_port[port].data_buffer = data;
		m_fifo.enqueue(m_port[port].data_buffer);

		fifo_update();

		LOGMASKED(LOG_FIFO, "fifo_w port %d data 0x%02x\n", port + 1, m_port[port].data_buffer);
	}
	else
		m_port[port].interrupt_status[2] |= (ISR2_OF | ISR2_EIP);

	// schedule interrupt check
	m_int_check->adjust(attotime::zero);
}

u8 z8038_device::control_1_r(u8 const port)
{
	/*
	 * Bit 5 (Message Register Out Full), if set, indicates that the CPU has
	 * placed a message in its Message Out register. This bit is reset when the
	 * receiving CPU reads the message in its Message In register. This bit is
	 * the other CPU's message IP bit and is a read-only bit. Bit 4 (Message
	 * Register Interrupt Under Service), if set, indicates that the other CPU
	 * has received a message in its Message In register. This bit is the
	 * message IUS (Interrupt Under Service) bit of the other CPU and is a
	 * read-only bit.
	 */
	u8 data = m_port[port].control_1;

	if (m_port[!port].interrupt_status[0] & ISR0_MIP)
		data |= CR1_MMRF;

	if (m_port[!port].interrupt_status[0] & ISR0_MIUS)
		data |= CR1_MMRUS;

	return data;
}

u8 z8038_device::interrupt_vector_r(u8 const port)
{
	/*
	 * When MIE is 1, other than during an Interrupt Acknowledge cycle, the
	 * Interrupt Vector register always reflects the FIO status in these bits,
	 * regardless of whether or not the Vector Includes Status bit is set.
	 */
	if (m_port[port].control_0 & CR0_MIE)
		return (m_port[port].interrupt_vector & 0xf1) | (m_port[port].int_code << 1);
	else
		return m_port[port].interrupt_vector;
}

u8 z8038_device::byte_count_r(u8 const port)
{
	/*
	 * Bit 6 is reset upon completion of the CPU read of the Byte Count
	 * register. The ongoing count appears in the Byte Count register after
	 * the read.
	 */
	if (m_port[port].control_1 & CR1_FBCR)
		m_port[port].control_1 &= ~CR1_FBCR;

	return m_port[port].byte_count;
}

u8 z8038_device::control_3_r(u8 const port)
{
	u8 const mask = (port == 0) ? 0xff : 0xf0;

	// return direction relative to controlling port
	if (bool(m_control_3 & CR3_P2DIR) != bool(port))
		return (m_control_3 & mask) ^ CR3_DIR;
	else
		return (m_control_3 & mask);
}

u8 z8038_device::message_in_r(u8 const port)
{
	/*
	 * When the Port 2 CPU reads the data from its Message In register, the
	 * Port 2 IP is cleared.
	 */
	m_port[port].interrupt_status[0] &= ~ISR0_MIP;

	return m_port[port].message_in;
}

void z8038_device::control_0_w(u8 const port, u8 data)
{
	if (!(data & CR0_RESET))
	{
		if (port == 0)
			m_port[port].control_0 = data;
		else
			m_port[port].control_0 = (m_port[!port].control_0 & CR0_P2M) | (data & ~CR0_P2M);
	}
	else
		port_reset(port);
}

void z8038_device::control_1_w(u8 const port, u8 data)
{
	m_port[port].control_1 = data & CR1_WMASK;
}

template <u8 Number> void z8038_device::interrupt_status_w(u8 const port, u8 data)
{
	// high interrupt status
	switch (data & ISR_HMASK)
	{
	case 0x20: m_port[port].interrupt_status[Number] &= ~(ISR_HIUS | ISR_HIP); break;
	case 0x40: m_port[port].interrupt_status[Number] |= ISR_HIUS; break;
	case 0x60: m_port[port].interrupt_status[Number] &= ~ISR_HIUS; break;
	case 0x80: m_port[port].interrupt_status[Number] |= ISR_HIP; break;
	case 0xa0: m_port[port].interrupt_status[Number] &= ~ISR_HIP; break;
	case 0xc0: m_port[port].interrupt_status[Number] |= ISR_HIE; break;
	case 0xe0: m_port[port].interrupt_status[Number] &= ~ISR_HIE; break;
	}

	// low interrupt status
	if (Number != 0)
	{
		switch (data & ISR_LMASK)
		{
		case 0x02: m_port[port].interrupt_status[Number] &= ~(ISR_LIUS | ISR_LIP); break;
		case 0x04: m_port[port].interrupt_status[Number] |= ISR_LIUS; break;
		case 0x06: m_port[port].interrupt_status[Number] &= ~ISR_LIUS; break;
		case 0x08: m_port[port].interrupt_status[Number] |= ISR_LIP; break;
		case 0x0a: m_port[port].interrupt_status[Number] &= ~ISR_LIP; break;
		case 0x0c: m_port[port].interrupt_status[Number] |= ISR_LIE; break;
		case 0x0e: m_port[port].interrupt_status[Number] &= ~ISR_LIE; break;
		}
	}
}

// instantiate helpers for each interrupt status register
template void z8038_device::interrupt_status_w<0>(u8 const port, u8 data);
template void z8038_device::interrupt_status_w<1>(u8 const port, u8 data);
template void z8038_device::interrupt_status_w<2>(u8 const port, u8 data);
template void z8038_device::interrupt_status_w<3>(u8 const port, u8 data);

void z8038_device::byte_count_comparison_w(u8 const port, u8 data)
{
	/*
	 * The largest programmable value is 7Fh (127 decimal).
	 */
	m_port[port].byte_count_comparison = data & 0x7f;

	// check byte count comparison
	if (m_fifo.queue_length() == m_port[port].byte_count_comparison)
		m_port[port].interrupt_status[2] |= ISR2_BCCIP;
}

void z8038_device::control_2_w(u8 const port, u8 data)
{
	if (port == 0)
		m_control_2 = data & CR2_WMASK;
	else
		logerror("cannot write to control register 2 from port 2\n");
}

void z8038_device::control_3_w(u8 const port, u8 data)
{
	if (m_port[port].control_0 & CR0_P2M_IO)
	{
		// update all except unused and input line bits
		m_control_3 = data & ~(CR3_UNUSED | CR3_P2IN0);

		// update output lines
		m_out_H_cb(m_control_3 & CR3_P2OUT1 ? 1 : 0);
		m_out_J_cb(m_control_3 & CR3_P2OUT3 ? 1 : 0);

		// update clear if configured as output
		if (!(m_control_3 & CR3_P2CLR))
		{
			if (!(m_control_3 & CR3_CLR))
				fifo_clear();

			m_out_E_cb(m_control_3 & CR3_CLR ? 1 : 0);
		}

		// update direction if configured as output
		if (!(m_control_3 & CR3_P2DIR))
			m_out_F_cb(m_control_3 & CR3_DIR ? 1 : 0);

		// TODO: resample input lines?
	}
	else
	{
		if (port == 0)
		{
			// flag interrupt pending if in control and changing direction
			if (!(data & CR3_P2DIR) && ((data ^ m_control_3) & CR3_DIR))
				m_port[!port].interrupt_status[1] |= ISR1_DDCIP;

			// update clear and direction bits only if in control
			u8 const mask = (CR3_P2CLR | CR3_P2DIR | CR3_P2OUT3 | CR3_P2OUT1)
				| (data & CR3_P2CLR ? 0 : CR3_CLR)
				| (data & CR3_P2DIR ? 0 : CR3_DIR);

			m_control_3 = (m_control_3 & ~mask) | (data & mask);

			// clear fifo
			if (!(m_control_3 & (CR3_P2CLR | CR3_CLR)))
				fifo_clear();
		}
		else
		{
			// flag interrupt pending if in control and changing direction
			if ((data & CR3_P2DIR) && ((data ^ m_control_3) & CR3_DIR))
				m_port[!port].interrupt_status[1] |= ISR1_DDCIP;

			// update clear and direction bits only if in control
			u8 const mask = (m_control_3 & (CR3_P2CLR | CR3_P2DIR)) >> 1;

			m_control_3 = (m_control_3 & ~mask) | (data & mask);

			// clear fifo
			if ((m_control_3 & CR3_P2CLR) && !(m_control_3 & CR3_CLR))
				fifo_clear();
		}
	}
}

void z8038_device::message_out_w(u8 const port, u8 data)
{
	/*
	 * When Port 1's CPU writes to the Message Out register which is also Port
	 * 2's Message In register, Port 2's Message Interrupt Pending bit is set.
	 */
	m_port[!port].message_in = data;
	m_port[!port].interrupt_status[0] |= ISR0_MIP;
}

WRITE_LINE_MEMBER(z8038_device::in_E)
{
	// check port 2 in i/o mode and pin 35 configured as input
	if ((m_port[0].control_0 & CR0_P2M_IO) && (m_control_3 & CR3_P2CLR))
	{
		// active low - clear fifo
		if (!state)
		{
			m_control_3 &= ~CR3_CLR;
			fifo_clear();
		}
		else
			m_control_3 |= CR3_CLR;
	}
}

WRITE_LINE_MEMBER(z8038_device::in_F)
{
	// check port 2 in i/o mode and pin 34 configured as input
	if ((m_port[0].control_0 & CR0_P2M_IO) && (m_control_3 & CR3_P2DIR))
	{
		// check for direction change and flag interrupt
		if (bool(state) != bool(m_control_3 & CR3_DIR))
		{
			if (state)
				m_control_3 |= CR3_DIR;
			else
				m_control_3 &= ~CR3_DIR;

			// flag interrupt pending
			m_port[0].interrupt_status[1] |= ISR1_DDCIP;

			// schedule interrupt check
			m_int_check->adjust(attotime::zero);
		}
	}
}

WRITE_LINE_MEMBER(z8038_device::in_G)
{
	// check port 2 in i/o mode
	if (m_port[0].control_0 & CR0_P2M_IO)
	{
		if (state)
			m_control_3 |= CR3_P2IN0;
		else
			m_control_3 &= ~CR3_P2IN0;
	}
}

void z8038_device::port_reset(u8 const port)
{
	LOG("port_reset port %d \n", port + 1);

	m_port[port].control_1 = 0;
	if (port == 0)
	{
		m_port[port].control_0 = CR0_RESET;

		m_control_2 = 0;
		m_control_3 = 0;

		fifo_clear();
	}
	else
	{
		// port 2 mode is not reset
		m_port[port].control_0 = CR0_RESET;
		m_port[port].control_0 |= (m_port[!port].control_0 & CR0_P2M);

		/*
		 * It should be noted that if the Port 2 side is reset when it has
		 * control of the C̅L̅E̅A̅R̅ bit, the C̅L̅E̅A̅R̅ bit is also reset (0). It should
		 * be noted that if the Port 2 side is reset when it has control of the
		 * Data Direction bit, the Data Direction is also reset.
		 */
		if (m_control_3 & CR3_P2CLR)
		{
			m_control_3 &= ~CR3_CLR;
			fifo_clear();
		}

		if (m_control_3 & CR3_P2DIR)
			m_control_3 &= ~CR3_DIR;
	}

	m_port[port].pattern_mask = 0;
	m_port[port].interrupt_status[0] = 0;
	/*
	 * All bits except D1 and D0 are cleared by reset. Bits D1 and D0 may be a 1
	 * or 0 depending on whether a match condition exists or not.
	 */
	m_port[port].interrupt_status[1] = 0; // opt to ignore "random" pattern matches
	m_port[port].interrupt_status[2] = 0;

	/*
	 * All bits except D0 are cleared by reset.
	 */
	m_port[port].interrupt_status[3] &= ~ISR3_BE;

	/*
	 * When Port 1 is reset, Port 2 is also reset. If Port 2 is reset by itself,
	 * Port 1 is not reset.
	 */
	if (port == 0)
		port_reset(!port);
}

void z8038_device::fifo_clear()
{
	m_fifo.clear();

	// FIXME: should clearing the fifo trigger buffer empty interrupts?
	m_port[0].interrupt_status[3] |= ISR3_BE;
	m_port[0].interrupt_status[3] &= ~ISR3_BF;
	m_port[1].interrupt_status[3] |= ISR3_BE;
	m_port[1].interrupt_status[3] &= ~ISR3_BF;
}

void z8038_device::fifo_update()
{
	for (u8 port = 0; port < 2; port++)
	{
		// update byte count
		if (!(m_port[port].control_1 & CR1_FBCR))
			m_port[port].byte_count = m_fifo.queue_length();

		// pattern match check
		if ((m_port[port].data_buffer & ~m_port[port].pattern_mask) == (m_port[port].pattern_match & ~m_port[port].pattern_mask))
			m_port[port].interrupt_status[1] |= (ISR1_PMIP | ISR1_PMF);
		else
			m_port[port].interrupt_status[1] &= ~ISR1_PMF;

		// byte count comparison check
		if (m_fifo.queue_length() == m_port[port].byte_count_comparison)
			m_port[port].interrupt_status[2] |= ISR2_BCCIP;

		// buffer full check
		// TODO: test full pin (pin 37)
		if (m_fifo.full())
			m_port[port].interrupt_status[3] |= (ISR3_BF | ISR3_FIP);

		// buffer empty check
		// TODO: test empty pin (pin 37)
		if (m_fifo.empty())
			m_port[port].interrupt_status[3] |= (ISR3_BE | ISR3_EIP);
	}
}

TIMER_CALLBACK_MEMBER(z8038_device::int_check)
{
	for (u8 port = 0; port < 2; port++)
	{
		// check master interrupt enable
		if (!(m_port[port].control_0 & CR0_MIE))
		{
			set_int_state(port, false);
			continue;
		}

		// check any interrupts under service
		if (std::any_of(
			std::begin(m_port[port].interrupt_status),
			std::end(m_port[port].interrupt_status),
			[](u8 const val) { return bool(val & (ISR_HIUS | ISR_LIUS)); }))
			continue;

		// check for enabled and pending interrupts in priority order
		m_port[port].int_code = 7;
		for (u8 &isr : m_port[port].interrupt_status)
		{
			// check high interrupt enable and pending
			if ((isr & ISR_HIE) && (isr & ISR_HIP))
			{
				// set interrupt under service
				isr |= ISR_HIUS;
				break;
			}
			m_port[port].int_code--;

			if (m_port[port].int_code != 6)
			{
				// check low interrupt enable and pending
				if ((isr & ISR_LIE) && (isr & ISR_LIP))
				{
					// set interrupt under service
					isr |= ISR_LIUS;
					break;
				}
				m_port[port].int_code--;
			}
		}

		if (m_port[port].int_code)
			LOGMASKED(LOG_INT, "int_check port %d interrupt code %d detected\n", port + 1, m_port[port].int_code);

		// update interrupt state
		set_int_state(port, bool(m_port[port].int_code));
	}
}

void z8038_device::set_int_state(u8 const port, bool asserted)
{
	if (m_port[port].int_asserted != asserted)
	{
		LOGMASKED(LOG_INT, "set_int_state port %d interrupt %s\n",
			port + 1, asserted ? "asserted" : "deasserted");

		m_port[port].int_asserted = asserted;

		// line is active low
		m_out_int_cb[port](asserted ? 0 : 1);
	}
}
