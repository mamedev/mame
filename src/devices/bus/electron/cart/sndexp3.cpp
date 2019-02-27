// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Sound Expansion v3 cartridge (Complex Software Systems)

**********************************************************************/


#include "emu.h"
#include "sndexp3.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_SNDEXP3, electron_sndexp3_device, "electron_sndexp3", "Electron Sound Expansion v3 cartridge")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_sndexp3_device::device_add_mconfig(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SN76489(config, m_sn, DERIVED_CLOCK(1, 4));
	m_sn->add_route(ALL_OUTPUTS, "mono", 1.0);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_sndexp3_device - constructor
//-------------------------------------------------

electron_sndexp3_device::electron_sndexp3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_SNDEXP3, tag, owner, clock)
	, device_electron_cart_interface(mconfig, *this)
	, m_sn(*this, "sn76489")
	, m_sound_latch(0)
	, m_sound_enable(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_sndexp3_device::device_start()
{
	save_item(NAME(m_sound_latch));
	save_item(NAME(m_sound_enable));
}

//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t electron_sndexp3_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (oe && romqa)
	{
		if (offset < 0x2000)
		{
			data = m_rom[offset & 0x1fff];
		}
		else
		{
			data = m_ram[offset & 0x1fff];
		}
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void electron_sndexp3_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (infc)
	{
		switch (offset & 0xff)
		{
		case 0x98:
			m_sound_latch = data;
			break;
		case 0x99:
			if ((data & 0x01) && !m_sound_enable)
			{
				m_sn->write(m_sound_latch);
			}
			m_sound_enable = data & 0x01;
			break;
		}
	}
	else if (oe && romqa)
	{
		if (offset >= 0x2000)
		{
			m_ram[offset & 0x1fff] = data;
		}
	}
}
