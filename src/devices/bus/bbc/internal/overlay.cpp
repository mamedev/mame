// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Vine Micros Romboard '3' (Master OS Overlay Board)

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Vine_MasterOSoverlay.html

**********************************************************************/


#include "emu.h"
#include "overlay.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_OVERLAY, bbc_overlay_device, "bbc_overlay", "Vine Micros Romboard '3' (Master OS Overlay)");


//-------------------------------------------------
//  INPUT_PORTS( overlay )
//-------------------------------------------------

static INPUT_PORTS_START(overlay)
	PORT_START("link_rom1")
	PORT_CONFNAME(0x1f, 0x0a, "ROM 1")
	PORT_CONFSETTING(0x10, "Operating System (reserved)")
	PORT_CONFSETTING(0x09, "1770 DFS")
	PORT_CONFSETTING(0x0a, "Viewsheet")
	PORT_CONFSETTING(0x0b, "Edit")
	PORT_CONFSETTING(0x0c, "BASIC")
	PORT_CONFSETTING(0x0d, "ADFS")
	PORT_CONFSETTING(0x0e, "View (and some GXR)")
	PORT_CONFSETTING(0x0f, "Terminal and Operating System (reserved)")
	PORT_START("link_rom2")
	PORT_CONFNAME(0x1f, 0x0b, "ROM 2")
	PORT_CONFSETTING(0x10, "Operating System (reserved)")
	PORT_CONFSETTING(0x09, "1770 DFS")
	PORT_CONFSETTING(0x0a, "Viewsheet")
	PORT_CONFSETTING(0x0b, "Edit")
	PORT_CONFSETTING(0x0c, "BASIC")
	PORT_CONFSETTING(0x0d, "ADFS")
	PORT_CONFSETTING(0x0e, "View (and some GXR)")
	PORT_CONFSETTING(0x0f, "Terminal and Operating System (reserved)")
	PORT_START("link_rom3")
	PORT_CONFNAME(0x1f, 0x0e, "ROM 3")
	PORT_CONFSETTING(0x10, "Operating System (reserved)")
	PORT_CONFSETTING(0x09, "1770 DFS")
	PORT_CONFSETTING(0x0a, "Viewsheet")
	PORT_CONFSETTING(0x0b, "Edit")
	PORT_CONFSETTING(0x0c, "BASIC")
	PORT_CONFSETTING(0x0d, "ADFS")
	PORT_CONFSETTING(0x0e, "View (and some GXR)")
	PORT_CONFSETTING(0x0f, "Terminal and Operating System (reserved)")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_overlay_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(overlay);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_overlay_device::device_add_mconfig(machine_config &config)
{
	/* 3 x 16K rom sockets */
	BBC_ROMSLOT16(config, m_rom[0], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[1], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[2], bbc_rom_devices, nullptr);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_overlay_device - constructor
//-------------------------------------------------

bbc_overlay_device::bbc_overlay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_OVERLAY, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_rom(*this, "romslot%u", 0U)
	, m_link_rom(*this, "link_rom%u", 1U)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_overlay_device::device_start()
{
	/* register for save states */
	save_item(NAME(m_romsel));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_overlay_device::paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_romsel)
	{
	case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		/* overlay banks */
		if (m_link_rom[0]->read() == m_romsel && m_rom[0]->present())
		{
			data = m_rom[0]->read(offset);
		}
		else if (m_link_rom[1]->read() == m_romsel && m_rom[1]->present())
		{
			data = m_rom[1]->read(offset);
		}
		else if (m_link_rom[2]->read() == m_romsel && m_rom[2]->present())
		{
			data = m_rom[2]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_romsel << 14)];
		}
		break;
	case 4: case 5: case 6: case 7:
		/* motherboard ram selected */
		if (m_mb_rom[m_romsel & 0x0e] && m_mb_rom[m_romsel & 0x0e]->present())
		{
			data = m_mb_rom[m_romsel & 0x0e]->read(offset | (m_romsel & 0x01) << 14);
		}
		else
		{
		data = m_region_swr->base()[offset + (m_romsel << 14)];
		}
		break;
	default:
		/* motherboard rom sockets */
		if (m_mb_rom[m_romsel] && m_mb_rom[m_romsel]->present())
		{
			data = m_mb_rom[m_romsel]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_romsel << 14)];
		}
		break;
	}

	return data;
}

void bbc_overlay_device::paged_w(offs_t offset, uint8_t data)
{
	switch (m_romsel)
	{
	case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		/* overlay banks */
		if (m_link_rom[0]->read() == m_romsel && m_rom[0]->present())
		{
			m_rom[0]->write(offset, data);
		}
		else if (m_link_rom[1]->read() == m_romsel && m_rom[1]->present())
		{
			m_rom[1]->write(offset, data);
		}
		else if (m_link_rom[2]->read() == m_romsel && m_rom[2]->present())
		{
			m_rom[2]->write(offset, data);
		}
		break;
	case 4: case 5: case 6: case 7:
		/* motherboard ram selected */
		if (m_mb_rom[m_romsel & 0x0e])
		{
			m_mb_rom[m_romsel & 0x0e]->write(offset | (m_romsel & 0x01) << 14, data);
		}
		break;
	default:
		/* motherboard rom sockets */
		if (m_mb_rom[m_romsel])
		{
			m_mb_rom[m_romsel]->write(offset, data);
		}
		break;
	}
}


uint8_t bbc_overlay_device::mos_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_link_rom[0]->read() == 0x10 && m_rom[0]->present())
	{
		data = m_rom[0]->read(offset);
	}
	else if (m_link_rom[1]->read() == 0x10 && m_rom[1]->present())
	{
		data = m_rom[1]->read(offset);
	}
	else if (m_link_rom[2]->read() == 0x10 && m_rom[2]->present())
	{
		data = m_rom[2]->read(offset);
	}
	else
	{
		data = m_region_mos->base()[offset];
	}

	return data;
}
