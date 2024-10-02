// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "arc.h"

namespace {

class msx_cart_arc_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_arc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_ARC, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_7f(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void io_7f_w(u8 data);
	u8 io_7f_r();

	u8 m_7f;
};

void msx_cart_arc_device::device_start()
{
	save_item(NAME(m_7f));

	// Install IO read/write handlers
	io_space().install_write_handler(0x7f, 0x7f, emu::rw_delegate(*this, FUNC(msx_cart_arc_device::io_7f_w)));
	io_space().install_read_handler(0x7f, 0x7f, emu::rw_delegate(*this, FUNC(msx_cart_arc_device::io_7f_r)));
}

void msx_cart_arc_device::device_reset()
{
	m_7f = 0;
}

std::error_condition msx_cart_arc_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_arc_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() != 0x8000)
	{
		message = "msx_cart_arc_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	page(2)->install_rom(0x8000, 0xbfff, cart_rom_region()->base() + 0x4000);

	return std::error_condition();
}

void msx_cart_arc_device::io_7f_w(u8 data)
{
	if (data == 0x35)
		m_7f++;
}

u8 msx_cart_arc_device::io_7f_r()
{
	return ((m_7f & 0x03) == 0x03) ? 0xda : 0xff;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_ARC, msx_cart_interface, msx_cart_arc_device, "msx_cart_arc", "MSX Cartridge - Arc")
