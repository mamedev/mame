// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 ROM Module

****************************************************************************/

#include "emu.h"
#include "rom.h"


//**************************************************************************
//  Switchable ROM
//**************************************************************************

class switchable_rom_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	switchable_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
private:
	required_memory_region m_rom;
	required_ioport m_rom_selector;
};

switchable_rom_device::switchable_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_SWITCH_ROM, tag, owner, clock)
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

static INPUT_PORTS_START( rom_selector )
	PORT_START("A13-A15")   /* jumpers to select ROM region */
	PORT_CONFNAME( 0x7, 0x7, "ROM Bank" )
	PORT_CONFSETTING( 0x0, "BASIC" )
	PORT_CONFSETTING( 0x1, "EMPTY1" )
	PORT_CONFSETTING( 0x2, "EMPTY2" )
	PORT_CONFSETTING( 0x3, "EMPTY3" )
	PORT_CONFSETTING( 0x4, "EMPTY4" )
	PORT_CONFSETTING( 0x5, "EMPTY5" )
	PORT_CONFSETTING( 0x6, "EMPTY6" )
	PORT_CONFSETTING( 0x7, "SCM" )
INPUT_PORTS_END

ioport_constructor switchable_rom_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( rom_selector );
}

ROM_START(rc2014_rom)
	ROM_REGION( 0x10000, "rom",0 )
	ROM_LOAD( "r0000009.bin",    0x0000, 0x10000, CRC(3fb1ced7) SHA1(40a030b931ebe6cca654ce056c228297f245b057) )
ROM_END

const tiny_rom_entry *switchable_rom_device::device_rom_region() const
{
	return ROM_NAME( rc2014_rom );
}

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_SWITCH_ROM, device_rc2014_card_interface, switchable_rom_device, "rc2014_switchable_rom", "RC2014 Switchable ROM Module")

//**************************************************************************
//  Pageable ROM
//**************************************************************************

class pagable_rom_device : public device_t, public device_rc2014_ext_card_interface
{
public:
	// construction/destruction
	pagable_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
private:
	required_memory_region m_rom;
	required_ioport m_page_size;
	required_ioport_array<6> m_page_addr;
};

pagable_rom_device::pagable_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_PAGABLE_ROM, tag, owner, clock)
	, device_rc2014_ext_card_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_page_size(*this, "PAGE_SIZE")
	, m_page_addr(*this, "A1%u", 0U)
{
}

void pagable_rom_device::device_start()
{
}

void pagable_rom_device::device_reset()
{
	static u16 page_size[] = { 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000 };
	static u16 page_mask[] = { 0xfc00, 0xf800, 0xf000, 0xe000, 0xc000, 0x8000 };

	int index = 0;
	u16 start_addr = 0x0000;
	for (auto& addr : m_page_addr)
	{
		if (addr->read() != 0)
		{
			start_addr += (addr->read()-1) * page_size[index];
		}		
		index++;
	}
	start_addr &= page_mask[m_page_size->read()];
	m_bus->installer(AS_PROGRAM)->install_rom(0x0000, page_size[m_page_size->read()], 0x0000, m_rom->base() + start_addr);
}

static INPUT_PORTS_START( page_selector )
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
	return INPUT_PORTS_NAME( page_selector );
}

ROM_START(rc2014_pagable_rom)
	ROM_REGION( 0x10000, "rom",0 )
	ROM_LOAD( "24886009.bin", 0x00000, 0x10000, CRC(2731ca52) SHA1(e9ac663cb85de4e3e041bce444712c00f46b6eb2) )
ROM_END

const tiny_rom_entry *pagable_rom_device::device_rom_region() const
{
	return ROM_NAME( rc2014_pagable_rom );
}

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_PAGABLE_ROM, device_rc2014_ext_card_interface, pagable_rom_device, "rc2014_pagable_rom", "RC2014 Pagable ROM Module")
