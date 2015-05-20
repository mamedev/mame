// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari vector hardware

***************************************************************************/

#include "emu.h"
#include "atari_vg.h"


// device type definition
const device_type ATARIVGEAROM = &device_creator<atari_vg_earom_device>;

//-------------------------------------------------
//  atari_vg_earom_device - constructor
//-------------------------------------------------

atari_vg_earom_device::atari_vg_earom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ATARIVGEAROM, "Atari VG EAROM", tag, owner, clock, "atari_vg_earom", __FILE__),
		device_nvram_interface(mconfig, *this)
{
}


READ8_MEMBER( atari_vg_earom_device::read )
{
	logerror("read earom: %02x(%02x):%02x\n", m_offset, offset, m_data);
	return (m_data);
}


WRITE8_MEMBER( atari_vg_earom_device::write )
{
	logerror("write earom: %02x:%02x\n", offset, data);
	m_offset = offset;
	m_data = data;
}


/* 0,8 and 14 get written to this location, too.
 * Don't know what they do exactly
 */
WRITE8_MEMBER( atari_vg_earom_device::ctrl_w )
{
	logerror("earom ctrl: %02x:%02x\n",offset, data);
	/*
	    0x01 = clock
	    0x02 = set data latch? - writes only (not always)
	    0x04 = write mode? - writes only
	    0x08 = set addr latch?
	*/
	if (data & 0x01)
	{
		m_data = m_rom[m_offset];
//printf("Read %02X = %02X\n", m_offset, m_data);
	}
	if ((data & 0x0c) == 0x0c)
	{
		m_rom[m_offset]=m_data;
		logerror("    written %02x:%02x\n", m_offset, m_data);
//printf("Write %02X = %02X\n", m_offset, m_data);
	}
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void atari_vg_earom_device::nvram_default()
{
	memset(m_rom,0,EAROM_SIZE);
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void atari_vg_earom_device::nvram_read(emu_file &file)
{
	file.read(m_rom,EAROM_SIZE);
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void atari_vg_earom_device::nvram_write(emu_file &file)
{
	file.write(m_rom,EAROM_SIZE);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void atari_vg_earom_device::device_start()
{
	/* register for save states */
	save_item(NAME(m_offset));
	save_item(NAME(m_data));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void atari_vg_earom_device::device_reset()
{
	m_data = 0;
	m_offset = 0;
}
