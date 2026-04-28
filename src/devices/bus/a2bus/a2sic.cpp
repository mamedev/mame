// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    Apple II (High-Speed) Serial Interface Card (A2B0005, 670-X005)

    This was Apple's second attempt at an RS-232 serial interface,
    announced in 1978 but not released until 1979. Unlike the earlier
    Communications Card, it uses neither an UART nor a clock divider
    circuit, instead providing bitbanged half-duplex communications
    at a baud rate determined by firmware delay loops. (In practice,
    however, the firmware's normal mode spends so much time post-
    processing each received character that continuous reception of
    any data stream tends to fail even at low baud rates.)

**********************************************************************/

#include "emu.h"
#include "a2sic.h"

#include "bus/rs232/rs232.h"

namespace {

class a2sic_device : public device_t, public device_a2bus_card_interface
{
public:
	// device type constructor
	a2sic_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual u8 read_c800(u16 offset) override;
	virtual bool take_c800() const override;
	virtual void reset_from_bus() override;

private:
	// line handlers
	void cts_loopback_w(int state);
	void dsr_loopback_w(int state);

	// object finders
	required_device<rs232_port_device> m_rs232;
	required_region_ptr<u8> m_p7;
	required_region_ptr<u8> m_p8;
	required_ioport m_switches;

	// internal state
	bool m_cts;
	bool m_dsr;
};

//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

a2sic_device::a2sic_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, A2BUS_SIC, tag, owner, clock)
	, device_a2bus_card_interface(mconfig, *this)
	, m_rs232(*this, "rs232")
	, m_p7(*this, "p7")
	, m_p8(*this, "p8")
	, m_switches(*this, "SWITCHES")
	, m_cts(false)
	, m_dsr(false)
{
}

void a2sic_device::device_start()
{
	save_item(NAME(m_cts));
	save_item(NAME(m_dsr));
}

void a2sic_device::device_reset()
{
	reset_from_bus();

	m_rs232->write_rts(m_cts);
	m_rs232->write_dtr(m_dsr);
}

u8 a2sic_device::read_c0nx(u8 offset)
{
	u8 result = m_switches->read() | m_rs232->rxd_r() << 7;

	// clock A0 into 74LS109 flip-flop (inverted output)
	if (!machine().side_effects_disabled())
		m_rs232->write_txd(!BIT(offset, 0));

	return result;
}

void a2sic_device::write_c0nx(u8 offset, u8 data)
{
	m_rs232->write_txd(!BIT(offset, 0));
}

u8 a2sic_device::read_cnxx(u8 offset)
{
	return m_p7[offset];
}

u8 a2sic_device::read_c800(u16 offset)
{
	if (BIT(offset, 9))
		return 0xff; // TODO: select open bus
	else
		return m_p8[offset & 0x1ff];
}

bool a2sic_device::take_c800() const
{
	return true;
}

void a2sic_device::reset_from_bus()
{
	m_rs232->write_txd(1);
}

void a2sic_device::cts_loopback_w(int state)
{
	if (m_cts != bool(state))
	{
		m_cts = state;
		m_rs232->write_rts(state);
	}
}

void a2sic_device::dsr_loopback_w(int state)
{
	if (m_dsr != bool(state))
	{
		m_dsr = state;
		m_rs232->write_dtr(state);
	}
}

static INPUT_PORTS_START(a2sic)
	PORT_START("SWITCHES")
	PORT_DIPNAME(0x07, 0x06, "Baud Rate") PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(0x00, "110")
	PORT_DIPSETTING(0x01, "134.5")
	PORT_DIPSETTING(0x02, "300")
	PORT_DIPSETTING(0x03, "1200")
	PORT_DIPSETTING(0x04, "2400")
	PORT_DIPSETTING(0x05, "4800")
	PORT_DIPSETTING(0x06, "9600")
	PORT_DIPSETTING(0x07, "19200")
	PORT_DIPNAME(0x08, 0x08, "Carriage Return Delay") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x30, 0x00, "Line Width") PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(0x00, "40 (Video Enabled)")
	PORT_DIPSETTING(0x10, "72 (Video Disabled)")
	PORT_DIPSETTING(0x20, "80 (Video Disabled)")
	PORT_DIPSETTING(0x30, "132 (Video Disabled)")
	PORT_DIPNAME(0x40, 0x00, "Line Feed After Carriage Return") PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x40, DEF_STR(On))
INPUT_PORTS_END

ioport_constructor a2sic_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(a2sic);
}

void a2sic_device::device_add_mconfig(machine_config &config)
{
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->cts_handler().set(FUNC(a2sic_device::cts_loopback_w));
	m_rs232->dsr_handler().set(FUNC(a2sic_device::dsr_loopback_w));
}

ROM_START(a2sic)
	ROM_SYSTEM_BIOS(0, "r4", "Revision 4")
	ROM_SYSTEM_BIOS(1, "r3", "Revision 3")

	ROM_REGION(0x100, "p7", 0)
	ROMX_LOAD("341-0017.p7", 0x000, 0x100, CRC(5be57c50) SHA1(897c92517566f74b179165b8c7b1c9a9df89f014), ROM_BIOS(0)) // probably 317-0017-04 (P7-04)
	ROMX_LOAD("p7-03", 0x000, 0x100, CRC(99d96608) SHA1(3abc7054f00fd09cc9ee7e76df913912990452f5), ROM_BIOS(1)) // from published listing dated July 21, 1978

	ROM_REGION(0x200, "p8", 0)
	ROMX_LOAD("341-0048.p8a", 0x000, 0x200, CRC(361c0462) SHA1(369fb95586efa184025d4770ae3686517b13598b), ROM_BIOS(0))
	ROMX_LOAD("p8-01", 0x000, 0x200, CRC(7eafe564) SHA1(8ff44e892b2510573e9528d781b60429c38dbe30), ROM_BIOS(1)) // from published listing dated July 21, 1978
ROM_END

const tiny_rom_entry *a2sic_device::device_rom_region() const
{
	return ROM_NAME(a2sic);
}

} // anonymous namespace

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_SIC, device_a2bus_card_interface, a2sic_device, "a2sic", "Apple II Serial Interface Card")
