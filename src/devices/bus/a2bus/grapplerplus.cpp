// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "emu.h"
#include "grapplerplus.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

namespace {

ROM_START(grapplerplus)
	// TODO: add other revisions - 3.3 is known to exist

	ROM_DEFAULT_BIOS("v32")
	ROM_SYSTEM_BIOS(0, "v30", "ROM 3.0")
	ROM_SYSTEM_BIOS(1, "v32", "ROM 3.2")

	ROM_REGION(0x1000, "rom", 0)
	ROMX_LOAD( "3.0.u9", 0x0000, 0x1000, CRC(17cf5e02) SHA1(9b01a9b1cf7752987e03f9b0285eb9329d63b777), ROM_BIOS(0) )
	ROMX_LOAD( "3.2.u9", 0x0000, 0x1000, CRC(6f88b70c) SHA1(433ae61a0553ee9c1628ea5b6376dac848c04cad), ROM_BIOS(1) )
ROM_END


INPUT_PORTS_START(grapplerplus)
	PORT_START("S1")
	PORT_DIPNAME(0x07, 0x00, "Printer Type")                    PORT_DIPLOCATION("S1:4,3,2")
	PORT_DIPSETTING(   0x00, "Epson Series")
	PORT_DIPSETTING(   0x01, "NEC 8023/C. Itoh 8510/DMP 85")
	PORT_DIPSETTING(   0x02, "Star Gemini")
	PORT_DIPSETTING(   0x03, "Anadex Printers")
	PORT_DIPSETTING(   0x04, "Okidata 82A, 83A, 92, 93, 84")
	PORT_DIPSETTING(   0x06, "Okidata 84 w/o Step II Graphics")
	PORT_DIPSETTING(   0x05, "Apple Dot Matrix")
	PORT_DIPSETTING(   0x07, "invalid")
	PORT_DIPNAME(0x08, 0x08, "Most Significant Bit")            PORT_DIPLOCATION("S1:1")        PORT_CHANGED_MEMBER(DEVICE_SELF, a2bus_grapplerplus_device, sw_msb, 0)
	PORT_DIPSETTING(   0x08, "Software Control")
	PORT_DIPSETTING(   0x00, "Not Transmitted")
INPUT_PORTS_END

} // anonymous namespace



DEFINE_DEVICE_TYPE(A2BUS_GRAPPLERPLUS, a2bus_grapplerplus_device, "a2grapplerplus", "Orange Micro Grappler+ Printer Interface")



a2bus_grapplerplus_device::a2bus_grapplerplus_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, A2BUS_GRAPPLERPLUS, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_printer_conn(*this, "prn"),
	m_printer_out(*this, "prn_out"),
	m_s1(*this, "S1"),
	m_rom(*this, "rom"),
	m_strobe_timer(nullptr),
	m_rom_bank(0x0000U),
	m_data_latch(0xffU),
	m_ack_latch(0x01U),
	m_irq_disable(1U),
	m_irq(0x00U),
	m_next_strobe(1U),
	m_ack_in(1U),
	m_busy_in(0x8U),
	m_pe_in(0x04U),
	m_slct_in(0x02U)
{
}



//----------------------------------------------
//  DIP switch handlers
//----------------------------------------------

INPUT_CHANGED_MEMBER(a2bus_grapplerplus_device::sw_msb)
{
	if (BIT(m_data_latch, 7))
		m_printer_out->write(m_data_latch & (BIT(m_s1->read(), 3) ? 0xffU : 0x7fU));
}



//----------------------------------------------
//  device_a2bus_card_interface implementation
//----------------------------------------------

u8 a2bus_grapplerplus_device::read_c0nx(u8 offset)
{
	return
			m_irq |
			((m_s1->read() & 0x07U) << 4) |
			m_busy_in |
			m_pe_in |
			m_slct_in |
			m_ack_latch;
}


void a2bus_grapplerplus_device::write_c0nx(u8 offset, u8 data)
{
	LOG("Write C0n%01X=%02X\n", offset, data);

	if (!(offset & 0x03U)) // !A0 && !A1 - write data
	{
		// latch output data - remember MSB can be forced low by DIP switch
		LOG("Latch data %02X\n", data);
		m_data_latch = data;
		m_printer_out->write(data & (BIT(m_s1->read(), 3) ? 0xffU : 0x7fU));

		// clearing the ACK latch will acknowledge an interrupt
		if (m_ack_in)
		{
			if (m_ack_latch)
				LOG("Clearing acknowledge latch\n");
			else
				LOG("Previous data not acknowledged\n");
			m_ack_latch = 0x00U;
			if (m_irq)
			{
				assert(!m_irq_disable);
				LOG("Releasing slot IRQ\n");
				m_irq = 0x00U;
				lower_slot_irq();
			}
		}
		else
		{
			LOG("/ACK asserted, not clearing acknowledge latch\n");
		}

		// generate strobe pulse after one clock cycle
		m_next_strobe = 0U;
		if (!m_strobe_timer->enabled())
		{
			LOG("Start strobe timer\n");
			m_strobe_timer->adjust(attotime::from_ticks(1, clock()));
		}
	}

	if (BIT(offset, 0)) // A0 - select high ROM bank
	{
		if (!m_rom_bank)
			LOG("Select high ROM bank\n");
		else
			LOG("High ROM bank already selected\n");
		m_rom_bank = 0x0800U;
	}

	if (BIT(offset, 1)) // A1 - disable interrupt
	{
		if (!m_irq_disable)
			LOG("Disable interrupt request\n");
		else
			LOG("Interrupt request already disabled\n");
		m_irq_disable = 1U;
		if (m_irq)
		{
			assert(m_ack_latch);
			LOG("Releasing slot IRQ\n");
			m_irq = 0x00U;
			lower_slot_irq();
		}
	}
	else if (BIT(offset, 2)) // A2 - enable interrupt
	{
		if (m_irq_disable)
			LOG("Enable interrupt request\n");
		else
			LOG("Interrupt request already enabled\n");
		m_irq_disable = 0U;
		if (m_ack_latch && !m_irq)
		{
			LOG("Asserting slot IRQ\n");
			m_irq = 0x80U;
			raise_slot_irq();
		}
	}
}


u8 a2bus_grapplerplus_device::read_cnxx(u8 offset)
{
	if (!machine().side_effects_disabled())
	{
		if (m_rom_bank)
			LOG("Select low ROM bank\n");
		m_rom_bank = 0x0000U;
	}
	return m_rom[(!m_ack_latch && BIT(offset, 7)) ? (offset & 0xbfU) : offset];
}


void a2bus_grapplerplus_device::write_cnxx(u8 offset, u8 data)
{
	LOG("Write Cn%02X=%02X (bus conflict)\n", offset, data);

	if (m_rom_bank)
		LOG("Select low ROM bank\n");
	m_rom_bank = 0x0000U;
}


u8 a2bus_grapplerplus_device::read_c800(u16 offset)
{
	return m_rom[(offset & 0x07ffU) | m_rom_bank];
}



//----------------------------------------------
//  device_t implementation
//----------------------------------------------

tiny_rom_entry const *a2bus_grapplerplus_device::device_rom_region() const
{
	return ROM_NAME(grapplerplus);
}


void a2bus_grapplerplus_device::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_printer_conn, centronics_devices, "printer");
	m_printer_conn->ack_handler().set(FUNC(a2bus_grapplerplus_device::ack_w));
	m_printer_conn->busy_handler().set(FUNC(a2bus_grapplerplus_device::busy_w));
	m_printer_conn->perror_handler().set(FUNC(a2bus_grapplerplus_device::pe_w));
	m_printer_conn->select_handler().set(FUNC(a2bus_grapplerplus_device::slct_w));

	OUTPUT_LATCH(config, m_printer_out);
	m_printer_conn->set_output_latch(*m_printer_out);
}


ioport_constructor a2bus_grapplerplus_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(grapplerplus);
}


void a2bus_grapplerplus_device::device_start()
{
	m_strobe_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(a2bus_grapplerplus_device::update_strobe), this));

	m_next_strobe = 1U;

	save_item(NAME(m_rom_bank));
	save_item(NAME(m_data_latch));
	save_item(NAME(m_ack_latch));
	save_item(NAME(m_irq_disable));
	save_item(NAME(m_irq));
	save_item(NAME(m_next_strobe));
	save_item(NAME(m_ack_in));
	save_item(NAME(m_busy_in));
	save_item(NAME(m_pe_in));
	save_item(NAME(m_slct_in));

	m_strobe_timer->adjust(attotime::from_ticks(1, clock()));
}


void a2bus_grapplerplus_device::device_reset()
{
	m_ack_latch = 0x01U;
	m_irq_disable = 1U;
	if (m_irq)
	{
		m_irq = 0x00U;
		lower_slot_irq();
	}
}



//----------------------------------------------
//  printer status inputs
//----------------------------------------------

WRITE_LINE_MEMBER(a2bus_grapplerplus_device::ack_w)
{
	if (bool(state) != bool(m_ack_in))
	{
		LOG("/ACK=%d\n", state);
		m_ack_in = state ? 1U : 0U;
		if (!state)
		{
			if (!m_ack_latch)
				LOG("Set acknowledge latch\n");
			else
				LOG("No data written since previous acknowledge\n");
			m_ack_latch = 0x01U;
			if (!m_irq_disable && !m_irq)
			{
				LOG("Asserting slot IRQ\n");
				m_irq = 0x80U;
				raise_slot_irq();
			}
		}
	}
}


WRITE_LINE_MEMBER(a2bus_grapplerplus_device::busy_w)
{
	if (bool(state) != bool(m_busy_in))
	{
		LOG("BUSY=%d\n", state);
		m_busy_in = state ? 0x08U : 0x00U;
	}
}


WRITE_LINE_MEMBER(a2bus_grapplerplus_device::pe_w)
{
	if (bool(state) != bool(m_pe_in))
	{
		LOG("PAPER EMPTY=%d\n", state);
		m_pe_in = state ? 0x04U : 0x00U;
	}
}


WRITE_LINE_MEMBER(a2bus_grapplerplus_device::slct_w)
{
	if (bool(state) != bool(m_slct_in))
	{
		LOG("SELECT=%d\n", state);
		m_slct_in = state ? 0x02U : 0x00U;
	}
}



//----------------------------------------------
//  timer handlers
//----------------------------------------------

TIMER_CALLBACK_MEMBER(a2bus_grapplerplus_device::update_strobe)
{
	LOG("Output /STROBE=%u\n", m_next_strobe);
	m_printer_conn->write_strobe(m_next_strobe);
	if (!m_next_strobe)
	{
		LOG("Start strobe timer\n");
		m_next_strobe = 1U;
		m_strobe_timer->adjust(attotime::from_ticks(1, clock()));
	}
}
