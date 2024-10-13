// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 ROM Module

****************************************************************************/

#include "emu.h"
#include "rom.h"

namespace {

//**************************************************************************
//  RC2014 Switchable ROM module
//  Module author: Spencer Owen
//**************************************************************************

class switchable_rom_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	switchable_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
private:
	required_memory_region m_rom;
	required_ioport m_rom_selector;
};

switchable_rom_device::switchable_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_SWITCHABLE_ROM, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_rom_selector(*this, "A13-A15")
{
}

void switchable_rom_device::device_start()
{
}

void switchable_rom_device::device_reset()
{
	m_bus->installer(AS_PROGRAM)->install_rom(0x0000, 0x1fff, 0x0000, m_rom->base() + (m_rom_selector->read() & 7) * 0x2000);
}

static INPUT_PORTS_START( switchable_rom_jumpers )
	PORT_START("A13-A15")   /* jumpers to select ROM region */
	PORT_CONFNAME( 0x7, 0x7, "ROM Bank" )
	PORT_CONFSETTING( 0x0, "Bank 0" )
	PORT_CONFSETTING( 0x1, "Bank 1" )
	PORT_CONFSETTING( 0x2, "Bank 2" )
	PORT_CONFSETTING( 0x3, "Bank 3" )
	PORT_CONFSETTING( 0x4, "Bank 4" )
	PORT_CONFSETTING( 0x5, "Bank 5" )
	PORT_CONFSETTING( 0x6, "Bank 6" )
	PORT_CONFSETTING( 0x7, "Bank 7" )
INPUT_PORTS_END

ioport_constructor switchable_rom_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( switchable_rom_jumpers );
}

ROM_START(rc2014_switchable_rom)
	ROM_REGION( 0x10000, "rom",0 )
	//
	// Decoding ROM labels
	//
	// 0 - Empty bank, available for user to program
	// R - Microsoft BASIC, for 32k RAM, 68B50 ACIA, with origin 0x0000
	// K - Microsoft BASIC, for 56k RAM, 68B50 ACIA, with origin 0x0000
	// 1 - CP/M Monitor, for pageable ROM, 64k RAM, 68B50 ACIA, CF Module at 0x10, with origin at 0x0000
	// 2 - Microsoft BASIC, for 32k RAM, SIO/2, with origin 0x0000
	// 4 - Microsoft BASIC, for 56k RAM, SIO/2, with origin 0x0000
	// 6 - CP/M Monitor, for pageable ROM, 64k RAM, SIO/2, CF Module at 0x10, with origin at 0x0000
	// 88 - Small Computer Monitor for pageable ROM, 64k RAM, SIO/2 or 68B50 ACIA, with Microsoft BASIC a
	//      and CP/M boot options [Note that this is a 16k image, so Page Size needs to be set to 16k and
	//      only A14 and A15 jumpers to select]
	// 9 - Small Computer Monitor for any ROM, any RAM, any UART
	//
	ROM_DEFAULT_BIOS("r0000009")
	ROM_SYSTEM_BIOS(0, "r0000009", "BASIC 32K ACIA + SCM")
	ROMX_LOAD( "r0000009.bin",    0x0000, 0x10000, CRC(3fb1ced7) SHA1(40a030b931ebe6cca654ce056c228297f245b057), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "00001000", "CP/M Monitor ACIA")
	ROMX_LOAD( "00001000.bin",    0x0000, 0x10000, CRC(e6509de9) SHA1(e5f3b528e972f63cd243bd5057e7e7850436b671), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "k0001000", "BASIC 56K + CP/M Monitor ACIA")
	ROMX_LOAD( "k0001000.bin",    0x0000, 0x10000, CRC(619a9c3e) SHA1(c1213316746a6e2a7d2030b283f75c4f3a9c177a), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "r0001000", "BASIC 32K + CP/M Monitor ACIA")
	ROMX_LOAD( "r0001000.bin",    0x0000, 0x10000, CRC(5b174833) SHA1(97c204ada65129ef61507faf74e29c8ec7a05064), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "r0001009", "BASIC 32K + CP/M Monitor ACIA + SCM")
	ROMX_LOAD( "r0001009.bin",    0x0000, 0x10000, CRC(074a2d70) SHA1(0077ab5e669da6e95ed2f66c9e759067c4236e36), ROM_BIOS(4))
ROM_END

const tiny_rom_entry *switchable_rom_device::device_rom_region() const
{
	return ROM_NAME( rc2014_switchable_rom );
}

//**************************************************************************
//  RC2014 Pageable ROM module
//  Module author: Spencer Owen
//**************************************************************************

class pagable_rom_device : public device_t, public device_rc2014_ext_card_interface
{
public:
	// construction/destruction
	pagable_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override { update_banks(); }
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void reset_bank_w(offs_t, uint8_t) { m_bank = 0; update_banks(); m_bus->page_w(CLEAR_LINE);}
	void toggle_bank_w(offs_t, uint8_t) { m_bank = m_bank ? 0 : 1; update_banks(); m_bus->page_w(m_bank ? ASSERT_LINE : CLEAR_LINE); }

	void update_banks();
private:
	int m_bank;
	u16 m_start_offset;
	u16 m_end_addr;
	required_memory_region m_rom;
	required_ioport m_page_size_conf;
	required_ioport_array<6> m_page_addr_conf;
};

pagable_rom_device::pagable_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_PAGABLE_ROM, tag, owner, clock)
	, device_rc2014_ext_card_interface(mconfig, *this)
	, m_bank(0)
	, m_rom(*this, "rom")
	, m_page_size_conf(*this, "PAGE_SIZE")
	, m_page_addr_conf(*this, "A1%u", 0U)
{
}

void pagable_rom_device::device_start()
{
	save_item(NAME(m_bank));
}

void pagable_rom_device::device_reset()
{
	static constexpr u16 page_size[] = { 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000 };
	static constexpr u16 page_mask[] = { 0xfc00, 0xf800, 0xf000, 0xe000, 0xc000, 0x8000 };

	int index = 0;
	m_start_offset = 0x0000;
	for (auto& addr : m_page_addr_conf)
	{
		if (addr->read() != 0)
			m_start_offset += (addr->read() - 1) * page_size[index];
		index++;
	}
	m_start_offset &= page_mask[m_page_size_conf->read()];
	m_end_addr = page_size[m_page_size_conf->read()] - 1;
	reset_bank_w(0,0);

	// A15-A8, A7 and A2-A0 not connected, A6 must be 0
	m_bus->installer(AS_IO)->install_write_handler(0x30, 0x30, 0, 0xff87, 0, write8sm_delegate(*this, FUNC(pagable_rom_device::reset_bank_w)));
	m_bus->installer(AS_IO)->install_write_handler(0x38, 0x38, 0, 0xff87, 0, write8sm_delegate(*this, FUNC(pagable_rom_device::toggle_bank_w)));
}

void pagable_rom_device::update_banks()
{
	if (m_bank == 0)
		m_bus->installer(AS_PROGRAM)->install_rom(0x0000, m_end_addr, 0x0000, m_rom->base() + m_start_offset);
}

static INPUT_PORTS_START( pagable_rom_jumpers )
	PORT_START("PAGE_SIZE")
	PORT_CONFNAME( 0x7, 0x4, "Page Size" )
	PORT_CONFSETTING( 0x0, "1K" )
	PORT_CONFSETTING( 0x1, "2K" )
	PORT_CONFSETTING( 0x2, "4K" )
	PORT_CONFSETTING( 0x3, "8K" )
	PORT_CONFSETTING( 0x4, "16K" )
	PORT_CONFSETTING( 0x5, "32K" )
	PORT_START("A10")
	PORT_CONFNAME( 0x3, 0x0, "A10" )
	PORT_CONFSETTING( 0x0, DEF_STR( None ) )
	PORT_CONFSETTING( 0x1, "0" )
	PORT_CONFSETTING( 0x2, "1" )
	PORT_START("A11")
	PORT_CONFNAME( 0x3, 0x0, "A11" )
	PORT_CONFSETTING( 0x0, DEF_STR( None ) )
	PORT_CONFSETTING( 0x1, "0" )
	PORT_CONFSETTING( 0x2, "1" )
	PORT_START("A12")
	PORT_CONFNAME( 0x3, 0x0, "A12" )
	PORT_CONFSETTING( 0x0, DEF_STR( None ) )
	PORT_CONFSETTING( 0x1, "0" )
	PORT_CONFSETTING( 0x2, "1" )
	PORT_START("A13")
	PORT_CONFNAME( 0x3, 0x0, "A13" )
	PORT_CONFSETTING( 0x0, DEF_STR( None ) )
	PORT_CONFSETTING( 0x1, "0" )
	PORT_CONFSETTING( 0x2, "1" )
	PORT_START("A14")
	PORT_CONFNAME( 0x3, 0x2, "A14" )
	PORT_CONFSETTING( 0x0, DEF_STR( None ) )
	PORT_CONFSETTING( 0x1, "0" )
	PORT_CONFSETTING( 0x2, "1" )
	PORT_START("A15")
	PORT_CONFNAME( 0x3, 0x1, "A15" )
	PORT_CONFSETTING( 0x0, DEF_STR( None ) )
	PORT_CONFSETTING( 0x1, "0" )
	PORT_CONFSETTING( 0x2, "1" )
INPUT_PORTS_END

ioport_constructor pagable_rom_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pagable_rom_jumpers );
}

ROM_START(rc2014_pagable_rom)
	//
	// Decoding ROM labels
	//
	// 0 - Empty bank, available for user to program
	// R - Microsoft BASIC, for 32k RAM, 68B50 ACIA, with origin 0x0000
	// K - Microsoft BASIC, for 56k RAM, 68B50 ACIA, with origin 0x0000
	// 1 - CP/M Monitor, for pageable ROM, 64k RAM, 68B50 ACIA, CF Module at 0x10, with origin at 0x0000
	// 2 - Microsoft BASIC, for 32k RAM, SIO/2, with origin 0x0000
	// 4 - Microsoft BASIC, for 56k RAM, SIO/2, with origin 0x0000
	// 6 - CP/M Monitor, for pageable ROM, 64k RAM, SIO/2, CF Module at 0x10, with origin at 0x0000
	// 88 - Small Computer Monitor for pageable ROM, 64k RAM, SIO/2 or 68B50 ACIA, with Microsoft BASIC a
	//      and CP/M boot options [Note that this is a 16k image, so Page Size needs to be set to 16k and
	//      only A14 and A15 jumpers to select]
	// 9 - Small Computer Monitor for any ROM, any RAM, any UART
	//
	ROM_REGION( 0x10000, "rom",0 )
	ROM_DEFAULT_BIOS("24886009")
	ROM_SYSTEM_BIOS(0, "24886009", "BASIC 32K/56K SIO/2 + CP/M + SCM Pagable + SCM")
	ROMX_LOAD( "24886009.bin", 0x00000, 0x10000, CRC(2731ca52) SHA1(e9ac663cb85de4e3e041bce444712c00f46b6eb2), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "24006000", "BASIC 32K/56K SIO/2 + CP/M Monitor")
	ROMX_LOAD( "24006000.bin", 0x00000, 0x10000, CRC(3532872b) SHA1(ff5b3873abdcbe8ab48f62a9dbdf39314b938e89), ROM_BIOS(1))
ROM_END

const tiny_rom_entry *pagable_rom_device::device_rom_region() const
{
	return ROM_NAME( rc2014_pagable_rom );
}

}
//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_SWITCHABLE_ROM, device_rc2014_card_interface, switchable_rom_device, "rc2014_switchable_rom", "RC2014 Switchable ROM module")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_PAGABLE_ROM, device_rc2014_ext_card_interface, pagable_rom_device, "rc2014_pagable_rom", "RC2014 Pageable ROM module")
