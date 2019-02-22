// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "majutsushi.h"

#include "sound/volt_reg.h"
#include "speaker.h"


DEFINE_DEVICE_TYPE(MSX_CART_MAJUTSUSHI, msx_cart_majutsushi_device, "msx_cart_majutsushi", "MSX Cartridge - Majutsushi")


msx_cart_majutsushi_device::msx_cart_majutsushi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_CART_MAJUTSUSHI, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_dac(*this, "dac")
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


void msx_cart_majutsushi_device::device_add_mconfig(machine_config &config)
{
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.05); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}


void msx_cart_majutsushi_device::device_start()
{
	save_item(NAME(m_selected_bank));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_cart_majutsushi_device::restore_banks), this));
}


void msx_cart_majutsushi_device::restore_banks()
{
	m_bank_base[0] = get_rom_base() + ( m_selected_bank[0] & 0x0f ) * 0x2000;
	m_bank_base[1] = get_rom_base() + ( m_selected_bank[1] & 0x0f ) * 0x2000;
	m_bank_base[2] = get_rom_base() + ( m_selected_bank[0] & 0x0f ) * 0x2000;
	m_bank_base[3] = get_rom_base() + ( m_selected_bank[1] & 0x0f ) * 0x2000;
	m_bank_base[4] = get_rom_base() + ( m_selected_bank[2] & 0x0f ) * 0x2000;
	m_bank_base[5] = get_rom_base() + ( m_selected_bank[3] & 0x0f ) * 0x2000;
	m_bank_base[6] = get_rom_base() + ( m_selected_bank[2] & 0x0f ) * 0x2000;
	m_bank_base[7] = get_rom_base() + ( m_selected_bank[3] & 0x0f ) * 0x2000;
}


void msx_cart_majutsushi_device::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = i;
	}
}


void msx_cart_majutsushi_device::initialize_cartridge()
{
	if ( get_rom_size() != 0x20000 )
	{
		fatalerror("majutsushi: Invalid ROM size\n");
	}

	restore_banks();
}


READ8_MEMBER(msx_cart_majutsushi_device::read_cart)
{
	return m_bank_base[offset >> 13][offset & 0x1fff];
}


WRITE8_MEMBER(msx_cart_majutsushi_device::write_cart)
{
	switch (offset & 0xe000)
	{
		case 0x4000:
			if (offset & 0x1000)
			{
				m_dac->write(data);
			}
			break;

		case 0x6000:
			m_selected_bank[1] = data & 0x0f;
			m_bank_base[1] = get_rom_base() + m_selected_bank[1] * 0x2000;
			m_bank_base[3] = get_rom_base() + m_selected_bank[1] * 0x2000;
			break;

		case 0x8000:
			m_selected_bank[2] = data & 0x0f;
			m_bank_base[4] = get_rom_base() + m_selected_bank[2] * 0x2000;
			m_bank_base[6] = get_rom_base() + m_selected_bank[2] * 0x2000;
			break;

		case 0xa000:
			m_selected_bank[3] = data & 0x0f;
			m_bank_base[5] = get_rom_base() + m_selected_bank[3] * 0x2000;
			m_bank_base[7] = get_rom_base() + m_selected_bank[3] * 0x2000;
			break;
	}
}
