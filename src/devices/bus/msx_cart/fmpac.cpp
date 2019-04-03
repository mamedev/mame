// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************************

When backing up the SRAM from an FM-PAC the file seems to be prefixed
with: PAC2 BACKUP DATA. We only store the raw sram contents.

**********************************************************************************/

#include "emu.h"
#include "fmpac.h"
#include "speaker.h"

DEFINE_DEVICE_TYPE(MSX_CART_FMPAC, msx_cart_fmpac_device, "msx_cart_fmpac", "MSX Cartridge - FM-PAC")


msx_cart_fmpac_device::msx_cart_fmpac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_FMPAC, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_ym2413(*this, "ym2413")
	, m_selected_bank(0)
	, m_bank_base(nullptr)
	, m_sram_active(false)
	, m_opll_active(false)
	, m_1ffe(0)
	, m_1fff(0)
	, m_7ff6(0)
{
}


void msx_cart_fmpac_device::device_add_mconfig(machine_config &config)
{
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	SPEAKER(config, "mono").front_center();
	YM2413(config, m_ym2413, XTAL(10'738'635)/3).add_route(ALL_OUTPUTS, "mono", 0.40);
}


void msx_cart_fmpac_device::device_start()
{
	save_item(NAME(m_selected_bank));
	save_item(NAME(m_sram_active));
	save_item(NAME(m_opll_active));
	save_item(NAME(m_1ffe));
	save_item(NAME(m_1fff));
	save_item(NAME(m_7ff6));

	// Install IO read/write handlers
	io_space().install_write_handler(0x7c, 0x7d, write8sm_delegate(FUNC(msx_cart_fmpac_device::write_ym2413), this));
}


void msx_cart_fmpac_device::device_post_load()
{
	restore_banks();
}


void msx_cart_fmpac_device::restore_banks()
{
	m_bank_base = get_rom_base() + ( m_selected_bank & 0x03 ) * 0x4000;
}


void msx_cart_fmpac_device::device_reset()
{
	m_selected_bank = 0;
	m_sram_active = false;
	m_opll_active = false;
	m_1ffe = 0;
	m_1fff = 0;
	m_7ff6 = 0;
}


void msx_cart_fmpac_device::initialize_cartridge()
{
	if ( get_rom_size() != 0x10000 )
	{
		fatalerror("fmpac: Invalid ROM size\n");
	}

	if ( get_sram_size() != 0x2000 )
	{
		fatalerror("fmpac: Invalid SRAM size\n");
	}

	restore_banks();
}


uint8_t msx_cart_fmpac_device::read_cart(offs_t offset)
{
	if (offset >= 0x4000 && offset < 0x8000)
	{
		if (offset == 0x7ff6)
		{
			return m_7ff6;
		}
		if (offset == 0x7ff7)
		{
			return m_selected_bank & 0x03;
		}
		if (m_sram_active)
		{
			if (offset & 0x2000)
			{
				return 0xff;
			}
			return get_sram_base()[offset & 0x1fff];
		}
		else
		{
			return m_bank_base[offset & 0x3fff];
		}
	}
	return 0xff;
}


void msx_cart_fmpac_device::write_cart(offs_t offset, uint8_t data)
{
	if (offset >= 0x4000 && offset < 0x6000)
	{
		if (m_sram_active)
		{
			get_sram_base()[offset & 0x1fff] = data;
		}
		if (offset == 0x5ffe)
		{
			m_1ffe = data;
		}
		if (offset == 0x5fff)
		{
			m_1fff = data;
		}
		m_sram_active = (m_1ffe == 0x4d) && (m_1fff == 0x69);
	}

	switch (offset)
	{
		case 0x7ff4:
		case 0x7ff5:
			if (m_opll_active)
			{
				m_ym2413->write(offset & 1, data);
			}
			break;

		case 0x7ff6:
			m_7ff6 = data & 0x11;
			m_opll_active = (m_7ff6 & 0x01);
			break;

		case 0x7ff7:
			m_selected_bank = data;
			restore_banks();
			break;
	}

}


void msx_cart_fmpac_device::write_ym2413(offs_t offset, uint8_t data)
{
	if (m_opll_active)
	{
		m_ym2413->write(offset & 1, data);
	}
}
