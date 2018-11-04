// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Sound Expansion cartridge (Project Expansions)

    TODO:
    - compare with actual hardware, sounds awful compared to v3
    - implement jumper to configure 8K as sideways RAM

**********************************************************************/


#include "emu.h"
#include "sndexp.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_SNDEXP, electron_sndexp_device, "electron_sndexp", "Electron Sound Expansion cartridge")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(electron_sndexp_device::device_add_mconfig)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sn76489", SN76489, 16_MHz_XTAL / 4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  INPUT_PORTS( sndexp )
//-------------------------------------------------

INPUT_PORTS_START(sndexp)
	// TODO: Not known how jumper affects RAM access
	PORT_START("JUMPER")
	PORT_DIPNAME(0x01, 0x00, "SOUND / RAM")
	PORT_DIPSETTING(0x00, "SOUND")
	PORT_DIPSETTING(0x01, "RAM")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor electron_sndexp_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sndexp);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_sndexp_device - constructor
//-------------------------------------------------

electron_sndexp_device::electron_sndexp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_SNDEXP, tag, owner, clock)
	, device_electron_cart_interface(mconfig, *this)
	, m_sn(*this, "sn76489")
	, m_jumper(*this, "JUMPER")
	, m_sound_latch(0)
	, m_sound_enable(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_sndexp_device::device_start()
{
	save_item(NAME(m_sound_latch));
	save_item(NAME(m_sound_enable));
}

//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t electron_sndexp_device::read(address_space &space, offs_t offset, int infc, int infd, int romqa)
{
	uint8_t data = 0xff;

	if (!infc && !infd)
	{
		if (m_jumper->read())
		{
			if (romqa == 0)
			{
				data = m_rom[offset & 0x1fff];
			}
			else
			{
				data = m_ram[offset & 0x1fff];
			}
		}
		else
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
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void electron_sndexp_device::write(address_space &space, offs_t offset, uint8_t data, int infc, int infd, int romqa)
{
	if (infc)
	{
		switch (offset & 0xff)
		{
		case 0x84:
			m_sound_latch = data;
			break;
		case 0x85:
			if ((data & 0x01) && !m_sound_enable)
			{
				m_sn->write(m_sound_latch);
			}
			m_sound_enable = data & 0x01;
			break;
		}
	}

	if (!infc && !infd)
	{
		if (m_jumper->read())
		{
			if (romqa == 1)
			{
				m_ram[offset & 0x1fff] = data;
			}
		}
		else
		{
			if (offset < 0x2000)
			{
				m_ram[offset & 0x1fff] = data;
			}
			else
			{
				m_ram[offset & 0x1fff] = data;
			}
		}
	}
}
