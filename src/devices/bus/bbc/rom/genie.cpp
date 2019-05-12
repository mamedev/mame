// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
// thanks-to:Derek Mathieson (original author of Genie)
/***************************************************************************

    PMS Genie

***************************************************************************/

#include "emu.h"
#include "genie.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_PMSGENIE, bbc_pmsgenie_device, "bbc_pmsgenie", "PMS Genie ROM Board")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_palprom_device - constructor
//-------------------------------------------------

bbc_pmsgenie_device::bbc_pmsgenie_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_PMSGENIE, tag, owner, clock)
	, device_bbc_rom_interface(mconfig, *this)
	, m_write_latch(0)
	, m_bank_latch(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_pmsgenie_device::device_start()
{
	save_item(NAME(m_write_latch));
	save_item(NAME(m_bank_latch));
}

//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t bbc_pmsgenie_device::read(offs_t offset)
{
	uint8_t data = 0xff;

	if (!machine().side_effects_disabled())
	{
		switch (offset >> 8)
		{
		case 0x1e:
			/* &9E00-&9EFF - Write value latch */
			m_write_latch = offset & 0xff;
			break;
		case 0x1f:
			/* &9F00-&9FFF - Bank select latch
         Bit
         0-2: RAM bank select
           3: Not used
         4-5: ROM bank select
           6: Not used
           7: Read / NOT Write for internal RAM */
			m_bank_latch = offset & 0xff;
			break;
		}
	}

	switch (offset & 0x2000)
	{
	case 0x0000:
		/* &8000-&9FFF - 4 Pages of 8K ROM (32K in total) */
		data = get_rom_base()[(offset & 0x1fff) | (m_bank_latch & 0x30) << 9];
		break;

	case 0x2000:
		/* &A000-&BFFF - 8 Pages of 8K RAM (64K in total) */
		if (m_bank_latch & 0x80)
		{
			/* RAM read */
			if (m_bank_latch & 0x04)
			{
				data = get_nvram_base()[(offset & 0x1fff) | (m_bank_latch & 0x03) << 13];
			}
			else
			{
				data = get_ram_base()[(offset & 0x1fff) | (m_bank_latch & 0x03) << 13];
			}
		}
		else
		{
			/* RAM write */
			if (m_bank_latch & 0x04)
			{
				get_nvram_base()[(offset & 0x1fff) | (m_bank_latch & 0x03) << 13] = m_write_latch;
			}
			else
			{
				get_ram_base()[(offset & 0x1fff) | (m_bank_latch & 0x03) << 13] = m_write_latch;
			}
			data = m_write_latch;
		}
		break;
	}

	return data;
}
