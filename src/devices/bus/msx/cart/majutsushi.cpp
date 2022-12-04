// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "majutsushi.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(MSX_CART_MAJUTSUSHI, msx_cart_majutsushi_device, "msx_cart_majutsushi", "MSX Cartridge - Majutsushi")


msx_cart_majutsushi_device::msx_cart_majutsushi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_CART_MAJUTSUSHI, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_dac(*this, "dac")
	, m_rombank(*this, "rombank%u", 0U)
{
}

void msx_cart_majutsushi_device::device_add_mconfig(machine_config &config)
{
	// This is actually incorrect. The sound output is passed back into the MSX machine where it is mixed internally and output through the system 'speaker'.
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.05); // unknown DAC
}

image_init_result msx_cart_majutsushi_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_majutsushi_device: Required region 'rom' was not found.";
		return image_init_result::FAIL;
	}

	if (cart_rom_region()->bytes() != 0x20000)
	{
		message = "msx_cart_majutsushi_device: Region 'rom' has unsupported size.";
		return image_init_result::FAIL;
	}

	for (int i = 0; i < 3; i++)
		m_rombank[i]->configure_entries(0, 16, cart_rom_region()->base(), 0x2000);

	page(0)->install_rom(0x0000, 0x1fff, cart_rom_region()->base());
	page(0)->install_read_bank(0x2000, 0x3fff, m_rombank[0]);
	page(1)->install_rom(0x4000, 0x5fff, cart_rom_region()->base());
	page(1)->install_write_handler(0x5000, 0x5000, 0, 0x0fff, 0, write8smo_delegate(m_dac, FUNC(dac_byte_interface::write)));
	page(1)->install_read_bank(0x6000, 0x7fff, m_rombank[0]);
	page(1)->install_write_handler(0x6000, 0x6000, 0, 0x1fff, 0, write8smo_delegate(*this, FUNC(msx_cart_majutsushi_device::bank_w<0>)));
	page(2)->install_read_bank(0x8000, 0x9fff, m_rombank[1]);
	page(2)->install_write_handler(0x8000, 0x8000, 0, 0x1fff, 0, write8smo_delegate(*this, FUNC(msx_cart_majutsushi_device::bank_w<1>)));
	page(2)->install_read_bank(0xa000, 0xbfff, m_rombank[2]);
	page(2)->install_write_handler(0xa000, 0xa000, 0, 0x1fff, 0, write8smo_delegate(*this, FUNC(msx_cart_majutsushi_device::bank_w<2>)));
	page(3)->install_read_bank(0xc000, 0xdfff, m_rombank[1]);
	page(3)->install_read_bank(0xe000, 0xffff, m_rombank[2]);

	return image_init_result::PASS;
}

template <int Bank>
void msx_cart_majutsushi_device::bank_w(u8 data)
{
	m_rombank[Bank]->set_entry(data & 0x0f);
}
