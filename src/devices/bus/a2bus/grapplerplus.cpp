// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 * TODO for Buffered Grappler+:
 * - /RST does not reset the MCU, it’s connected to P27
 * - On the schematic, P22 is pulled up, but the program checks it - perhaps a test mode?
 */
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


ROM_START(bufgrapplerplus)
	// TODO: get proper ROM labels and confirm contents
	// despite having the same version string, this differs from the Grapper+ 3.2.u9 ROM

	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD( "3.2.u18", 0x0000, 0x1000, CRC(cd07c7ef) SHA1(6c2f1375b5df6bb65dfca1444c064661242fef1a) )

	ROM_REGION(0x0400, "mcu", 0)
	ROM_LOAD( "8048.u10", 0x0000, 0x0400, CRC(f60c14a9) SHA1(e7d87dd2f7af42f0eed776b839dea0a351ab67bc) )
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
	PORT_DIPNAME(0x08, 0x08, "Most Significant Bit")            PORT_DIPLOCATION("S1:1")        PORT_CHANGED_MEMBER(DEVICE_SELF, a2bus_grapplerplus_device_base, sw_msb, 0)
	PORT_DIPSETTING(   0x08, "Software Control")
	PORT_DIPSETTING(   0x00, "Not Transmitted")
INPUT_PORTS_END

} // anonymous namespace



DEFINE_DEVICE_TYPE(A2BUS_GRAPPLERPLUS, a2bus_grapplerplus_device, "a2grapplerplus", "Orange Micro Grappler+ Printer Interface")
DEFINE_DEVICE_TYPE(A2BUS_BUFGRAPPLERPLUS, a2bus_buf_grapplerplus_device, "a2bufgrapplerplus", "Orange Micro Buffered Grappler+ Printer Interface")



//==============================================================
//  Grappler+ base
//==============================================================

a2bus_grapplerplus_device_base::a2bus_grapplerplus_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_printer_conn(*this, "prn"),
	m_printer_out(*this, "prn_out"),
	m_s1(*this, "S1"),
	m_rom(*this, "rom"),
	m_rom_bank(0x0000U),
	m_ack_latch(1U),
	m_ack_in(1U),
	m_busy_in(1U),
	m_pe_in(1U),
	m_slct_in(1U)
{
}



//--------------------------------------------------
//  device_t implementation
//--------------------------------------------------

void a2bus_grapplerplus_device_base::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_printer_conn, centronics_devices, "printer");
	m_printer_conn->busy_handler().set(FUNC(a2bus_grapplerplus_device_base::busy_w));
	m_printer_conn->perror_handler().set(FUNC(a2bus_grapplerplus_device_base::pe_w));
	m_printer_conn->select_handler().set(FUNC(a2bus_grapplerplus_device_base::slct_w));

	OUTPUT_LATCH(config, m_printer_out);
	m_printer_conn->set_output_latch(*m_printer_out);
}


ioport_constructor a2bus_grapplerplus_device_base::device_input_ports() const
{
	return INPUT_PORTS_NAME(grapplerplus);
}


void a2bus_grapplerplus_device_base::device_start()
{
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_ack_latch));
	save_item(NAME(m_ack_in));
	save_item(NAME(m_busy_in));
	save_item(NAME(m_pe_in));
	save_item(NAME(m_slct_in));
}


void a2bus_grapplerplus_device_base::device_reset()
{
	m_ack_latch = 1U;
}



//--------------------------------------------------
//  device_a2bus_card_interface implementation
//--------------------------------------------------

void a2bus_grapplerplus_device_base::write_c0nx(u8 offset, u8 data)
{
	LOG("Write C0n%01X=%02X\n", offset, data);

	if (!(offset & 0x03U)) // !A0 && !A1 - write data
	{
		// latch output data
		LOG("Latch data %02X\n", data);
		data_latched(data);

		// clearing the ACK latch will acknowledge an interrupt
		if (m_ack_in)
		{
			if (m_ack_latch)
				LOG("Clearing acknowledge latch\n");
			else
				LOG("Previous data not acknowledged\n");
			m_ack_latch = 0U;
			ack_latch_cleared();
		}
		else
		{
			LOG("/ACK asserted, not clearing acknowledge latch\n");
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
}


u8 a2bus_grapplerplus_device_base::read_cnxx(u8 offset)
{
	if (!machine().side_effects_disabled())
	{
		if (m_rom_bank)
			LOG("Select low ROM bank\n");
		m_rom_bank = 0x0000U;
	}
	return m_rom[(!m_ack_latch && BIT(offset, 7)) ? (offset & 0xbfU) : offset];
}


void a2bus_grapplerplus_device_base::write_cnxx(u8 offset, u8 data)
{
	LOG("Write Cn%02X=%02X (bus conflict)\n", offset, data);

	if (m_rom_bank)
		LOG("Select low ROM bank\n");
	m_rom_bank = 0x0000U;
}


u8 a2bus_grapplerplus_device_base::read_c800(u16 offset)
{
	return m_rom[(offset & 0x07ffU) | m_rom_bank];
}



//--------------------------------------------------
//  printer status inputs
//--------------------------------------------------

WRITE_LINE_MEMBER(a2bus_grapplerplus_device_base::ack_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_grapplerplus_device_base::set_ack_in), this), state ? 1 : 0);
}


WRITE_LINE_MEMBER(a2bus_grapplerplus_device_base::busy_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_grapplerplus_device_base::set_busy_in), this), state ? 1 : 0);
}


WRITE_LINE_MEMBER(a2bus_grapplerplus_device_base::pe_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_grapplerplus_device_base::set_pe_in), this), state ? 1 : 0);
}


WRITE_LINE_MEMBER(a2bus_grapplerplus_device_base::slct_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_grapplerplus_device_base::set_slct_in), this), state ? 1 : 0);
}



//--------------------------------------------------
//  synchronised printer status inputs
//--------------------------------------------------

void a2bus_grapplerplus_device_base::set_ack_in(void *ptr, s32 param)
{
	if (u32(param) != m_ack_in)
	{
		LOG("/ACK=%d\n", param);
		m_ack_in = u8(u32(param));
		if (!param)
		{
			if (!m_ack_latch)
				LOG("Set acknowledge latch\n");
			else
				LOG("No data written since previous acknowledge\n");
			m_ack_latch = 1U;
			ack_latch_set();
		}
	}
}


void a2bus_grapplerplus_device_base::set_busy_in(void *ptr, s32 param)
{
	if (u32(param) != m_busy_in)
	{
		LOG("BUSY=%d\n", param);
		m_busy_in = u8(u32(param));
	}
}


void a2bus_grapplerplus_device_base::set_pe_in(void *ptr, s32 param)
{
	if (u32(param) != m_pe_in)
	{
		LOG("PAPER EMPTY=%d\n", param);
		m_pe_in = u8(u32(param));
	}
}


void a2bus_grapplerplus_device_base::set_slct_in(void *ptr, s32 param)
{
	if (u32(param) != m_slct_in)
	{
		LOG("SELECT=%d\n", state);
		m_slct_in = u8(u32(param));
	}
}



//==============================================================
//  Grappler+ implementation
//==============================================================

a2bus_grapplerplus_device::a2bus_grapplerplus_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	a2bus_grapplerplus_device_base(mconfig, A2BUS_GRAPPLERPLUS, tag, owner, clock),
	m_strobe_timer(nullptr),
	m_data_latch(0xffU),
	m_irq_disable(1U),
	m_irq(0x00U),
	m_next_strobe(1U)
{
}



//--------------------------------------------------
//  DIP switch handlers
//--------------------------------------------------

INPUT_CHANGED_MEMBER(a2bus_grapplerplus_device::sw_msb)
{
	if (BIT(m_data_latch, 7))
		m_printer_out->write(m_data_latch & (BIT(m_s1->read(), 3) ? 0xffU : 0x7fU));
}



//--------------------------------------------------
//  device_a2bus_card_interface implementation
//--------------------------------------------------

u8 a2bus_grapplerplus_device::read_c0nx(u8 offset)
{
	return
			m_irq |
			((m_s1->read() & 0x07U) << 4) |
			(busy_in() << 3) |
			(pe_in() << 2) |
			(slct_in() << 1) |
			ack_latch();
}


void a2bus_grapplerplus_device::write_c0nx(u8 offset, u8 data)
{
	a2bus_grapplerplus_device_base::write_c0nx(offset, data);

	if (BIT(offset, 1)) // A1 - disable interrupt
	{
		if (!m_irq_disable)
			LOG("Disable interrupt request\n");
		else
			LOG("Interrupt request already disabled\n");
		m_irq_disable = 1U;
		if (m_irq)
		{
			assert(ack_latch());
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
		if (ack_latch() && !m_irq)
		{
			LOG("Asserting slot IRQ\n");
			m_irq = 0x80U;
			raise_slot_irq();
		}
	}
}



//--------------------------------------------------
//  device_t implementation
//--------------------------------------------------

tiny_rom_entry const *a2bus_grapplerplus_device::device_rom_region() const
{
	return ROM_NAME(grapplerplus);
}


void a2bus_grapplerplus_device::device_add_mconfig(machine_config &config)
{
	a2bus_grapplerplus_device_base::device_add_mconfig(config);

	m_printer_conn->ack_handler().set(FUNC(a2bus_grapplerplus_device::ack_w));
}


void a2bus_grapplerplus_device::device_start()
{
	a2bus_grapplerplus_device_base::device_start();

	m_strobe_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(a2bus_grapplerplus_device::update_strobe), this));

	m_next_strobe = 1U;

	save_item(NAME(m_data_latch));
	save_item(NAME(m_irq_disable));
	save_item(NAME(m_irq));
	save_item(NAME(m_next_strobe));

	m_strobe_timer->adjust(attotime::from_ticks(7, clock()));
}


void a2bus_grapplerplus_device::device_reset()
{
	a2bus_grapplerplus_device_base::device_reset();

	m_irq_disable = 1U;
	if (m_irq)
	{
		m_irq = 0x00U;
		lower_slot_irq();
	}
}



//--------------------------------------------------
//  a2bus_grapplerplus_device_base implementation
//--------------------------------------------------

void a2bus_grapplerplus_device::data_latched(u8 data)
{
	// remember MSB can be forced low by DIP switch
	m_data_latch = data;
	m_printer_out->write(m_data_latch & (BIT(m_s1->read(), 3) ? 0xffU : 0x7fU));

	// generate strobe pulse after one clock cycle
	m_next_strobe = 0U;
	if (!m_strobe_timer->enabled())
	{
		LOG("Start strobe timer\n");
		m_strobe_timer->adjust(attotime::from_ticks(7, clock()));
	}
}


void a2bus_grapplerplus_device::ack_latch_set()
{
	if (!m_irq_disable && !m_irq)
	{
		LOG("Asserting slot IRQ\n");
		m_irq = 0x80U;
		raise_slot_irq();
	}
}


void a2bus_grapplerplus_device::ack_latch_cleared()
{
	if (m_irq)
	{
		assert(!m_irq_disable);
		LOG("Releasing slot IRQ\n");
		m_irq = 0x00U;
		lower_slot_irq();
	}
}



//--------------------------------------------------
//  timer handlers
//--------------------------------------------------

TIMER_CALLBACK_MEMBER(a2bus_grapplerplus_device::update_strobe)
{
	LOG("Output /STROBE=%u\n", m_next_strobe);
	m_printer_conn->write_strobe(m_next_strobe);
	if (!m_next_strobe)
	{
		LOG("Start strobe timer\n");
		m_next_strobe = 1U;
		m_strobe_timer->adjust(attotime::from_ticks(7, clock()));
	}
}



//==============================================================
//  Buffered Grappler+ implementation
//==============================================================

a2bus_buf_grapplerplus_device::a2bus_buf_grapplerplus_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	a2bus_grapplerplus_device_base(mconfig, A2BUS_BUFGRAPPLERPLUS, tag, owner, clock),
	m_mcu(*this, "mcu"),
	m_ram(),
	m_ram_row(0xff00),
	m_mcu_p2(0xffU),
	m_data_latch(0xffU),
	m_ibusy(1U),
	m_buf_ack_latch(1U),
	m_buf_ack_in(1U)
{
}



//--------------------------------------------------
//  device_a2bus_card_interface implementation
//--------------------------------------------------

u8 a2bus_buf_grapplerplus_device::read_c0nx(u8 offset)
{
	return
			((m_s1->read() & 0x0fU) << 4) |
			(m_ibusy << 3) |
			(pe_in() << 2) |
			(slct_in() << 1) |
			ack_latch();
}



//--------------------------------------------------
//  device_t implementation
//--------------------------------------------------

tiny_rom_entry const *a2bus_buf_grapplerplus_device::device_rom_region() const
{
	return ROM_NAME(bufgrapplerplus);
}


void a2bus_buf_grapplerplus_device::device_add_mconfig(machine_config &config)
{
	a2bus_grapplerplus_device_base::device_add_mconfig(config);

	m_printer_conn->ack_handler().set(FUNC(a2bus_buf_grapplerplus_device::buf_ack_w));

	I8048(config, m_mcu, DERIVED_CLOCK(1, 1));
	m_mcu->set_addrmap(AS_IO, &a2bus_buf_grapplerplus_device::mcu_io);
	m_mcu->p2_out_cb().set(FUNC(a2bus_buf_grapplerplus_device::mcu_p2_w));
	m_mcu->t0_in_cb().set([this] () { return busy_in(); });
	m_mcu->t1_in_cb().set([this] () { return m_buf_ack_latch; });
	m_mcu->bus_in_cb().set(FUNC(a2bus_buf_grapplerplus_device::mcu_bus_r));
	m_mcu->bus_out_cb().set(FUNC(a2bus_buf_grapplerplus_device::mcu_bus_w));
}


void a2bus_buf_grapplerplus_device::device_start()
{
	a2bus_grapplerplus_device_base::device_start();

	m_ram = std::make_unique<u8 []>(0x10000);

	save_pointer(NAME(m_ram), 0x10000);
	save_item(NAME(m_ram_row));
	save_item(NAME(m_mcu_p2));
	save_item(NAME(m_data_latch));
	save_item(NAME(m_ibusy));
	save_item(NAME(m_buf_ack_latch));
	save_item(NAME(m_buf_ack_in));
}



//--------------------------------------------------
//  a2bus_grapplerplus_device_base implementation
//--------------------------------------------------

void a2bus_buf_grapplerplus_device::data_latched(u8 data)
{
	// IBUSY is exposed on C0nX
	if (!m_ibusy)
		LOG("Setting IBUSY\n");
	else
		LOG("IBUSY already set\n");
	m_ibusy = 1U;

	// these signals cross executable device domains
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_buf_grapplerplus_device::set_buf_data), this), int(unsigned(data)));
	m_mcu->set_input_line(MCS48_INPUT_IRQ, ASSERT_LINE);
}



//--------------------------------------------------
//  printer status inputs
//--------------------------------------------------

DECLARE_WRITE_LINE_MEMBER(a2bus_buf_grapplerplus_device::buf_ack_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_buf_grapplerplus_device::set_buf_ack_in), this), state ? 1 : 0);
}



//--------------------------------------------------
//  MCU I/O handlers
//--------------------------------------------------

void a2bus_buf_grapplerplus_device::mcu_io(address_map &map)
{
	map(0x00, 0xff).nopr(); // read to put the BUS lines in high-impedance state before a real read using INS
}


void a2bus_buf_grapplerplus_device::mcu_p2_w(u8 data)
{
	//P22 = pulled up
	//P27 = /RST

	// check for changed bits
	u8 const diff(data ^ m_mcu_p2);
	m_mcu_p2 = data;

	// P20 enables /CAS on /RD or /WR
	if (BIT(diff, 0))
		LOG("RAM EN=%u\n", BIT(data, 0));

	// row address strobe
	if (BIT(diff, 1))
	{
		if (!BIT(data, 1))
		{
			LOG("Row address %02X\n", m_mcu->p1_r());
			m_ram_row = u16(m_mcu->p1_r()) << 8;
		}
		else
		{
			LOG("Released /RAS\n");
		}
	}

	// P23 is the /IOEN signal
	if (BIT(diff, 3))
		LOG("/IOEN=%u\n", BIT(data, 3));

	// P24 allows fast DRAM refresh using ALE for /RAS
	if (BIT(diff, 4))
		LOG(BIT(data, 4) ? "Start DRAM refresh\n" : "End DRAM refresh\n");

	// P25 is the strobe output and clears the acknowledge latch
	if (BIT(diff, 5))
	{
		if (BIT(data, 5))
		{
			if (m_buf_ack_in)
			{
				if (m_buf_ack_latch)
					LOG("T1 already set\n");
				else
					LOG("Setting T1\n");
				m_buf_ack_latch = 1U;
			}
			else
			{
				LOG("/ACK asserted, not setting T1\n");
			}
		}
		LOG("Output /STROBE=%u\n", BIT(data, 5));
		m_printer_conn->write_strobe(BIT(data, 5));
	}
}


u8 a2bus_buf_grapplerplus_device::mcu_bus_r()
{
	u8 result(0xffU);

	// RAM EN enables RAM
	if (BIT(m_mcu_p2, 0))
	{
		if (!BIT(m_mcu_p2, 1))
		{
			u16 const addr(m_ram_row | u16(m_mcu->p1_r()));
			LOG("Read RAM @%04X=%02X\n", addr, m_ram[addr]);
			result &= m_ram[addr];
		}
		else
		{
			LOG("/RD asserted /CAS while /RAS not asserted\n");
		}
	}

	// /IOEN enables the data latch, pulses /IACK and clears IBUSY on /RD
	if (!BIT(m_mcu_p2, 3))
	{
		LOG("Read data latch %02X\n", m_data_latch);
		result &= m_data_latch;
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_buf_grapplerplus_device::clear_ibusy), this));
		m_mcu->set_input_line(MCS48_INPUT_IRQ, CLEAR_LINE);
		ack_w(0);
		ack_w(1);
	}

	return result;
}


void a2bus_buf_grapplerplus_device::mcu_bus_w(u8 data)
{
	// RAM EN enables RAM
	if (BIT(m_mcu_p2, 0))
	{
		if (!BIT(m_mcu_p2, 1))
		{
			u16 const addr(m_ram_row | u16(m_mcu->p1_r()));
			LOG("Wrote RAM @%04X=%02X\n", addr, data);
			m_ram[addr] = data;
		}
		else
		{
			LOG("/WR asserted /CAS while /RAS not asserted\n");
		}
	}

	// /IOEN enables output latch
	if (!BIT(m_mcu_p2, 3))
	{
		LOG("Output data %02X\n", data);
		m_printer_out->write(data);
	}
}



//--------------------------------------------------
//  synchronised signals
//--------------------------------------------------

void a2bus_buf_grapplerplus_device::set_buf_data(void *ptr, s32 param)
{
	m_data_latch = u8(u32(param));
}


void a2bus_buf_grapplerplus_device::set_buf_ack_in(void *ptr, s32 param)
{
	if (u32(param) != m_buf_ack_in)
	{
		LOG("/ACK=%d\n", param);
		m_buf_ack_in = u8(u32(param));
		if (!param)
		{
			if (m_buf_ack_latch)
				LOG("Clearing T1\n");
			else
				LOG("No data output since previous acknowledge\n");
			m_buf_ack_latch = 0U;
		}
	}
}


void a2bus_buf_grapplerplus_device::clear_ibusy(void *ptr, s32 param)
{
	if (m_ibusy)
	{
		LOG("Clearing IBUSY\n");
		m_ibusy = 0U;
	}
	else
	{
		LOG("IBUSY already clear\n");
	}
}
