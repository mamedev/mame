// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************************

When backing up the SRAM from an FM-PAC the file seems to be prefixed
with: PAC2 BACKUP DATA. We only store the raw sram contents.

**********************************************************************************/

#include "emu.h"
#include "fmpac.h"

const device_type MSX_CART_FMPAC = &device_creator<msx_cart_fmpac>;


msx_cart_fmpac::msx_cart_fmpac(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_FMPAC, "MSX Cartridge - FM-PAC", tag, owner, clock, "msx_cart_fmpac", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_ym2413(*this, "ym2413")
	, m_selected_bank(0)
	, m_bank_base(NULL)
	, m_sram_active(false)
	, m_opll_active(false)
	, m_1ffe(0)
	, m_1fff(0)
	, m_7ff6(0)
{
}


static MACHINE_CONFIG_FRAGMENT( fmpac )
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ym2413", YM2413, XTAL_10_738635MHz/3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END


machine_config_constructor msx_cart_fmpac::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( fmpac );
}


void msx_cart_fmpac::device_start()
{
	save_item(NAME(m_selected_bank));
	save_item(NAME(m_sram_active));
	save_item(NAME(m_opll_active));
	save_item(NAME(m_1ffe));
	save_item(NAME(m_1fff));
	save_item(NAME(m_7ff6));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_fmpac::restore_banks), this));

	// Install IO read/write handlers
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_write_handler(0x7c, 0x7d, write8_delegate(FUNC(msx_cart_fmpac::write_ym2413), this));
}


void msx_cart_fmpac::restore_banks()
{
	m_bank_base = get_rom_base() + ( m_selected_bank & 0x03 ) * 0x4000;
}


void msx_cart_fmpac::device_reset()
{
	m_selected_bank = 0;
	m_sram_active = false;
	m_opll_active = false;
	m_1ffe = 0;
	m_1fff = 0;
	m_7ff6 = 0;
}


void msx_cart_fmpac::initialize_cartridge()
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


READ8_MEMBER(msx_cart_fmpac::read_cart)
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


WRITE8_MEMBER(msx_cart_fmpac::write_cart)
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
				m_ym2413->write(space, offset & 1, data);
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


WRITE8_MEMBER(msx_cart_fmpac::write_ym2413)
{
	if (m_opll_active)
	{
		m_ym2413->write(space, offset & 1, data);
	}
}
