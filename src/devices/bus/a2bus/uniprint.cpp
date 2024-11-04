// license:BSD-3-Clause
// copyright-holders:Vas Crabb, R. Belmont
#include "emu.h"
#include "uniprint.h"

#include "bus/centronics/ctronics.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace {

class a2bus_uniprint_device : public device_t, public device_a2bus_card_interface
{
public:
	a2bus_uniprint_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual u8 read_c800(u16 offset) override;

protected:
	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// timer handlers
	TIMER_CALLBACK_MEMBER(update_strobe);

private:
	// printer status inputs
	void ack_w(int state);

	required_device<centronics_device>      m_printer_conn;
	required_device<output_latch_device>    m_printer_out;
	required_ioport                         m_s1, m_s2;
	required_region_ptr<u8>                 m_rom;
	emu_timer *                             m_strobe_timer;

	u8  m_data_latch;   // U6
	u8  m_ack_latch;    // U2 (pin 9)
	u8  m_next_strobe;  // U4 (pin 5)
	u8  m_ack_in;       // printer connector pin 10
};


ROM_START(uniprint)
	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD("videx_uniprint_2732a.bin", 0x000000, 0x001000, CRC(e8423ef6) SHA1(0a2789ec2cb28f10a2836a91feabc24ceca8f142))
ROM_END

INPUT_PORTS_START(uniprint)
	PORT_START("S1")
	PORT_DIPNAME(0x70, 0x00, "Printer Type")                    PORT_DIPLOCATION("S1:1,2,3")
	PORT_DIPSETTING(   0x00, "Apple DMP/C. Itoh 8510/NEC 8023A/Leading Edge Prowriter")
	PORT_DIPSETTING(   0x10, "Epson MX, FX, RX/Mannesman Tally 160L, Sprit-80")
	PORT_DIPSETTING(   0x20, "Star Gemini 10, 15")
	PORT_DIPSETTING(   0x30, "Data Products (IDS) 440, 460, Prism, uPrism/Okidata 82A, 83A, 93A")
	PORT_DIPSETTING(   0x40, "Anadex 9000, 9000A, 9001, 9001A, 9500A, 9501, 9501A")
	PORT_DIPSETTING(   0x50, "Leading Edge Gorilla Banana/Commodore VIC Printer/Radio Shack DMP-100/Sekiosha GP-100A")
	PORT_DIPSETTING(   0x60, "Centronics 739-1")
	PORT_DIPSETTING(   0x70, "Reserved")
	PORT_DIPNAME(0x0c, 0x00, "Default Page Width")              PORT_DIPLOCATION("S1:4,5")
	PORT_DIPSETTING(   0x00, "No Width")
	PORT_DIPSETTING(   0x04, "80 Columns")
	PORT_DIPSETTING(   0x08, "96 Columns")
	PORT_DIPSETTING(   0x0c, "132 Columns")
	PORT_DIPNAME(0x03, 0x00, "Default Page Length")             PORT_DIPLOCATION("S1:6,7")
	PORT_DIPSETTING(   0x00, "No Length")
	PORT_DIPSETTING(   0x01, "54 Lines")
	PORT_DIPSETTING(   0x02, "57 Lines")
	PORT_DIPSETTING(   0x03, "60 Lines")

	PORT_START("S2")
	PORT_DIPNAME(0x01, 0x01, "Strobe Polarity")                 PORT_DIPLOCATION("S1:8")
	PORT_DIPSETTING(   0x00, "Positive")
	PORT_DIPSETTING(   0x01, "Negative")
	PORT_DIPNAME(0x02, 0x02, "Acknowledge Polarity")            PORT_DIPLOCATION("S1:9")
	PORT_DIPSETTING(   0x00, "Positive")
	PORT_DIPSETTING(   0x02, "Negative")
INPUT_PORTS_END


a2bus_uniprint_device::a2bus_uniprint_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, A2BUS_UNIPRINT, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_printer_conn(*this, "prn"),
	m_printer_out(*this, "prn_out"),
	m_s1(*this, "S1"),
	m_s2(*this, "S2"),
	m_rom(*this, "rom"),
	m_strobe_timer(nullptr),
	m_data_latch(0xffU),
	m_ack_latch(0x01U),
	m_next_strobe(1U),
	m_ack_in(1U)
{
}



//----------------------------------------------
//  device_a2bus_card_interface implementation
//----------------------------------------------

// address lines are connected only to the EPROM, so all C0NX accesses are the same
u8 a2bus_uniprint_device::read_c0nx(u8 offset)
{
	// inverted /ACK?
	if (m_s2->read() & 0x02U)
	{
		return (m_s1->read() & 0x07FU) | ((m_ack_latch ^ 1) << 7);
	}

	return (m_s1->read() & 0x07FU) | (m_ack_latch << 7);
}

void a2bus_uniprint_device::write_c0nx(u8 offset, u8 data)
{
	m_data_latch = data;
	m_printer_out->write(data);

	// clearing the ACK latch will acknowledge an interrupt
	if (m_ack_in)
	{
		m_ack_latch = 0x00U;
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

u8 a2bus_uniprint_device::read_cnxx(u8 offset)
{
	return m_rom[offset + (slotno() * 0x100)];
}

u8 a2bus_uniprint_device::read_c800(u16 offset)
{
	return m_rom[offset + 0x800];
}


//----------------------------------------------
//  device_t implementation
//----------------------------------------------

tiny_rom_entry const *a2bus_uniprint_device::device_rom_region() const
{
	return ROM_NAME(uniprint);
}


void a2bus_uniprint_device::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_printer_conn, centronics_devices, "printer");
	m_printer_conn->ack_handler().set(FUNC(a2bus_uniprint_device::ack_w));

	OUTPUT_LATCH(config, m_printer_out);
	m_printer_conn->set_output_latch(*m_printer_out);
}


ioport_constructor a2bus_uniprint_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(uniprint);
}


void a2bus_uniprint_device::device_start()
{
	m_strobe_timer = timer_alloc(FUNC(a2bus_uniprint_device::update_strobe), this);

	m_next_strobe = 1U;

	save_item(NAME(m_data_latch));
	save_item(NAME(m_ack_latch));
	save_item(NAME(m_next_strobe));
	save_item(NAME(m_ack_in));

	m_strobe_timer->adjust(attotime::from_ticks(1, clock()));
}


void a2bus_uniprint_device::device_reset()
{
	m_ack_latch = 0x01U;
}



//----------------------------------------------
//  printer status inputs
//----------------------------------------------

void a2bus_uniprint_device::ack_w(int state)
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
		}
	}
}

//----------------------------------------------
//  timer handlers
//----------------------------------------------

TIMER_CALLBACK_MEMBER(a2bus_uniprint_device::update_strobe)
{
	LOG("Output /STROBE=%u\n", m_next_strobe);
	// invert strobe if DIP set to positive polarity
	m_printer_conn->write_strobe(m_next_strobe ^ (m_s2->read() & 0x01) ? 0 : 1);
	if (!m_next_strobe)
	{
		LOG("Start strobe timer\n");
		m_next_strobe = 1U;
		m_strobe_timer->adjust(attotime::from_ticks(1, clock()));
	}
}

} // anonymous namespace



DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_UNIPRINT, device_a2bus_card_interface, a2bus_uniprint_device, "a2uniprint", "Videx Uniprint Printer Interface")
