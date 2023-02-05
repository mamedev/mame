// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/******************************************************************************

Al-Alamiah Al-Qur'an Al-Karim
Only works on Arabian MSXes.

GCMK-16X PCB, 2 ROM chips, Yamaha XE297A0 mapper chip.

******************************************************************************/

#include "emu.h"
#include "holy_quran.h"


DEFINE_DEVICE_TYPE(MSX_CART_HOLY_QURAN, msx_cart_holy_quran_device, "msx_cart_holy_quran", "MSX Cartridge - Holy Quran")


msx_cart_holy_quran_device::msx_cart_holy_quran_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_CART_HOLY_QURAN, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_rombank(*this, "rombank%u", 0U)
	, m_view1(*this, "view1")
	, m_view2(*this, "view2")
{
}

image_init_result msx_cart_holy_quran_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_holy_quran_device: Required region 'rom' was not found.";
		return image_init_result::FAIL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / BANK_SIZE;

	if (size > 256 * BANK_SIZE || size < 0x10000 || size != banks * BANK_SIZE || (~(banks - 1) % banks))
	{
		message = "msx_cart_holy_quran_device: Region 'rom' has unsupported size.";
		return image_init_result::FAIL;
	}

	m_bank_mask = banks - 1;

	m_decrypted.resize(size);

	u8 lookup_prot[256];
	// protection uses a simple rotation on databus, some lines inverted
	for (int i = 0; i < 0x100; i++)
		lookup_prot[i] = bitswap<8>(i,6,2,4,0,1,5,7,3) ^ 0x4d;

	u8 *rom = cart_rom_region()->base();
	for (u32 i = 0; i < size; i++)
		m_decrypted[i] = lookup_prot[rom[i]];

	for (int i = 0; i < 4; i++)
		m_rombank[i]->configure_entries(0, banks, m_decrypted.data(), BANK_SIZE);

	page(1)->install_view(0x4000, 0x7fff, m_view1);
	m_view1[0].install_read_handler(0x4000, 0x7fff, read8sm_delegate(*this, FUNC(msx_cart_holy_quran_device::read)));
	m_view1[1].install_read_bank(0x4000, 0x5fff, m_rombank[0]);
	m_view1[1].install_read_bank(0x6000, 0x7fff, m_rombank[1]);
	m_view1[1].install_write_handler(0x5000, 0x5000, write8smo_delegate(*this, FUNC(msx_cart_holy_quran_device::bank_w<0>)));
	m_view1[1].install_write_handler(0x5400, 0x5400, write8smo_delegate(*this, FUNC(msx_cart_holy_quran_device::bank_w<1>)));
	m_view1[1].install_write_handler(0x5800, 0x5800, write8smo_delegate(*this, FUNC(msx_cart_holy_quran_device::bank_w<2>)));
	m_view1[1].install_write_handler(0x5c00, 0x5c00, write8smo_delegate(*this, FUNC(msx_cart_holy_quran_device::bank_w<3>)));

	page(2)->install_view(0x8000, 0xbfff, m_view2);
	m_view2[0].install_read_handler(0x8000, 0xbfff, read8sm_delegate(*this, FUNC(msx_cart_holy_quran_device::read2)));
	m_view2[1].install_read_bank(0x8000, 0x9fff, m_rombank[2]);
	m_view2[1].install_read_bank(0xa000, 0xbfff, m_rombank[3]);

	return image_init_result::PASS;
}

void msx_cart_holy_quran_device::device_reset()
{
	m_view1.select(0);
	m_view2.select(0);
	for (int i = 0; i < 4; i++)
		m_rombank[i]->set_entry(0);
}

u8 msx_cart_holy_quran_device::read(offs_t offset)
{
	u8 data = cart_rom_region()->base()[offset];
	// The decryption should actually start working after the first M1 cycle executing something from the cartridge.
	if (offset + 0x4000 == ((cart_rom_region()->base()[3] << 8) | cart_rom_region()->base()[2]) && !machine().side_effects_disabled())
	{
		// Switch to decrypted contents
		m_view1.select(1);
		m_view2.select(1);
	}
	return data;
}

u8 msx_cart_holy_quran_device::read2(offs_t offset)
{
	u8 data = cart_rom_region()->base()[offset + 0x4000];
	// The decryption should actually start working after the first M1 cycle executing something from the cartridge.
	if (offset + 0x8000 == ((cart_rom_region()->base()[3] << 8) | cart_rom_region()->base()[2]) && !machine().side_effects_disabled())
	{
		// Switch to decrypted contents
		m_view1.select(1);
		m_view2.select(1);
	}
	return data;
}

template <int Bank>
void msx_cart_holy_quran_device::bank_w(u8 data)
{
	m_rombank[Bank]->set_entry(data & m_bank_mask);
}
