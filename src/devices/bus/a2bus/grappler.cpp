// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "emu.h"
#include "grappler.h"

#include "bus/centronics/ctronics.h"
#include "cpu/mcs48/mcs48.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

namespace {

ROM_START(grappler)
	ROM_REGION(0x0800, "rom", 0)
	ROM_LOAD( "eps-1_c1981.u6", 0x0000, 0x0800, CRC(862773cb) SHA1(791ebae64a7fad8f42bdfaec36b9e2d34f12ded8) )
ROM_END


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
	// TODO: confirm contents
	// despite having the same version string, this differs from the Grapper+ 3.2.u9 ROM

	// 9433B-0141
	// 90ROM00002
	// GI8603CEY
	// TAIWAN
	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD( "90rom00002.u18", 0x0000, 0x1000, CRC(cd07c7ef) SHA1(6c2f1375b5df6bb65dfca1444c064661242fef1a) )

	// S 8450
	// SCN8048A
	// C6N40 B
	// 95ROM08048
	ROM_REGION(0x0400, "mcu", 0)
	ROM_LOAD( "95rom08048.u10", 0x0000, 0x0400, CRC(55990f67) SHA1(f0907cf02d8c5dbf1bfacb0d626f2231142f0ff7) )
ROM_END



//==============================================================
//  Grappler base
//==============================================================

class a2bus_grappler_device_base : public device_t, public device_a2bus_card_interface
{
public:
	// device_a2bus_card_interface implementation
	virtual u8 read_c800(u16 offset) override;

protected:
	a2bus_grappler_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// signal state
	u8 busy_in() const { return m_busy_in; }
	u8 pe_in() const { return m_pe_in; }
	u8 slct_in() const { return m_slct_in; }

	required_device<centronics_device>      m_printer_conn;
	required_device<output_latch_device>    m_printer_out;
	required_region_ptr<u8>                 m_rom;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// helpers
	void set_rom_bank(u16 rom_bank);

private:
	// printer status inputs
	void busy_w(int state);
	void pe_w(int state);
	void slct_w(int state);

	// synchronised printer status inputs
	void set_busy_in(s32 param);
	void set_pe_in(s32 param);
	void set_slct_in(s32 param);

	u16 m_rom_bank;     // U2D (pin 13)
	u8  m_busy_in;      // printer connector pin 21 (synchronised)
	u8  m_pe_in;        // printer connector pin 23 (synchronised)
	u8  m_slct_in;      // printer connector pin 25 (synchronised)
};


a2bus_grappler_device_base::a2bus_grappler_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_printer_conn(*this, "prn"),
	m_printer_out(*this, "prn_out"),
	m_rom(*this, "rom"),
	m_rom_bank(0x0000U),
	m_busy_in(1U),
	m_pe_in(1U),
	m_slct_in(1U)
{
}



//--------------------------------------------------
//  device_a2bus_card_interface implementation
//--------------------------------------------------

u8 a2bus_grappler_device_base::read_c800(u16 offset)
{
	return m_rom[(offset & 0x07ffU) | m_rom_bank];
}



//--------------------------------------------------
//  device_t implementation
//--------------------------------------------------

void a2bus_grappler_device_base::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_printer_conn, centronics_devices, "printer");
	m_printer_conn->busy_handler().set(FUNC(a2bus_grappler_device_base::busy_w));
	m_printer_conn->perror_handler().set(FUNC(a2bus_grappler_device_base::pe_w));
	m_printer_conn->select_handler().set(FUNC(a2bus_grappler_device_base::slct_w));

	OUTPUT_LATCH(config, m_printer_out);
	m_printer_conn->set_output_latch(*m_printer_out);
}


void a2bus_grappler_device_base::device_start()
{
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_busy_in));
	save_item(NAME(m_pe_in));
	save_item(NAME(m_slct_in));
}



//--------------------------------------------------
//  printer status inputs
//--------------------------------------------------

void a2bus_grappler_device_base::set_rom_bank(u16 rom_bank)
{
	if (m_rom_bank != rom_bank)
		LOG("Select ROM bank %04X\n", rom_bank);
	m_rom_bank = rom_bank;
}



//--------------------------------------------------
//  printer status inputs
//--------------------------------------------------

void a2bus_grappler_device_base::busy_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_grappler_device_base::set_busy_in), this), state ? 1 : 0);
}


void a2bus_grappler_device_base::pe_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_grappler_device_base::set_pe_in), this), state ? 1 : 0);
}


void a2bus_grappler_device_base::slct_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_grappler_device_base::set_slct_in), this), state ? 1 : 0);
}



//--------------------------------------------------
//  synchronised printer status inputs
//--------------------------------------------------

void a2bus_grappler_device_base::set_busy_in(s32 param)
{
	if (u32(param) != m_busy_in)
	{
		LOG("BUSY=%d\n", param);
		m_busy_in = u8(u32(param));
	}
}


void a2bus_grappler_device_base::set_pe_in(s32 param)
{
	if (u32(param) != m_pe_in)
	{
		LOG("PAPER EMPTY=%d\n", param);
		m_pe_in = u8(u32(param));
	}
}


void a2bus_grappler_device_base::set_slct_in(s32 param)
{
	if (u32(param) != m_slct_in)
	{
		LOG("SELECT=%d\n", param);
		m_slct_in = u8(u32(param));
	}
}



//==============================================================
//  Grappler implementation
//==============================================================

class a2bus_grappler_device : public a2bus_grappler_device_base
{
public:
	a2bus_grappler_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual void write_cnxx(u8 offset, u8 data) override;

protected:
	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override { return ROM_NAME(grappler); }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// printer status inputs
	void ack_w(int state);

	// synchronised signals
	void set_data(s32 param);
	void set_strobe(s32 param);
	void set_ack_in(s32 param);

	u8  m_strobe;       // U3 (pin 4)
	u8  m_ack_latch;    // U3 (pin 13)
	u8  m_ack_in;       // printer connector pin 19 (synchronised)
};


a2bus_grappler_device::a2bus_grappler_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	a2bus_grappler_device_base(mconfig, A2BUS_GRAPPLER, tag, owner, clock),
	m_strobe(1U),
	m_ack_latch(1U),
	m_ack_in(1U)
{
}



//--------------------------------------------------
//  device_a2bus_card_interface implementation
//--------------------------------------------------

u8 a2bus_grappler_device::read_c0nx(u8 offset)
{
	LOG("Read C0n%01X\n", offset);

	if (!machine().side_effects_disabled())
	{
		if (BIT(offset, 1)) // A1 - assert strobe
			machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_grappler_device::set_strobe), this), 0);
		else if (BIT(offset, 2)) // A2 - release strobe
			machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_grappler_device::set_strobe), this), 1);
	}

	if (BIT(offset, 0)) // A0 - printer status
	{
		return
				0xf0U | // TODO: actually open bus
				(busy_in() << 3) |
				(pe_in() << 2) |
				(slct_in() << 1) |
				m_ack_latch;
	}
	else
	{
		return 0xffU; // TODO: actually open bus
	}
}


void a2bus_grappler_device::write_c0nx(u8 offset, u8 data)
{
	LOG("Write C0n%01X=%02X\n", offset, data);

	if (BIT(offset, 0)) // A0 - write data
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_grappler_device::set_data), this), int(unsigned(data)));

	if (BIT(offset, 1)) // A1 - assert strobe
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_grappler_device::set_strobe), this), 0);
	else if (BIT(offset, 2)) // A2 - release strobe
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_grappler_device::set_strobe), this), 1);
}


u8 a2bus_grappler_device::read_cnxx(u8 offset)
{
	return m_rom[offset | (slotno() << 8)];
}


void a2bus_grappler_device::write_cnxx(u8 offset, u8 data)
{
	LOG("Write Cn%02X=%02X (bus conflict)\n", offset, data);
}



//--------------------------------------------------
//  device_t implementation
//--------------------------------------------------

void a2bus_grappler_device::device_add_mconfig(machine_config &config)
{
	a2bus_grappler_device_base::device_add_mconfig(config);

	m_printer_conn->ack_handler().set(FUNC(a2bus_grappler_device::ack_w));
}


void a2bus_grappler_device::device_start()
{
	a2bus_grappler_device_base::device_start();

	save_item(NAME(m_strobe));
	save_item(NAME(m_ack_latch));
	save_item(NAME(m_ack_in));
}


void a2bus_grappler_device::device_reset()
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_grappler_device::set_strobe), this), 1);
}



//--------------------------------------------------
//  printer status inputs
//--------------------------------------------------

void a2bus_grappler_device::ack_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_grappler_device::set_ack_in), this), state ? 1 : 0);
}



//--------------------------------------------------
//  synchronised signals
//--------------------------------------------------

void a2bus_grappler_device::set_data(s32 param)
{
	LOG("Output data %02X\n", u8(u32(param)));
	m_printer_out->write(u8(u32(param)));
}


void a2bus_grappler_device::set_strobe(s32 param)
{
	LOG("Output /STROBE=%d\n", param);
	m_printer_conn->write_strobe(param);
	if (m_strobe && !param)
	{
		if (m_ack_in)
		{
			if (!m_ack_latch)
				LOG("Clearing acknowledge latch\n");
			else
				LOG("Previous data not acknowledged\n");
			m_ack_latch = 1U;
		}
		else
		{
			LOG("/ACK asserted, not clearing acknowledge latch\n");
		}
	}
	m_strobe = u8(u32(param));
}


void a2bus_grappler_device::set_ack_in(s32 param)
{
	if (u32(param) != m_ack_in)
	{
		LOG("/ACK=%d\n", param);
		m_ack_in = u8(u32(param));
		if (!param)
		{
			if (m_ack_latch)
				LOG("Set acknowledge latch\n");
			else
				LOG("No data written since previous acknowledge\n");
			m_ack_latch = 0U;
		}
		else if (!m_strobe)
		{
			LOG("Clearing acknowledge latch\n");
			m_ack_latch = 1U;
		}
	}
}



//==============================================================
//  Grappler+ base
//==============================================================

class a2bus_grapplerplus_device_base : public a2bus_grappler_device_base
{
public:
	// DIP switch handlers
	virtual DECLARE_INPUT_CHANGED_MEMBER(sw_msb) { }

	// device_a2bus_card_interface implementation
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual void write_cnxx(u8 offset, u8 data) override;

protected:
	a2bus_grapplerplus_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// ACK latch set input
	void ack_w(int state);

	// signal state
	u8 ack_latch() const { return m_ack_latch; }

	required_ioport m_s1;

private:
	// synchronised printer status inputs
	void set_ack_in(s32 param);

	// for derived devices to implement
	virtual void data_latched(u8 data) = 0;
	virtual void ack_latch_set() { }
	virtual void ack_latch_cleared() { }

	u8  m_ack_latch;    // U2C (pin 9)
	u8  m_ack_in;       // printer connector pin 19 (synchronised)
};


a2bus_grapplerplus_device_base::a2bus_grapplerplus_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
	a2bus_grappler_device_base(mconfig, type, tag, owner, clock),
	m_s1(*this, "S1"),
	m_ack_latch(1U),
	m_ack_in(1U)
{
}



//--------------------------------------------------
//  device_t implementation
//--------------------------------------------------

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


ioport_constructor a2bus_grapplerplus_device_base::device_input_ports() const
{
	return INPUT_PORTS_NAME(grapplerplus);
}


void a2bus_grapplerplus_device_base::device_start()
{
	a2bus_grappler_device_base::device_start();

	save_item(NAME(m_ack_latch));
	save_item(NAME(m_ack_in));
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
		set_rom_bank(0x0800U);
}


u8 a2bus_grapplerplus_device_base::read_cnxx(u8 offset)
{
	if (!machine().side_effects_disabled())
		set_rom_bank(0x0000U);
	return m_rom[(!m_ack_latch && BIT(offset, 7)) ? (offset & 0xbfU) : offset];
}


void a2bus_grapplerplus_device_base::write_cnxx(u8 offset, u8 data)
{
	LOG("Write Cn%02X=%02X (bus conflict)\n", offset, data);

	set_rom_bank(0x0000U);
}



//--------------------------------------------------
//  ACK latch set input
//--------------------------------------------------

void a2bus_grapplerplus_device_base::ack_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_grapplerplus_device_base::set_ack_in), this), state ? 1 : 0);
}



//--------------------------------------------------
//  synchronised printer status inputs
//--------------------------------------------------

void a2bus_grapplerplus_device_base::set_ack_in(s32 param)
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



//==============================================================
//  Grappler+ implementation
//==============================================================

class a2bus_grapplerplus_device : public a2bus_grapplerplus_device_base
{
public:
	a2bus_grapplerplus_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// DIP switch handlers
	virtual DECLARE_INPUT_CHANGED_MEMBER(sw_msb) override;

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;

protected:
	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override { return ROM_NAME(grapplerplus); }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// a2bus_grapplerplus_device_base implementation
	virtual void data_latched(u8 data) override;
	virtual void ack_latch_set() override;
	virtual void ack_latch_cleared() override;

	// timer handlers
	TIMER_CALLBACK_MEMBER(update_strobe);

	emu_timer * m_strobe_timer;

	u8  m_data_latch;   // U10
	u8  m_irq_disable;  // U2A (pin 4)
	u8  m_irq;          // U3D (pin 13)
	u8  m_next_strobe;  // U5A (pin 5)
};


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

void a2bus_grapplerplus_device::device_add_mconfig(machine_config &config)
{
	a2bus_grapplerplus_device_base::device_add_mconfig(config);

	m_printer_conn->ack_handler().set(FUNC(a2bus_grapplerplus_device::ack_w));
}


void a2bus_grapplerplus_device::device_start()
{
	a2bus_grapplerplus_device_base::device_start();

	m_strobe_timer = timer_alloc(FUNC(a2bus_grapplerplus_device::update_strobe), this);

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

class a2bus_buf_grapplerplus_device : public a2bus_grapplerplus_device_base
{
public:
	a2bus_buf_grapplerplus_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		a2bus_buf_grapplerplus_device(mconfig, A2BUS_BUFGRAPPLERPLUS, tag, owner, clock)
	{
	}

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;

protected:
	a2bus_buf_grapplerplus_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override { return ROM_NAME(bufgrapplerplus); }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// helpers
	template <typename T> void device_add_mconfig(machine_config &config, T &&mcu_clock);

private:
	// a2bus_grapplerplus_device_base implementation
	virtual void data_latched(u8 data) override;

	// printer status inputs
	void buf_ack_w(int state);

	// MCU I/O handlers
	void mcu_io(address_map &map) ATTR_COLD;
	void mcu_p2_w(u8 data);
	u8 mcu_bus_r();
	void mcu_bus_w(u8 data);

	// synchronised signals
	void set_buf_data(s32 param);
	void set_buf_ack_in(s32 param);
	void clear_ibusy(s32 param);

	required_device<mcs48_cpu_device>   m_mcu;
	std::unique_ptr<u8 []>              m_ram;

	u16 m_ram_row;          // U1-U8
	u8  m_ram_mask;         // mask out chips that are not installed
	u8  m_mcu_p2;           // U10
	u8  m_data_latch;       // U14 (synchronised)
	u8  m_ibusy;            // U12
	u8  m_buf_ack_latch;    // U12
	u8  m_buf_ack_in;       // printer connector pin 19 (synchronised)
};



a2bus_buf_grapplerplus_device::a2bus_buf_grapplerplus_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
	a2bus_grapplerplus_device_base(mconfig, type, tag, owner, clock),
	m_mcu(*this, "mcu"),
	m_ram(),
	m_ram_row(0xff00),
	m_ram_mask(0x00U),
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

INPUT_PORTS_START(bufgrapplerplus)
	PORT_INCLUDE(grapplerplus)

	PORT_START("CNF")
	PORT_CONFNAME(0xff, 0x00, "RAM Size")
	PORT_CONFSETTING(   0xfc, "16K (2 chips)")
	PORT_CONFSETTING(   0xf0, "32K (4 chips)")
	PORT_CONFSETTING(   0x00, "64K (8 chips)")
INPUT_PORTS_END


void a2bus_buf_grapplerplus_device::device_add_mconfig(machine_config &config)
{
	// 1982 schematics show MCU driven by 7M clock
	device_add_mconfig(config, DERIVED_CLOCK(1, 1));
}


ioport_constructor a2bus_buf_grapplerplus_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(bufgrapplerplus);
}


void a2bus_buf_grapplerplus_device::device_start()
{
	a2bus_grapplerplus_device_base::device_start();

	m_ram = std::make_unique<u8 []>(0x10000);

	save_pointer(NAME(m_ram), 0x10000);
	save_item(NAME(m_ram_row));
	save_item(NAME(m_ram_mask));
	save_item(NAME(m_mcu_p2));
	save_item(NAME(m_data_latch));
	save_item(NAME(m_ibusy));
	save_item(NAME(m_buf_ack_latch));
	save_item(NAME(m_buf_ack_in));
}


void a2bus_buf_grapplerplus_device::device_reset()
{
	// The MCU is not reset when /RST is asserted.  Instead, /RST is
	// connected to P27.  Pressing the reset key(s) momentarily will not
	// clear the print buffer - the host Apple II can be reset without
	// interrupting a long print job.  Holding the reset key(s) for two
	// seconds clears the buffer.  Holding the reset key(s) on initial
	// boot enters the RAM test (results are printed).  MAME doesn't
	// currently cater for devices that don't reset their children on
	// reset, and Apple II slots don't currently expose the /RST signal
	// directly.

	a2bus_grapplerplus_device_base::device_reset();

	// should only do this on initial reset, but we get away with it here because the MCU is automatically reset even if it shouldn't be
	m_ram_mask = ioport("CNF")->read();
}



//--------------------------------------------------
//  helpers
//--------------------------------------------------

template <typename T>
void a2bus_buf_grapplerplus_device::device_add_mconfig(machine_config &config, T &&mcu_clock)
{
	a2bus_grapplerplus_device_base::device_add_mconfig(config);

	m_printer_conn->ack_handler().set(FUNC(a2bus_buf_grapplerplus_device::buf_ack_w));

	// P22 is tied high, pulling it low is used for some factory test mode
	I8048(config, m_mcu, std::forward<T>(mcu_clock));
	m_mcu->set_addrmap(AS_IO, &a2bus_buf_grapplerplus_device::mcu_io);
	m_mcu->p2_out_cb().set(FUNC(a2bus_buf_grapplerplus_device::mcu_p2_w));
	m_mcu->t0_in_cb().set([this] () { return busy_in(); });
	m_mcu->t1_in_cb().set([this] () { return m_buf_ack_latch; });
	m_mcu->bus_in_cb().set(FUNC(a2bus_buf_grapplerplus_device::mcu_bus_r));
	m_mcu->bus_out_cb().set(FUNC(a2bus_buf_grapplerplus_device::mcu_bus_w));
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

void a2bus_buf_grapplerplus_device::buf_ack_w(int state)
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
			result &= m_ram[addr] | m_ram_mask;
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
			m_ram[addr] = data | m_ram_mask;
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

void a2bus_buf_grapplerplus_device::set_buf_data(s32 param)
{
	m_data_latch = u8(u32(param));
}


void a2bus_buf_grapplerplus_device::set_buf_ack_in(s32 param)
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


void a2bus_buf_grapplerplus_device::clear_ibusy(s32 param)
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



//==============================================================
//  Buffered Grappler+ rev A implementation
//==============================================================

class a2bus_buf_grapplerplus_reva_device : public a2bus_buf_grapplerplus_device
{
public:
	a2bus_buf_grapplerplus_reva_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		a2bus_buf_grapplerplus_device(mconfig, A2BUS_BUFGRAPPLERPLUSA, tag, owner, clock)
	{
	}

	static auto parent_rom_device_type() { return &A2BUS_BUFGRAPPLERPLUS; }

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override
	{
		// boards with 6 MHz clock crystal for MCU have been seen with both UVEPROM and mask ROM parts
		// ORANGE MICRO INC., 1983
		// ASSY NO, 72 BGP 00001 REV A
		// PART NO. 95PCB00003
		a2bus_buf_grapplerplus_device::device_add_mconfig(config, 6_MHz_XTAL);
	}
};

} // anonymous namespace



DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_GRAPPLER, device_a2bus_card_interface, a2bus_grappler_device, "a2grappler", "Orange Micro Grappler Printer Interface")
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_GRAPPLERPLUS, device_a2bus_card_interface, a2bus_grapplerplus_device, "a2grapplerplus", "Orange Micro Grappler+ Printer Interface")
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_BUFGRAPPLERPLUS, device_a2bus_card_interface, a2bus_buf_grapplerplus_device, "a2bufgrapplerplus", "Orange Micro Buffered Grappler+ Printer Interface")
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_BUFGRAPPLERPLUSA, device_a2bus_card_interface, a2bus_buf_grapplerplus_reva_device, "a2bufgrapplerplusa", "Orange Micro Buffered Grappler+ (rev A) Printer Interface")
