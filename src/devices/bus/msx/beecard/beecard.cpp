// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "beecard.h"
#include "bus/msx/cart/beepack.h"


DECLARE_DEVICE_TYPE(BEE_CARD_NOMAPPER, bee_card_interface)


void bee_card(device_slot_interface &device)
{
	device.option_add_internal("nomapper", BEE_CARD_NOMAPPER);
}


namespace {

class bee_card_nomapper_device : public device_t
								, public bee_card_interface
{
public:
	bee_card_nomapper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override { }

	virtual image_init_result initialize_cartridge(std::string &message) override;

};


bee_card_nomapper_device::bee_card_nomapper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, BEE_CARD_NOMAPPER, tag, owner, clock)
	, bee_card_interface(mconfig, *this)
{
}

image_init_result bee_card_nomapper_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "bee_card_nomapper_device: Required region 'rom' was not found.";
		return image_init_result::FAIL;
	}

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	if (cart_rom_region()->bytes() == 0x8000)
		page(2)->install_rom(0x8000, 0xbfff, cart_rom_region()->base() + 0x4000);

	return image_init_result::PASS;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(BEE_CARD_NOMAPPER, bee_card_interface, bee_card_nomapper_device, "bee_card_nomapper", "Bee Card ROM")
