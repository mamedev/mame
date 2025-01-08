// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "dooly.h"

namespace {

class msx_cart_dooly_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_dooly_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_DOOLY, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_rombank(*this, "bank%u", 0U)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override { }
	virtual void device_reset() override ATTR_COLD;

private:
	void prot_w(u8 data);
	u8 mode4_page1_r(offs_t offset);
	u8 mode4_page2_r(offs_t offset);

	std::unique_ptr<u8[]> m_bitswapped;
	memory_bank_array_creator<2> m_rombank;
};

void msx_cart_dooly_device::device_reset()
{
	m_rombank[0]->set_entry(0);
	m_rombank[1]->set_entry(0);
}

std::error_condition msx_cart_dooly_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_dooly_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() != 0x8000)
	{
		message = "msx_cart_dooly_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	m_bitswapped = std::make_unique<u8[]>(0x8000);
	for (int i = 0; i < 0x8000; i++)
		m_bitswapped[i] = bitswap<8>(cart_rom_region()->base()[i], 7, 6, 5, 4, 3, 1, 0, 2);

	m_rombank[0]->configure_entry(0, cart_rom_region()->base());
	m_rombank[0]->configure_entry(1, m_bitswapped.get());
	m_rombank[1]->configure_entry(0, cart_rom_region()->base() + 0x4000);
	m_rombank[1]->configure_entry(1, m_bitswapped.get() + 0x4000);

	page(1)->install_read_bank(0x4000, 0x7fff, m_rombank[0]);
	page(1)->install_write_handler(0x4000, 0x7fff, emu::rw_delegate(*this, FUNC(msx_cart_dooly_device::prot_w)));
	page(2)->install_read_bank(0x8000, 0xbfff, m_rombank[1]);
	page(2)->install_write_handler(0x8000, 0xbfff, emu::rw_delegate(*this, FUNC(msx_cart_dooly_device::prot_w)));

	return std::error_condition();
}

void msx_cart_dooly_device::prot_w(u8 data)
{
	data &= 0x07;
	m_rombank[0]->set_entry(BIT(data, 2) ? 1 : 0);
	m_rombank[1]->set_entry(BIT(data, 2) ? 1 : 0);

	if (data != 0 && data != 4)
	{
		logerror("msx_cart_dooly_device: unhandled protection mode %02x\n", data);
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_DOOLY, msx_cart_interface, msx_cart_dooly_device, "msx_cart_dooly", "MSX Cartridge - Dooly")
