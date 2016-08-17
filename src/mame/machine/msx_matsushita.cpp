// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "msx_matsushita.h"


const device_type MSX_MATSUSHITA = &device_creator<msx_matsushita_device>;


msx_matsushita_device::msx_matsushita_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: msx_switched_device(mconfig, MSX_MATSUSHITA, "Matsushita switched device", tag, owner, clock, "msx_matsushita", __FILE__)
	, m_io_config(*this, "CONFIG")
	, m_nvram(*this, "nvram")
	, m_turbo_out_cb(*this)
	, m_address(0)
	, m_nibble1(0)
	, m_nibble2(0)
	, m_pattern(0)
{
}


UINT8 msx_matsushita_device::get_id()
{
	return 0x08;
}


static MACHINE_CONFIG_FRAGMENT( matsushita )
	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END


machine_config_constructor msx_matsushita_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( matsushita );
}


static INPUT_PORTS_START( matsushita )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x80, 0x00, "Firmware switch")
	PORT_CONFSETTING( 0x00, "On" )
	PORT_CONFSETTING( 0x80, "Off" )
	PORT_BIT(0x7F, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


ioport_constructor msx_matsushita_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( matsushita );
}


void msx_matsushita_device::device_start()
{
	msx_switched_device::device_start();

	m_turbo_out_cb.resolve_safe();

	m_sram.resize(0x800);

	m_nvram->set_base(&m_sram[0], 0x000);
}


READ8_MEMBER(msx_matsushita_device::io_read)
{
	switch (offset)
	{
		case 0x00:
			return ~get_id();

		case 0x01:
			return m_io_config->read();

		case 0x03:
			{
				UINT8 result = (((m_pattern & 0x80) ? m_nibble1 : m_nibble2) << 4) | ((m_pattern & 0x40) ? m_nibble1 : m_nibble2);
				m_pattern = (m_pattern << 2) | (m_pattern >> 6);
				return result;
			}

		case 0x09:   // Data
			if (m_address < m_sram.size())
			{
				return m_sram[m_address];
			}
			break;

		default:
			logerror("msx_matsushita: unhandled read from offset %02x\n", offset);
			break;
	}

	return 0xFF;
}


/*
  03 <- 10
  04 <- fe
  4x read 04 and store at CC46-CC49

  03 <- 10
  04 <- ce
  4x read 04 and store at CC4A-CC4D

  03 <- 10
  04 <- fe
  4x read 04 and store at CC4E-CC51


  03 <- 10
  04 <- fc
  4x read 04 and store at CC46-CC49

  03 <- 10
  04 <- cc
  4x read 04 and store at CC4A-CC4D

  03 <- 10
  04 <- fc
  4x read 04 and store at CC4E-CC51

*/


WRITE8_MEMBER(msx_matsushita_device::io_write)
{
	switch (offset)
	{
		// bit 0: CPU clock select
		//        0 - 5.369317 MHz
		//        1 - 3.579545 MHz
		case 0x01:
			m_turbo_out_cb((data & 1) ? ASSERT_LINE : CLEAR_LINE);
			break;

		case 0x03:
			m_nibble1 = data & 0x0f;
			m_nibble2 = data >> 4;
			break;

		case 0x04:
			m_pattern = data;
			break;

		case 0x07:   // Address low
			m_address = (m_address & 0xff00) | data;
			break;

		case 0x08:   // Address high
			m_address = (m_address & 0xff) | (data << 8);
			break;

		case 0x09:   // Data
			if (m_address < m_sram.size())
			{
				m_sram[m_address] = data;
			}
			break;

		default:
			logerror("msx_matsushita: unhandled write %02x to offset %02x\n", data, offset);
			break;
	}
}
