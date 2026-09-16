// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "matra.h"

namespace {

class msx_cart_matra_comp_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_matra_comp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MSX_CART_MATRA_COMP, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_rombank(*this, "rombank%u", 0U)
		, m_bank_mask(0)
		, m_banking_enabled(true)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	template <int Bank> void bank_w(u8 data);
	void disable_banking_w(u8 data);

	memory_bank_array_creator<4> m_rombank;

	u8 m_bank_mask;
	bool m_banking_enabled;
};

void msx_cart_matra_comp_device::device_start()
{
	save_item(NAME(m_banking_enabled));
}

void msx_cart_matra_comp_device::device_reset()
{
	for (int i = 0; i < 4; i++)
		m_rombank[i]->set_entry(i);
	m_banking_enabled = true;
}

std::error_condition msx_cart_matra_comp_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_matra_comp_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / 0x2000;

	if (size > 256 * 0x2000 || size < 0x8000 || size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		message = "msx_cart_matra_comp_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	m_bank_mask = banks - 1;

	for (int i = 0; i < 4; i++)
		m_rombank[i]->configure_entries(0, banks, cart_rom_region()->base(), 0x2000);

	page(1)->install_read_bank(0x4000, 0x5fff, m_rombank[0]);
	page(1)->install_write_handler(0x5000, 0x5000, emu::rw_delegate(*this, FUNC(msx_cart_matra_comp_device::bank_w<0>)));
	page(1)->install_read_bank(0x6000, 0x7fff, m_rombank[1]);
	page(1)->install_write_handler(0x6000, 0x6000, emu::rw_delegate(*this, FUNC(msx_cart_matra_comp_device::bank_w<1>)));
	page(2)->install_read_bank(0x8000, 0x9fff, m_rombank[2]);
	page(2)->install_write_handler(0x8000, 0x8000, emu::rw_delegate(*this, FUNC(msx_cart_matra_comp_device::bank_w<2>)));
	page(2)->install_read_bank(0xa000, 0xbfff, m_rombank[3]);
	page(2)->install_write_handler(0xa000, 0xa000, emu::rw_delegate(*this, FUNC(msx_cart_matra_comp_device::bank_w<3>)));
	page(2)->install_write_handler(0xba00, 0xba00, emu::rw_delegate(*this, FUNC(msx_cart_matra_comp_device::disable_banking_w)));

	return std::error_condition();
}

template <int Bank>
void msx_cart_matra_comp_device::bank_w(u8 data)
{
	if (m_banking_enabled)
		m_rombank[Bank]->set_entry(data & m_bank_mask);
}

void msx_cart_matra_comp_device::disable_banking_w(u8 data)
{
	// Assuming that any write here disables banking
	m_banking_enabled = false;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_MATRA_COMP, msx_cart_interface, msx_cart_matra_comp_device, "msx_cart_matra_comp", "MSX Cartridge - Matra Compilation")
