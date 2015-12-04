// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "konami.h"

const device_type MSX_CART_KONAMI = &device_creator<msx_cart_konami>;
const device_type MSX_CART_KONAMI_SCC = &device_creator<msx_cart_konami_scc>;
const device_type MSX_CART_GAMEMASTER2 = &device_creator<msx_cart_gamemaster2>;
const device_type MSX_CART_SYNTHESIZER = &device_creator<msx_cart_synthesizer>;
const device_type MSX_CART_SOUND_SNATCHER = &device_creator<msx_cart_konami_sound_snatcher>;
const device_type MSX_CART_SOUND_SDSNATCHER = &device_creator<msx_cart_konami_sound_sdsnatcher>;
const device_type MSX_CART_KEYBOARD_MASTER = &device_creator<msx_cart_keyboard_master>;


msx_cart_konami::msx_cart_konami(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_KONAMI, "MSX Cartridge - KONAMI", tag, owner, clock, "msx_cart_konami", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_bank_mask(0)
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = i;
	}
	for (int i = 0; i < 8; i++)
	{
		m_bank_base[i] = nullptr;
	}
}


void msx_cart_konami::device_start()
{
	save_item(NAME(m_selected_bank));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_konami::restore_banks), this));
}


void msx_cart_konami::restore_banks()
{
	m_bank_base[0] = get_rom_base() + ( m_selected_bank[0] & m_bank_mask ) * 0x2000;
	m_bank_base[1] = get_rom_base() + ( m_selected_bank[1] & m_bank_mask ) * 0x2000;
	m_bank_base[2] = get_rom_base() + ( m_selected_bank[0] & m_bank_mask ) * 0x2000;
	m_bank_base[3] = get_rom_base() + ( m_selected_bank[1] & m_bank_mask ) * 0x2000;
	m_bank_base[4] = get_rom_base() + ( m_selected_bank[2] & m_bank_mask ) * 0x2000;
	m_bank_base[5] = get_rom_base() + ( m_selected_bank[3] & m_bank_mask ) * 0x2000;
	m_bank_base[6] = get_rom_base() + ( m_selected_bank[2] & m_bank_mask ) * 0x2000;
	m_bank_base[7] = get_rom_base() + ( m_selected_bank[3] & m_bank_mask ) * 0x2000;
}


void msx_cart_konami::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = i;
	}
}


void msx_cart_konami::initialize_cartridge()
{
	UINT32 size = get_rom_size();

	if ( get_rom_size() > 256 * 0x2000 )
	{
		fatalerror("konami: ROM is too big\n");
	}

	UINT16 banks = size / 0x2000;

	if (size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		fatalerror("konami: Invalid ROM size\n");
	}

	m_bank_mask = banks - 1;

	restore_banks();
}


READ8_MEMBER(msx_cart_konami::read_cart)
{
	return m_bank_base[offset >> 13][offset & 0x1fff];
}


WRITE8_MEMBER(msx_cart_konami::write_cart)
{
	switch (offset & 0xe000)
	{
		case 0x4000:
			m_selected_bank[0] = data;
			m_bank_base[0] = get_rom_base() + ( m_selected_bank[0] & m_bank_mask ) * 0x2000;
			m_bank_base[2] = get_rom_base() + ( m_selected_bank[0] & m_bank_mask ) * 0x2000;
			break;

		case 0x6000:
			m_selected_bank[1] = data;
			m_bank_base[1] = get_rom_base() + ( m_selected_bank[1] & m_bank_mask ) * 0x2000;
			m_bank_base[3] = get_rom_base() + ( m_selected_bank[1] & m_bank_mask ) * 0x2000;
			break;

		case 0x8000:
			m_selected_bank[2] = data;
			m_bank_base[4] = get_rom_base() + ( m_selected_bank[2] & m_bank_mask ) * 0x2000;
			m_bank_base[6] = get_rom_base() + ( m_selected_bank[2] & m_bank_mask ) * 0x2000;
			break;

		case 0xa000:
			m_selected_bank[3] = data;
			m_bank_base[5] = get_rom_base() + ( m_selected_bank[3] & m_bank_mask ) * 0x2000;
			m_bank_base[7] = get_rom_base() + ( m_selected_bank[3] & m_bank_mask ) * 0x2000;
			break;
	}
}




msx_cart_konami_scc::msx_cart_konami_scc(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_KONAMI_SCC, "MSX Cartridge - KONAMI+SCC", tag, owner, clock, "msx_cart_konami_scc", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_k051649(*this, "k051649")
	, m_bank_mask(0)
	, m_scc_active(false)
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = i;
	}
	for (int i = 0; i < 8; i++)
	{
		m_bank_base[i] = nullptr;
	}
}


static MACHINE_CONFIG_FRAGMENT( konami_scc )
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("k051649", K051649, XTAL_10_738635MHz/3/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)
MACHINE_CONFIG_END


machine_config_constructor msx_cart_konami_scc::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( konami_scc );
}


void msx_cart_konami_scc::device_start()
{
	save_item(NAME(m_selected_bank));
	save_item(NAME(m_scc_active));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_konami_scc::restore_banks), this));
}


void msx_cart_konami_scc::restore_banks()
{
	m_bank_base[0] = get_rom_base() + ( m_selected_bank[2] & m_bank_mask ) * 0x2000;
	m_bank_base[1] = get_rom_base() + ( m_selected_bank[3] & m_bank_mask ) * 0x2000;
	m_bank_base[2] = get_rom_base() + ( m_selected_bank[0] & m_bank_mask ) * 0x2000;
	m_bank_base[3] = get_rom_base() + ( m_selected_bank[1] & m_bank_mask ) * 0x2000;
	m_bank_base[4] = get_rom_base() + ( m_selected_bank[2] & m_bank_mask ) * 0x2000;
	m_bank_base[5] = get_rom_base() + ( m_selected_bank[3] & m_bank_mask ) * 0x2000;
	m_bank_base[6] = get_rom_base() + ( m_selected_bank[0] & m_bank_mask ) * 0x2000;
	m_bank_base[7] = get_rom_base() + ( m_selected_bank[1] & m_bank_mask ) * 0x2000;
}


void msx_cart_konami_scc::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = i;
	}
	m_scc_active = false;
}


void msx_cart_konami_scc::initialize_cartridge()
{
	UINT32 size = get_rom_size();

	if ( get_rom_size() > 256 * 0x2000 )
	{
		fatalerror("konami_scc: ROM is too big\n");
	}

	UINT16 banks = size / 0x2000;

	if (size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		fatalerror("konami_scc: Invalid ROM size\n");
	}

	m_bank_mask = banks - 1;

	restore_banks();
}


READ8_MEMBER(msx_cart_konami_scc::read_cart)
{
	if ( m_scc_active && offset >= 0x9800 && offset < 0xa000 )
	{
		if (offset & 0x80)
		{
			if ((offset & 0xff) >= 0xe0)
			{
				return m_k051649->k051649_test_r(space, offset & 0xff);
			}
			return 0xff;
		}
		else
		{
			return m_k051649->k051649_waveform_r(space, offset & 0x7f);
		}
	}

	return m_bank_base[offset >> 13][offset & 0x1fff];
}


WRITE8_MEMBER(msx_cart_konami_scc::write_cart)
{
	switch (offset & 0xf800)
	{
		case 0x5000:
			m_selected_bank[0] = data;
			m_bank_base[2] = get_rom_base() + ( m_selected_bank[0] & m_bank_mask ) * 0x2000;
			m_bank_base[6] = get_rom_base() + ( m_selected_bank[0] & m_bank_mask ) * 0x2000;
			break;

		case 0x7000:
			m_selected_bank[1] = data;
			m_bank_base[3] = get_rom_base() + ( m_selected_bank[1] & m_bank_mask ) * 0x2000;
			m_bank_base[7] = get_rom_base() + ( m_selected_bank[1] & m_bank_mask ) * 0x2000;
			break;

		case 0x9000:
			m_selected_bank[2] = data;
			m_scc_active = ( ( data & 0x3f ) == 0x3f );
			m_bank_base[0] = get_rom_base() + ( m_selected_bank[2] & m_bank_mask ) * 0x2000;
			m_bank_base[4] = get_rom_base() + ( m_selected_bank[2] & m_bank_mask ) * 0x2000;
			break;

		case 0x9800:
			if ( m_scc_active )
			{
				offset &= 0xff;

				if (offset < 0x80)
				{
					m_k051649->k051649_waveform_w(space, offset, data);
				}
				else if (offset < 0xa0)
				{
					offset &= 0x0f;
					if (offset < 0x0a)
					{
						m_k051649->k051649_frequency_w(space, offset, data);
					}
					else if (offset < 0x0f)
					{
						m_k051649->k051649_volume_w(space, offset - 0xa, data);
					}
					else
					{
						m_k051649->k051649_keyonoff_w(space, 0, data);
					}
				}
				else if (offset >= 0xe0)
				{
					m_k051649->k051649_test_w(space, offset, data);
				}
			}
			break;

		case 0xb000:
			m_selected_bank[3] = data;
			m_bank_base[1] = get_rom_base() + ( m_selected_bank[3] & m_bank_mask ) * 0x2000;
			m_bank_base[5] = get_rom_base() + ( m_selected_bank[3] & m_bank_mask ) * 0x2000;
			break;
	}
}






msx_cart_gamemaster2::msx_cart_gamemaster2(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_GAMEMASTER2, "MSX Cartridge - GAMEMASTER2", tag, owner, clock, "msx_cart_gamemaster2", __FILE__)
	, msx_cart_interface(mconfig, *this)
{
	for (int i = 0; i < 3; i++)
	{
		m_selected_bank[i] = 0;
	}
	for (int i = 0; i < 8; i++)
	{
		m_bank_base[i] = nullptr;
	}
}


void msx_cart_gamemaster2::device_start()
{
	save_item(NAME(m_selected_bank));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_gamemaster2::restore_banks), this));
}


void msx_cart_gamemaster2::setup_bank(UINT8 bank)
{
	switch (bank)
	{
		case 0:
			if (m_selected_bank[0] & 0x10)
			{
				m_bank_base[1] = get_sram_base() + ((m_selected_bank[0] & 0x20) ? 0x1000 : 0);
				m_bank_base[3] = get_sram_base() + ((m_selected_bank[0] & 0x20) ? 0x1000 : 0);
			}
			else
			{
				m_bank_base[1] = get_rom_base() + ( m_selected_bank[0] & 0x0f ) * 0x2000;
				m_bank_base[3] = get_rom_base() + ( m_selected_bank[0] & 0x0f ) * 0x2000;
			}
			break;

		case 1:
			if (m_selected_bank[1] & 0x10)
			{
				m_bank_base[4] = get_sram_base() + ((m_selected_bank[1] & 0x20) ? 0x1000 : 0);
				m_bank_base[6] = get_sram_base() + ((m_selected_bank[1] & 0x20) ? 0x1000 : 0);
			}
			else
			{
				m_bank_base[4] = get_rom_base() + ( m_selected_bank[1] & 0x0f ) * 0x2000;
				m_bank_base[6] = get_rom_base() + ( m_selected_bank[1] & 0x0f ) * 0x2000;
			}
			break;

		case 2:
			if (m_selected_bank[2] & 0x10)
			{
				m_bank_base[5] = get_sram_base() + ((m_selected_bank[2] & 0x20) ? 0x1000 : 0);
				m_bank_base[7] = get_sram_base() + ((m_selected_bank[2] & 0x20) ? 0x1000 : 0);
			}
			else
			{
				m_bank_base[5] = get_rom_base() + ( m_selected_bank[2] & 0x0f ) * 0x2000;
				m_bank_base[7] = get_rom_base() + ( m_selected_bank[2] & 0x0f ) * 0x2000;
			}
			break;
	}
}


void msx_cart_gamemaster2::restore_banks()
{
	m_bank_base[0] = get_rom_base();
	m_bank_base[2] = get_rom_base();
	setup_bank(0);
	setup_bank(1);
	setup_bank(2);
}


void msx_cart_gamemaster2::device_reset()
{
	for (int i = 0; i < 3; i++)
	{
		m_selected_bank[i] = i + 1;
	}
}


void msx_cart_gamemaster2::initialize_cartridge()
{
	if ( get_rom_size() != 0x20000 )
	{
		fatalerror("gamemaster2: Invalid ROM size\n");
	}

	if (get_sram_size() != 0x2000)
	{
		fatalerror("gamemaster2: Invalid SRAM size\n");
	}

	restore_banks();
}


READ8_MEMBER(msx_cart_gamemaster2::read_cart)
{
	UINT8 bank = offset >> 13;

	switch (bank)
	{
		case 1:
		case 3:
			if (m_selected_bank[0] & 0x10)
			{
				return m_bank_base[bank][offset & 0x0fff];
			}
			break;

		case 4:
		case 6:
			if (m_selected_bank[1] & 0x10)
			{
				return m_bank_base[bank][offset & 0x0fff];
			}
			break;

		case 5:
		case 7:
			if (m_selected_bank[2] & 0x10)
			{
				return m_bank_base[bank][offset & 0x0fff];
			}
			break;
	}
	return m_bank_base[bank][offset & 0x1fff];
}


WRITE8_MEMBER(msx_cart_gamemaster2::write_cart)
{
	switch (offset & 0xf000)
	{
		case 0x6000:
			m_selected_bank[0] = data;
			setup_bank(0);
			break;

		case 0x8000:
			m_selected_bank[1] = data;
			setup_bank(1);
			break;

		case 0xa000:
			m_selected_bank[2] = data;
			setup_bank(2);
			break;

		case 0xb000:
			if (m_selected_bank[2] & 0x10)
			{
				m_bank_base[5][offset & 0x0fff] = data;
			}
			break;
	}
}





msx_cart_synthesizer::msx_cart_synthesizer(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_SYNTHESIZER, "MSX Cartridge - Synthesizer", tag, owner, clock, "msx_cart_synthesizer", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_bank_base(nullptr)
	, m_dac(*this, "dac")
{
}


static MACHINE_CONFIG_FRAGMENT( synthesizer )
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_CONFIG_END


machine_config_constructor msx_cart_synthesizer::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( synthesizer );
}


void msx_cart_synthesizer::device_start()
{
}


void msx_cart_synthesizer::initialize_cartridge()
{
	if ( get_rom_size() != 0x8000 )
	{
		fatalerror("synthesizer: Invalid ROM size\n");
	}

	m_bank_base = get_rom_base();
}


READ8_MEMBER(msx_cart_synthesizer::read_cart)
{
	if (offset >= 0x4000 && offset < 0xc000 )
	{
		return m_bank_base[offset - 0x4000];
	}
	return 0xff;
}


WRITE8_MEMBER(msx_cart_synthesizer::write_cart)
{
	if ((offset & 0xc010) == 0x4000)
	{
		m_dac->write_unsigned8(data);
	}
}




msx_cart_konami_sound::msx_cart_konami_sound(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	, msx_cart_interface(mconfig, *this)
	, m_k052539(*this, "k052539")
	, m_scc_active(false)
	, m_sccplus_active(false)
	, m_scc_mode(0)
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = 0;
	}
	for (int i = 0; i < 8; i++)
	{
		m_bank_base[i] = nullptr;
	}
	for (int i = 0; i < 16; i++)
	{
		m_ram_bank[i] = nullptr;
	}
	for (int i = 0; i < 4; i++)
	{
		m_ram_enabled[i] = false;
	}
}


static MACHINE_CONFIG_FRAGMENT( konami_sound )
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("k052539", K051649, XTAL_10_738635MHz/3/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)
MACHINE_CONFIG_END


machine_config_constructor msx_cart_konami_sound::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( konami_sound );
}


void msx_cart_konami_sound::device_start()
{
	save_item(NAME(m_selected_bank));
	save_item(NAME(m_scc_active));
	save_item(NAME(m_sccplus_active));
	save_item(NAME(m_ram_enabled));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_konami_sound::restore_banks), this));
}


void msx_cart_konami_sound::restore_banks()
{
	for (int i = 0; i < 4; i++)
	{
		setup_bank(i);
	}
}


void msx_cart_konami_sound::setup_bank(UINT8 bank)
{
	switch (bank)
	{
		case 0:
			m_bank_base[2] = m_ram_bank[m_selected_bank[0] & 0x0f];
			m_bank_base[6] = m_ram_bank[m_selected_bank[0] & 0x0f];
			break;

		case 1:
			m_bank_base[3] = m_ram_bank[m_selected_bank[1] & 0x0f];
			m_bank_base[7] = m_ram_bank[m_selected_bank[1] & 0x0f];
			break;

		case 2:
			m_bank_base[0] = m_ram_bank[m_selected_bank[2] & 0x0f];
			m_bank_base[4] = m_ram_bank[m_selected_bank[2] & 0x0f];
			break;

		case 3:
			m_bank_base[1] = m_ram_bank[m_selected_bank[3] & 0x0f];
			m_bank_base[5] = m_ram_bank[m_selected_bank[3] & 0x0f];
			break;
	}
}


void msx_cart_konami_sound::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = i;
		m_ram_enabled[i] = false;
	}
	m_scc_active = false;
	m_sccplus_active = false;
}


void msx_cart_konami_sound::initialize_cartridge()
{
	restore_banks();
}


READ8_MEMBER(msx_cart_konami_sound::read_cart)
{
	if ( m_scc_active && offset >= 0x9800 && offset < 0x9fe0 )
	{
		offset &= 0xff;
		if (offset < 0x80)
		{
			return m_k052539->k051649_waveform_r(space, offset);
		}
		if (offset < 0xa0)
		{
			return 0xff;
		}
		if (offset < 0xc0)
		{
			return m_k052539->k051649_waveform_r(space, offset & 0x9f);
		}
		if (offset < 0xe0)
		{
			return m_k052539->k051649_test_r(space, offset & 0xff);
		}
		return 0xff;
	}
	else if ( m_sccplus_active && offset >= 0xb800 && offset < 0xbfe0)
	{
		offset &= 0xff;

		if (offset < 0xa0)
		{
			return m_k052539->k052539_waveform_r(space, offset);
		}
		if (offset >= 0xc0 && offset < 0xe0)
		{
			return m_k052539->k051649_test_r(space, offset);
		}
		return 0xff;
	}

	UINT8 *base = m_bank_base[offset >> 13];

	if (base != nullptr)
	{
		return base[offset & 0x1fff];
	}
	return 0xff;
}


WRITE8_MEMBER(msx_cart_konami_sound::write_cart)
{
	switch (offset & 0xe000)
	{
		case 0x4000:
			if (m_ram_enabled[0] && m_bank_base[2] != nullptr)
			{
				m_bank_base[2][offset & 0x1fff] = data;
			}
			if ((offset & 0x1800) == 0x1000)
			{
				m_selected_bank[0] = data;
				setup_bank(0);
			}
			break;

		case 0x6000:
			if (m_ram_enabled[1] && m_bank_base[3] != nullptr)
			{
				m_bank_base[3][offset & 0x1fff] = data;
			}
			if ((offset & 0x1800) == 0x1000)
			{
				m_selected_bank[1] = data;
				setup_bank(1);
			}
			break;

		case 0x8000:
			if (m_ram_enabled[2] && m_bank_base[0] != nullptr)
			{
				m_bank_base[0][offset & 0x1fff] = data;
			}
			switch (offset & 0x1800)
			{
				case 0x1000:        // 0x9000-0x97ff
					m_selected_bank[2] = data;
					m_scc_active = ( ( data & 0x3f ) == 0x3f );
					setup_bank(2);
					break;

				case 0x1800:        // 0x9800-0x9fff
					if ( m_scc_active )
					{
						offset &= 0xff;

						if (offset < 0x80)
						{
							m_k052539->k051649_waveform_w(space, offset, data);
						}
						else if (offset < 0xa0)
						{
							offset &= 0x0f;
							if (offset < 0x0a)
							{
								m_k052539->k051649_frequency_w(space, offset, data);
							}
							else if (offset < 0x0f)
							{
								m_k052539->k051649_volume_w(space, offset - 0xa, data);
							}
							else
							{
								m_k052539->k051649_keyonoff_w(space, 0, data);
							}
						}
						else if (offset >= 0xe0)
						{
							m_k052539->k051649_test_w(space, offset, data);
						}
					}
					break;
			}
			break;

		case 0xa000:
			if (m_ram_enabled[3] && m_bank_base[1] != nullptr)
			{
				m_bank_base[1][offset & 0x1fff] = data;
			}
			switch (offset & 0x1800)
			{
				// 0xb000-0xb7ff
				case 0x1000:
					m_selected_bank[3] = data;
					setup_bank(3);
					break;

				// 0xb800-0xbfff
				case 0x1800:
					if ((offset & 0x7fe) == 0x7fe)
					{
						// 0xbffe-0xbfff
						/* write to mode register */
						m_scc_mode = data;

						m_ram_enabled[0] = ((m_scc_mode & 0x10) || (m_scc_mode & 0x01));
						m_ram_enabled[1] = ((m_scc_mode & 0x10) || (m_scc_mode & 0x02));
						m_ram_enabled[2] = ((m_scc_mode & 0x10) || ((m_scc_mode & 0x04) && (m_scc_mode & 0x20)));
						m_ram_enabled[3] = (m_scc_mode & 0x10);

						m_scc_active = ((m_selected_bank[2] & 0x3f) == 0x3f) && !(m_scc_mode & 0x20);
						m_sccplus_active = (m_selected_bank[3] & 0x80) && (m_scc_mode & 0x20);
					}
					else
					{
						if (m_sccplus_active)
						{
							offset &= 0xff;
							if (offset < 0xa0)
							{
								m_k052539->k052539_waveform_w(space, offset, data);
							}
							else if (offset < 0xc0)
							{
								offset &= 0x0f;
								if (offset < 0x0a)
								{
									m_k052539->k051649_frequency_w(space, offset, data);
								}
								else if (offset < 0x0f)
								{
									m_k052539->k051649_volume_w(space, offset - 0x0a, data);
								}
								else if (offset == 0x0f)
								{
									m_k052539->k051649_keyonoff_w(space, 0, data);
								}
							}
							else if (offset < 0xe0)
							{
								m_k052539->k051649_test_w(space, offset, data);
							}
						}
					}
					break;
			}
			break;
	}
}


msx_cart_konami_sound_snatcher::msx_cart_konami_sound_snatcher(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: msx_cart_konami_sound(mconfig, MSX_CART_SOUND_SNATCHER, "MSX Cartridge - Sound Snatcher", tag, owner, clock, "msx_cart_sound_snatcher", __FILE__)
{
}


void msx_cart_konami_sound_snatcher::initialize_cartridge()
{
	msx_cart_konami_sound::initialize_cartridge();

	if (get_ram_size() != 0x10000)
	{
		fatalerror("sound_snatcher: Invalid RAM size\n");
	}

	// The Snatcher Sound cartridge has 64KB RAM available by selecting ram banks 0-7

	for (int i = 0; i < 8; i++)
	{
		m_ram_bank[i] = get_ram_base() + i * 0x2000;
	}
}


msx_cart_konami_sound_sdsnatcher::msx_cart_konami_sound_sdsnatcher(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: msx_cart_konami_sound(mconfig, MSX_CART_SOUND_SDSNATCHER, "MSX Cartridge - Sound SD Snatcher", tag, owner, clock, "msx_cart_sound_sdsnatcher", __FILE__)
{
}


void msx_cart_konami_sound_sdsnatcher::initialize_cartridge()
{
	msx_cart_konami_sound::initialize_cartridge();

	if (get_ram_size() != 0x10000)
	{
		fatalerror("sound_sdsnatcher: Invalid RAM size\n");
	}

	// The SD Snatcher Sound cartrdige has 64KB RAM available by selecting ram banks 8-15

	for (int i = 0; i < 8; i++)
	{
		m_ram_bank[8+i] = get_ram_base() + i * 0x2000;
	}

}



msx_cart_keyboard_master::msx_cart_keyboard_master(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_CART_KEYBOARD_MASTER, "MSX Cartridge - Keyboard Master", tag, owner, clock, "msx_cart_keyboard_master", __FILE__)
	, msx_cart_interface(mconfig, *this)
	, m_vlm5030(*this, "vlm5030")
{
}


static MACHINE_CONFIG_FRAGMENT( msx_cart_keyboard_master )
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("vlm5030", VLM5030, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END


machine_config_constructor msx_cart_keyboard_master::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( msx_cart_keyboard_master );
}


void msx_cart_keyboard_master::device_start()
{
	// Install IO read/write handlers
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_write_handler(0x00, 0x00, write8_delegate(FUNC(vlm5030_device::data_w), m_vlm5030.target()));
	space.install_write_handler(0x20, 0x20, write8_delegate(FUNC(msx_cart_keyboard_master::io_20_w), this));
	space.install_read_handler(0x00, 0x00, read8_delegate(FUNC(msx_cart_keyboard_master::io_00_r), this));
}


void msx_cart_keyboard_master::initialize_cartridge()
{
	if (get_rom_size() != 0x4000)
	{
		fatalerror("keyboard_master: Invalid ROM size\n");
	}
	m_vlm5030->set_rom(&m_rom_vlm5030[0]);
}


READ8_MEMBER(msx_cart_keyboard_master::read_cart)
{
	if (offset >= 0x4000 && offset < 0x8000)
	{
		return m_rom[offset & 0x3fff];
	}
	return 0xff;
}


WRITE8_MEMBER(msx_cart_keyboard_master::io_20_w)
{
	m_vlm5030->rst((data & 0x01) ? 1 : 0);
	m_vlm5030->vcu((data & 0x04) ? 1 : 0);
	m_vlm5030->st((data & 0x02) ? 1 : 0);
}


READ8_MEMBER(msx_cart_keyboard_master::io_00_r)
{
	return m_vlm5030->bsy() ? 0x10 : 0x00;
}
