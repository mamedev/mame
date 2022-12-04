// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "bus/msx/cart/softcard.h"


DECLARE_DEVICE_TYPE(SOFTCARD_NOMAPPER, softcard_interface)


void softcard(device_slot_interface &device)
{
	device.option_add_internal("nomapper", SOFTCARD_NOMAPPER);
}


namespace {

class softcard_nomapper_device : public device_t
								, public softcard_interface
{
public:
	softcard_nomapper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override { }

	virtual image_init_result initialize_cartridge(std::string &message) override;

};


softcard_nomapper_device::softcard_nomapper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SOFTCARD_NOMAPPER, tag, owner, clock)
	, softcard_interface(mconfig, *this)
{
}

image_init_result softcard_nomapper_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "softcard_nomapper: Required region 'rom' was not found.";
		return image_init_result::FAIL;
	}

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	page(2)->install_rom(0x8000, 0xbfff, cart_rom_region()->base() + 0x4000);

	return image_init_result::PASS;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(SOFTCARD_NOMAPPER, softcard_interface, softcard_nomapper_device, "softcard_nomapper", "SoftCard ROM")
