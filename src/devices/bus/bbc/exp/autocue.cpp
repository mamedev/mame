// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Autocue RAM Disc

  The RAM is accessible through JIM (page &FD). One page is visible in JIM at a time.
  The selected page is controlled by the two paging registers:

  &FCFE       Paging register MSB
  &FCFF       Paging register LSB

  256K board has 1024 pages &000 to &3FF
  512K board has 2048 pages &000 to &7FF

**********************************************************************/


#include "emu.h"
#include "autocue.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_AUTOCUE, bbc_autocue_device, "bbc_autocue", "Autocue RAM Disc");


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_autocue_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_autocue_device - constructor
//-------------------------------------------------

bbc_autocue_device::bbc_autocue_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_AUTOCUE, tag, owner, clock)
	, device_bbc_exp_interface(mconfig, *this)
	, m_nvram(*this, "nvram")
	, m_ram_page(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_autocue_device::device_start()
{
	/* ram disk - board with 8 x HM62256LFP-12 - 256K expandable to 512K */
	m_ram = make_unique_clear<uint8_t[]>(0x40000);
	m_nvram->set_base(m_ram.get(), 0x40000);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_autocue_device::fred_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0xff: m_ram_page = (m_ram_page & 0x00ff) | (data << 8); break;
	case 0xfe: m_ram_page = (m_ram_page & 0xff00) | (data << 0); break;
	}
	//logerror("Write ram_page=%04x\n", m_ram_page);
}

uint8_t bbc_autocue_device::jim_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_ram_page < 0x400)
	{
		data = m_ram[(m_ram_page << 8) | offset];
	}
	//logerror("Read %04x -> %02x\n", offset | 0xfd00, data);
	return data;
}

void bbc_autocue_device::jim_w(offs_t offset, uint8_t data)
{
	//logerror("Write %04x <- %02x\n", offset | 0xfd00, data);
	if (m_ram_page < 0x400)
	{
		m_ram[(m_ram_page << 8) | offset] = data;
	}
}