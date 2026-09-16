// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    California Computer Systems, Inc. Model 7710
    Asynchronous Serial Interface (ASI-1)

    One unusual feature of this small RS-232-C communications card
    which is not relevant for emulation is that the DP8304B bus
    buffer and program ROMs are powered down when they are not being
    accessed. Another unemulated feature is a suggested modification
    to replace the two 256×4 program ROMs with like-sized 2112 RAMs
    (the R/W bus signal is enabled to these with a jumper connection)
    so that custom firmware can be uploaded instead.

    The 7710-02 terminal firmware is incompatible with the Apple IIe
    due to the use of a II+-specific monitor location. One recommended
    way to work around this problem was to load the II+ monitor into
    the IIe's language card. The incompatibility was corrected in
    7710-03, and the 7710-03 ROMs were the only ones delivered after
    July 1983.

**********************************************************************/

#include "emu.h"
#include "ccs7710.h"

#include "bus/rs232/rs232.h"
#include "machine/6850acia.h"
#include "machine/f4702.h"

namespace {

class ccs7710_device : public device_t, public device_a2bus_card_interface
{
public:
	// device type constructor
	ccs7710_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;

private:
	// miscellaneous handlers
	void acia_irq_w(int state);
	void external_clock_w(int state);
	u8 baud_select_r(offs_t offset);

	// object finders
	required_device<acia6850_device> m_acia;
	required_device<f4702_device> m_brg;
	required_region_ptr<u8> m_firmware;
	required_ioport m_baud_select;

	// internal state
	bool m_external_clock;
};

//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

ccs7710_device::ccs7710_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, A2BUS_CCS7710, tag, owner, clock)
	, device_a2bus_card_interface(mconfig, *this)
	, m_acia(*this, "acia")
	, m_brg(*this, "brg")
	, m_firmware(*this, "firmware")
	, m_baud_select(*this, "BAUD")
	, m_external_clock(false)
{
}

void ccs7710_device::device_start()
{
	save_item(NAME(m_external_clock));
}

u8 ccs7710_device::read_c0nx(u8 offset)
{
	return m_acia->read(offset & 1);
}

void ccs7710_device::write_c0nx(u8 offset, u8 data)
{
	m_acia->write(offset & 1, data);
}

u8 ccs7710_device::read_cnxx(u8 offset)
{
	return m_firmware[offset];
}

void ccs7710_device::acia_irq_w(int state)
{
	if (state == ASSERT_LINE)
		raise_slot_irq();
	else
		lower_slot_irq();
}

void ccs7710_device::external_clock_w(int state)
{
	m_external_clock = state;
	if (!BIT(m_baud_select->read(), 0))
		m_brg->im_w(state);
}

u8 ccs7710_device::baud_select_r(offs_t offset)
{
	u8 baud = m_baud_select->read();
	m_brg->im_w(BIT(baud, 0) ? !BIT(offset, 2) : m_external_clock);
	return baud;
}

static INPUT_PORTS_START(ccs7710)
	PORT_START("BAUD")
	PORT_DIPNAME(0xf, 0x8, "Baud Rate") PORT_DIPLOCATION("S1:1,2,3,4")
	PORT_DIPSETTING(0x0, "External")
	PORT_DIPSETTING(0x2, "50")
	PORT_DIPSETTING(0x3, "75")
	PORT_DIPSETTING(0xf, "110")
	PORT_DIPSETTING(0x4, "134.5")
	PORT_DIPSETTING(0xe, "150")
	PORT_DIPSETTING(0x5, "200")
	PORT_DIPSETTING(0xd, "300")
	PORT_DIPSETTING(0x6, "600")
	PORT_DIPSETTING(0xb, "1200")
	PORT_DIPSETTING(0xa, "1800")
	PORT_DIPSETTING(0x7, "2400")
	PORT_DIPSETTING(0x9, "4800")
	PORT_DIPSETTING(0x8, "9600")
	PORT_DIPSETTING(0x1, "19200")
INPUT_PORTS_END

ioport_constructor ccs7710_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ccs7710);
}

void ccs7710_device::device_add_mconfig(machine_config &config)
{
	ACIA6850(config, m_acia);
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->irq_handler().set(FUNC(ccs7710_device::acia_irq_w));

	F4702(config, m_brg, 2.4576_MHz_XTAL);
	m_brg->s_callback().set(FUNC(ccs7710_device::baud_select_r));
	m_brg->z_callback().set(m_acia, FUNC(acia6850_device::write_rxc));
	m_brg->z_callback().append(m_acia, FUNC(acia6850_device::write_txc));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));
	rs232.dcd_handler().set(m_acia, FUNC(acia6850_device::write_dcd));
	rs232.txc_handler().set(FUNC(ccs7710_device::external_clock_w));
}

ROM_START(ccs7710)
	ROM_REGION(0x100, "firmware", 0)
	ROM_DEFAULT_BIOS("7710-03")
	ROM_SYSTEM_BIOS(0, "7710-01", "CCS 7710-01")
	ROMX_LOAD("7710-01h.u5", 0x000, 0x100, CRC(4f4b1023) SHA1(4f96f9cbbad383c259d4a3f8762263d01532c735), ROM_BIOS(0) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI)
	ROMX_LOAD("7710-01l.u6", 0x000, 0x100, CRC(1dab7113) SHA1(c9c7b8b73db90fa008c8779b1d75ac265ec6ca2f), ROM_BIOS(0) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO)
	ROM_SYSTEM_BIOS(1, "7710-02", "CCS 7710-02")
	ROMX_LOAD("7710-02h.u5", 0x000, 0x100, CRC(46ded0cc) SHA1(62a087e3a1790a9910561d84001acaa9ec804194), ROM_BIOS(1) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI)
	ROMX_LOAD("7710-02l.u6", 0x000, 0x100, CRC(d4922d69) SHA1(a651dbd3faba9476aff382952266188f3cb05f16), ROM_BIOS(1) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO)
	ROM_SYSTEM_BIOS(2, "7710-03", "CCS 7710-03")
	ROMX_LOAD("7710-03h.u5", 0x000, 0x100, CRC(a2cdb6bd) SHA1(6ecfb6b145256d4787a943da8ef267303df562d6), ROM_BIOS(2) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI)
	ROMX_LOAD("7710-03l.u6", 0x000, 0x100, CRC(18e3983f) SHA1(d0df2aaf0134f8c1b929a4de0036fedc7e7164e2), ROM_BIOS(2) | ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO)
ROM_END

const tiny_rom_entry *ccs7710_device::device_rom_region() const
{
	return ROM_NAME(ccs7710);
}

} // anonymous namespace

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_CCS7710, device_a2bus_card_interface, ccs7710_device, "ccs7710", "CCS Model 7710 Asynchronous Serial Interface")
