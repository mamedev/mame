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
		, m_view1(*this, "view1")
		, m_view2(*this, "view2")
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override { }
	virtual void device_reset() override;

private:
	void prot_w(u8 data);
	u8 mode4_page1_r(offs_t offset);
	u8 mode4_page2_r(offs_t offset);

	memory_view m_view1;
	memory_view m_view2;
};

void msx_cart_dooly_device::device_reset()
{
	m_view1.select(0);
	m_view2.select(0);
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

	page(1)->install_view(0x4000, 0x7fff, m_view1);
	m_view1[0].install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	m_view1[1].install_read_handler(0x4000, 0x7fff, emu::rw_delegate(*this, FUNC(msx_cart_dooly_device::mode4_page1_r)));
	page(2)->install_view(0x8000, 0xbfff, m_view2);
	m_view2[0].install_rom(0x8000, 0xbfff, cart_rom_region()->base() + 0x4000);
	m_view2[1].install_read_handler(0x8000, 0xbfff, emu::rw_delegate(*this, FUNC(msx_cart_dooly_device::mode4_page2_r)));

	page(1)->install_write_handler(0x4000, 0x7fff, emu::rw_delegate(*this, FUNC(msx_cart_dooly_device::prot_w)));
	page(2)->install_write_handler(0x8000, 0xbfff, emu::rw_delegate(*this, FUNC(msx_cart_dooly_device::prot_w)));

	return std::error_condition();
}

u8 msx_cart_dooly_device::mode4_page1_r(offs_t offset)
{
	return bitswap<8>(cart_rom_region()->base()[offset], 7, 6, 5, 4, 3, 1, 0, 2);
}

u8 msx_cart_dooly_device::mode4_page2_r(offs_t offset)
{
	return bitswap<8>(cart_rom_region()->base()[0x4000 | offset], 7, 6, 5, 4, 3, 1, 0, 2);
}

void msx_cart_dooly_device::prot_w(u8 data)
{
	data &= 0x07;
	m_view1.select(BIT(data, 2) ? 1 : 0);
	m_view2.select(BIT(data, 2) ? 1 : 0);
	if (data != 0 && data != 4)
	{
		logerror("msx_cart_dooly_device: unhandled protection mode %02x\n", data);
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_DOOLY, msx_cart_interface, msx_cart_dooly_device, "msx_cart_dooly", "MSX Cartridge - Dooly")
