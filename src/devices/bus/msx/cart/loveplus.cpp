// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/******************************************************************************

Banking for Love Plus Pack 0, Korean 83-in-1 pirate.

******************************************************************************/
#include "emu.h"
#include "loveplus.h"


namespace {

class msx_cart_loveplus_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_loveplus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MSX_CART_LOVEPLUS, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_rombank(*this, "rombank%u", 0U)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	template <int Bank> void bank_w(u8 data);
	void bank_9000_w(u8 data);

	memory_bank_array_creator<4> m_rombank;

	u8 m_bank_mask;
	bool m_bank_locked[4];
};


void msx_cart_loveplus_device::device_start()
{
	save_item(NAME(m_bank_locked));
}

void msx_cart_loveplus_device::device_reset()
{
	m_rombank[0]->set_entry(0);
	m_rombank[1]->set_entry(1);
	m_rombank[2]->set_entry(2);
	m_rombank[3]->set_entry(3);
}

std::error_condition msx_cart_loveplus_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_loveplus_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / 0x2000;

	if (size > 256 * 0x8000 || size < 0x10000 || size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		message = "msx_cart_loveplus_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	m_bank_mask = banks - 1;

	m_rombank[0]->configure_entries(0, banks, cart_rom_region()->base(), 0x2000);
	m_rombank[1]->configure_entries(0, banks, cart_rom_region()->base(), 0x2000);
	m_rombank[2]->configure_entries(0, banks, cart_rom_region()->base(), 0x2000);
	m_rombank[3]->configure_entries(0, banks, cart_rom_region()->base(), 0x2000);

	page(1)->install_read_bank(0x4000, 0x5fff, m_rombank[0]);
	page(1)->install_read_bank(0x6000, 0x7fff, m_rombank[1]);
	page(1)->nop_write(0x5000, 0x5000); // Referenced from code but never used
	page(1)->install_write_handler(0x6000, 0x6000, emu::rw_delegate(*this, FUNC(msx_cart_loveplus_device::bank_w<0>)));
	page(1)->install_write_handler(0x6800, 0x6800, emu::rw_delegate(*this, FUNC(msx_cart_loveplus_device::bank_w<1>)));
	page(1)->install_write_handler(0x7000, 0x7000, emu::rw_delegate(*this, FUNC(msx_cart_loveplus_device::bank_w<2>)));
	page(1)->install_write_handler(0x7800, 0x7800, emu::rw_delegate(*this, FUNC(msx_cart_loveplus_device::bank_w<3>)));
	page(2)->install_read_bank(0x8000, 0x9fff, m_rombank[2]);
	page(2)->install_read_bank(0xa000, 0xbfff, m_rombank[3]);
	page(2)->install_write_handler(0x9000, 0x9000, emu::rw_delegate(*this, FUNC(msx_cart_loveplus_device::bank_9000_w)));
	page(2)->nop_write(0xb000, 0xb000); // Referenced from code but never used

	return std::error_condition();
}

template <int Bank>
void msx_cart_loveplus_device::bank_w(u8 data)
{
	// The banking registers can be only written to once otherwise Q-Bert will not
	// start and drop back to Basic.
	if (!m_bank_locked[Bank])
	{
		m_rombank[Bank]->set_entry(data & m_bank_mask);
		m_bank_locked[Bank] = true;
	}
}

void msx_cart_loveplus_device::bank_9000_w(u8 data)
{
	// Before switching to a game the sequence 00, 3f, c2 is written here.
	// This unlocks and/or configures the write-once banking somehow
	for (int i = 0; i < 4; i++)
		m_bank_locked[i] = false;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_LOVEPLUS, msx_cart_interface, msx_cart_loveplus_device, "msx_cart_loveplus", "MSX Cartridge - Love Plus Pack")
