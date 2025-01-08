// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Ink cartridge (Matra, 2006)

48KB ROM data on an AMD Am29F040B(label scratched off), the game uses it
for protection.

******************************************************************************/

#include "emu.h"
#include "ink.h"
#include "machine/intelfsh.h"

namespace {

class msx_cart_ink_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_ink_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MSX_CART_INK, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_flash(*this, "flash")
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<amd_29f040_device> m_flash;

	template <int Page> u8 read_page(offs_t offset);
	template <int Page> void write_page(offs_t offset, u8 data);
};

ROM_START(msx_cart_ink)
	ROM_REGION(0x80000, "flash", ROMREGION_ERASEFF)
ROM_END

const tiny_rom_entry *msx_cart_ink_device::device_rom_region() const
{
	return ROM_NAME(msx_cart_ink);
}

void msx_cart_ink_device::device_add_mconfig(machine_config &config)
{
	AMD_29F040(config, m_flash);
}

std::error_condition msx_cart_ink_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_ink_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	const size_t size = std::min<size_t>(0x80000, cart_rom_region()->bytes());

	u8 *flash = memregion("flash")->base();
	memcpy(flash, cart_rom_region()->base(), size);

	page(0)->install_readwrite_handler(0x0000, 0x3fff, emu::rw_delegate(*this, FUNC(msx_cart_ink_device::read_page<0>)), emu::rw_delegate(*this, FUNC(msx_cart_ink_device::write_page<0>)));
	page(1)->install_readwrite_handler(0x4000, 0x7fff, emu::rw_delegate(*this, FUNC(msx_cart_ink_device::read_page<1>)), emu::rw_delegate(*this, FUNC(msx_cart_ink_device::write_page<1>)));
	page(2)->install_readwrite_handler(0x8000, 0xbfff, emu::rw_delegate(*this, FUNC(msx_cart_ink_device::read_page<2>)), emu::rw_delegate(*this, FUNC(msx_cart_ink_device::write_page<2>)));
	page(3)->install_readwrite_handler(0xc000, 0xffff, emu::rw_delegate(*this, FUNC(msx_cart_ink_device::read_page<3>)), emu::rw_delegate(*this, FUNC(msx_cart_ink_device::write_page<3>)));

	return std::error_condition();
}

template <int Page>
u8 msx_cart_ink_device::read_page(offs_t offset)
{
	return m_flash->read(offset | (Page * 0x4000));
}

template <int Page>
void msx_cart_ink_device::write_page(offs_t offset, u8 data)
{
	// /RD connects to flashrom A16-A18
	m_flash->write(offset | 0x70000 | (Page * 0x4000), data);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_INK, msx_cart_interface, msx_cart_ink_device, "msx_cart_ink", "MSX Cartridge - Ink")
