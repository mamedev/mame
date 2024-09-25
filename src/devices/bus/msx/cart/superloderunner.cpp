// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "superloderunner.h"

namespace {

class msx_cart_superloderunner_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_superloderunner_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_SUPERLODERUNNER, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_rombank(*this, "rombank")
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	void bank_w(u8 data);

	memory_bank_creator m_rombank;
};

void msx_cart_superloderunner_device::device_start()
{
	// Install evil system wide memory write handler
	memory_space().install_write_handler(0x0000, 0x0000, emu::rw_delegate(*this, FUNC(msx_cart_superloderunner_device::bank_w)));
}

std::error_condition msx_cart_superloderunner_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_superloderunner_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() != 0x20000)
	{
		message = "msx_cart_superloderunner_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	m_rombank->configure_entries(0, 8, cart_rom_region()->base(), 0x4000);

	page(2)->install_read_bank(0x8000, 0xbfff, m_rombank);

	return std::error_condition();
}

void msx_cart_superloderunner_device::bank_w(u8 data)
{
	m_rombank->set_entry(data & 0x07);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_SUPERLODERUNNER, msx_cart_interface, msx_cart_superloderunner_device, "msx_cart_superloderunner", "MSX Cartridge - Super Lode Runner")
