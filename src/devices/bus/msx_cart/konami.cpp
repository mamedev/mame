// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "konami.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(MSX_CART_KONAMI,           msx_cart_konami_device,                  "msx_cart_konami",           "MSX Cartridge - KONAMI")
DEFINE_DEVICE_TYPE(MSX_CART_KONAMI_SCC,       msx_cart_konami_scc_device,              "msx_cart_konami_scc",       "MSX Cartridge - KONAMI+SCC")
DEFINE_DEVICE_TYPE(MSX_CART_GAMEMASTER2,      msx_cart_gamemaster2_device,             "msx_cart_gamemaster2",      "MSX Cartridge - GAMEMASTER2")
DEFINE_DEVICE_TYPE(MSX_CART_SYNTHESIZER,      msx_cart_synthesizer_device,             "msx_cart_synthesizer",      "MSX Cartridge - Synthesizer")
DEFINE_DEVICE_TYPE(MSX_CART_SOUND_SNATCHER,   msx_cart_konami_sound_snatcher_device,   "msx_cart_sound_snatcher",   "MSX Cartridge - Sound Snatcher")
DEFINE_DEVICE_TYPE(MSX_CART_SOUND_SDSNATCHER, msx_cart_konami_sound_sdsnatcher_device, "msx_cart_sound_sdsnatcher", "MSX Cartridge - Sound SD Snatcher")
DEFINE_DEVICE_TYPE(MSX_CART_KEYBOARD_MASTER,  msx_cart_keyboard_master_device,         "msx_cart_keyboard_master",  "MSX Cartridge - Keyboard Master")


msx_cart_konami_device::msx_cart_konami_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_KONAMI, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_bank_mask(0)
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = i;
	}
	for (auto & elem : m_bank_base)
	{
		elem = nullptr;
	}
}


void msx_cart_konami_device::device_start()
{
	save_item(NAME(m_selected_bank));
}


void msx_cart_konami_device::device_post_load()
{
	restore_banks();
}


void msx_cart_konami_device::restore_banks()
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


void msx_cart_konami_device::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = i;
	}
}


void msx_cart_konami_device::initialize_cartridge()
{
	uint32_t size = get_rom_size();

	if ( get_rom_size() > 256 * 0x2000 )
	{
		fatalerror("konami: ROM is too big\n");
	}

	uint16_t banks = size / 0x2000;

	if (size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		fatalerror("konami: Invalid ROM size\n");
	}

	m_bank_mask = banks - 1;

	restore_banks();
}


uint8_t msx_cart_konami_device::read_cart(offs_t offset)
{
	return m_bank_base[offset >> 13][offset & 0x1fff];
}


void msx_cart_konami_device::write_cart(offs_t offset, uint8_t data)
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




msx_cart_konami_scc_device::msx_cart_konami_scc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_KONAMI_SCC, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_k051649(*this, "k051649")
	, m_bank_mask(0)
	, m_scc_active(false)
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = i;
	}
	for (auto & elem : m_bank_base)
	{
		elem = nullptr;
	}
}


void msx_cart_konami_scc_device::device_add_mconfig(machine_config &config)
{
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	SPEAKER(config, "mono").front_center();
	K051649(config, m_k051649, XTAL(10'738'635)/3).add_route(ALL_OUTPUTS, "mono", 0.15);
}


void msx_cart_konami_scc_device::device_start()
{
	save_item(NAME(m_selected_bank));
	save_item(NAME(m_scc_active));
}


void msx_cart_konami_scc_device::device_post_load()
{
	restore_banks();
}


void msx_cart_konami_scc_device::restore_banks()
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


void msx_cart_konami_scc_device::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = i;
	}
	m_scc_active = false;
}


void msx_cart_konami_scc_device::initialize_cartridge()
{
	uint32_t size = get_rom_size();

	if ( get_rom_size() > 256 * 0x2000 )
	{
		fatalerror("konami_scc: ROM is too big\n");
	}

	uint16_t banks = size / 0x2000;

	if (size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		fatalerror("konami_scc: Invalid ROM size\n");
	}

	m_bank_mask = banks - 1;

	restore_banks();
}


uint8_t msx_cart_konami_scc_device::read_cart(offs_t offset)
{
	if ( m_scc_active && offset >= 0x9800 && offset < 0xa000 )
	{
		if (offset & 0x80)
		{
			if ((offset & 0xff) >= 0xe0)
			{
				return m_k051649->k051649_test_r();
			}
			return 0xff;
		}
		else
		{
			return m_k051649->k051649_waveform_r(offset & 0x7f);
		}
	}

	return m_bank_base[offset >> 13][offset & 0x1fff];
}


void msx_cart_konami_scc_device::write_cart(offs_t offset, uint8_t data)
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
					m_k051649->k051649_waveform_w(offset, data);
				}
				else if (offset < 0xa0)
				{
					offset &= 0x0f;
					if (offset < 0x0a)
					{
						m_k051649->k051649_frequency_w(offset, data);
					}
					else if (offset < 0x0f)
					{
						m_k051649->k051649_volume_w(offset - 0xa, data);
					}
					else
					{
						m_k051649->k051649_keyonoff_w(data);
					}
				}
				else if (offset >= 0xe0)
				{
					m_k051649->k051649_test_w(data);
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






msx_cart_gamemaster2_device::msx_cart_gamemaster2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_GAMEMASTER2, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
{
	for (auto & elem : m_selected_bank)
	{
		elem = 0;
	}
	for (auto & elem : m_bank_base)
	{
		elem = nullptr;
	}
}


void msx_cart_gamemaster2_device::device_start()
{
	save_item(NAME(m_selected_bank));
}


void msx_cart_gamemaster2_device::device_post_load()
{
	restore_banks();
}


void msx_cart_gamemaster2_device::setup_bank(uint8_t bank)
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


void msx_cart_gamemaster2_device::restore_banks()
{
	m_bank_base[0] = get_rom_base();
	m_bank_base[2] = get_rom_base();
	setup_bank(0);
	setup_bank(1);
	setup_bank(2);
}


void msx_cart_gamemaster2_device::device_reset()
{
	for (int i = 0; i < 3; i++)
	{
		m_selected_bank[i] = i + 1;
	}
}


void msx_cart_gamemaster2_device::initialize_cartridge()
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


uint8_t msx_cart_gamemaster2_device::read_cart(offs_t offset)
{
	uint8_t bank = offset >> 13;

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


void msx_cart_gamemaster2_device::write_cart(offs_t offset, uint8_t data)
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





msx_cart_synthesizer_device::msx_cart_synthesizer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_SYNTHESIZER, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_bank_base(nullptr)
	, m_dac(*this, "dac")
{
}


void msx_cart_synthesizer_device::device_add_mconfig(machine_config &config)
{
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.1); // unknown DAC
}


void msx_cart_synthesizer_device::device_start()
{
}


void msx_cart_synthesizer_device::initialize_cartridge()
{
	if ( get_rom_size() != 0x8000 )
	{
		fatalerror("synthesizer: Invalid ROM size\n");
	}

	m_bank_base = get_rom_base();
}


uint8_t msx_cart_synthesizer_device::read_cart(offs_t offset)
{
	if (offset >= 0x4000 && offset < 0xc000 )
	{
		return m_bank_base[offset - 0x4000];
	}
	return 0xff;
}


void msx_cart_synthesizer_device::write_cart(offs_t offset, uint8_t data)
{
	if ((offset & 0xc010) == 0x4000)
	{
		m_dac->write(data);
	}
}




msx_cart_konami_sound_device::msx_cart_konami_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_k052539(*this, "k052539")
	, m_scc_active(false)
	, m_sccplus_active(false)
	, m_scc_mode(0)
{
	for (auto & elem : m_selected_bank)
	{
		elem = 0;
	}
	for (auto & elem : m_bank_base)
	{
		elem = nullptr;
	}
	for (auto & elem : m_ram_bank)
	{
		elem = nullptr;
	}
	for (auto & elem : m_ram_enabled)
	{
		elem = false;
	}
}


void msx_cart_konami_sound_device::device_add_mconfig(machine_config &config)
{
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	SPEAKER(config, "mono").front_center();
	K051649(config, m_k052539, XTAL(10'738'635)/3).add_route(ALL_OUTPUTS, "mono", 0.15);
}


void msx_cart_konami_sound_device::device_start()
{
	save_item(NAME(m_selected_bank));
	save_item(NAME(m_scc_active));
	save_item(NAME(m_sccplus_active));
	save_item(NAME(m_ram_enabled));
}


void msx_cart_konami_sound_device::device_post_load()
{
	restore_banks();
}


void msx_cart_konami_sound_device::restore_banks()
{
	for (int i = 0; i < 4; i++)
	{
		setup_bank(i);
	}
}


void msx_cart_konami_sound_device::setup_bank(uint8_t bank)
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


void msx_cart_konami_sound_device::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = i;
		m_ram_enabled[i] = false;
	}
	m_scc_active = false;
	m_sccplus_active = false;
}


void msx_cart_konami_sound_device::initialize_cartridge()
{
	restore_banks();
}


uint8_t msx_cart_konami_sound_device::read_cart(offs_t offset)
{
	if ( m_scc_active && offset >= 0x9800 && offset < 0x9fe0 )
	{
		offset &= 0xff;
		if (offset < 0x80)
		{
			return m_k052539->k051649_waveform_r(offset);
		}
		if (offset < 0xa0)
		{
			return 0xff;
		}
		if (offset < 0xc0)
		{
			return m_k052539->k051649_waveform_r(offset & 0x9f);
		}
		if (offset < 0xe0)
		{
			return m_k052539->k051649_test_r();
		}
		return 0xff;
	}
	else if ( m_sccplus_active && offset >= 0xb800 && offset < 0xbfe0)
	{
		offset &= 0xff;

		if (offset < 0xa0)
		{
			return m_k052539->k052539_waveform_r(offset);
		}
		if (offset >= 0xc0 && offset < 0xe0)
		{
			return m_k052539->k051649_test_r();
		}
		return 0xff;
	}

	uint8_t *base = m_bank_base[offset >> 13];

	if (base != nullptr)
	{
		return base[offset & 0x1fff];
	}
	return 0xff;
}


void msx_cart_konami_sound_device::write_cart(offs_t offset, uint8_t data)
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
							m_k052539->k051649_waveform_w(offset, data);
						}
						else if (offset < 0xa0)
						{
							offset &= 0x0f;
							if (offset < 0x0a)
							{
								m_k052539->k051649_frequency_w(offset, data);
							}
							else if (offset < 0x0f)
							{
								m_k052539->k051649_volume_w(offset - 0xa, data);
							}
							else
							{
								m_k052539->k051649_keyonoff_w(data);
							}
						}
						else if (offset >= 0xe0)
						{
							m_k052539->k051649_test_w(data);
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
								m_k052539->k052539_waveform_w(offset, data);
							}
							else if (offset < 0xc0)
							{
								offset &= 0x0f;
								if (offset < 0x0a)
								{
									m_k052539->k051649_frequency_w(offset, data);
								}
								else if (offset < 0x0f)
								{
									m_k052539->k051649_volume_w(offset - 0x0a, data);
								}
								else if (offset == 0x0f)
								{
									m_k052539->k051649_keyonoff_w(data);
								}
							}
							else if (offset < 0xe0)
							{
								m_k052539->k051649_test_w(data);
							}
						}
					}
					break;
			}
			break;
	}
}


msx_cart_konami_sound_snatcher_device::msx_cart_konami_sound_snatcher_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_cart_konami_sound_device(mconfig, MSX_CART_SOUND_SNATCHER, tag, owner, clock)
{
}


void msx_cart_konami_sound_snatcher_device::initialize_cartridge()
{
	msx_cart_konami_sound_device::initialize_cartridge();

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


msx_cart_konami_sound_sdsnatcher_device::msx_cart_konami_sound_sdsnatcher_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_cart_konami_sound_device(mconfig, MSX_CART_SOUND_SDSNATCHER, tag, owner, clock)
{
}


void msx_cart_konami_sound_sdsnatcher_device::initialize_cartridge()
{
	msx_cart_konami_sound_device::initialize_cartridge();

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



msx_cart_keyboard_master_device::msx_cart_keyboard_master_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_KEYBOARD_MASTER, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_vlm5030(*this, "vlm5030")
{
}


void msx_cart_keyboard_master_device::vlm_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(msx_cart_keyboard_master_device::read_vlm));
}


void msx_cart_keyboard_master_device::device_add_mconfig(machine_config &config)
{
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	SPEAKER(config, "mono").front_center();
	VLM5030(config, m_vlm5030, XTAL(3'579'545));
	m_vlm5030->add_route(ALL_OUTPUTS, "mono", 0.40);
	m_vlm5030->set_addrmap(0, &msx_cart_keyboard_master_device::vlm_map);
}


void msx_cart_keyboard_master_device::device_start()
{
	// Install IO read/write handlers
	io_space().install_write_handler(0x00, 0x00, write8smo_delegate(*m_vlm5030, FUNC(vlm5030_device::data_w)));
	io_space().install_write_handler(0x20, 0x20, write8smo_delegate(*this, FUNC(msx_cart_keyboard_master_device::io_20_w)));
	io_space().install_read_handler(0x00, 0x00, read8smo_delegate(*this, FUNC(msx_cart_keyboard_master_device::io_00_r)));
}


void msx_cart_keyboard_master_device::initialize_cartridge()
{
	if (get_rom_size() != 0x4000)
	{
		fatalerror("keyboard_master: Invalid ROM size\n");
	}
}


uint8_t msx_cart_keyboard_master_device::read_cart(offs_t offset)
{
	if (offset >= 0x4000 && offset < 0x8000)
	{
		return m_rom[offset & 0x3fff];
	}
	return 0xff;
}


uint8_t msx_cart_keyboard_master_device::read_vlm(offs_t offset)
{
	return m_rom_vlm5030[offset];
}


void msx_cart_keyboard_master_device::io_20_w(uint8_t data)
{
	m_vlm5030->rst((data & 0x01) ? 1 : 0);
	m_vlm5030->vcu((data & 0x04) ? 1 : 0);
	m_vlm5030->st((data & 0x02) ? 1 : 0);
}


uint8_t msx_cart_keyboard_master_device::io_00_r()
{
	return m_vlm5030->bsy() ? 0x10 : 0x00;
}
