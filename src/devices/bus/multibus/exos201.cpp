// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Excelan EXOS 201 Intelligent Ethernet Controller For Multibus Systems
 *
 * Sources:
 *  - EXOS 201 Intelligent Ethernet Controller For Multibus Systems Reference Manual
 *    (Publication No. 4200006-00), Revision C, June 5, 1986, Excelan Inc.
 *
 * TODO:
 *  - remaining jumpers
 *  - 8-bit and byte swap modes
 *  - ISBX interface
 */

#include "emu.h"
#include "exos201.h"

#include "cpu/i86/i186.h"
#include "machine/i82586.h"

#define LOG_REGR (1U << 1)
#define LOG_REGW (1U << 2)
#define LOG_BUS  (1U << 3)

//#define VERBOSE (LOG_GENERAL|LOG_REGW|LOG_BUS)
#include "logmacro.h"

namespace {

enum u83_mask : u8
{
	U83_STATUS   = 0x01,
	U83_INTFLAG  = 0x02,
	U83_EXOSBUSY = 0x08,
	U83_STATUS1  = 0x20,
	U83_STATUS2  = 0x40,
};

enum u21_bits : unsigned
{
	U21_STATUS    = 0,
	U21_DISABLEIO = 1,
	U21_LPBK      = 2,
	U21_BYTESWAPW = 3,
	U21_LED       = 4,
	U21_STATUS1   = 5,
	U21_STATUS2   = 6,
	U21_BYTESWAPB = 7,
};

class multibus_exos201_device
	: public device_t
	, public device_multibus_interface
{
public:
	multibus_exos201_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MULTIBUS_EXOS201, tag, owner, clock)
		, device_multibus_interface(mconfig, *this)
		, m_cpu(*this, "cpu")
		, m_net(*this, "net")
		, m_u20(*this, "u20")
		, m_j4j7(*this, "J4-J7")
		, m_j9j12(*this, "J9-J12")
		, m_j52(*this, "J52")
		, m_j53(*this, "J53")
		, m_j54(*this, "J54")
		, m_led(*this, "DS%u", 1U)
		, m_interrupt(false)
		, m_installed(false)
	{
	}

protected:
	// device_t implementation
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void cpu_mem(address_map &map);
	void net_mem(address_map &map);

	// host handlers
	u8 porta_r();
	u8 portb_r();
	void porta_w(u8 data);
	void portb_w(u8 data);

	// card handlers
	u16 pcs2_r(offs_t offset);
	u16 u111_r();
	void u21_w(u16 data);
	void admpstb_w(offs_t offset, u16 data, u16 mem_mask);
	u16 bus_r(offs_t offset, u16 mem_mask);
	void bus_w(offs_t offset, u16 data, u16 mem_mask);

	void interrupt(bool state);

private:
	required_device<i80186_cpu_device> m_cpu;
	required_device<i82586_device> m_net;
	required_region_ptr<u8> m_u20;

	required_ioport m_j4j7;
	required_ioport m_j9j12;
	required_ioport m_j52;
	required_ioport m_j53;
	required_ioport m_j54;

	output_finder<3> m_led;

	u8 m_u21;     // control register (LS273)
	u8 m_u83;     // status register (LS240)
	u8 m_u111;    // host data register (LS534)
	u8 m_admp[4]; // Multibus address map (74LS670 x2)

	bool m_interrupt;
	bool m_installed;
};

void multibus_exos201_device::device_start()
{
	m_led.resolve();

	save_item(NAME(m_u21));
	save_item(NAME(m_u83));
	save_item(NAME(m_u111));
	save_item(NAME(m_admp));
	save_item(NAME(m_interrupt));
}

void multibus_exos201_device::device_reset()
{
	if (!m_installed)
	{
		offs_t const base = (m_j53->read() << 8 | m_j52->read() << 1) ^ 0xfffe;

		m_bus->space(AS_IO).install_readwrite_handler(base + 0, base + 0,
			emu::rw_delegate(*this, FUNC(multibus_exos201_device::porta_r)),
			emu::rw_delegate(*this, FUNC(multibus_exos201_device::porta_w)));

		m_bus->space(AS_IO).install_readwrite_handler(base + 1, base + 1,
			emu::rw_delegate(*this, FUNC(multibus_exos201_device::portb_r)),
			emu::rw_delegate(*this, FUNC(multibus_exos201_device::portb_w)));

		m_installed = true;
	}

	m_u21 = 0;
	m_net->set_loopback(false);
	m_led[0] = 1;

	m_u83 &= ~U83_EXOSBUSY;
	m_cpu->int1_w(CLEAR_LINE);

	interrupt(false);
}

void multibus_exos201_device::device_add_mconfig(machine_config &config)
{
	// FIXME: dynamically adapt to 80186 PCSn configuration
	I80186(config, m_cpu, 16_MHz_XTAL);
	m_cpu->set_addrmap(AS_PROGRAM, &multibus_exos201_device::cpu_mem);

	I82586(config, m_net, m_cpu->clock() / 2);
	m_net->out_irq_cb().set(m_cpu, FUNC(i80186_cpu_device::int2_w));
	m_net->set_addrmap(AS_PROGRAM, &multibus_exos201_device::net_mem);
}

void multibus_exos201_device::cpu_mem(address_map &map)
{
	// TMS4164-15NL (65536x1) x16
	map(0x0'0000, 0x1'ffff).mirror(0x6'0000).ram().share("ram");

	map(0x8'0000, 0xb'ffff).rw(FUNC(multibus_exos201_device::bus_r), FUNC(multibus_exos201_device::bus_w));
	map(0xc'0000, 0xc'ffff).noprw(); // user firmware

	// pcs0: 0xe'fc00-0xe'fc7f
	map(0xe'fc00, 0xe'fc7f).lrw16([this]() { m_net->ca(1); return 0; }, "pcs0_r", [this](u16 data) { m_net->ca(1); }, "pcs0_w");

	// pcs1: 0xe'fc80-0xe'fcff
	map(0xe'fc80, 0xe'fc81).r(FUNC(multibus_exos201_device::u111_r));
	map(0xe'fc80, 0xe'fc81).mirror(0x4e).w(FUNC(multibus_exos201_device::u21_w));
	map(0xe'fc90, 0xe'fc97).w(FUNC(multibus_exos201_device::admpstb_w));
	map(0xe'fca0, 0xe'fca1).lw16([this](u16 data) { interrupt(true); }, "inthost_w");
	//map(0xe'fcb0, 0xe'fcb1).mirror(0x4e).w(FUNC(multibus_exos201_device::ifinit_w));

	// pcs2: 0xe'fd00-0xe'fd7f
	map(0xe'fd00, 0xe'fd7f).r(FUNC(multibus_exos201_device::pcs2_r));

	// pcs3: 0xe'fd80-0xe'fdff
	map(0xe'fd80, 0xe'fdff).lw16([this](u16 data) { m_cpu->int0_w(ASSERT_LINE); }, "pcs3_w");

	map(0xf'0000, 0xf'ffff).rom().region("nx200", 0);
}

void multibus_exos201_device::net_mem(address_map &map)
{
	map(0x0'0000, 0x1'ffff).mirror(0xfe'0000).ram().share("ram");
}

u8 multibus_exos201_device::porta_r()
{
	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_REGR, "%s: porta_r\n", machine().describe_context());
		reset();
	}

	return 0;
}

u8 multibus_exos201_device::portb_r()
{
	u8 data = m_u83;

	if (BIT(m_u21, U21_STATUS))
		data |= U83_STATUS;
	if (BIT(m_u21, U21_STATUS1))
		data |= U83_STATUS1;
	if (BIT(m_u21, U21_STATUS2))
		data |= U83_STATUS2;

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REGR, "%s: portb_r 0x%02x\n", machine().describe_context(), data);

	return data;
}

void multibus_exos201_device::porta_w(u8 data)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REGW, "%s: porta_w 0x%02x\n", machine().describe_context(), data);

	interrupt(false);
}

void multibus_exos201_device::portb_w(u8 data)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REGW, "%s: portb_w 0x%02x\n", machine().describe_context(), data);

	m_u111 = data;

	m_u83 |= U83_EXOSBUSY;
	m_cpu->int1_w(ASSERT_LINE);
}

u16 multibus_exos201_device::pcs2_r(offs_t offset)
{
	// read U20 S287
	u16 data = m_u20[(offset & 0xf) << 4 | m_j4j7->read()];

	// read U18 LS251
	if (BIT(m_j9j12->read() | 0xf0, offset & 7))
		data |= 0x8000;

	LOGMASKED(LOG_REGR, "%s: pcs2_r offset 0x%x data 0x%04x\n", machine().describe_context(), offset, data);

	return data;
}

u16 multibus_exos201_device::u111_r()
{
	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_REGR, "%s: u111_r 0x%04x\n", machine().describe_context(), m_u111);

		if (m_u83 & U83_EXOSBUSY)
		{
			m_u83 &= ~U83_EXOSBUSY;
			m_cpu->int1_w(CLEAR_LINE);
		}
	}

	return m_u111;
}

void multibus_exos201_device::u21_w(u16 data)
{
	LOGMASKED(LOG_REGW, "%s: u21_w 0x%04x\n", machine().describe_context(), data);

	m_net->set_loopback(!BIT(data, U21_LPBK));
	m_net->reset_w(!BIT(data, U21_DISABLEIO));
	m_led[0] = !BIT(data, U21_LED);

	m_u21 = data;
}

void multibus_exos201_device::admpstb_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_REGW, "%s: admpstb_w %u 0x%04x\n", machine().describe_context(), offset, data);

	m_admp[offset] = data;
}

u16 multibus_exos201_device::bus_r(offs_t offset, u16 mem_mask)
{
	offs_t const physical = offs_t(m_admp[BIT(offset, 15, 2)]) << 15 | BIT(offset, 0, 15);
	u16 const data = m_bus->space(AS_PROGRAM).read_word(physical << 1, mem_mask);

	if (!machine().side_effects_disabled())
		LOG("%s: bus_r 0x%05x translated 0x%06x data 0x%04x\n", machine().describe_context(), offset << 1, physical << 1, data);

	return data;
}

void multibus_exos201_device::bus_w(offs_t offset, u16 data, u16 mem_mask)
{
	offs_t const physical = offs_t(m_admp[BIT(offset, 15, 2)]) << 15 | BIT(offset, 0, 15);

	if (!machine().side_effects_disabled())
		LOG("%s: bus_w 0x%05x translated 0x%06x data 0x%04x\n", machine().describe_context(), offset << 1, physical << 1, data);

	m_bus->space(AS_PROGRAM).write_word(physical << 1, data, mem_mask);
}

void multibus_exos201_device::interrupt(bool state)
{
	if (m_interrupt != state)
	{
		LOG("host interrupt %u\n", state);

		if (state)
			m_u83 |= U83_INTFLAG;
		else
			m_u83 &= ~U83_INTFLAG;

		switch (m_j54->read())
		{
		case 0x01: m_bus->int_w<0>(state ? 0 : 1); break;
		case 0x02: m_bus->int_w<1>(state ? 0 : 1); break;
		case 0x04: m_bus->int_w<2>(state ? 0 : 1); break;
		case 0x08: m_bus->int_w<3>(state ? 0 : 1); break;
		case 0x10: m_bus->int_w<4>(state ? 0 : 1); break;
		case 0x20: m_bus->int_w<5>(state ? 0 : 1); break;
		case 0x40: m_bus->int_w<6>(state ? 0 : 1); break;
		case 0x80: m_bus->int_w<7>(state ? 0 : 1); break;
		}

		m_interrupt = state;
	}
}

ROM_START(exos201)
	ROM_SYSTEM_BIOS(0, "53", "Rev 5.3")

	ROM_REGION16_LE(0x1'0000, "nx200", 0)
	ROMX_LOAD("nx200l__9520002_00__rev_5.3.u82", 0x0000, 0x8000, CRC(fbdf49b3) SHA1(605860ab62b6d4d3a43ce9afb79a336e0d68e735), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("nx200h__9520001_00__rev_5.3.u83", 0x0001, 0x8000, CRC(87efd134) SHA1(3a2f692d6372c34831d7fd73c962373b572193c2), ROM_SKIP(1) | ROM_BIOS(0))

	/*
	 * U20 is a 74S287 256x4 TTL PROM, addressed using jumpers J5,J6,J7,J4 for
	 * the low-order bits and address lines A1..A4 for the upper. It contains
	 * the least-significant 24 bits of the default MAC address, followed by a
	 * checksum and then a device type/config nybble. Data at offset 0xf0 to
	 * 0xff is used to read back the state of the jumpers. The checksum is
	 * computed by summing the MAC nybbles, then computing the sum of the
	 * high and low nybbles of the result, exclusive-or'd with 5.
	 *
	 *  Offset     Content
	 *  ---------  -------
	 *  0x00-0x0f  unused
	 *  0x10-0x6f  device portion of MAC (low nybble first)
	 *  0x70-0x7f  checksum
	 *  0x80-0x9f  type/config? (
	 *  0xa0-0xdf  unused
	 *  0xe0-0xef  revision?
	 *  0xf0-0xff  sequentially incrementing values
	 */
	ROM_REGION(0x100, "u20", 0)
	ROM_LOAD("080014123456.u20", 0x000, 0x100, CRC(681c3a86) SHA1(2eb6f6a5b2ebbd3489721ef632f3fbf162044cf4))
ROM_END

const tiny_rom_entry *multibus_exos201_device::device_rom_region() const
{
	return ROM_NAME(exos201);
}

static INPUT_PORTS_START(exos201)

	PORT_START("J4-J7")
	PORT_DIPNAME(0x08, 0x08, "512 Kbyte Memory") PORT_DIPLOCATION("J4:!1")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x01, 0x01, "Disable Carrier Sense") PORT_DIPLOCATION("J5:!1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "Boot from network") PORT_DIPLOCATION("J6:!1")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "256 Kbyte Memory") PORT_DIPLOCATION("J7:!1")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("J9-J12")
	PORT_DIPNAME(0x01, 0x01, "DBUG") PORT_DIPLOCATION("J9:!1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, "BUF DMA") PORT_DIPLOCATION("J10:!1")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x00, "Disable SQE (Heartbeat) check") PORT_DIPLOCATION("J11:!1")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, "NON STD CON") PORT_DIPLOCATION("J12:!1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("J52")
	PORT_DIPNAME(0x01, 0x01, "I/O Port Address (A1)") PORT_DIPLOCATION("J52:!1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "I/O Port Address (A2)") PORT_DIPLOCATION("J52:!2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "I/O Port Address (A3)") PORT_DIPLOCATION("J52:!3")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, "I/O Port Address (A4)") PORT_DIPLOCATION("J52:!4")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, "I/O Port Address (A5)") PORT_DIPLOCATION("J52:!5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, "I/O Port Address (A6)") PORT_DIPLOCATION("J52:!6")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, "I/O Port Address (A7)") PORT_DIPLOCATION("J52:!7")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("J53")
	PORT_DIPNAME(0x01, 0x01, "I/O Port Address (A8)") PORT_DIPLOCATION("J53:!1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "I/O Port Address (A9)") PORT_DIPLOCATION("J53:!2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "I/O Port Address (A10)") PORT_DIPLOCATION("J53:!3")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, "I/O Port Address (A11)") PORT_DIPLOCATION("J53:!4")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, "I/O Port Address (A12)") PORT_DIPLOCATION("J53:!5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, "I/O Port Address (A13)") PORT_DIPLOCATION("J53:!6")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, "I/O Port Address (A14)") PORT_DIPLOCATION("J53:!7")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "I/O Port Address (A15)") PORT_DIPLOCATION("J53:!8")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("J54")
	PORT_DIPNAME(0xff, 0x20, "Interrupt Level") PORT_DIPLOCATION("J54:1,J54:2,J54:3,J54:4,J54:5,J54:6,J54:7,J54:8")
	PORT_DIPSETTING(0x01, "0")
	PORT_DIPSETTING(0x02, "1")
	PORT_DIPSETTING(0x04, "2")
	PORT_DIPSETTING(0x08, "3")
	PORT_DIPSETTING(0x10, "4")
	PORT_DIPSETTING(0x20, "5")
	PORT_DIPSETTING(0x40, "6")
	PORT_DIPSETTING(0x80, "7")

INPUT_PORTS_END

ioport_constructor multibus_exos201_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(exos201);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MULTIBUS_EXOS201, device_multibus_interface, multibus_exos201_device, "exos201", "Excelan EXOS 201 Intelligent Ethernet Controller")
