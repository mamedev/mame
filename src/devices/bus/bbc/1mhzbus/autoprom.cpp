// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ATPL AutoPrommer - Eprom Programmer with Auto-Run

    Emulation is purely based upon contents of boot ROM. The manual
    is apparently quite detailed, according to reviews, but has not
    been found.

    Emulating an EPROM programmer is not very useful but the Auto-Run
    on this device makes it different. The Auto-Run ROMs are paged into
    JIM at startup, and as very few devices make use of this feature
    it provides a good test case.

**********************************************************************/


#include "emu.h"
#include "autoprom.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_AUTOPROM, bbc_autoprom_device, "bbc_autoprom", "ATPL AutoPrommer")


//-------------------------------------------------
//  INPUT_PORTS( autoprom )
//-------------------------------------------------

static INPUT_PORTS_START( autoprom )
	PORT_START("boot")
	PORT_DIPNAME(0x03, 0x01, "EPROM")   PORT_DIPLOCATION("BOOT:1,2")
	PORT_DIPSETTING(0x00, "Disable Auto-Run")
	PORT_DIPSETTING(0x01, "4K")
	PORT_DIPSETTING(0x02, "8K")
	PORT_DIPSETTING(0x03, "16K")
	PORT_DIPNAME(0x04, 0x04, "Program") PORT_DIPLOCATION("BOOT:3")
	PORT_DIPSETTING(0x00, "BASIC")
	PORT_DIPSETTING(0x04, "M/C")
	PORT_DIPNAME(0x08, 0x08, "OS")      PORT_DIPLOCATION("BOOT:4")
	PORT_DIPSETTING(0x00, "0.1")
	PORT_DIPSETTING(0x08, "1.x")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_autoprom_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( autoprom );
}

//-------------------------------------------------
//  ROM( autoprom )
//-------------------------------------------------

ROM_START( autoprom )
	ROM_REGION(0x8000, "autorun", 0)
	// 2 Auto-Run sockets, each can take 2732/2764/27128 types
	ROM_LOAD("prommer.rom", 0x0000, 0x2000, CRC(9ec497ec) SHA1(48d76606d55bfce866baac1d9ac6390f5243c244))
	ROM_RELOAD(0x2000, 0x2000)
	ROM_LOAD("boot.rom",    0x4000, 0x1000, CRC(0843bb06) SHA1(9f584f0cc54bbbf6128e96f5f97fa811cc3d68ce))
	ROM_RELOAD(0x5000, 0x1000)
	ROM_RELOAD(0x6000, 0x1000)
	ROM_RELOAD(0x7000, 0x1000)
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_autoprom_device::device_add_mconfig(machine_config &config)
{
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_autoprom_device::device_rom_region() const
{
	return ROM_NAME( autoprom );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_autoprom_device - constructor
//-------------------------------------------------

bbc_autoprom_device::bbc_autoprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_AUTOPROM, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_autorun(*this, "autorun")
	, m_boot(*this, "boot")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_autoprom_device::device_start()
{
	save_item(NAME(m_rom_offset));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_autoprom_device::device_reset()
{
	m_slot->irq_w(ASSERT_LINE);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_autoprom_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0xc0:
		data = m_unk_c0;
		break;
	case 0xc1:
		data = m_autorun->base()[m_rom_offset];
		break;
	case 0xcc:
		break;
	case 0xd0:
		data = (m_rom_offset >> 8) & 0xff;
		break;
	case 0xd1:
		data = (m_rom_offset >> 0) & 0xff;
		break;
	case 0xdc:
		break;
	}

	return data;
}

void bbc_autoprom_device::fred_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0xc0:
		m_unk_c0 = data;
		break;
	case 0xc1:
		break;
	case 0xcc:
		break;
	case 0xd0:
		m_rom_offset = (m_rom_offset & 0x00ff) | (data << 8);
		break;
	case 0xd1:
		m_rom_offset = (m_rom_offset & 0xff00) | (data << 0);
		break;
	case 0xdc:
		break;
	}
}

uint8_t bbc_autoprom_device::jim_r(offs_t offset)
{
	// not known when to clear the IRQ, but has been acknowledged to get here
	if (!machine().side_effects_disabled())
	{
		if (offset == 0xfe) m_slot->irq_w(CLEAR_LINE);
	}
	// the switches seem to determine which page of the boot ROM is accessed via JIM
	return m_autorun->base()[0x4000 | ((m_boot->read() << 8) + offset)];
}
